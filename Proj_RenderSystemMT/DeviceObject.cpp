d/********************************************************************
	created:	2006/06/04
	created:	4:6:2006   20:07
	filename: 	d:\IxEngine\Proj_RenderSystem\DeviceObject.cpp
	author:		cxi
	
	purpose:	an object representing the d3d device,IDeviceObject implement
*********************************************************************/
#include "stdh.h"
#include "Base.h"
#include "interface/interface.h"
#include "RenderSystem.h"
#include "DeviceObject.h"
#include "DeviceChooser.h"

#include "Log/LastError.h"
#include "Log\LogFile.h"
#include "Log/LogDump.h"

#include "assert.h"

#include "timer/profiler.h"
#include "fvfex/fvfex.h"

#include "VertexMgr.h"



// #define _NVPerfHUD

DEFINE_DQ();


LogFile g_logDevice("DeviceObject");

//--------------------------------------------------------------------------------------
// Returns the string for the given D3DFORMAT.
//--------------------------------------------------------------------------------------
const char *d3dfmtToString( D3DFORMAT format)
{
	char* pstr = NULL;
	switch( format )
	{
	case D3DFMT_UNKNOWN:         pstr = "D3DFMT_UNKNOWN"; break;
	case D3DFMT_R8G8B8:          pstr = "D3DFMT_R8G8B8"; break;
	case D3DFMT_A8R8G8B8:        pstr = "D3DFMT_A8R8G8B8"; break;
	case D3DFMT_X8R8G8B8:        pstr = "D3DFMT_X8R8G8B8"; break;
	case D3DFMT_R5G6B5:          pstr = "D3DFMT_R5G6B5"; break;
	case D3DFMT_X1R5G5B5:        pstr = "D3DFMT_X1R5G5B5"; break;
	case D3DFMT_A1R5G5B5:        pstr = "D3DFMT_A1R5G5B5"; break;
	case D3DFMT_A4R4G4B4:        pstr ="D3DFMT_A4R4G4B4"; break;
	case D3DFMT_R3G3B2:          pstr = "D3DFMT_R3G3B2"; break;
	case D3DFMT_A8:              pstr ="D3DFMT_A8"; break;
	case D3DFMT_A8R3G3B2:        pstr ="D3DFMT_A8R3G3B2"; break;
	case D3DFMT_X4R4G4B4:        pstr ="D3DFMT_X4R4G4B4"; break;
	case D3DFMT_A2B10G10R10:     pstr = "D3DFMT_A2B10G10R10"; break;
	case D3DFMT_A8B8G8R8:        pstr = "D3DFMT_A8B8G8R8"; break;
	case D3DFMT_X8B8G8R8:        pstr = "D3DFMT_X8B8G8R8"; break;
	case D3DFMT_G16R16:          pstr = "D3DFMT_G16R16"; break;
	case D3DFMT_A2R10G10B10:     pstr = "D3DFMT_A2R10G10B10"; break;
	case D3DFMT_A16B16G16R16:    pstr = "D3DFMT_A16B16G16R16"; break;
	case D3DFMT_A8P8:            pstr = "D3DFMT_A8P8"; break;
	case D3DFMT_P8:              pstr = "D3DFMT_P8"; break;
	case D3DFMT_L8:              pstr = "D3DFMT_L8"; break;
	case D3DFMT_A8L8:            pstr = "D3DFMT_A8L8"; break;
	case D3DFMT_A4L4:            pstr = "D3DFMT_A4L4"; break;
	case D3DFMT_V8U8:            pstr = "D3DFMT_V8U8"; break;
	case D3DFMT_L6V5U5:          pstr = "D3DFMT_L6V5U5"; break;
	case D3DFMT_X8L8V8U8:        pstr = "D3DFMT_X8L8V8U8"; break;
	case D3DFMT_Q8W8V8U8:        pstr = "D3DFMT_Q8W8V8U8"; break;
	case D3DFMT_V16U16:          pstr = "D3DFMT_V16U16"; break;
	case D3DFMT_A2W10V10U10:     pstr = "D3DFMT_A2W10V10U10"; break;
	case D3DFMT_UYVY:            pstr = "D3DFMT_UYVY"; break;
	case D3DFMT_YUY2:            pstr = "D3DFMT_YUY2"; break;
	case D3DFMT_DXT1:            pstr = "D3DFMT_DXT1"; break;
	case D3DFMT_DXT2:            pstr = "D3DFMT_DXT2"; break;
	case D3DFMT_DXT3:            pstr = "D3DFMT_DXT3"; break;
	case D3DFMT_DXT4:            pstr = "D3DFMT_DXT4"; break;
	case D3DFMT_DXT5:            pstr = "D3DFMT_DXT5"; break;
	case D3DFMT_D16_LOCKABLE:    pstr = "D3DFMT_D16_LOCKABLE"; break;
	case D3DFMT_D32:             pstr = "D3DFMT_D32"; break;
	case D3DFMT_D15S1:           pstr = "D3DFMT_D15S1"; break;
	case D3DFMT_D24S8:           pstr = "D3DFMT_D24S8"; break;
	case D3DFMT_D24X8:           pstr = "D3DFMT_D24X8"; break;
	case D3DFMT_D24X4S4:         pstr = "D3DFMT_D24X4S4"; break;
	case D3DFMT_D16:             pstr = "D3DFMT_D16"; break;
	case D3DFMT_L16:             pstr = "D3DFMT_L16"; break;
	case D3DFMT_VERTEXDATA:      pstr = "D3DFMT_VERTEXDATA"; break;
	case D3DFMT_INDEX16:         pstr = "D3DFMT_INDEX16"; break;
	case D3DFMT_INDEX32:         pstr = "D3DFMT_INDEX32"; break;
	case D3DFMT_Q16W16V16U16:    pstr = "D3DFMT_Q16W16V16U16"; break;
	case D3DFMT_MULTI2_ARGB8:    pstr = "D3DFMT_MULTI2_ARGB8"; break;
	case D3DFMT_R16F:            pstr = "D3DFMT_R16F"; break;
	case D3DFMT_G16R16F:         pstr = "D3DFMT_G16R16F"; break;
	case D3DFMT_A16B16G16R16F:   pstr = "D3DFMT_A16B16G16R16F"; break;
	case D3DFMT_R32F:            pstr = "D3DFMT_R32F"; break;
	case D3DFMT_G32R32F:         pstr = "D3DFMT_G32R32F"; break;
	case D3DFMT_A32B32G32R32F:   pstr = "D3DFMT_A32B32G32R32F"; break;
	case D3DFMT_CxV8U8:          pstr = "D3DFMT_CxV8U8"; break;
	default:                     pstr = "Unknown format"; break;
	}
	return pstr;
}

//////////////////////////////////////////////////////////////////////////
//CDeviceObject


void CDeviceObject::_Error( HRESULT hr )
{
	char strBuffer[512];

	bool bFound = true; 
	switch( hr )
	{
	case DeviceErr_INITRTMANAGER:             strncpy( strBuffer, "Could not intialize the render target manager!.", 512 ); break;
	case DeviceErr_INITTEXTUREMANAGER:             strncpy( strBuffer, "Could not intialize the texture manager!.", 512 ); break;
	case DeviceErr_INITSHADERMANAGER:             strncpy( strBuffer, "Could not intialize the shader manager!.", 512 ); break;
	case DeviceErr_INITVBPOOL:             strncpy( strBuffer, "Could not intialize the vertex buffer pool!.", 512 ); break;
	case DeviceErr_NODIRECT3D:            strncpy( strBuffer, "Could not initialize Direct3D. You may want to check that the latest version of DirectX is correctly installed on your system.  Also make sure that this program was compiled with header files that match the installed DirectX DLLs.", 512 ); break;
	case DeviceErr_INCORRECTVERSION:       strncpy( strBuffer, "Incorrect version of Direct3D and/or D3DX.", 512 ); break;
	case DeviceErr_MEDIANOTFOUND:          strncpy( strBuffer, "Could not find required media. Ensure that the DirectX SDK is correctly installed.", 512 ); break;
	case DeviceErr_NONZEROREFCOUNT:        strncpy( strBuffer, "The D3D device has a non-zero reference count, meaning some objects were not released.", 512 ); break;
	case DeviceErr_CREATINGDEVICE:         strncpy( strBuffer, "Failed creating the Direct3D device.", 512 ); break;
	case DeviceErr_RESETTINGDEVICE:        strncpy( strBuffer, "Failed resetting the Direct3D device.", 512 ); break;
	case DeviceErr_CREATINGDEVICEOBJECTS:  strncpy( strBuffer, "Failed creating Direct3D device objects.", 512 ); break;
	case DeviceErr_RESETTINGDEVICEOBJECTS: strncpy( strBuffer, "Failed resetting Direct3D device objects.", 512 ); break;
	case DeviceErr_SWITCHEDTOREF:          strncpy( strBuffer, "Switching to the reference rasterizer,\na software device that implements the entire\nDirect3D feature set, but runs very slowly.", 512 ); break;
	case DeviceErr_NOCOMPATIBLEDEVICES:    
		if( GetSystemMetrics(SM_REMOTESESSION) != 0 )
			strncpy( strBuffer, "Direct3D does not work over a remote session.", 512 ); 
		else
			strncpy( strBuffer, "Could not find any compatible Direct3D devices.", 512 ); 
		break;
	default: bFound = false;break;
	}   
	strBuffer[511] = 0;

	if( bFound)
		g_logDevice.Prompt(strBuffer);

	SetLastErrCode((DWORD)hr);
}

void CDeviceObject::Zero()
{
	_d3d=NULL;
	_device=NULL;
	_event=NULL;
	_hwnd=NULL;
	_bLost=FALSE;
	ZeroMemory( &_caps, sizeof(_caps) );
	ZeroMemory( &_devicesetting, sizeof(_devicesetting) );

	memset(_vbCur,0,sizeof(_vbCur));
	_ibCur=NULL;

	_state.SetInvalid();

	_bMRT=FALSE;

	_pRS=NULL;
	_w=_h=0;
}


CDeviceObject::CDeviceObject():_config(NULL)
{
	Zero();
}

CDeviceObject::~CDeviceObject() 
{ 
}


BOOL CDeviceObject::Init(CRenderSystem *pRS)
{
	// Verify D3DX version
	if( !XD3DXCheckVersion( D3D_SDK_VERSION, D3DX_SDK_VERSION ) )
	{
		_Error( DeviceErr_INCORRECTVERSION );
		return FALSE;
	}

	if( !_d3d)
	{
		// This may fail if DirectX 9 isn't installed
		// This may fail if the DirectX headers are out of sync with the installed DirectX DLLs
		_d3d= XDirect3DCreate( D3D_SDK_VERSION );
	}

	if( !_d3d)
	{
		// If still NULL, then something went wrong
		_Error( DeviceErr_NODIRECT3D);
		return FALSE;
	}

	_enum.Enumerate(this);//enumerate all the device combo

	_pRS=pRS;


	return TRUE;
}

//if and leak found,return FALSE,otherwise return TRUE
BOOL CDeviceObject::UnInit()
{
	BOOL bLeak=FALSE;


	Clean();
	_enum.Reset();


	_pRS=NULL;

	if(_d3d)
	{
		if (0!=_d3d->Release())
			bLeak=FALSE;
	}
	return !bLeak;
}

void CDeviceObject::Clean()
{
	if( _device!= NULL )
	{
		_device->EvictManagedResources();

		// Call the app's device lost callback
		_OnLost();

		// Call the app's device destroyed callback
		_OnDestroyed();

		// Release the D3D device and in debug configs, displays a message box if there 
		// are unrelease objects.
		int v;
		if( (v=_device->Release()) > 0 )  
		{
			_Error( DeviceErr_NONZEROREFCOUNT );
		}
		_device=NULL;
		ZeroMemory( &_caps, sizeof(_caps) );
		ZeroMemory( &_devicesetting, sizeof(_devicesetting) );
	}
}


//return XDirect3DDevice ptr
void *CDeviceObject::GetDevice()
{
	return (void *)_device;
}


BOOL CDeviceObject::ResetConfig(DeviceConfig &config)
{
	DeviceConfig old(NULL);
	old=_config;
	_config=config;

	_SetWindow(config.m_hWnd);
	
	DeviceChooser chooser;
	_OnChoose(&chooser);
	DeviceSettings devicesetting;
	if (!chooser.Choose(this,&_enum,devicesetting))
	{
		_config=old;
		_Error(DeviceErr_RESETTINGDEVICE);
		return FALSE;
	}

	return _ChangeDevice( &devicesetting);
}

BOOL CDeviceObject::TestConfig(DeviceConfig &config)
{
	DeviceConfig old(NULL);
	old=_config;
	_config=config;
	DeviceChooser chooser;
	_OnChoose(&chooser);

	DeviceSettings devicesetting;
	BOOL bOK=chooser.Choose(this,&_enum,devicesetting);

	_config=old;

	return bOK;
}

BOOL CDeviceObject::GetConfig(DeviceConfig &config)
{
	config=_config;
	return TRUE;
}

BOOL CDeviceObject::Restore()
{
	if (!_device)
		return FALSE;
	if (!_bLost)
		return TRUE;
	// Test the cooperative level to see if it's okay to render
	HRESULT hr;
	if( FAILED( hr = _device->TestCooperativeLevel() ) )
	{
		if( D3DERR_DEVICELOST == hr )
		{
			// The device has been lost but cannot be reset at this time.  
			// So wait until it can be reset.
			return FALSE;
		}

		// If we are windowed, read the desktop format and 
		// ensure that the Direct3D device is using the same format 
		// since the user could have changed the desktop bitdepth 
		if( _IsWindowed() )
		{
			D3DDISPLAYMODE adapterDesktopDisplayMode;
			_d3d->GetAdapterDisplayMode(_devicesetting.AdapterOrdinal, &adapterDesktopDisplayMode );
			if( _devicesetting.AdapterFormat != adapterDesktopDisplayMode.Format )
			{
				//Need recreate the device
				DeviceChooser chooser;

				_OnChoose(&chooser);

				DeviceSettings devicesetting;
				if (!chooser.Choose(this,&_enum,devicesetting))
				{
					_Error(DeviceErr_NOCOMPATIBLEDEVICES);
					return FALSE;
				}

				// Change to a Direct3D device created from the new device settings.  
				// If there is an existing device, then either reset or recreated the scene
				hr = _ChangeDevice( &devicesetting);
				if( FAILED(hr) )
					return FALSE;

				_bLost=FALSE;
				return TRUE;
			}
		}

		// Try to reset the device
		if( FAILED( hr = _Reset() ) )
		{
			if( D3DERR_DEVICELOST == hr )
			{
				// The device was lost again, so continue waiting until it can be reset.
				return FALSE;
			}
			else
			{
				_Error( hr );
				return FALSE;
			}
		}
		_bLost=FALSE;
	}

	_bLost=FALSE;
	return TRUE;
}

BOOL CDeviceObject::Present(i_math::recti *rcDest,i_math::recti *rcSrc,HWND hwndOverride)
{
	if (!_device)
		return FALSE;
	if (_bLost)
	{
		Restore();
		return FALSE;
	}
	HRESULT hr;
	if (rcDest)
	{
		if (!rcDest->isValid())
			return TRUE;
	}
	if (rcSrc)
	{
		if (!rcSrc->isValid())
			return TRUE;
	}
	hr = _device->Present( (RECT*)rcDest,(RECT*)rcSrc, hwndOverride,NULL);
	if( FAILED(hr) )
	{
		if( D3DERR_DEVICELOST == hr )
			_bLost=true;
		return FALSE;
	}

	return TRUE;
}

BOOL CDeviceObject::IsLost()
{
	if ((_device)&&(_bLost))
		return TRUE;

	return FALSE;
}

void CDeviceObject::_SetWindow( HWND hWnd)
{
	_hwnd=hWnd;
}

BOOL CDeviceObject::_ChangeDevice( DeviceSettings* pNewDeviceSettings)
{
	HRESULT hr;
	DeviceSettings oldsetting= _devicesetting;
	DeviceSettings newsetting=*pNewDeviceSettings;

	_devicesetting=*pNewDeviceSettings;


	// If AdapterOrdinal and DeviceType are the same, we can just do a Reset().
	// If they've changed, we need to do a complete device tear down/rebuild.
	// Also only allow a reset if pd3dDevice is the same as the current device 
	if(oldsetting.AdapterOrdinal == newsetting.AdapterOrdinal &&
		oldsetting.DeviceType == newsetting.DeviceType &&
		oldsetting.BehaviorFlags == newsetting.BehaviorFlags )
	{
		// Reset the Direct3D device 
		hr = _Reset();
		if( FAILED(hr) )
		{
			if( D3DERR_DEVICELOST == hr )
			{
				_bLost=TRUE;
				return TRUE;
			}
			else
			{
				_Error( hr );
				Clean();
				return FALSE;
			}
		}
	}
	else
	{
		// The adapter and device type don't match so 
		// cleanup and create the 3D device again
		Clean();

		XDirect3DDevice* pd3dDevice = NULL;

		// Set default settings
		UINT AdapterToUse=newsetting.AdapterOrdinal;
		D3DDEVTYPE DeviceType=newsetting.DeviceType;
#ifdef _NVPerfHUD
		// Look for 'NVIDIA NVPerfHUD' adapter
		// If it is present, override default settings
		for (UINT Adapter=0;Adapter<_d3d->GetAdapterCount();Adapter++)
		{
			D3DADAPTER_IDENTIFIER9 Identifier;
			HRESULT Res;
			Res = _d3d->GetAdapterIdentifier(Adapter,0,&Identifier);
			if (strcmp(Identifier.Description,"NVIDIA PerfHUD") == 0)
			{
				AdapterToUse=Adapter;
				DeviceType=D3DDEVTYPE_REF;
				break;
			}
		}
#endif
		hr = _d3d->CreateDevice( AdapterToUse, DeviceType, 
			_hwnd, newsetting.BehaviorFlags,&newsetting.pp, &_device);
		if( FAILED(hr) )
		{
			_Error( DeviceErr_CREATINGDEVICE);
			return FALSE;
		}

		hr=_OnCreated(_device);
		if( FAILED(hr) )  
		{
			// Cleanup upon failure
			Clean();
			return FALSE;
		}
		else
		{
			hr=_OnReset(_device);
			if( FAILED(hr) )  
			{
				// Cleanup upon failure
				Clean();
				return FALSE;
			}
		}
	}

	return TRUE;
}

BOOL CDeviceObject::_IsWindowed()                               
{ 
	return (_devicesetting.pp.Windowed != 0); 
}


HRESULT CDeviceObject::_Reset()
{
	HRESULT hr;

	_OnLost();

	// Reset the device
	hr = _device->Reset( &_devicesetting.pp );
	if( FAILED(hr) )  
	{
		if( hr == D3DERR_DEVICELOST )
			return D3DERR_DEVICELOST; // Reset could legitimately fail if the device is lost
		else
			return DeviceErr_RESETTINGDEVICE;
	}


	// Initialize the app's device-dependent objects
	hr = _OnReset(_device);
	if( FAILED(hr) )
	{
		LogFile::Prompt("_OnReset failed!");

		_OnLost();
	}

	return hr;
}


//Enumeration callback
BOOL CDeviceObject::_OnGetDisplayModeSizeLimit(int &wMin,int &hMin,int &wMax,int &hMax)//return whether the limit is overriden
{
	return FALSE;
}
BOOL CDeviceObject::_OnGetDisplayModeRefreshLimit(int &refreshMin,int &refreshMax)//return whether the limit is overriden
{
	return FALSE;
}
BOOL CDeviceObject::_OnGetPostPixelShaderBlendingRequirement(BOOL &bRequire)//return whether the requirement is overiden
{
	bRequire=_config.m_bPostPSBlending;
	return TRUE;
}
BOOL CDeviceObject::_OnGetDeviceAcceptable(D3DCAPS9 *pCaps,D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat, BOOL bWindowed,BOOL &bAcceptable)
{
	return FALSE;
}
BOOL CDeviceObject::_OnGetMultisampleQualityMax(UINT &nMultisampleQualityMax)
{
	return FALSE;
}
//

//Choose callback
void CDeviceObject::_OnChoose(DeviceChooser *pChooser)
{
	pChooser->SetRequest_VertexProcessingType(D3DCREATE_MIXED_VERTEXPROCESSING);
	pChooser->SetRequest_SwapEffect(D3DSWAPEFFECT_DISCARD);

	if (_config.m_bRefDevice)
		pChooser->SetRequest_DeviceType(D3DDEVTYPE_REF,TRUE);

	pChooser->SetRequest_Resolution(_config.m_Width,_config.m_Height);

	if (_config.m_bFullScreen)
	{
		pChooser->SetRequest_FullScreen();

		D3DFORMAT fmt;
		fmt=D3DFMT_R5G6B5;
		if (_config.m_bpp==32)
			fmt=D3DFMT_X8R8G8B8;
		pChooser->SetRequest_AdapterFormat(fmt,TRUE);
		pChooser->SetRequest_BackBufferFormat(fmt);
	}

	pChooser->SetRequest_DepthStencilFormat(D3DFMT_D24S8,TRUE);

	if (_config.m_bCopySwap)
	{
		pChooser->SetRequest_BackBufferCount(1);
		pChooser->SetRequest_SwapEffect(D3DSWAPEFFECT_COPY,TRUE);
	}
	else
		pChooser->SetRequest_BackBufferCount(1);

//	 	pChooser->SetRequest_PresentInterval(D3DPRESENT_INTERVAL_FOUR);
//		pChooser->SetRequest_PresentFlags(D3DPRESENTFLAG_LOCKABLE_BACKBUFFER);
//		pChooser->SetRequest_MultiSample(D3DMULTISAMPLE_NONMASKABLE,8);

}

//Device lost/reset/created/destroyed
void CDeviceObject::_OnLost()
{
	SAFE_RELEASE(_event);
	for (int i=0;i<_stackRT.size();i++)
	{
		for (int j=0;j<MAX_RT_COUNT;j++)
			SAFE_RELEASE(_stackRT[i].surfs[j]);
	}
	_stackRT.clear();
	_bMRT=FALSE;
	for (int i=0;i<_stackDSBuffer.size();i++)
		SAFE_RELEASE(_stackDSBuffer[i]);
	_stackDSBuffer.clear();
	_w=_h=0;

	ClearVBBind();

	std::map<_FVFExKey,XDirect3DVertexDeclaration *>::iterator it;
	for (it=_mapDecl.begin();it!=_mapDecl.end();it++)
		SAFE_RELEASE((*it).second);
	_mapDecl.clear();

	((CRenderSystem*)_pRS)->OnDeviceLost();

}

void GetSurfSize(XDirect3DSurface *surf,DWORD &w,DWORD &h)
{
	if (!surf)
		w=h=0;
	else
	{
		D3DSURFACE_DESC desc;
		surf->GetDesc(&desc);
		w=desc.Width;
		h=desc.Height;
	}
}

HRESULT CDeviceObject::_OnReset(XDirect3DDevice *pDevice)
{
	_device->GetDeviceCaps(&_caps);

	XDirect3DSurface *p=NULL;
	HRESULT hr=_device->GetRenderTarget(0,&p);
	if (hr==D3D_OK)
	{
		_MRT mrt;
		mrt.surfs[0]=p;
		_stackRT.push_back(mrt);
		GetSurfSize(p,_w,_h);
		_bMRT=FALSE;
	}
	hr=_device->GetDepthStencilSurface(&p);
	if (hr==D3D_OK)
		_stackDSBuffer.push_back(p);

	((CRenderSystem*)_pRS)->OnDeviceReset();


	if (D3D_OK!=_device->CreateQuery(D3DQUERYTYPE_EVENT, &_event))
		_event=NULL;

	_state.SetInvalid();

	//Fill some cap
	if (TRUE)
	{
		_cap.bDSTex_D24S8=SupportDSTexture(D3DFMT_D24S8);
		_cap.bDSTex_D16=SupportDSTexture(D3DFMT_D16);
		_cap.bNullRT=SupportNullRT();
	}

	return S_OK;
}
void CDeviceObject::_OnDestroyed()
{
	((CRenderSystem*)_pRS)->OnDeviceDestroy();
}
HRESULT CDeviceObject::_OnCreated(XDirect3DDevice *pDevice)
{
	((CRenderSystem*)_pRS)->OnDeviceCreate();


	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
//State management

BOOL CDeviceObject::PushRenderTarget(SurfHandle *rts,DWORD count)
{
	_MRT mrt;
	if ((count>MAX_RT_COUNT)||(count<=0))
		return FALSE;

	for (int i=0;i<count;i++)
	{
		SurfHandle &rt=rts[i];
		XDirect3DSurface *&surf=mrt.surfs[i];
		if (rt.IsEmpty())
			return FALSE;

		if (rt.tex)
		{
			if (rt.tex->Touch()!=A_Ok)
				return FALSE;
			surf=(XDirect3DSurface *)rt.tex->GetSurf(0,rt.iFace);
			if (!surf)
				return FALSE;
		}
		else
		{
			if (A_Ok!=rt.surf->Touch())
				return FALSE;
			surf=(XDirect3DSurface *)rt.surf->GetSurf();
			if (!surf)
				return FALSE;
		}

	}

	_stackRT.push_back(mrt);
	_bMRT=(count>1);


	for (int i=0;i<MAX_RT_COUNT;i++)
	{
		if (D3D_OK!=_device->SetRenderTarget(i,mrt.surfs[i]))
		{
			g_logDevice.Prompt("Failed to SetRenderTarget() when PushRenderTarget(..)");
			PopRenderTarget();
			return FALSE;
		}
	}


	GetSurfSize(mrt.surfs[0],_w,_h);

	return TRUE;
}


void CDeviceObject::PopRenderTarget()
{
	if (_stackRT.size()<=1)
		return;

	_MRT *mrt=&_stackRT[_stackRT.size()-1];
	for (int i=0;i<MAX_RT_COUNT;i++)
		SAFE_RELEASE(mrt->surfs[i]);
	_stackRT.pop_back();


	mrt=&_stackRT[_stackRT.size()-1];
	for (int i=0;i<MAX_RT_COUNT;i++)
		_device->SetRenderTarget(i,mrt->surfs[i]);
	GetSurfSize(mrt->surfs[0],_w,_h);
	_bMRT=(mrt->surfs[1]!=NULL);
}

BOOL CDeviceObject::PushDSBuffer(SurfHandle &ds)
{
	XDirect3DSurface *surf=NULL;
	if (ds.IsEmpty())
		return FALSE;
	if (ds.surf!=SurfHandle_DisableDS.surf)
	{
		if (ds.surf)
		{
			if (A_Ok!=ds.surf->Touch())
				return FALSE;
			surf=(XDirect3DSurface *)ds.surf->GetSurf();
			if (surf)
				surf->AddRef();
		}
		else
		{
			if (A_Ok!=ds.tex->Touch())
				return FALSE;
			surf=(XDirect3DSurface *)ds.tex->GetSurf(0,0);
		}
		if (!surf)
			return FALSE;

		if (D3D_OK!=_device->SetDepthStencilSurface(surf))
			return FALSE;

	}
	else
	{
		if (D3D_OK!=_device->SetDepthStencilSurface(NULL))
			return FALSE;
	}

	_stackDSBuffer.push_back(surf);

	return TRUE;
}

void CDeviceObject::PopDSBuffer()
{
	if (_stackDSBuffer.size()<=1)
		return;

	SAFE_RELEASE(_stackDSBuffer[_stackDSBuffer.size()-1]);
	_stackDSBuffer.pop_back();

	XDirect3DSurface *surf=_stackDSBuffer[_stackDSBuffer.size()-1];
	_device->SetDepthStencilSurface(surf);
}


BOOL CDeviceObject::BeginScene()
{
	if (!_device)
		return FALSE;
	if (D3D_OK!=_device->BeginScene())
		return FALSE;
	return TRUE;
}
BOOL CDeviceObject::EndScene()
{
	if (!_device)
		return FALSE;
	_device->EndScene();
	return TRUE;
}

BOOL CDeviceObject::SetViewport(ViewportInfo &v)
{
	if (D3D_OK!=_device->SetViewport((D3DVIEWPORT9*)&v))
		return FALSE;
	return TRUE;
}





//////////////////////////////////////
//Helper functions
BOOL CDeviceObject::ColorFill(DWORD color,DWORD iBackBuffer,RECT *pRect)
{
	XDirect3DSurface * pRT;

	if (_device)
	{
		HRESULT hr;
		hr=_device->GetRenderTarget(0,&pRT);

		if (hr!=S_OK)
			return FALSE;

		_device->ColorFill(pRT,pRect,(D3DCOLOR)color);
		pRT->Release();

		return TRUE;
	}
	return FALSE;
}

BOOL CDeviceObject::ClearZBuffer(RECT *pRect)
{
	return ClearBuffer(ClearBuffer_Depth,pRect);
}

BOOL CDeviceObject::ClearBuffer(ClearBufferFlag flag,RECT *pRect,D3DCOLOR col,float z,DWORD s)
{
	D3DVIEWPORT9 vi,viBack;
	_device->GetViewport(&viBack);

	RECT rcFull;
	RECT_SET(rcFull,0,0,_w,_h);
	if (!pRect)
		pRect=&rcFull;
	vi.Width=RECT_WIDTH(*pRect);
	vi.Height=RECT_HEIGHT(*pRect);
	vi.X=pRect->left;
	vi.Y=pRect->top;
	vi.MinZ=0;
	vi.MaxZ=1;

	BOOL bRet=TRUE;
	_device->SetViewport(&vi);//XDirect3DDevice::Clear() is clipped by the viewport,so we should set a big enough viewport

	if (D3D_OK!=_device->Clear(1, (D3DRECT*)pRect,flag,col, z,s))
	{
		if (D3D_OK!=_device->Clear(1, (D3DRECT*)pRect,flag&(~ClearBuffer_Stencil),col, z,s))//maybe the depth-stencil buffer doesnot contain stencil bits,ignore the bit and try again
			bRet=FALSE;
	}

	_device->SetViewport(&viBack);//restore
	return bRet;
}



//Retrieve width/height from the window handle
DWORD CDeviceObject::Width()
{
	return _w;
}
DWORD CDeviceObject::Height()
{
	return _h;
}


D3DCOLORVALUE CDeviceObject::D3DCOLORVALUEFromD3DCOLOR(D3DCOLOR color)
{
	D3DCOLORVALUE v;
	v.a=((float)((color&0xff000000)>>24))/255.0f;
	v.r=((float)((color&0x00ff0000)>>16))/255.0f;
	v.g=((float)((color&0x0000ff00)>>8))/255.0f;
	v.b=((float)((color&0x000000ff)))/255.0f;
	return v;
}



//HW,SW,or Mixed
DWORD CDeviceObject::GetVertexProcessFlag()
{
	DWORD v;
	v=_devicesetting.BehaviorFlags&(D3DCREATE_HARDWARE_VERTEXPROCESSING|
		D3DCREATE_SOFTWARE_VERTEXPROCESSING|
		D3DCREATE_MIXED_VERTEXPROCESSING);

	assert(v!=0);
	return v;
}


BOOL CDeviceObject::SupportVertexShader(DWORD verMajor,DWORD verMinor)
{
	if (_caps.VertexShaderVersion < D3DVS_VERSION(verMajor,verMinor))
		return FALSE;
	return TRUE;

}
BOOL CDeviceObject::SupportPixelShader(DWORD verMajor,DWORD verMinor)
{
	if (_caps.PixelShaderVersion < D3DPS_VERSION(verMajor,verMinor))
		return FALSE;
	return TRUE;

}

DWORD CDeviceObject::MaxIndexValue()
{
	DWORD v;
	v=0xffff;
	if (_caps.MaxVertexIndex<v)
		v=_caps.MaxVertexIndex;
	return v;
}

DWORD CDeviceObject::MaxTextureCoord()
{
	DWORD v;
	v=0xffffffff;
	if (_caps.MaxSimultaneousTextures<v)
		v=_caps.MaxSimultaneousTextures;

	if (SupportPixelShader(2,0))
	{
		if (v>16)
			v=16;
	}
	else
	{
		if (SupportPixelShader(1,0))
		{
			if (v>6)
				v=6;
		}
	}

	return v;

}



BOOL CDeviceObject::CheckDepthStencilMatch(D3DFORMAT fmt,D3DFORMAT fmtDS)
{
	if (!_d3d)
		return FALSE;
	HRESULT hr;
	hr=_d3d->CheckDepthStencilMatch(_devicesetting.AdapterOrdinal,
		_devicesetting.DeviceType,_devicesetting.AdapterFormat,fmt,fmtDS);

	if (hr==D3D_OK)
		return TRUE;

	return FALSE;
}

BOOL CDeviceObject::SupportDSTexture(D3DFORMAT fmtDS)
{
	if (!_d3d)
		return FALSE;
	HRESULT hr;
	hr=_d3d->CheckDeviceFormat(_devicesetting.AdapterOrdinal,
		_devicesetting.DeviceType,_devicesetting.AdapterFormat,D3DUSAGE_DEPTHSTENCIL,D3DRTYPE_TEXTURE,fmtDS);

	if (hr==D3D_OK)
		return TRUE;

	return FALSE;

}

BOOL CDeviceObject::SupportNullRT()
{
	if (!_d3d)
		return FALSE;
	HRESULT hr;
	hr=_d3d->CheckDeviceFormat(_devicesetting.AdapterOrdinal,
		_devicesetting.DeviceType,_devicesetting.AdapterFormat,D3DUSAGE_RENDERTARGET,D3DRTYPE_SURFACE,((D3DFORMAT)MAKEFOURCC('N','U','L','L')));

	if (hr==D3D_OK)
		return TRUE;

	return FALSE;

}


BOOL CDeviceObject::FlushCommand()
{
	if (!_event)
		return FALSE;
#ifdef MULTITHREAD_D3D
	_event->Flush();
#else
	_event->Issue(D3DISSUE_END);
	while(S_FALSE == _event->GetData( NULL, 0, D3DGETDATA_FLUSH ));
#endif
	return TRUE;
}

void CDeviceObject::ClearVBBind()
{
	BindVB(NULL,0,NULL,NULL);
}



BOOL CDeviceObject::BindVB(VBInfo** vbs,DWORD nVB,IBInfo*ib,VBBindArg *arg)
{
	if (!arg)
		arg=&_bindarg;
	if (_bLost)
		return FALSE;
	assert(nVB<=MAX_BIND_VB);

	if (nVB==0)
	{
		_device->SetIndices(0);
		_ibCur=NULL;

		_CleanStreamSource(0);

		_fvfCur.Zero();
		_drawarg._bValid=FALSE;
		return TRUE;
	}


	_drawarg._bValid=FALSE;

	if (TRUE)
	{
		//设vertex declaration
		if (TRUE)
		{
			_FVFExKey k;
			k.fvfDraw=arg->fvfDraw;
			if (TRUE)
			{
				int i;
				for (i=0;i<nVB;i++)
					k.fvfs[i]=vbs[i]->_fvf;
				for (;i<MAX_BIND_VB;i++)
					k.fvfs[i]=0;
			}

			if (!(k==_fvfCur))
			{
				XDirect3DVertexDeclaration *pDecl;
				std::map<_FVFExKey,XDirect3DVertexDeclaration *>::iterator it;
				it=_mapDecl.find(k);//CreateVertexDeclaration() is a very slow function,so we should cache it
				if (it==_mapDecl.end())
				{
					D3DVERTEXELEMENT9 aVertexElements[256];//Big enough

					DWORD count=0;
					for (int i=0;i<nVB;i++)
					{
						VBInfo *vb=vbs[i];
						count+=fvfToD3DVERTEXELEMENT9(vb->_fvf,
							arg->fvfDraw==0?vb->_fvf:arg->fvfDraw,
							(D3DVERTEXELEMENT9_x*)&aVertexElements[count],i);
					}
					D3DVERTEXELEMENT9 temp[1]=D3DDECL_END();
					aVertexElements[count]=temp[0]; 

					if (D3D_OK!=_device->CreateVertexDeclaration((D3DVERTEXELEMENT9*)aVertexElements,&pDecl))
						return FALSE;

					_mapDecl[k]=pDecl;
				}
				else
					pDecl=(*it).second;

				_device->SetFVF(0);
				if (D3D_OK!=_device->SetVertexDeclaration(pDecl))
				{
					_fvfCur.Zero();//Not sure of the internal state when failing
					return FALSE;
				}
				_fvfCur=k;
			}
		}

		//设stream
		for (int i=0;i<nVB;i++)
		{
			VBInfo *vb=vbs[i];
			assert(vb);
			vb->Touch();
			_device->SetStreamSource(i,(XD3DVertexBuffer*)vb->_d3dhandle,0,vb->_fvfsize);
		}
	}


	if (ib)
	{
		if (A_Ok!=ib->Touch())
			return FALSE;
		if (!ib->_d3dhandle)
			return FALSE;

		if (D3D_OK!=_device->SetIndices((XD3DIndexBuffer*)ib->_d3dhandle))
			return FALSE;
		_drawarg._bIndexed=TRUE;
		_drawarg._nIndice=ib->_count;
	}
	else
	{
		_device->SetIndices(NULL);
		_drawarg._bIndexed=FALSE;
	}

	if (arg->vcount==0)
		_drawarg._nVertice=vbs[0]->_count-arg->vbase;
	else
		_drawarg._nVertice=arg->vcount;
	_drawarg._iStreamOff=arg->iFrame*vbs[0]->_count+arg->vbase;
	_drawarg._primstart=arg->primstart;
	_drawarg._primcount=arg->primcount;
	_drawarg._ds=arg->fillmode;
	_drawarg._dpt=arg->dpt;

	_drawarg._bValid=TRUE;//ok

	for (int i=0;i<nVB;i++)
		_vbCur[i]=vbs[i];
	_CleanStreamSource(nVB);//清空后面的

	_ibCur=ib;

	return TRUE;
}

BOOL CDeviceObject::SetVBInstanceCount(DWORD count)
{
	BOOL bOk=TRUE;

#ifndef MULTITHREAD_D3D

	if (count>0)
	{
		if (D3D_OK!=_device->SetStreamSourceFreq(0,D3DSTREAMSOURCE_INDEXEDDATA|count))
			bOk=FALSE;
		if (D3D_OK!=_device->SetStreamSourceFreq(1,D3DSTREAMSOURCE_INSTANCEDATA|1))
			bOk=FALSE;
	}
	else
	{
		_device->SetStreamSourceFreq(0,1);
		_device->SetStreamSourceFreq(1,1);
	}
#endif

	return bOk;
}


BOOL CDeviceObject::DrawPrim()
{
	if (_bLost)
		return FALSE;

	if (_vbCur==VBHANDLE_NULL)
		return FALSE;

	if (!_drawarg._bValid)
		return FALSE;

	if ((_drawarg._dpt==(DWORD)D3DPT_POINTLIST)&&(_drawarg._bIndexed))
		return FALSE;//not supported

	_device->SetRenderState(D3DRS_FILLMODE,_drawarg._ds);
	DWORD nPrim,iStart;
	if (_drawarg._bIndexed)
		nPrim=_drawarg._nIndice;
	else
		nPrim=_drawarg._nVertice;
	if ((_drawarg._primstart==-1)&&(_drawarg._primcount==-1))
		iStart=0;
	else
		iStart=_drawarg._primstart;
	switch((D3DPRIMITIVETYPE)_drawarg._dpt)
	{
		case D3DPT_POINTLIST:
			break;
		case D3DPT_LINELIST:
			nPrim/=2;iStart*=2;break;
		case D3DPT_LINESTRIP:
			nPrim--;break;
		case D3DPT_TRIANGLELIST:
			nPrim/=3;iStart*=3;break;
		case D3DPT_TRIANGLESTRIP:
		case D3DPT_TRIANGLEFAN:
			nPrim-=2;break;
		default:
			return FALSE;
	}
	if ((_drawarg._primcount!=-1)||(_drawarg._primstart!=-1))
	{
		if (_drawarg._primstart+_drawarg._primcount>nPrim)//exceeds range
			return FALSE;
		nPrim=_drawarg._primcount;
	}
	HRESULT hr;
	if (_drawarg._bIndexed)
		hr=_device->DrawIndexedPrimitive((D3DPRIMITIVETYPE)_drawarg._dpt,
				_drawarg._iStreamOff,//base
				0,_drawarg._nVertice,//vertex start/count
				iStart,nPrim);//index start/prim count
	else
		hr=_device->DrawPrimitive((D3DPRIMITIVETYPE)_drawarg._dpt,
				_drawarg._iStreamOff+iStart,	nPrim);//vertex start/prim count

	if (hr==D3D_OK)
		return TRUE;
	return FALSE;
}


bool operator<(const CDeviceObject::_FVFExKey&v1, const CDeviceObject::_FVFExKey&v2)
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

bool operator==(const CDeviceObject::_FVFExKey&v1, const CDeviceObject::_FVFExKey&v2)
{
	return memcmp(&v1,&v2,sizeof(v1))==0;
}



void CDeviceObject::_TestDraw_SetRenderState(DWORD rs,DWORD v)
{
	static DWORD t=0;

	if (t==v)
		return;

	_device->SetRenderState((D3DRENDERSTATETYPE)rs,v);
	t=v;
}


BOOL CDeviceObject::TestDraw()
{

	XDirect3DSurface *surf[4];
	memset(surf,0,sizeof(surf));
	for (int i=0;i<4;i++)
		_device->GetRenderTarget(i,&surf[i]);

	for (int i=1;i<4;i++)
		_device->SetRenderTarget(i,NULL);

	for (int i=0;i<4;i++)
		_device->GetRenderTarget(i,&surf[i]);

	ProfilerStart(SetRenderState_Switch);
	for (int i=0;i<10000;i++)
	{
		_TestDraw_SetRenderState(D3DRS_STENCILENABLE,TRUE);
		_TestDraw_SetRenderState(D3DRS_STENCILENABLE,FALSE);
		_TestDraw_SetRenderState(D3DRS_STENCILENABLE,TRUE);
		_TestDraw_SetRenderState(D3DRS_STENCILENABLE,TRUE);
	}
	ProfilerEnd();

	ProfilerStart(SetRenderState);
	for (int i=0;i<10000;i++)
	{
		_device->SetRenderState(D3DRS_STENCILENABLE,TRUE);
		_device->SetRenderState(D3DRS_STENCILENABLE,TRUE);
		_device->SetRenderState(D3DRS_STENCILENABLE,TRUE);
		_device->SetRenderState(D3DRS_STENCILENABLE,TRUE);
	}
	ProfilerEnd();

	return TRUE;
}


void CDeviceObject::SetAlphaTest(AlphaTestMode mode,DWORD ref)
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
	switch(mode)
	{
	case AlphaTest_Disable:
		_device->SetRenderState(D3DRS_ALPHATESTENABLE,FALSE);
		break;
	case AlphaTest_Standard:
		{
			if (ref<=0)
				_device->SetRenderState(D3DRS_ALPHATESTENABLE,FALSE);
			else
			{
				_device->SetRenderState(D3DRS_ALPHATESTENABLE,TRUE);
				_device->SetRenderState(D3DRS_ALPHAREF,ref);
				_device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
			}
			break;
		}
	}
}


void CDeviceObject::SetBlend(ShaderBlendMode mode)
{
#ifdef _DEBUG
// 	if (_bMRT)
// 	{
// 		if (mode!=Blend_Opaque)
// 		{
// 			mode=Blend_Opaque;
// 			LOG_DUMP("DeviceObject",Log_Error,"在MRT的情况下使用了非Opaque的Blend模式!");
// 		}
// 	}
#endif

	if (_state.modeBlend==mode)
		return;


	_state.modeBlend=mode;
	if (mode==Blend_Opaque)
	{
		_device->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);

		_device->SetRenderState(D3DRS_BLENDOP,D3DBLENDOP_ADD);
		_device->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_ONE);
		_device->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ZERO);

		return;
	}
	if (mode==Blend_AlphaBlend)
	{
		_device->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);

		_device->SetRenderState(D3DRS_BLENDOP,D3DBLENDOP_ADD);
		_device->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
		_device->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
		return;
	}
	if (mode==Blend_Additive)
	{
		_device->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);

		_device->SetRenderState(D3DRS_BLENDOP,D3DBLENDOP_ADD);
		_device->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
		_device->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ONE);
		return;
	}

	if (mode==Blend_Modulate)
	{
		_device->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);

		_device->SetRenderState(D3DRS_BLENDOP,D3DBLENDOP_ADD);
		_device->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_ZERO);
		_device->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_SRCCOLOR);
		return;
	}

	if (mode==Blend_Enlighten)
	{
		_device->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);

		_device->SetRenderState(D3DRS_BLENDOP,D3DBLENDOP_ADD);
		_device->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCCOLOR);
		_device->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_DESTALPHA);
		return;
	}

	if (mode==Blend_Inverse)
	{
		_device->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);

		_device->SetRenderState(D3DRS_BLENDOP,D3DBLENDOP_REVSUBTRACT);
		_device->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
		_device->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
		return;
	}

	//XXXXX:more blend mode

}

void CDeviceObject::SetStencilOp(StencilMode mode,WORD ref,WORD mask)
{
	if ((_state.modeStencil==mode)&&(_state.refStencil==ref)&&(_state.maskStencil==mask))
		return;
	_state.modeStencil=mode;
	_state.refStencil=ref;
	_state.maskStencil=mask;

	switch(mode)
	{
	case Sten_Disable:
		_device->SetRenderState(D3DRS_STENCILENABLE,FALSE);
		_device->SetRenderState(D3DRS_COLORWRITEENABLE,0xf);
		break;
	case Sten_Write:
	case Sten_WriteNoColor:
		_device->SetRenderState(D3DRS_STENCILENABLE,TRUE);
		_device->SetRenderState(D3DRS_COLORWRITEENABLE,mode==Sten_Write?0xf:0x0);
		_device->SetRenderState(D3DRS_STENCILFUNC,D3DCMP_ALWAYS);
		_device->SetRenderState(D3DRS_STENCILMASK,(DWORD)0xffffffff);
		_device->SetRenderState(D3DRS_STENCILWRITEMASK,(DWORD)0xffffffff);
		_device->SetRenderState(D3DRS_STENCILREF,ref);
		_device->SetRenderState(D3DRS_STENCILPASS,D3DSTENCILOP_REPLACE);
		break;
	case Sten_Filter:
	case Sten_InvFilter:
		_device->SetRenderState(D3DRS_STENCILENABLE,TRUE);
		_device->SetRenderState(D3DRS_COLORWRITEENABLE,0xf);
		_device->SetRenderState(D3DRS_STENCILFUNC,mode==Sten_Filter?D3DCMP_EQUAL:D3DCMP_NOTEQUAL);
		_device->SetRenderState(D3DRS_STENCILMASK,(DWORD)0xffffffff);
		_device->SetRenderState(D3DRS_STENCILWRITEMASK,(DWORD)0xffffffff);
		_device->SetRenderState(D3DRS_STENCILREF,ref);
		_device->SetRenderState(D3DRS_STENCILPASS,D3DSTENCILOP_KEEP);
		break;

	case Sten_Inc:
	case Sten_IncNoColor:
		_device->SetRenderState(D3DRS_STENCILENABLE,TRUE);
		_device->SetRenderState(D3DRS_COLORWRITEENABLE,mode==Sten_Inc?0xf:0x0);
		_device->SetRenderState(D3DRS_STENCILFUNC,D3DCMP_ALWAYS);
		_device->SetRenderState(D3DRS_STENCILMASK,(DWORD)0xffff);
		_device->SetRenderState(D3DRS_STENCILWRITEMASK,(DWORD)0xffff);
		_device->SetRenderState(D3DRS_STENCILREF,0xffffffff);
		_device->SetRenderState(D3DRS_STENCILPASS,D3DSTENCILOP_INCR);
		break;

	case Sten_FilterWrite:
	case Sten_InvFilterWrite:
		_device->SetRenderState(D3DRS_STENCILENABLE,TRUE);
		_device->SetRenderState(D3DRS_COLORWRITEENABLE,0xf);
		_device->SetRenderState(D3DRS_STENCILFUNC,mode==Sten_FilterWrite?D3DCMP_EQUAL:D3DCMP_NOTEQUAL);
		_device->SetRenderState(D3DRS_STENCILMASK,(DWORD)0xffffffff);
		_device->SetRenderState(D3DRS_STENCILWRITEMASK,(DWORD)0xffffffff);
		_device->SetRenderState(D3DRS_STENCILREF,ref);
		_device->SetRenderState(D3DRS_STENCILPASS,D3DSTENCILOP_REPLACE);
		break;

	case Sten_FilterInc:
		_device->SetRenderState(D3DRS_STENCILENABLE,TRUE);
		_device->SetRenderState(D3DRS_COLORWRITEENABLE,0xf);
		_device->SetRenderState(D3DRS_STENCILFUNC,D3DCMP_EQUAL);
		_device->SetRenderState(D3DRS_STENCILMASK,(DWORD)0xffffffff);
		_device->SetRenderState(D3DRS_STENCILWRITEMASK,(DWORD)0xffffffff);
		_device->SetRenderState(D3DRS_STENCILREF,ref);
		_device->SetRenderState(D3DRS_STENCILPASS,D3DSTENCILOP_INCR);
		break;


		//XXXXX:more stencil mode
	}
}

void CDeviceObject::SetDepthMethod(DepthMode mode)
{
	if (_state.modeDepth==mode)
		return;
	_state.modeDepth=mode;
	switch(mode)
	{
	case Depth_Default:
		_device->SetRenderState(D3DRS_ZENABLE,D3DZB_TRUE);
		_device->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL);
		_device->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
		break;
	case Depth_NoCmp:
		_device->SetRenderState(D3DRS_ZENABLE,D3DZB_TRUE);
		_device->SetRenderState(D3DRS_ZFUNC,D3DCMP_ALWAYS);
		_device->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
		break;
	case Depth_NoWrite:
		_device->SetRenderState(D3DRS_ZENABLE,D3DZB_TRUE);
		_device->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL);
		_device->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
		break;
	case Depth_Disable:
		_device->SetRenderState(D3DRS_ZENABLE,D3DZB_FALSE);
		break;
	case Depth_FartherNoWrite:
		_device->SetRenderState(D3DRS_ZENABLE,D3DZB_TRUE);
		_device->SetRenderState(D3DRS_ZFUNC,D3DCMP_GREATEREQUAL);
		_device->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
		break;
	}
}

void CDeviceObject::SetFacing(FacingMode mode)
{
	if (_state.modeFacing==mode)
		return;
	_state.modeFacing=mode;
	switch (mode)
	{
	case Facing_Front:
		_device->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);
		break;
	case Facing_Back:
		_device->SetRenderState(D3DRS_CULLMODE,D3DCULL_CW);
		break;
	case Facing_Both:
		_device->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
		break;
	}
}
