/********************************************************************
	created:	2006/10/20   17:15
	filename: 	e:\IxEngine\Proj_RenderSystem\Renderer.cpp
	author:		cxi
	
	purpose:	renderer 
*********************************************************************/
#include "stdh.h"

#include "RenderSystem.h"

#include "Renderer.h"

#include "DeviceObject.h"

#include "Log/LogFile.h"

#include "math/vector3d.h"

#include "MtrlMgr.h"
#include "MeshMgr.h"
#include "VertexMgr.h"
#include "Camera.h"
#include "Light.h"
#include "Matrice43.h"

#include "ShaderLibMgr.h"

#include "timer/profiler.h"

#include <deque>

#include <assert.h>

LogFile g_logShaderMgr("ShaderMgr");

inline DWORD FTODW(float v)
{
	return *(DWORD*)&v;
}


CRenderer::CRenderer()
{
	_devobj=NULL;
	_slmgr=NULL;

	_nBeginScene=0;

	_cMats=0;

	_ZeroRenderContent(FALSE);
}


BOOL CRenderer::Init(IRenderSystem *pRS)
{
	
	_devobj=((CRenderSystem*)pRS)->GetDeviceObj();
	_slmgr=((CRenderSystem*)pRS)->GetShaderLibMgr();

	return TRUE;
}

void CRenderer::UnInit()
{
	_CleanRenderContent(FALSE);
	_devobj=NULL;
	_slmgr=NULL;

}

void CRenderer::OnDeviceReset()
{
	_nBeginScene=0;

}


void CRenderer::OnDeviceLost()
{
	_nBeginScene=0;

}


BOOL CRenderer::SetRenderState(D3DRENDERSTATETYPE state, DWORD value)
{
	return _devobj->SetRenderState(state,value);
}

BOOL CRenderer::_BeginScene()
{
	if (_nBeginScene<=0)
	{
		if (FALSE==_devobj->BeginScene())
			return FALSE;
	}
	_nBeginScene++;
	return TRUE;
}

void CRenderer::_EndScene()
{
	if (_nBeginScene<=0)
		return;
	_nBeginScene--;
	if (_nBeginScene<=0)
	{
		if (_devobj)
			_devobj->EndScene();
		_CleanRenderContent(FALSE);
	}
}


BOOL CRenderer::Begin()
{
	return _BeginScene();
}

void CRenderer::End()
{
	_EndScene();
}

//clear all the binding,light,and additional features/feature params,
BOOL CRenderer::ResetContent()
{
	_CleanRenderContent(TRUE);
	return TRUE;
}



#define TRY_GEN_SHADER(scMtrl,fcCam,fcMesh,fcLight,fcAdd)\
{\
	ShaderCode scTry;\
	scTry=(scMtrl);\
	scTry.Add(fcCam);\
	scTry.Add(fcMesh);\
	scTry.Add(fcLight);\
	scTry.Add(fcAdd);\
	shader=(CShader*)_slmgr->ObtainShader(scTry);\
	if ((shader)&&(A_Ok==shader->Touch()))\
		goto _gen_ok;\
	SAFE_RELEASE(shader);\
}

//generate a shader for a batch rendering
CShader *CRenderer::_GenBatchShader(IMtrl *mtrl,int iMtrlLod,IMesh *mesh,ILight *lgt,FeatureCode *fcAdd)
{
	FeatureCode fcZero;
	FeatureCode fcMesh;
	FeatureCode *fcLight=NULL;
	DWORD nFCLight;
	ShaderCode scMtrl;
	DWORD nFCCam;
	FeatureCode *fcCam=_cam->GetFC(nFCCam);
	FeatureCode fc;//additional feature

	if (mesh)
		fcMesh=mesh->GetFeature();

	MtrlLod*lod=mtrl->GetLod(iMtrlLod);
	if (!lod)
		return NULL;
	scMtrl=lod->sc;

	if (lgt)
		fcLight=lgt->GetFC(nFCLight);
	if (!fcLight)
	{
		fcLight=&fcZero;
		nFCLight=1;
	}

	if (fcAdd)
		fc=*fcAdd;


	CShader *shader=NULL;
	int iFCLight=0;

	//the standard feature combo
	TRY_GEN_SHADER(scMtrl,fcCam[0],fcMesh,fcLight[iFCLight],fc);

	//reduce light feature
	while(iFCLight<nFCLight-1)
	{
		iFCLight++;
		TRY_GEN_SHADER(scMtrl,fcCam[0],fcMesh,fcLight[iFCLight],fc);
	}

	//ignore additional feature
	if (!fc.IsEmpty())
		TRY_GEN_SHADER(scMtrl,fcCam[0],fcMesh,fcLight[iFCLight],FeatureCode(0));

	//ignore light
	TRY_GEN_SHADER(scMtrl,fcCam[0],fcMesh,FeatureCode(0),FeatureCode(0));

	return NULL;

_gen_ok:


	//let the camera add feature params
	if (_cam)
		_cam->Bind(shader);


	return shader;
}


BOOL CRenderer::Render()
{
#pragma message("Need add a ValidateRender() function,which will only check render's validity and do not do the real rendering")
// 	GetDevice()->FlushCommand();
// 	ProfilerStart(MeshDraw);

	if (_nBeginScene<=0)
		return FALSE;

	if (!_mtrl)
		return FALSE;
	if (A_Ok!=_mtrl->Touch())
		return FALSE;
	if (_mesh)
	{
		if (A_Ok!=_mesh->Touch())
			return FALSE;
	}
	else
	{
		if (!_vb)
			return FALSE;
	}

	CShader *shader;
	if (TRUE)
	{
		CMesh *mesh=_mesh;
		if(_mesh)
		{
			if ((_mesh->GetBoneCount()!=0)&&(_mesh->GetBoneCount()!=_cMats))
				mesh=NULL;
		}
		shader=(CShader *)BeginRaw(_mtrl,_mtrllayor,mesh,_lgt,&_fc);
	}
	if (!shader)
		return FALSE;

	_mtrl->BindEP(shader,_mtrllayor,ANIMTICK_INFINITE);
	_mtrl->BindState(shader,_mtrllayor);

	if (_lgt)
		_lgt->Bind(shader,0);

	if (EPFail==shader->SetEPs(_epk.buf,_epk.nEP))
	{
		EndRaw(shader);
		return FALSE;
	}

	if (_mesh)
		_mesh->Draw(shader,_mats,_cMats,_dmg);
	else
	{
		if (_mats)
			shader->SetEP_World(_mats,_cMats);
		else
			shader->SetEP_World(&_matIdentity,1);
		shader->BindVB(_vb,_ib,&_argVB);
		shader->DoShadeRaw();
	}

	EndRaw(shader);

// 	GetDevice()->FlushCommand();
// 	ProfilerEnd();

	return TRUE;
}


IShader *CRenderer::BeginRaw(IMtrl *mtrl,int iMtrlLayor,IMesh *mesh,ILight *lgt,FeatureCode *fc)
{
	if (_nBeginScene<=0)
		return NULL;

	if (!mtrl)
		return NULL;
	if (A_Ok!=mtrl->Touch())
		return NULL;

	if (mesh)
	{
		if (A_Ok!=mesh->Touch())
			return FALSE;
	}

	CShader *shader=_GenBatchShader(mtrl,iMtrlLayor,mesh,lgt,fc);
	if (!shader)
		return NULL;

	if (!shader->BeginRaw())
	{
		SAFE_RELEASE(shader);
		return NULL;
	}

	return shader;
}

IShader *CRenderer::BeginRaw(ShaderCode &sc0)
{
	if (_nBeginScene<=0)
		return NULL;

	ShaderCode sc=sc0;
	sc.Add(_fcCam);

	CShader*shader=(CShader*)_slmgr->ObtainShader(sc);
	if ((shader)&&(A_Ok==shader->Touch()))
	{
		if (_cam)
			_cam->Bind(shader);

		if (shader->BeginRaw())
			return shader;
	}
	SAFE_RELEASE(shader);

	return NULL;
}


void CRenderer::EndRaw(IShader *shader)
{
	if (shader)
		shader->EndRaw();
	SAFE_RELEASE(shader);
}


void CRenderer::_ZeroRenderContent(BOOL bKeepCamera)
{
	_mesh=NULL;
	_dmg.Zero();

	_vb=NULL;
	_ib=NULL;
	_argVB.Zero();

	_mtrl=NULL;

	if (!bKeepCamera)
		_cam=NULL;

	_mats=NULL;

	_lgt=NULL;

	_fc.Clear();
}

void CRenderer::_CleanRenderContent(BOOL bKeepCamera)
{
	SAFE_RELEASE(_mesh);
	SAFE_RELEASE(_vb);
	SAFE_RELEASE(_ib);
	SAFE_RELEASE(_mtrl);
	if (!bKeepCamera)
		SAFE_RELEASE(_cam);
	SAFE_RELEASE(_lgt);
	_mats=NULL;
	_cMats=0;

	ClearFeature();

	_ZeroRenderContent(bKeepCamera);
}

BOOL CRenderer::BindCamera(ICamera *cam)
{
	if (cam)
		cam->AddRef();
	SAFE_RELEASE(_cam);
	_cam=(CCamera*)cam;
	DWORD nFC;
	FeatureCode *fc=_cam->GetFC(nFC);
	_fcCam=*fc;

	return TRUE;
}


BOOL CRenderer::BindMesh(IMesh *mesh,DrawMeshArg &dmg)
{
	if (mesh)
		mesh->AddRef();//add ref first in case that they are the same
	SAFE_RELEASE(_mesh);
	_mesh=(CMesh*)mesh;
	_dmg=dmg;

	return TRUE;
}

BOOL CRenderer::BindVB(VBHandles &vbh,VBBindArg&arg)
{
	SAFE_ADDREF(vbh.vb);
	SAFE_ADDREF(vbh.ib);

	SAFE_RELEASE(_vb);
	SAFE_RELEASE(_ib);

	_vb=(VBInfo*)vbh.vb;
	_ib=(IBInfo*)vbh.ib;

	_argVB=arg;

	return TRUE;
}

BOOL CRenderer::BindMtrl(IMtrl *mtrl,int iLayor)
{
	if (mtrl)
		mtrl->AddRef();
	SAFE_RELEASE(_mtrl);
	_mtrl=(CMtrl*)mtrl;

	_mtrllayor=iLayor;

	return TRUE;
}



BOOL CRenderer::BindMats(i_math::matrix43f *mats,DWORD c)
{
	_mats=mats;
	_cMats=c;
// 	if (_cMats==0)
// 		_cMats=1;
	return TRUE;
}

BOOL CRenderer::BindLight(ILight *l)
{
	l->AddRef();
	SAFE_RELEASE(_lgt);
	_lgt=(CLight*)l;
	return TRUE;
}


//clear all the additional features and feature params
void CRenderer::ClearFeature()
{
	_fc.Clear();

	_epk.Zero();
}

//add a single additional feature
BOOL CRenderer::AddFeature(FeatureCode &fc)
{
	_fc.Add(fc);
	return TRUE;
}

BOOL CRenderer::RemoveFeature(FeatureCode &fc)
{
	_fc.Remove(fc);
	return TRUE;
}

