#pragma once
#include "d3d9A.h"

#include "devicequeue.h"

#include "Direct3DDeviceA.h"


class CDirect3D
{
public:
	DEFINE_D3D9A_CLASS(CDirect3D)

	CDirect3D()
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
			DQ_Flush();
			SAFE_RELEASE(_core);
			D3D9A_Delete(this);
		}
		return ret;
	}

	STDMETHOD_(UINT, GetAdapterCount)(THIS)
	{
		DQ_Flush();
		return _core->GetAdapterCount();
	}
	STDMETHOD(GetAdapterIdentifier)(THIS_ UINT Adapter,DWORD Flags,D3DADAPTER_IDENTIFIER9* pIdentifier)
	{
		DQ_Flush();
		return _core->GetAdapterIdentifier(Adapter,Flags,pIdentifier);
	}
	STDMETHOD_(UINT, GetAdapterModeCount)(THIS_ UINT Adapter,D3DFORMAT Format)
	{
		DQ_Flush();
		return _core->GetAdapterModeCount(Adapter,Format);
	}
	STDMETHOD(EnumAdapterModes)(THIS_ UINT Adapter,D3DFORMAT Format,UINT Mode,D3DDISPLAYMODE* pMode)
	{
		DQ_Flush();
		return _core->EnumAdapterModes(Adapter,Format,Mode,pMode);
	}
	STDMETHOD(GetAdapterDisplayMode)(THIS_ UINT Adapter,D3DDISPLAYMODE* pMode)
	{
		DQ_Flush();
		return _core->GetAdapterDisplayMode(Adapter,pMode);
	}
	STDMETHOD(CheckDeviceType)(THIS_ UINT Adapter,D3DDEVTYPE DevType,
						D3DFORMAT AdapterFormat,D3DFORMAT BackBufferFormat,BOOL bWindowed)
	{
		DQ_Flush();
		return _core->CheckDeviceType(Adapter,DevType,AdapterFormat,BackBufferFormat,bWindowed);
	}
	STDMETHOD(CheckDeviceFormat)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,
					D3DFORMAT AdapterFormat,DWORD Usage,D3DRESOURCETYPE RType,D3DFORMAT CheckFormat)
	{
		DQ_Flush();
		return _core->CheckDeviceFormat(Adapter,DeviceType,AdapterFormat,Usage,RType,CheckFormat);
	}
	STDMETHOD(CheckDeviceMultiSampleType)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,
					D3DFORMAT SurfaceFormat,BOOL Windowed,D3DMULTISAMPLE_TYPE MultiSampleType,DWORD* pQualityLevels)
	{
		DQ_Flush();
		return _core->CheckDeviceMultiSampleType(Adapter,DeviceType,SurfaceFormat,Windowed,MultiSampleType,pQualityLevels);
	}
	STDMETHOD(CheckDepthStencilMatch)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,
										D3DFORMAT AdapterFormat,D3DFORMAT RenderTargetFormat,D3DFORMAT DepthStencilFormat)
	{
		DQ_Flush();
		return _core->CheckDepthStencilMatch(Adapter,DeviceType,AdapterFormat,RenderTargetFormat,DepthStencilFormat);
	}
	STDMETHOD(GetDeviceCaps)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,D3DCAPS9* pCaps)
	{
		DQ_Flush();
		return _core->GetDeviceCaps(Adapter,DeviceType,pCaps);
	}
	STDMETHOD(CreateDevice)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,
													HWND hFocusWindow,DWORD BehaviorFlags,
													D3DPRESENT_PARAMETERS* pPresentationParameters,
													CDirect3DDevice** ppReturnedDeviceInterface)
	{
		DQ_Flush();

		IDirect3DDevice9 *dev;
		HRESULT hr=_core->CreateDevice(Adapter,DeviceType,hFocusWindow,
											BehaviorFlags,pPresentationParameters,&dev);
		if (hr!=D3D_OK)
			return hr;

		(*ppReturnedDeviceInterface)=D3D9A_New(CDirect3DDevice);
		(*ppReturnedDeviceInterface)->_core=dev;
		(*ppReturnedDeviceInterface)->_BuildCache();

		return D3D_OK;
	}

	IDirect3D9 *_core;
	int _nRef;
};

