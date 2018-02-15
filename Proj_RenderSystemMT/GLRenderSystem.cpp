/********************************************************************
	created:	2006/06/25
	created:	25:6:2006   20:18
	filename: 	d:\IxEngine\Proj_RenderSystem\RenderSystem.cpp
	author:		cxi
	
	purpose:	RenderSystem entry file,IRenderSystem implement
*********************************************************************/
#include "stdh.h"

#include "assert.h"

#include "interface/interface.h" 

#include "GLRenderSystem.h"

#include "ResourceBase.h"

#include "Camera.h"
#include "Light.h"
#include "RenderPort.h"
#include "Matrice43.h"
#include "Skeleton.h"


#include "TexAtlasMap.h"
#include "PatchGeom.h"
#include "PatchQueue.h"
#include "ConvexVolume.h"
#include "AnimPlayer.h"

#include "Log/LogFile.h"

#include "Log/LogDump.h"

#include "timer/profiler.h"

#include "strlib/strlib.h"




//CRenderSystemGL g_rs;


LogFile g_logRS("RenderSystem");


CRenderSystemGL::CRenderSystemGL()
{
	_pFS=NULL;
	_pFS2=NULL;
	_pFS3=NULL;

}

extern IFolder *LocateFolder(RenderSystemPath path,CRenderSystemGL *pRS);
BOOL CRenderSystemGL::Init(const RenderSystemInit &param)
{

//	g_logRS.Dump("RS Init");

	_pFS=param.pFS;
	_pFS2=param.pFS2;
	_pFS3=param.pFS3;
	_curport=NULL;

	_path[Path_Effect]=param.pathEffect;
	_path[Path_Res]=param.pathRes;
	_path[Path_Font]=param.pathFont;
	_path[Path_ShaderCache]=param.pathShaderCache;
	_path[Path_FontCache]=param.pathFontCache;
	_path[Path_Sheet]=param.pathSheet;
	_path[Path_Records]=param.pathRecords;
	_path[Path_BehaviorGraph]=param.pathBehaviorGraph;

	_cfg=param.cfg;

	if (FALSE==_deviceobj.Init(this))
		return FALSE;

// 	_texmgr.Init(this,"Texture Manager");

	_vertexmgr.Init(this,"Vertex Manager");

// 	_wtexmgr.Init(this,"Writable Texture Manager2");
// 	_rtexmgr.Init(this,"Render Target Texture Manager");
// 	_surfmgr.Init(this,"Surface Manager");
	_boneanimmgr.Init(this,"BoneAnim2 manager");
	_dynboneanimmgr.Init(this,"BoneAnim2 dynamic manager");
	//
// 	_slmgr.Init(this,"ShaderLib Manager");
// 	_slmgr.AddShaderLib("main.slb");
// 	_slmgr.AddShaderLib("fixfunc.slb");
// 	_slmgr.AddShaderLib("draw2d.slb");
// 	_slmgr.AddShaderLib("terrain_ml.slb");
// 	_slmgr.AddShaderLib("terrain_nk.slb");
// 	_slmgr.AddShaderLib("postprocess.slb");
// 	_slmgr.AddShaderLib("bake.slb");
// 	_slmgr.AddShaderLib("liquid.slb");
// 	_slmgr.AddShaderLib("deferredshading.slb");
// 	_slmgr.AddShaderLib("speedtree.slb");
// 	_slmgr.AddShaderLib("sky.slb");
// 	_slmgr.AddShaderLib("vegetable.slb");
// 	_slmgr.AddShaderLib("particle.slb");
// 	_slmgr.AddShaderLib("shell.slb");
// 	_slmgr.AddShaderLib("road.slb");
// 	_slmgr.AddShaderLib("user.slb");

	_sheetmgr.Init(this,"Sheet Manager");


// 	_renderer.Init(this);

// 	if (FALSE==_fontmgr.Init(this))//init AFTER deviceobj is Init()
// 		return FALSE;
// 
// 	if (FALSE==_meshmgr.Init(this,"Mesh Manager"))
// 		return FALSE;
// 
// 	if (FALSE==_mtrlmgr.Init(this,"Material Manager"))
// 		return FALSE;

	if (FALSE==_animmgr.Init(this,"Anim Manager"))
		return FALSE;

	if (FALSE==_dynanimmgr.Init(this,"DynAnim Manager"))
		return FALSE;

	if(FALSE==_dummiesmgr.Init(this,"Dummies Manager"))
		return FALSE;

	if(FALSE==_dyndummiesmgr.Init(this,"Dynamic Dummies Manager"))
		return FALSE;

// 	if(FALSE==_sptmgr.Init(this,"SpeedTree manager"))
// 		return FALSE;
// 	if(FALSE==_dynsptmgr.Init(this,"Dynamic SpeedTree Manager"))
// 		return FALSE;

	if(FALSE==_moppmgr.Init(this,"Mopp Manager"))
		return FALSE;

// 	if(FALSE==_spgmgr.Init(this,"SpeedGrass Manager"))
// 		return FALSE;
// 
// 	if(FALSE==_dynspgmgr.Init(this,"Dynamic SpeedGrass Manager"))
// 		return FALSE;

	if(FALSE==_animtreemgr.Init(this,"Anim Tree Manager"))
		return FALSE;

	if(FALSE==_dynanimtreemgr.Init(this,"Dynamic Anim Tree Manager"))
		return FALSE;

// 	if(FALSE==_mtrlextmgr.Init(this,"Material Feature Manager"))
// 		return FALSE;
// 
// 	if(FALSE==_soundmgr.Init(this,_pFS3,"Sound Manager"))
// 		return FALSE;

	if(FALSE==_recordsmgr.Init(this,"Records Manager"))
		return FALSE;
	if(FALSE==_rgdmgr.Init(this,"Ragdoll Manager"))
		return FALSE;
	if(FALSE==_dtrmgr.Init(this,"Destructable Manager"))
		return FALSE;
	if(FALSE==_behaviorgraphmgr.Init(this,"BehaviorGraph Manager"))
		return FALSE;


// 	_patchBuilder.Create(this,FVFEX_XYZW0|FVFEX_NORMAL0,15000,60000);
	//XXXXX:more res type


	_tStart=GetTickCount();
	_tPresent=0;

	return TRUE;

}


//if any leak found,return FALSE,otherwise return TRUE
BOOL CRenderSystemGL::UnInit()
{
	_curport=NULL;

	BOOL bLeak=FALSE;

	//XXXXX:more res type

	RESMGR_UNINIT(_behaviorgraphmgr,bLeak);
	RESMGR_UNINIT(_dtrmgr,bLeak);
	RESMGR_UNINIT(_recordsmgr,bLeak);
	RESMGR_UNINIT(_animtreemgr,bLeak);
	RESMGR_UNINIT(_dynanimtreemgr,bLeak);
//	RESMGR_UNINIT(_mtrlmgr,bLeak);
	RESMGR_UNINIT(_animmgr,bLeak);
	RESMGR_UNINIT(_dynanimmgr,bLeak);
//	RESMGR_UNINIT(_meshmgr,bLeak);
//	RESMGR_UNINIT(_sptmgr,bLeak);
//	RESMGR_UNINIT(_dynsptmgr,bLeak);
	RESMGR_UNINIT(_dummiesmgr,bLeak);
	RESMGR_UNINIT(_dyndummiesmgr,bLeak);
	RESMGR_UNINIT(_moppmgr,bLeak);
//	RESMGR_UNINIT(_spgmgr,bLeak);
//	RESMGR_UNINIT(_dynspgmgr,bLeak);
	RESMGR_UNINIT(_boneanimmgr,bLeak);
	RESMGR_UNINIT(_dynboneanimmgr,bLeak);
//	RESMGR_UNINIT(_mtrlextmgr,bLeak);

//	_fontmgr.UnInit();//UnInit BEFORE deviceobj is UnInit()

//	_renderer.UnInit();
//	RESMGR_UNINIT(_slmgr,bLeak);

	RESMGR_UNINIT(_sheetmgr,bLeak);
//	RESMGR_UNINIT(_soundmgr,bLeak);
	RESMGR_UNINIT(_rgdmgr,bLeak);

//	RESMGR_UNINIT(_texmgr,bLeak);
	RESMGR_UNINIT(_vertexmgr,bLeak);
// 	RESMGR_UNINIT(_wtexmgr,bLeak);
// 	RESMGR_UNINIT(_rtexmgr,bLeak);
// 	RESMGR_UNINIT(_surfmgr,bLeak);

	
// 	_patchBuilder.Destroy();

	bLeak|=(!_deviceobj.UnInit());

	_cfg=NULL;

//	D3D_FINISH();
//	g_logRS.Dump("RS UnInit");

	return !bLeak;
}

BOOL CRenderSystemGL::ResetDevice(DeviceConfig &cfg)
{

	if (FALSE==GetDeviceObj()->ResetConfig(cfg))
		return FALSE;

	return TRUE;
}

void CRenderSystemGL::CleanDevice()
{
	GetDeviceObj()->Clean();
}

BOOL CRenderSystemGL::IsDeviceReset()
{
	return TRUE;
// 	if (GetDeviceObj()->GetDevice())
// 		return TRUE;
// 	return FALSE;
}	


void CRenderSystemGL::OnIdle(__int64 &tIdle)
{
//	BOOL bMore;
//	bMore=TRUE;
//	while(bMore)
//	{
//		bMore=FALSE;
//		std::map<std::string,CResourceMgr *>::iterator it;
//		for (it=_allmgrs.begin();it!=_allmgrs.end();it++)
//		{
//			if (((*it).second)->_DoIdle(tIdle))
//				bMore=TRUE;
//			if (tIdle<=0)
//				return;
//		}
//	}
}



void CRenderSystemGL::Update(DWORD dwTick)
{
	GarbageCollect();
//	std::map<std::string,CResourceMgr *>::iterator it;
//	for (it=_allmgrs.begin();it!=_allmgrs.end();it++)
//		((*it).second)->Update()
	


}

//return whether there is any leak
BOOL CRenderSystemGL::CheckAllResLeak()
{
	BOOL bLeak=FALSE;
//	std::map<std::string,CResourceMgr *>::iterator it;
//	for (it=_allmgrs.begin();it!=_allmgrs.end();it++)
//	{
//		if (((*it).second)->CheckResLeak())
//		{
//			LogFile::Prompt("Error:Resource Leak found in \"%s\"!",((*it).second)->_name.c_str());
//			bLeak=TRUE;
//		}
//	}
	return bLeak;
}

void CRenderSystemGL::SetResSyncLoad(BOOL bSync)
{
	CResource::_bSyncTouch=bSync;
}



BOOL CRenderSystemGL::BeginFrame()
{
// 	if (FALSE==_renderer.Begin())
// 		return FALSE;
// 
// 	LockPort(NULL);//Unlock all the port

	return TRUE;
}
BOOL CRenderSystemGL::EndFrame()
{
// 	_renderer.End();
// 	_fontmgr.Flush();


	GarbageCollect();
 
	return TRUE;
}

void CRenderSystemGL::_UpdatePresentT()
{
	DWORD t=GetTickCount();
	t-=_tStart;

	_tPresent=ANIMTICK_FROM_SECOND(((float)t)/1000.0f);
}


BOOL CRenderSystemGL::Present(i_math::recti *rcDest,i_math::recti *rcSrc,HWND hwndOverride)
{
	BOOL b=_deviceobj.Present();
	D3D_FLUSH();

	_UpdatePresentT();

	return b;
}

BOOL CRenderSystemGL::PresentAsyn(i_math::recti *rcDest,i_math::recti *rcSrc,HWND hwndOverride)
{
	BOOL b=_deviceobj.Present();
	D3D_FLIP();

	_UpdatePresentT();

	return b;
}



CDeviceObjectGL *CRenderSystemGL::GetDeviceObj()
{
	return &_deviceobj;
}



IRenderPort *CRenderSystemGL::CreateRenderPort()
{
// 	CRenderPort *p=Class_New2(CRenderPort);
// 	if (FALSE==p->Init(this))
// 		goto _fail;
// 	p->SetRect_Total();
// 
// 	p->AddRef();
// 
// 	return p;
// 
// _fail:
// 	Class_Delete(p);
	return NULL;
}

ILight *CRenderSystemGL::CreateLight()
{
	CLight *p=Class_New2(CLight);

	if (!p->Init())
	{
		Class_Delete(p);
		return NULL;
	}

	p->AddRef();
	return p;
}


ICamera *CRenderSystemGL::CreateCamera()
{
	CCamera *p=Class_New2(CCamera);

	if (FALSE==p->Init())
	{
		Class_Delete(p);
		return NULL;
	}
	p->AddRef();
	return p;
}

IMatrice43 *CRenderSystemGL::CreateMatrice43()
{
	CMatrice43 *p=Class_New2(CMatrice43);
	p->AddRef();
	return p;
}

ISkeleton *CRenderSystemGL::CreateSkeleton(SkeletonInfo &si)
{
	CSkeleton *p=Class_New2(CSkeleton);
	p->_si=si;
	p->_pRS=this;
	p->AddRef();

	return p;
}


IAnimPlayer*CRenderSystemGL::CreateAnimPlayer()
{
	IAnimPlayer*player=Class_New2(CAnimPlayer);
	player->AddRef();
	return player;
}

IMeshSnapshot *CRenderSystemGL::CreateMeshSnapshot()
{
// 	CMeshSnapshot *ss=Class_New2(CMeshSnapshot);
// 	ss->AddRef();
// 	return ss;
	return NULL;
}

ITexAtlasPool *CRenderSystemGL::CreateTexAtlasPool(TexInfo &ti)
{
	CTexAtlasPool *pool=Class_New2(CTexAtlasPool);
	if (FALSE==pool->Init(this,ti))
	{
		Class_Delete(pool);
		return NULL;
	}
	pool->AddRef();
	return pool;
}

ITexAtlasMap *CRenderSystemGL::CreateTexAtlasMap(TexInfo &ti,BOOL bAllowResize)
{
	CTexAtlasMap *mp=Class_New2(CTexAtlasMap);
	if (FALSE==mp->Init(this,ti,bAllowResize))
	{
		Class_Delete(mp);
		return NULL;
	}
	mp->AddRef();
	return mp;
}

IPatchGeom *CRenderSystemGL::CreatePatchGeom()
{
	CPatchGeom *geo=Class_New2(CPatchGeom);
	geo->Init(this);
	geo->AddRef();
	return geo;
}
IPatchQueue *CRenderSystemGL::CreatePatchQueue()
{
	CPatchQueue *que=Class_New2(CPatchQueue);
	que->Init(this);
	que->AddRef();
	return que;
}


ICvxVolume*CRenderSystemGL::CreateConvexVolume()
{
	CCvxVolume *vol=Class_New2(CCvxVolume);
	vol->AddRef();
	return vol;
}

IPatchBuilder * CRenderSystemGL::GetPatchBuilder()
{
	return &_patchBuilder;
}

IFileSystem *CRenderSystemGL::GetFS()
{
	return _pFS;
}

const char *CRenderSystemGL::GetPath(RenderSystemPath rsp)
{
	if ((rsp<0)||(rsp>=Path_Max))
		return "";
	return _path[rsp].c_str();
}


ProfilerMgr *CRenderSystemGL::GetProfilerMgr()
{
	::GetProfilerMgr()->SetName("RenderSystem");
	return ::GetProfilerMgr();
}

void CRenderSystemGL::RegisterLogHandler(LogHandler &handler)
{
	extern void ::RegisterLogHandler(LogHandler &handler);
	::RegisterLogHandler(handler);
}


void CRenderSystemGL::SetStrLib(CStrLib *strlib)
{
	::StrLib_Set(strlib);
}


void CRenderSystemGL::OnDeviceLost()
{
	if (TRUE)
	{
		CClass *clss=Class_Ptr2(CPatchGeom);
		DWORD count;
		CPatchGeom **pgs=(CPatchGeom **)clss->GetInstances(count);
		for (int i=0;i<count;i++)
			pgs[i]->OnDeviceLost();
	}
// 
// 	_renderer.OnDeviceLost();
// 	_slmgr.OnDeviceLost();
 	_vertexmgr.OnDeviceLost();
// 	_rtexmgr.OnDeviceLost();
// 	_surfmgr.OnDeviceLost();
}

void CRenderSystemGL::OnDeviceReset()
{
 	_vertexmgr.OnDeviceReset();
// 	_slmgr.OnDeviceReset();
// 	_surfmgr.OnDeviceReset();
// 	_rtexmgr.OnDeviceReset();
// 	//
// 	_renderer.OnDeviceReset();
// 
	if (TRUE)
	{
		CClass *clss=Class_Ptr2(CPatchQueue);
		DWORD count;
		CPatchQueue**pqs=(CPatchQueue**)clss->GetInstances(count);
		for (int i=0;i<count;i++)
			pqs[i]->OnDeviceReset();
	}



}
void CRenderSystemGL::OnDeviceDestroy()
{
// 	_texmgr.OnDeviceDestroy();
// 	_wtexmgr.OnDeviceDestroy();
}
void CRenderSystemGL::OnDeviceCreate()
{
// 	_texmgr.OnDeviceCreate();
// 	_wtexmgr.OnDeviceCreate();
}


void CRenderSystemGL::GarbageCollect()
{
	DWORD step=100;
//	_texmgr.GarbageCollect(step);
	_vertexmgr.GarbageCollect(step);
// 	_rtexmgr.GarbageCollect(step);
// 	_wtexmgr.GarbageCollect(step);
// 	_surfmgr.GarbageCollect(step);
// 	_slmgr.GarbageCollect(step);

	_sheetmgr.GarbageCollect(step);

// 	_meshmgr.GarbageCollect(step);
// 	_mtrlmgr.GarbageCollect(step);
	_animmgr.GarbageCollect(step);
	_dynanimmgr.GarbageCollect(step);
	_dummiesmgr.GarbageCollect(step);
	_dyndummiesmgr.GarbageCollect(step);
// 	_sptmgr.GarbageCollect(step);
// 	_dynsptmgr.GarbageCollect(step);
	_moppmgr.GarbageCollect(step);
// 	_spgmgr.GarbageCollect(step);
// 	_dynspgmgr.GarbageCollect(step);
	_animtreemgr.GarbageCollect(step);
	_dynanimtreemgr.GarbageCollect(step);
	_boneanimmgr.GarbageCollect(step);
// 	_mtrlextmgr.GarbageCollect(step);
// 	_soundmgr.GarbageCollect(step);
	_recordsmgr.GarbageCollect(step);
	_rgdmgr.GarbageCollect(step);
	_dtrmgr.GarbageCollect(step);
	_behaviorgraphmgr.GarbageCollect(step);
	//XXXXX:more res type
}

void CRenderSystemGL::FlushCommand()
{
	_deviceobj.FlushCommand();
}
