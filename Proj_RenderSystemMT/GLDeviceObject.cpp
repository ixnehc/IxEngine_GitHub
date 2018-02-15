/********************************************************************
	created:	2006/06/04
	created:	4:6:2006   20:07
	filename: 	d:\IxEngine\Proj_RenderSystem\DeviceObject.cpp
	author:		cxi
	
	purpose:	an object representing the d3d device,IDeviceObject implement
*********************************************************************/
#include "stdh.h"
#include "GlBase.h"
#include "interface/interface.h"
#include "GLRenderSystem.h"
#include "GLDeviceObject.h"

#include "Log/LastError.h"
#include "Log\LogFile.h"
#include "Log/LogDump.h"

#include "assert.h"

#include "timer/profiler.h"
#include "fvfex/fvfex.h"

#include "GLVertexMgr.h"



// #define _NVPerfHUD

LogFile g_logDevice("DeviceObject");



//////////////////////////////////////////////////////////////////////////
//CDeviceObjectGL


void CDeviceObjectGL::Zero()
{
	_bLost=FALSE;

	_state.SetInvalid();

	_bFboDirty=FALSE;
	_fboCur=GLHandle_Null;

	_declCur=NULL;

	_pRS=NULL;
	_w=_h=0;
}


CDeviceObjectGL::CDeviceObjectGL():_config(NULL)
{
	Zero();
}

CDeviceObjectGL::~CDeviceObjectGL() 
{ 
}


BOOL CDeviceObjectGL::Init(CRenderSystemGL *pRS)
{
	_pRS=pRS;

	return TRUE;
}

//if any leak found,return FALSE,otherwise return TRUE
BOOL CDeviceObjectGL::UnInit()
{
	BOOL bLeak=FALSE;


	Clean();

	_pRS=NULL;

	return TRUE;
}

void CDeviceObjectGL::Clean()
{
	_OnLost();
	_OnDestroyed();
}



BOOL CDeviceObjectGL::ResetConfig(DeviceConfig &config)
{
	_config=config;

	_OnLost();

	glFrontFace(GL_CW);
	glEnable(GL_CULL_FACE);

	_OnReset();
	
	return TRUE;
}

BOOL CDeviceObjectGL::GetConfig(DeviceConfig &config)
{
	config=_config;
	return TRUE;
}


BOOL CDeviceObjectGL::Present()
{
	return TRUE;
}


//Device lost/reset/created/destroyed
void CDeviceObjectGL::_OnLost()
{
// 	for (int i=0;i<_stackRT.size();i++)
// 	{
// 		for (int j=0;j<MAX_RT_COUNT;j++)
// 			SAFE_RELEASE(_stackRT[i].surfs[j]);
// 	}
// 	_stackRT.clear();
// 	_bMRT=FALSE;
// 	for (int i=0;i<_stackDSBuffer.size();i++)
// 		SAFE_RELEASE(_stackDSBuffer[i]);
// 	_stackDSBuffer.clear();
// 	_w=_h=0;

	ClearVBBind();

	((CRenderSystemGL*)_pRS)->OnDeviceLost();
}

HRESULT CDeviceObjectGL::_OnReset()
{
	((CRenderSystemGL*)_pRS)->OnDeviceReset();

	_state.SetInvalid();

	return S_OK;
}
void CDeviceObjectGL::_OnDestroyed()
{
	_mapDecl.clear();

	((CRenderSystemGL*)_pRS)->OnDeviceDestroy();
}
HRESULT CDeviceObjectGL::_OnCreated()
{
	((CRenderSystemGL*)_pRS)->OnDeviceCreate();

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
//State management

BOOL CDeviceObjectGL::PushRenderTarget(SurfHandle *rts,DWORD count)
{
	return FALSE;
// 	_MRT mrt;
// 	if ((count>MAX_RT_COUNT)||(count<=0))
// 		return FALSE;
// 
// 	for (int i=0;i<count;i++)
// 	{
// 		SurfHandle &rt=rts[i];
// 		XDirect3DSurface *&surf=mrt.surfs[i];
// 		if (rt.IsEmpty())
// 			return FALSE;
// 
// 		if (rt.tex)
// 		{
// 			if (rt.tex->Touch()!=A_Ok)
// 				return FALSE;
// 			surf=(XDirect3DSurface *)rt.tex->GetSurf(0,rt.iFace);
// 			if (!surf)
// 				return FALSE;
// 		}
// 		else
// 		{
// 			if (A_Ok!=rt.surf->Touch())
// 				return FALSE;
// 			surf=(XDirect3DSurface *)rt.surf->GetSurf();
// 			if (!surf)
// 				return FALSE;
// 		}
// 
// 	}
// 
// 	_stackRT.push_back(mrt);
// 	_bMRT=(count>1);
// 
// 
// 	for (int i=0;i<MAX_RT_COUNT;i++)
// 	{
// 		if (D3D_OK!=_device->SetRenderTarget(i,mrt.surfs[i]))
// 		{
// 			g_logDevice.Prompt("Failed to SetRenderTarget() when PushRenderTarget(..)");
// 			PopRenderTarget();
// 			return FALSE;
// 		}
// 	}
// 
// 
// 	GetSurfSize(mrt.surfs[0],_w,_h);
// 
// 	return TRUE;
}


void CDeviceObjectGL::PopRenderTarget()
{
// 	if (_stackRT.size()<=1)
// 		return;
// 
// 	_MRT *mrt=&_stackRT[_stackRT.size()-1];
// 	for (int i=0;i<MAX_RT_COUNT;i++)
// 		SAFE_RELEASE(mrt->surfs[i]);
// 	_stackRT.pop_back();
// 
// 
// 	mrt=&_stackRT[_stackRT.size()-1];
// 	for (int i=0;i<MAX_RT_COUNT;i++)
// 		_device->SetRenderTarget(i,mrt->surfs[i]);
// 	GetSurfSize(mrt->surfs[0],_w,_h);
// 	_bMRT=(mrt->surfs[1]!=NULL);
}

BOOL CDeviceObjectGL::PushDSBuffer(SurfHandle &ds)
{
	return FALSE;
// 	XDirect3DSurface *surf=NULL;
// 	if (ds.IsEmpty())
// 		return FALSE;
// 	if (ds.surf!=SurfHandle_DisableDS.surf)
// 	{
// 		if (ds.surf)
// 		{
// 			if (A_Ok!=ds.surf->Touch())
// 				return FALSE;
// 			surf=(XDirect3DSurface *)ds.surf->GetSurf();
// 			if (surf)
// 				surf->AddRef();
// 		}
// 		else
// 		{
// 			if (A_Ok!=ds.tex->Touch())
// 				return FALSE;
// 			surf=(XDirect3DSurface *)ds.tex->GetSurf(0,0);
// 		}
// 		if (!surf)
// 			return FALSE;
// 
// 		if (D3D_OK!=_device->SetDepthStencilSurface(surf))
// 			return FALSE;
// 
// 	}
// 	else
// 	{
// 		if (D3D_OK!=_device->SetDepthStencilSurface(NULL))
// 			return FALSE;
// 	}
// 
// 	_stackDSBuffer.push_back(surf);
// 
// 	return TRUE;
}

void CDeviceObjectGL::PopDSBuffer()
{
// 	if (_stackDSBuffer.size()<=1)
// 		return;
// 
// 	SAFE_RELEASE(_stackDSBuffer[_stackDSBuffer.size()-1]);
// 	_stackDSBuffer.pop_back();
// 
// 	XDirect3DSurface *surf=_stackDSBuffer[_stackDSBuffer.size()-1];
// 	_device->SetDepthStencilSurface(surf);
}


BOOL CDeviceObjectGL::SetViewport(ViewportInfo &v)
{
	glViewport(v.x,v.y,v.x+v.w,v.y+v.h);
	glDepthRangef(v.minz,v.maxz);
	return TRUE;
}


//////////////////////////////////////
//Helper functions
BOOL CDeviceObjectGL::ColorFill(DWORD color,DWORD iBackBuffer,RECT *pRect)
{
	return FALSE;
}

BOOL CDeviceObjectGL::ClearZBuffer(RECT *pRect)
{

	return ClearBuffer(ClearBuffer_Depth,pRect);
}

BOOL CDeviceObjectGL::ClearBuffer(ClearBufferFlag flag,RECT *pRect,DWORD col,float z,DWORD s)
{
	GLbitfield mask=0;
	if (flag&ClearBuffer_RT)
	{
		mask|=GL_COLOR_BUFFER_BIT;
		i_math::vector4df vCol;
		vCol.fromDwordColor(col);
		glClearColor(vCol.r,vCol.g,vCol.b,vCol.a);
	}
	if (flag&ClearBuffer_Depth)
	{
		mask|=GL_DEPTH_BUFFER_BIT;
		glClearDepthf(z);
	}
	if (flag&ClearBuffer_Stencil)
	{
		mask|=GL_STENCIL_BUFFER_BIT;
		glClearStencil(s);
	}

	glClear(mask);

	return TRUE;
}



//Retrieve width/height from the window handle
DWORD CDeviceObjectGL::Width()
{
	return _w;
}
DWORD CDeviceObjectGL::Height()
{
	return _h;
}


BOOL CDeviceObjectGL::FlushCommand()
{
	glFinish();
	return TRUE;
}

void CDeviceObjectGL::ClearVBBind()
{
	BindVB(NULL,0,NULL,NULL);
}

void CDeviceObjectGL::_BindVB(VBInfoGL** vbs,VertexDeclarationGL *decl,int vbase)
{
	int iCurBuf=-1;
	for (int i=0;i<decl->attribs.size();i++)
	{
		VertexDeclarationGL::AttribInfo *info=&decl->attribs[i];

		if (info->iBuf!=iCurBuf)
		{
			glBindBuffer(GL_ARRAY_BUFFER, vbs[info->iBuf]->GetHandle());
			iCurBuf=info->iBuf;
		}

		glEnableVertexAttribArray(info->id);
	}

	for (int i=0;i<decl->attribs.size();i++)
	{
		VertexDeclarationGL::AttribInfo *info=&decl->attribs[i];
		glVertexAttribPointer(info->id,info->count,(GLenum)info->fmt,info->bNormalized?GL_TRUE:GL_FALSE,info->stride,(GLvoid*)(vbase*info->stride+info->off));
	}
}

void CDeviceObjectGL::_ClearBindVB(VertexDeclarationGL *decl)
{
	for (int i=0;i<decl->attribs.size();i++)
	{
		VertexDeclarationGL::AttribInfo *info=&decl->attribs[i];
		glDisableVertexAttribArray(info->id);
	}
}

void CDeviceObjectGL::_BindIB(IBInfoGL*ib)
{
	if (ib)
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib->_handle);
	else
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GLHandle_Null);
}


void AddAttrib(VertexDeclarationGL *decl,FVFEx fvfTotal,FVFEx fvf,int iBuf)
{
	if (fvfTotal==FVFEX_NULL)
		return;
	if (fvf==FVFEX_NULL)
		return;

	extern FVFExInfo g_aFVFList[];
	extern int g_SizeOfFVFList;
	for (int i=0;i<g_SizeOfFVFList;i++)
	{
		if ((fvf&g_aFVFList[i].m_fvf)==g_aFVFList[i].m_fvf)
		{
			FVFEx fvfElement;
			fvfElement=g_aFVFList[i].m_fvf;

			if ((fvfElement&fvfTotal)==fvfElement)//if this element exists in fvfTotal
			{
				decl->attribs.resize(decl->attribs.size()+1);
				VertexDeclarationGL::AttribInfo *attrib=&decl->attribs[decl->attribs.size()-1];

				attrib->id=(BYTE)i;
				attrib->iBuf=(BYTE)iBuf;
				attrib->count=(BYTE)g_aFVFList[i].m_countGL;
				attrib->fmt=g_aFVFList[i].m_fmtGL;
				attrib->bNormalized=(g_aFVFList[i].m_bNormalizeGL!=FALSE);
				attrib->stride=(BYTE)fvfSize(fvfTotal);
				attrib->off=(BYTE)fvfOffset(fvfTotal,fvfElement);
			}

			fvf&=(~fvfElement);//cull out this element from fvf
			if (fvf==FVFEX_NULL)
				return;
		}
	}
}



BOOL CDeviceObjectGL::BindVB(VBInfoGL** vbs,DWORD nVB,IBInfoGL*ib,VBBindArg *arg)
{
	if (!arg)
		arg=&_bindarg;


	assert(nVB<=MAX_BIND_VB);
	if (nVB==0)
	{
		if (_declCur)
			_ClearBindVB(_declCur);

		_BindIB(NULL);

		_drawarg._bValid=FALSE;
		return TRUE;
	}


	if (_declCur)
		_ClearBindVB(_declCur);
	_drawarg._bValid=FALSE;

	if (TRUE)
	{
		//设vertex declaration
		if (TRUE)
		{
			FVFExKey k;
			k.fvfDraw=arg->fvfDraw;
			if (TRUE)
			{
				int i;
				for (i=0;i<nVB;i++)
					k.fvfs[i]=vbs[i]->_fvf;
				for (;i<MAX_BIND_VB;i++)
					k.fvfs[i]=0;
			}

			BOOL bDeclChanged=TRUE;
			if (_declCur)
			{
				if (_declCur->key==k)
					bDeclChanged=FALSE;
			}

			if (bDeclChanged)
			{
				VertexDeclarationGL *pDecl;
				std::map<FVFExKey,VertexDeclarationGL>::iterator it;
				it=_mapDecl.find(k);
				if (it==_mapDecl.end())
				{
					pDecl=&_mapDecl[k];
					pDecl->attribs.reserve(16);

					DWORD count=0;
					for (int i=0;i<nVB;i++)
					{
						VBInfoGL *vb=vbs[i];
						AddAttrib(pDecl,vb->_fvf,arg->fvfDraw==0?vb->_fvf:arg->fvfDraw,i);
					}
					pDecl->key=k;
				}
				else
					pDecl=&(*it).second;

				_declCur=pDecl;
			}
		}

		//设stream
		for (int i=0;i<nVB;i++)
		{
			VBInfoGL *vb=vbs[i];
			assert(vb);
			vb->Touch();
		}

		_BindVB(vbs,_declCur,arg->vbase);
	}

	if (ib)
	{
		ib->Touch();

		_BindIB(ib);

		_drawarg._bIndexed=TRUE;
		_drawarg._nIndice=ib->_count;
	}
	else
	{
		_BindIB(NULL);
		_drawarg._bIndexed=FALSE;
	}

	if (arg->vcount==0)
		_drawarg._nVertice=vbs[0]->_count-arg->vbase;
	else
		_drawarg._nVertice=arg->vcount;
	_drawarg._iStreamOff=arg->vbase;
	_drawarg._primstart=arg->primstart;
	_drawarg._primcount=arg->primcount;
	_drawarg._ds=arg->fillmode;
	_drawarg._dpt=arg->dpt;

	_drawarg._bValid=TRUE;//ok

	return TRUE;
}


BOOL CDeviceObjectGL::DrawPrim()
{
	if (_bLost)
		return FALSE;

	if (!_drawarg._bValid)
		return FALSE;

	if ((_drawarg._dpt==(DWORD)D3DPT_POINTLIST)&&(_drawarg._bIndexed))
		return FALSE;//not supported

	DWORD nIndex,iStart;
	if (_drawarg._bIndexed)
		nIndex=_drawarg._nIndice;
	else
		nIndex=_drawarg._nVertice;
	if ((_drawarg._primstart==-1)&&(_drawarg._primcount==-1))
		iStart=0;
	else
		iStart=_drawarg._primstart;

	if (_drawarg._primcount!=-1)
	{
		switch((D3DPRIMITIVETYPE)_drawarg._dpt)
		{
			case D3DPT_LINELIST:
			{
				nIndex=_drawarg._primcount*2;
				break;
			}
			case D3DPT_LINESTRIP:
			{
				nIndex=_drawarg._primcount+1;
				break;
			}
			case D3DPT_TRIANGLELIST:
			{
				nIndex=_drawarg._primcount*3;
				break;
			}
			case D3DPT_TRIANGLESTRIP:
			case D3DPT_TRIANGLEFAN:
			{
				nIndex=_drawarg._primcount+2;
				break;
			}
		}
	}


	GLenum mode;
	switch((D3DPRIMITIVETYPE)_drawarg._dpt)
	{
		case D3DPT_POINTLIST:
		{
			mode=GL_POINTS;
			break;
		}
		case D3DPT_LINELIST:
		{
			iStart*=2;
			mode=GL_LINES;
			break;
		}
		case D3DPT_LINESTRIP:
		{
			mode=GL_LINE_STRIP;
			break;
		}
		case D3DPT_TRIANGLELIST:
		{
			iStart*=3;
			mode=GL_TRIANGLES;
			break;
		}
		case D3DPT_TRIANGLESTRIP:
		{
			mode=GL_TRIANGLE_STRIP;
			break;
		}
		case D3DPT_TRIANGLEFAN:
		{
			mode=GL_TRIANGLE_FAN;
			break;
		}
		default:
			return FALSE;
	}
	if (!_drawarg._bIndexed)
		glDrawArrays(mode,iStart,nIndex);
	else
		glDrawElements(mode,nIndex,GL_UNSIGNED_SHORT,(GLvoid *)iStart);
	return TRUE;
}

bool operator<(const FVFExKey&v1, const FVFExKey&v2)
{
	if (v1.fvfDraw<v2.fvfDraw)
		return true;
	else
	{
		for (int i=0;i<MAX_BIND_VB;i++)
		{
			if (v1.fvfs[i]<v2.fvfs[i])
				return true;
		}
		return false;
	}
}

bool operator==(const FVFExKey&v1, const FVFExKey&v2)
{
	return memcmp(&v1,&v2,sizeof(v1))==0;
}




void CDeviceObjectGL::SetAlphaTest(AlphaTestMode mode,DWORD ref)
{
//	if (_bMRT)
//	{
//		if (mode!=AlphaTest_Disable)
//			mode=AlphaTest_Disable;
//	}

	if ((mode==_state.modeAlphaTest)&&(ref==_state.refAlphaTest))
		return;
	_state.modeAlphaTest=mode;
	_state.refAlphaTest=(WORD)ref;

	//gles 不支持alpha test
}


void CDeviceObjectGL::SetBlend(ShaderBlendMode mode)
{
	if (_state.modeBlend==mode)
		return;


	_state.modeBlend=mode;
	if (mode==Blend_Opaque)
	{
		glBlendFunc(GL_ONE,GL_ZERO);
		glBlendEquation(GL_FUNC_ADD);

		return;
	}
	if (mode==Blend_AlphaBlend)
	{
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glBlendEquation(GL_FUNC_ADD);

		return;
	}
	if (mode==Blend_Additive)
	{
		glBlendFunc(GL_SRC_ALPHA,GL_ONE);
		glBlendEquation(GL_FUNC_ADD);

		return;
	}

	if (mode==Blend_Modulate)
	{
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ZERO,GL_SRC_COLOR);
		return;
	}

	if (mode==Blend_Enlighten)
	{
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_COLOR,GL_DST_ALPHA);

		return;
	}

	if (mode==Blend_Inverse)
	{
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
		return;
	}

	//XXXXX:more blend mode

}

void CDeviceObjectGL::SetStencilOp(StencilMode mode,WORD ref,WORD mask)
{
	if ((_state.modeStencil==mode)&&(_state.refStencil==ref)&&(_state.maskStencil==mask))
		return;
	_state.modeStencil=mode;
	_state.refStencil=ref;
	_state.maskStencil=mask;

	switch(mode)
	{
	case Sten_Disable:
		glStencilMask(0);
		glStencilFunc(GL_ALWAYS,0,0);
		glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
		glColorMask(true,true,true,true);
		break;
	case Sten_Write:
	case Sten_WriteNoColor:
		glStencilMask(0xffffffff);
		glStencilFunc(GL_ALWAYS,ref,0xffffffff);
		glStencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);
		if (mode==Sten_Write)
			glColorMask(true,true,true,true);
		else
			glColorMask(false,false,false,false);
		break;
	case Sten_Filter:
	case Sten_InvFilter:
		glColorMask(true,true,true,true);
		glStencilMask(0xffffffff);
		glStencilFunc(mode==Sten_Filter?GL_EQUAL:GL_NOTEQUAL,ref,0xffffffff);
		glStencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);
		break;

	case Sten_Inc:
	case Sten_IncNoColor:
		glStencilMask(0xffffffff);
		if (mode==Sten_Inc)
			glColorMask(true,true,true,true);
		else
			glColorMask(false,false,false,false);
		glStencilFunc(GL_ALWAYS,0xffffffff,0xffffffff);
		glStencilOp(GL_KEEP,GL_KEEP,GL_INCR);
		break;

	case Sten_FilterWrite:
	case Sten_InvFilterWrite:
		glStencilMask(0xffffffff);
		glColorMask(true,true,true,true);
		glStencilFunc(mode==Sten_FilterWrite?GL_EQUAL:GL_NOTEQUAL,ref,0xffffffff);
		glStencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);
		break;

	case Sten_FilterInc:
		glStencilMask(0xffffffff);
		glColorMask(true,true,true,true);
		glStencilFunc(GL_EQUAL,ref,0xffffffff);
		glStencilOp(GL_KEEP,GL_KEEP,GL_INCR);
		break;

		//XXXXX:more stencil mode
	}
}

void CDeviceObjectGL::SetDepthMethod(DepthMode mode)
{
	if (_state.modeDepth==mode)
		return;
	_state.modeDepth=mode;
	switch(mode)
	{
	case Depth_Default:
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(true);
		break;
	case Depth_NoCmp:
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_ALWAYS);
		glDepthMask(true);
		break;
	case Depth_NoWrite:
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(false);
		break;
	case Depth_Disable:
		glDisable(GL_DEPTH_TEST);
		break;
	case Depth_FartherNoWrite:
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_GEQUAL);
		glDepthMask(false);
		break;
	}
}

void CDeviceObjectGL::SetFacing(FacingMode mode)
{
	if (_state.modeFacing==mode)
		return;
	_state.modeFacing=mode;
	switch (mode)
	{
	case Facing_Front:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		break;
	case Facing_Back:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		break;
	case Facing_Both:
		glDisable(GL_CULL_FACE);
		break;
	}
}
