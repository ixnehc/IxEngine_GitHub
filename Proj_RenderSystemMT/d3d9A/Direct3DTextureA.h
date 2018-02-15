#pragma once
#include "d3d9A.h"

#include "devicequeue.h"

#include "Direct3DSurfaceA.h"

class CDirect3DDevice;


class CDirect3DBaseTexture
{
public:
	virtual CClass *GetClass()=0;
	CDirect3DBaseTexture()
	{
		_nRef=1;
		_core=NULL;
	}
	D3D9A_ADDREF()
	STDMETHOD_(ULONG,Release)(THIS)
	{
		ULONG ret=--_nRef;
		if (_nRef<=0)
		{
			for (int i=0;i<_surfs.size();i++)
				SAFE_RELEASE(_surfs[i]);
			_surfs.clear();

			DQ_PushFuncType(Texture_Release);
			DQ_Push4C(this);
		}
		return ret;
	}

	STDMETHOD(SetAutoGenFilterType)(THIS_ D3DTEXTUREFILTERTYPE FilterType)
	{
		DQ_PushFuncType(Texture_SetAutoGenFilterType);
		DQ_Push4C(this);
		DQ_Push4(FilterType);

		return D3D_OK;
	}


public:
	int _nRef;
	IDirect3DBaseTexture9 * _core;

	void _BuildSurfs(DWORD n,CDirect3DDevice *dev,
										UINT Width,UINT Height,D3DFORMAT  Format)
	{
		_surfs.resize(n);
		for (int i=0;i<n;i++)
		{
			_surfs[i]=D3D9A_New(CDirect3DSurface);
			_surfs[i]->_dev=dev;
			_surfs[i]->_AssignDesc(Width,Height,Format);
		}
	}
	
	//0:2D texture,1:cube texture,2:volume texture
	void _FillSurfs(int type)
	{
		if (!_core)
			return;
		IDirect3DSurface9*surf;
		switch(type)
		{
			case 0://2D texture
			{
				((IDirect3DTexture9*)_core)->GetSurfaceLevel(0,&surf);
				_surfs[0]->_core=surf;
				break;
			}
			case 1://Cube texture
			{
				for (int i=0;i<6;i++)
				{
					((IDirect3DCubeTexture9*)_core)->GetCubeMapSurface((D3DCUBEMAP_FACES)i,0,&surf);
					_surfs[i]->_core=surf;
				}

				break;
			}
			case 2://Volume texture
			{
				//Do nothing
				break;
			}
		}
	}
	std::vector<CDirect3DSurface*>_surfs;
};


class CDirect3DCubeTexture:public CDirect3DBaseTexture
{
public:
	DEFINE_D3D9A_CLASS(CDirect3DCubeTexture)

	STDMETHOD(GetCubeMapSurface)(THIS_ D3DCUBEMAP_FACES FaceType,
									UINT Level,CDirect3DSurface** ppCubeMapSurface)
	{
		*ppCubeMapSurface=NULL;
		if (((int)FaceType)>=_surfs.size())
			return D3DERR_INVALIDCALL;
		if (Level>0)
			return D3DERR_INVALIDCALL;
		*ppCubeMapSurface=_surfs[(int)FaceType];
		_surfs[(int)FaceType]->AddRef();

		return D3D_OK;
	}

};

class CDirect3DVolumeTexture:public CDirect3DBaseTexture
{
public:
	DEFINE_D3D9A_CLASS(CDirect3DVolumeTexture)


};

class CDirect3DTexture:public CDirect3DBaseTexture
{
public:
	DEFINE_D3D9A_CLASS(CDirect3DTexture)
	STDMETHOD(GetSurfaceLevel)(THIS_ UINT Level,CDirect3DSurface** ppSurfaceLevel)
	{
		*ppSurfaceLevel=NULL;
		if (Level>0)
			return D3DERR_INVALIDCALL;
		if (_surfs.size()<=0)
			return D3DERR_INVALIDCALL;
		*ppSurfaceLevel=_surfs[0];
		_surfs[0]->AddRef();

		return D3D_OK;
	}
	STDMETHOD(LockRect)(THIS_ UINT Level,D3DLOCKED_RECT* pLockedRect,CONST RECT* pRect,DWORD Flags)
	{
		DQ_Flush();
		return ((IDirect3DTexture9*)_core)->LockRect(Level,pLockedRect,pRect,Flags);
	}

	STDMETHOD(UnlockRect)(THIS_ UINT Level)
	{
		return ((IDirect3DTexture9*)_core)->UnlockRect(Level);
	}
};

