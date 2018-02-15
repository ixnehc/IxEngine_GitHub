/********************************************************************
	created:	2006/07/10
	created:	10:7:2006   14:11
	filename: 	e:\IxEngine\Proj_RenderSystem\SurfaceMgr.cpp
	file path:	e:\IxEngine\Proj_RenderSystem
	file base:	SurfaceMgr
	file ext:	cpp
	author:		cxi
	
	purpose:	surface resource manager,ISurface,ISurfaceMgr implement
*********************************************************************/
#include "stdh.h"

#pragma warning(disable:4018)

#include "RenderSystem.h"

#include "SurfaceMgr.h"
#include "DeviceObject.h"
#include <assert.h>


//////////////////////////////////////////////////////////////////////////
//CSurface
IMPLEMENT_CLASS(CSurface);
CSurface::CSurface()
{
	_surf=NULL;
}


int CSurface::Release()
{
	int c=CResource::Release();
	if (c<=0)
		_OnUnload();

	return c;
}


BOOL CSurface::_OnTouch(IRenderSystem *pRS)
{
	_surf=NULL;

	XDirect3DDevice *pDevice=(XDirect3DDevice *)((CRenderSystem*)pRS)->GetDeviceObj()->GetDevice();
	HRESULT hr=D3D_OK+1;//a not D3D_OK value
	if (_info.flag&SURFINFOFLAG_RENDERTARGET)
		hr=pDevice->CreateRenderTarget(_info.width,_info.height,
								(D3DFORMAT)_info.fmt,D3DMULTISAMPLE_NONE,0,FALSE,
								&_surf,NULL);
	if (_info.flag&SURFINFOFLAG_DEPTHSTENCILBUFFER)
		hr=pDevice->CreateDepthStencilSurface(_info.width,_info.height,
								(D3DFORMAT)_info.fmt,D3DMULTISAMPLE_NONE,0,TRUE,
								&_surf,NULL);
	if (hr!=D3D_OK)
	{
		_surf=NULL;
		return FALSE;
	} 
	return TRUE;
}

void CSurface::_OnUnload()
{
	SAFE_RELEASE_D3DRES(_surf);
}


DWORD CSurface::GetWidth()
{
	return _info.width;
}
DWORD CSurface::GetHeight()
{
	return _info.height;
}
DWORD CSurface::GetFormat()
{
	return _info.fmt;
}
DWORD CSurface::GetFlag()
{
	return _info.flag;
}

void* CSurface::GetSurf()
{
	return _surf;
}


//////////////////////////////////////////////////////////////////////////
//CSurfaceMgr

CSurfaceMgr::CSurfaceMgr()
{
}


void CSurfaceMgr::OnDeviceLost()
{
	for (int i=0;i<_vecRes.size();i++)
	{
		CSurface*p=(CSurface*)(_vecRes[i]->_owner);
		if (p)
		{
			SAFE_RELEASE(p->_surf);
			p->SetState(CResource::Abandoned);
		}
	}
}

void CSurfaceMgr::OnDeviceReset()
{
	for (int i=0;i<_vecRes.size();i++)
		_vecRes[i]->SetState(CResource::Loaded);
}

ISurface *CSurfaceMgr::CreateRT(DWORD w,DWORD h,DWORD fmt,BOOL bForce)
{
	CSurface *p=_ObtainRes<CSurface>();

	p->_info.fmt=fmt;
	p->_info.width=(WORD)w;
	p->_info.height=(WORD)h;
	p->_info.flag=SURFINFOFLAG_RENDERTARGET;

	p->SetState(CResource::Loaded);
	return p;
}

ISurface *CSurfaceMgr::CreateDS(DWORD w,DWORD h,DWORD fmt,BOOL bForce)
{
	CSurface *p=_ObtainRes<CSurface>();

	p->_info.fmt=fmt;
	p->_info.width=(WORD)w;
	p->_info.height=(WORD)h;
	p->_info.flag=SURFINFOFLAG_DEPTHSTENCILBUFFER;

	p->SetState(CResource::Loaded);
	return p;
}
