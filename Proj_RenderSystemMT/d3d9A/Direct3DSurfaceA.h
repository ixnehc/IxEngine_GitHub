#pragma once
#include "d3d9A.h"

#include "devicequeue.h"



class CDirect3DDevice;
class CDirect3DSurface
{
public:
	DEFINE_D3D9A_CLASS(CDirect3DSurface)
	CDirect3DSurface()
	{
		_nRef=1;
		_core=NULL;
		_dev=NULL;
	}
	D3D9A_ADDREF()
	D3D9A_RELEASE(Surface_Release)

	STDMETHOD(GetDevice)(THIS_ CDirect3DDevice** ppDevice);
	STDMETHOD(GetDesc)(THIS_ D3DSURFACE_DESC *pDesc)
	{
		*pDesc=_desc;
		return D3D_OK;
	}
	STDMETHOD(LockRect)(THIS_ D3DLOCKED_RECT* pLockedRect,CONST RECT* pRect,DWORD Flags)
	{
		DQ_Flush();

		if (!_core)
			return D3DERR_INVALIDCALL;
		return _core->LockRect(pLockedRect,pRect,Flags);
	}
	STDMETHOD(UnlockRect)(THIS)
	{
		DQ_Flush();

		if (!_core)
			return D3DERR_INVALIDCALL;
		return _core->UnlockRect();
	}

public:
	void _AssignDesc(UINT Width,UINT Height,D3DFORMAT  Format)
	{
		_desc.Width=Width;
		_desc.Height=Height;
		_desc.Format=Format;
	}
	int _nRef;

	IDirect3DSurface9 *_core;

	CDirect3DDevice *_dev;
	D3DSURFACE_DESC _desc;
};
