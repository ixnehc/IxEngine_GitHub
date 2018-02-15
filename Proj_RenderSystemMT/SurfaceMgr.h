#pragma once

#include "RenderSystem/ISurface.h"


#include "Base.h"
#include "ResourceBase.h"


#include <vector>
#include <string>
#include <map>

class CDeviceObject;
class CSurfaceMgr;


//depth stencil buffer
class CSurface:public ISurface,public CResource
{
protected:
	SurfInfo _info;
	XDirect3DSurface *_surf;
public:
	DECLARE_CLASS(CSurface);
	CSurface();

	//interfaces
	virtual IRenderSystem *GetRS(){	return CResource::GetRS();}
	virtual int AddRef(){	return CResource::AddRef();}
	virtual int Release();
	virtual const char *GetPath(){	return CResource::GetPath();}
	virtual AResult Touch(){	return CResource::Touch();}
	virtual BOOL ForceTouch(){	return CResource::ForceTouch();}
	virtual DWORD GetVer(){	return CResource::GetVer();}							


	DWORD GetWidth();
	DWORD GetHeight();
	DWORD GetFormat();//return a D3DFORMAT value
	DWORD GetFlag();
	void* GetSurf();//return a XDirect3DSurface ptr

protected:
	virtual BOOL _OnTouch(IRenderSystem *pRS)	;
	virtual void _OnUnload();

	friend class CSurfaceMgr;
};


class CSurfaceMgr:public ISurfaceMgr,public CResourceMgr
{
public:
	CSurfaceMgr();
	void OnDeviceLost();
	void OnDeviceReset();

	//interfaces
	RESOURCEMGR_CORE
	virtual IResource *ObtainRes(const char *path)	{		return NULL;	}
	virtual ISurface *CreateRT(DWORD w,DWORD h,DWORD fmt,BOOL bForce=TRUE);
	virtual ISurface *CreateDS(DWORD w,DWORD h,DWORD fmt,BOOL bForce=TRUE);


};


