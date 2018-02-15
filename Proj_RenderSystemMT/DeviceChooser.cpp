/********************************************************************
	created:	2006/06/04
	created:	4:6:2006   20:06
	filename: 	d:\IxEngine\Proj_RenderSystem\DeviceChooser.cpp
	author:		cxi
	
	purpose:	chooser for device(from Microsoft's D3D sample framework)
*********************************************************************/
#include "stdh.h"

#include "DeviceChooser.h"
#include "DeviceObject.h"

#pragma warning(disable:4018)

extern UINT D3DColorChannelBits( D3DFORMAT fmt );
extern UINT D3DAlphaChannelBits( D3DFORMAT fmt );
extern UINT D3DDepthBits( D3DFORMAT fmt );
extern UINT D3DStencilBits( D3DFORMAT fmt );



#define DEFAULT_FULLSCREEN_ADAPTERFORMAT (D3DFMT_X8R8G8B8)

void DeviceChooser::SetRequest_AdapterOrdinal(DWORD ordinalAdapter,BOOL bForce)
{
	m_RequestDeviceSetting.AdapterOrdinal=ordinalAdapter;
	m_ChooseOption.eAdapterOrdinal=Choose_ClosestToInput;
	if (bForce)
		m_ChooseOption.eAdapterOrdinal=Choose_PreserveInput;
}
void DeviceChooser::SetRequest_DeviceType(D3DDEVTYPE typeDevice,BOOL bForce)
{
	m_RequestDeviceSetting.DeviceType=typeDevice;
	m_ChooseOption.eDeviceType=Choose_ClosestToInput;
	if (bForce)
		m_ChooseOption.eDeviceType=Choose_PreserveInput;
}
void DeviceChooser::SetRequest_FullScreen(BOOL bFullScreen)
{
	m_RequestDeviceSetting.pp.Windowed=!bFullScreen;
	m_ChooseOption.eWindowed=Choose_PreserveInput;
}
void DeviceChooser::SetRequest_VertexProcessingType(DWORD typeVertexProcessing,BOOL bForce)
{
	m_RequestDeviceSetting.BehaviorFlags|=typeVertexProcessing;
	m_ChooseOption.eVertexProcessing=Choose_ClosestToInput;
	if (bForce)
		m_ChooseOption.eVertexProcessing=Choose_PreserveInput;
}
void DeviceChooser::SetRequest_Resolution(DWORD w,DWORD h)
{
	m_RequestDeviceSetting.pp.BackBufferWidth=w;
	m_RequestDeviceSetting.pp.BackBufferHeight=h;

	m_ChooseOption.eResolution=Choose_ClosestToInput;
}

void DeviceChooser::SetRequest_AdapterFormat(D3DFORMAT format,BOOL bForce)
{
	m_RequestDeviceSetting.AdapterFormat=format;
	m_ChooseOption.eAdapterFormat=Choose_ClosestToInput;
	if (bForce)
		m_ChooseOption.eAdapterFormat=Choose_PreserveInput;
}

void DeviceChooser::SetRequest_BackBufferFormat(D3DFORMAT format,BOOL bForce)
{
	m_RequestDeviceSetting.pp.BackBufferFormat=format;
	m_ChooseOption.eBackBufferFormat=Choose_ClosestToInput;
	if (bForce)
		m_ChooseOption.eBackBufferFormat=Choose_PreserveInput;
}
void DeviceChooser::SetRequest_BackBufferCount(int n,BOOL bForce)
{
	m_RequestDeviceSetting.pp.BackBufferCount=n;
	m_ChooseOption.eBackBufferCount=Choose_ClosestToInput;
	if (bForce)
		m_ChooseOption.eBackBufferCount=Choose_PreserveInput;
}
void DeviceChooser::SetRequest_MultiSample(D3DMULTISAMPLE_TYPE type,DWORD quality,BOOL bForce)
{
	m_RequestDeviceSetting.pp.MultiSampleType=type;
	m_RequestDeviceSetting.pp.MultiSampleQuality=quality;
	m_ChooseOption.eMultiSample=Choose_ClosestToInput;
	if (bForce)
		m_ChooseOption.eMultiSample=Choose_PreserveInput;
}
void DeviceChooser::SetRequest_SwapEffect(D3DSWAPEFFECT swapeffect,BOOL bForce)
{
	m_RequestDeviceSetting.pp.SwapEffect=swapeffect;
	m_ChooseOption.eSwapEffect=Choose_ClosestToInput;
	if (bForce)
		m_ChooseOption.eSwapEffect=Choose_PreserveInput;
}
void DeviceChooser::SetRequest_DepthStencilFormat(D3DFORMAT format,BOOL bForce)
{
	m_RequestDeviceSetting.pp.AutoDepthStencilFormat=format;
	m_ChooseOption.eDepthFormat=Choose_ClosestToInput;
	m_ChooseOption.eStencilFormat=Choose_ClosestToInput;
	if (bForce)
	{
		m_ChooseOption.eDepthFormat=Choose_PreserveInput;
		m_ChooseOption.eStencilFormat=Choose_PreserveInput;
	}
}
void DeviceChooser::SetRequest_PresentFlags(DWORD flag,BOOL bForce)
{
	m_RequestDeviceSetting.pp.Flags=flag;
	m_ChooseOption.ePresentFlags=Choose_ClosestToInput;
	if (bForce)
		m_ChooseOption.ePresentFlags=Choose_PreserveInput;
}
void DeviceChooser::SetRequest_PresentInterval(DWORD interval,BOOL bForce)
{
	m_RequestDeviceSetting.pp.PresentationInterval=interval;
	m_ChooseOption.ePresentInterval=Choose_ClosestToInput;
	if (bForce)
		m_ChooseOption.ePresentInterval=Choose_PreserveInput;
}
void DeviceChooser::SetRequest_RefreshRate(DWORD RefreshRate)
{
	m_RequestDeviceSetting.pp.FullScreen_RefreshRateInHz=RefreshRate;
	m_ChooseOption.eRefreshRate=Choose_ClosestToInput;
}


BOOL DeviceChooser::Choose(CDeviceObject *pDeviceObject,DeviceEnum *pEnum,DeviceSettings &devicesetting)//Return whether successfully choose one device setting.
{
	m_pDeviceObject=pDeviceObject;
	m_pEnum=pEnum;

	XDirect3D *pD3D;
	pD3D=m_pDeviceObject->_d3d;



	DeviceSettings OptimalDeviceSettings;
	BuildOptimalDeviceSettings(&OptimalDeviceSettings,&m_RequestDeviceSetting,&m_ChooseOption);


	// Find the best combination of:
	//      Adapter Ordinal
	//      Device Type
	//      Adapter Format
	//      Back Buffer Format
	//      Windowed
	// given what's available on the system and the match options combined with the device settings input.
	// This combination of settings is encapsulated by the EnumDeviceSettingsCombo class.
	float fBestRanking = -1.0f;
	EnumDeviceSettingsCombo* pBestDeviceSettingsCombo = NULL;
	D3DDISPLAYMODE adapterDesktopDisplayMode;

	std::vector<EnumAdapterInfo*>* pAdapterList = m_pEnum->GetAdapterInfoList();
	for( int iAdapter=0; iAdapter<pAdapterList->size(); iAdapter++ )
	{
		EnumAdapterInfo* pAdapterInfo = (*pAdapterList)[iAdapter];

		// Get the desktop display mode of adapter 
		pD3D->GetAdapterDisplayMode( pAdapterInfo->AdapterOrdinal, &adapterDesktopDisplayMode );

		// Enum all the device types supported by this adapter to find the best device settings
		for( int iDeviceInfo=0; iDeviceInfo<pAdapterInfo->deviceInfoList.size(); iDeviceInfo++ )
		{
			EnumDeviceInfo* pDeviceInfo = pAdapterInfo->deviceInfoList[iDeviceInfo];

			// Enum all the device settings combinations.  A device settings combination is 
			// a unique set of an adapter format, back buffer format, and IsWindowed.
			for( int iDeviceCombo=0; iDeviceCombo<pDeviceInfo->deviceSettingsComboList.size(); iDeviceCombo++ )
			{
				EnumDeviceSettingsCombo* pDeviceSettingsCombo = pDeviceInfo->deviceSettingsComboList[iDeviceCombo];

				// If windowed mode the adapter format has to be the same as the desktop 
				// display mode format so skip any that don't match
				if (pDeviceSettingsCombo->Windowed && (pDeviceSettingsCombo->AdapterFormat != adapterDesktopDisplayMode.Format))
					continue;

				// Skip any combo that doesn't meet the preserve match options
				if( FALSE== DoesDeviceComboMatchPreserveOptions( pDeviceSettingsCombo, &m_RequestDeviceSetting, &m_ChooseOption) )
					continue;           

				// Get a ranking number that describes how closely this device combo matches the optimal combo
				float fCurRanking=RankDeviceCombo( pDeviceSettingsCombo, &OptimalDeviceSettings, &adapterDesktopDisplayMode );

				// If this combo better matches the input device settings then save it
				if( fCurRanking > fBestRanking )
				{
					pBestDeviceSettingsCombo = pDeviceSettingsCombo;
					fBestRanking = fCurRanking;
				}                
			}
		}
	}

	// If no best device combination was found then fail
	if( pBestDeviceSettingsCombo == NULL ) 
		return FALSE;

	// Using the best device settings combo found, build valid device settings taking heed of 
	// the match options and the input device settings
	BuildValidDeviceSettings( &devicesetting, pBestDeviceSettingsCombo, &m_RequestDeviceSetting, &m_ChooseOption);

	return TRUE;
}


//--------------------------------------------------------------------------------------
// Internal helper function to build a device settings structure based upon the match 
// options.  If the match option is set to ignore, then a optimal default value is used.
// The default value may not exist on the system, but later this will be taken 
// into account.
//--------------------------------------------------------------------------------------
void DeviceChooser::BuildOptimalDeviceSettings( DeviceSettings* pOptimalDeviceSettings, 
									DeviceSettings* pDeviceSettingsIn, 
									ChooseOptions* pMatchOptions )
{
	XDirect3D* pD3D = m_pDeviceObject->_d3d;
	D3DDISPLAYMODE adapterDesktopDisplayMode;

	ZeroMemory( pOptimalDeviceSettings, sizeof(DeviceSettings) ); 

	//---------------------
	// Adapter ordinal
	//---------------------    
	if( pMatchOptions->eAdapterOrdinal == Choose_UseDefault )
		pOptimalDeviceSettings->AdapterOrdinal = D3DADAPTER_DEFAULT; 
	else
		pOptimalDeviceSettings->AdapterOrdinal = pDeviceSettingsIn->AdapterOrdinal;      

	//---------------------
	// Device type
	//---------------------
	if( pMatchOptions->eDeviceType == Choose_UseDefault )
		pOptimalDeviceSettings->DeviceType = D3DDEVTYPE_HAL; 
	else
		pOptimalDeviceSettings->DeviceType = pDeviceSettingsIn->DeviceType;

	//---------------------
	// Windowed
	//---------------------
	if( pMatchOptions->eWindowed == Choose_UseDefault )
		pOptimalDeviceSettings->pp.Windowed = TRUE; 
	else
		pOptimalDeviceSettings->pp.Windowed = pDeviceSettingsIn->pp.Windowed;

	//---------------------
	// Adapter format
	//---------------------
	if( pMatchOptions->eAdapterFormat == Choose_UseDefault )
	{
		// If windowed, default to the desktop display mode
		// If fullscreen, default to the desktop display mode for quick mode change or 
		// default to DEFAULT_FULLSCREEN_ADAPTERFORMAT if the desktop display mode is < 32bit
		pD3D->GetAdapterDisplayMode( pOptimalDeviceSettings->AdapterOrdinal, &adapterDesktopDisplayMode );
		if( pOptimalDeviceSettings->pp.Windowed || D3DColorChannelBits(adapterDesktopDisplayMode.Format) >= 8 )
			pOptimalDeviceSettings->AdapterFormat = adapterDesktopDisplayMode.Format;
		else
			pOptimalDeviceSettings->AdapterFormat = DEFAULT_FULLSCREEN_ADAPTERFORMAT;
	}
	else
	{
		pOptimalDeviceSettings->AdapterFormat = pDeviceSettingsIn->AdapterFormat;
	}

	//---------------------
	// Vertex processing
	//---------------------
	if( pMatchOptions->eVertexProcessing == Choose_UseDefault )
		pOptimalDeviceSettings->BehaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING; 
	else
		pOptimalDeviceSettings->BehaviorFlags = pDeviceSettingsIn->BehaviorFlags;

	//---------------------
	// Resolution
	//---------------------
	if( pMatchOptions->eResolution == Choose_UseDefault )
	{
		// default to the desktop res for quick mode change
		pD3D->GetAdapterDisplayMode( pOptimalDeviceSettings->AdapterOrdinal, &adapterDesktopDisplayMode );
		pOptimalDeviceSettings->pp.BackBufferWidth = adapterDesktopDisplayMode.Width;
		pOptimalDeviceSettings->pp.BackBufferHeight = adapterDesktopDisplayMode.Height;
	}
	else
	{
		pOptimalDeviceSettings->pp.BackBufferWidth = pDeviceSettingsIn->pp.BackBufferWidth;
		pOptimalDeviceSettings->pp.BackBufferHeight = pDeviceSettingsIn->pp.BackBufferHeight;
	}

	//---------------------
	// Back buffer format
	//---------------------
	if( pMatchOptions->eBackBufferFormat == Choose_UseDefault )
		pOptimalDeviceSettings->pp.BackBufferFormat = pOptimalDeviceSettings->AdapterFormat; // Default to match the adapter format
	else
		pOptimalDeviceSettings->pp.BackBufferFormat = pDeviceSettingsIn->pp.BackBufferFormat;

	//---------------------
	// Back buffer count
	//---------------------
	if( pMatchOptions->eBackBufferCount == Choose_UseDefault )
		pOptimalDeviceSettings->pp.BackBufferCount = 2; // Default to triple buffering for perf gain
	else
		pOptimalDeviceSettings->pp.BackBufferCount = pDeviceSettingsIn->pp.BackBufferCount;

	//---------------------
	// Multisample
	//---------------------
	if( pMatchOptions->eMultiSample == Choose_UseDefault )
	{
		// Default to no multisampling 
		pOptimalDeviceSettings->pp.MultiSampleType = D3DMULTISAMPLE_NONE;
		pOptimalDeviceSettings->pp.MultiSampleQuality = 0; 
	}
	else
	{
		pOptimalDeviceSettings->pp.MultiSampleType = pDeviceSettingsIn->pp.MultiSampleType;
		pOptimalDeviceSettings->pp.MultiSampleQuality = pDeviceSettingsIn->pp.MultiSampleQuality;
	}

	//---------------------
	// Swap effect
	//---------------------
	if( pMatchOptions->eSwapEffect == Choose_UseDefault )
		pOptimalDeviceSettings->pp.SwapEffect = D3DSWAPEFFECT_DISCARD; 
	else
		pOptimalDeviceSettings->pp.SwapEffect = pDeviceSettingsIn->pp.SwapEffect;

	//---------------------
	// Depth stencil 
	//---------------------
	if( pMatchOptions->eDepthFormat == Choose_UseDefault &&
		pMatchOptions->eStencilFormat == Choose_UseDefault )
	{
		UINT nBackBufferBits = D3DColorChannelBits( pOptimalDeviceSettings->pp.BackBufferFormat );
		if( nBackBufferBits >= 8 )
			pOptimalDeviceSettings->pp.AutoDepthStencilFormat = D3DFMT_D32; 
		else
			pOptimalDeviceSettings->pp.AutoDepthStencilFormat = D3DFMT_D16; 
	}
	else
	{
		pOptimalDeviceSettings->pp.AutoDepthStencilFormat = pDeviceSettingsIn->pp.AutoDepthStencilFormat;
	}

	//---------------------
	// Present flags
	//---------------------
	if( pMatchOptions->ePresentFlags == Choose_UseDefault )
		pOptimalDeviceSettings->pp.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	else
		pOptimalDeviceSettings->pp.Flags = pDeviceSettingsIn->pp.Flags;

	//---------------------
	// Refresh rate
	//---------------------
	if( pMatchOptions->eRefreshRate == Choose_UseDefault )
		pOptimalDeviceSettings->pp.FullScreen_RefreshRateInHz = 0;
	else
		pOptimalDeviceSettings->pp.FullScreen_RefreshRateInHz = pDeviceSettingsIn->pp.FullScreen_RefreshRateInHz;

	//---------------------
	// Present interval
	//---------------------
	if( pMatchOptions->ePresentInterval == Choose_UseDefault )
	{
		// For windowed, default to D3DPRESENT_INTERVAL_IMMEDIATE
		// which will wait not for the vertical retrace period to prevent tearing, 
		// but may introduce tearing.
		// For full screen, default to D3DPRESENT_INTERVAL_DEFAULT 
		// which will wait for the vertical retrace period to prevent tearing.
		if( pOptimalDeviceSettings->pp.Windowed )
			pOptimalDeviceSettings->pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		else
			pOptimalDeviceSettings->pp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	}
	else
	{
		pOptimalDeviceSettings->pp.PresentationInterval = pDeviceSettingsIn->pp.PresentationInterval;
	}
}


BOOL DeviceChooser::DoesDeviceComboMatchPreserveOptions( EnumDeviceSettingsCombo* pDeviceSettingsCombo, 
											 DeviceSettings* pDeviceSettingsIn, 
											 ChooseOptions* pMatchOptions )
{
	//---------------------
	// Adapter ordinal
	//---------------------
	if( pMatchOptions->eAdapterOrdinal == Choose_PreserveInput && 
		(pDeviceSettingsCombo->AdapterOrdinal != pDeviceSettingsIn->AdapterOrdinal) )
		return FALSE;

	//---------------------
	// Device type
	//---------------------
	if( pMatchOptions->eDeviceType == Choose_PreserveInput && 
		(pDeviceSettingsCombo->DeviceType != pDeviceSettingsIn->DeviceType) )
		return FALSE;

	//---------------------
	// Windowed
	//---------------------
	if( pMatchOptions->eWindowed == Choose_PreserveInput && 
		(pDeviceSettingsCombo->Windowed != pDeviceSettingsIn->pp.Windowed) )
		return FALSE;

	//---------------------
	// Adapter format
	//---------------------
	if( pMatchOptions->eAdapterFormat == Choose_PreserveInput && 
		(pDeviceSettingsCombo->AdapterFormat != pDeviceSettingsIn->AdapterFormat) )
		return FALSE;

	//---------------------
	// Vertex processing
	//---------------------
	// If keep VP and input has HWVP, then skip if this combo doesn't have HWTL 
	if( pMatchOptions->eVertexProcessing == Choose_PreserveInput && 
		((pDeviceSettingsIn->BehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING) != 0) && 
		((pDeviceSettingsCombo->pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0) )
		return FALSE;

	//---------------------
	// Resolution
	//---------------------
	// If keep resolution then check that width and height supported by this combo
	if( pMatchOptions->eResolution == Choose_PreserveInput )
	{
		BOOL bFound = FALSE;
		for( int i=0; i< pDeviceSettingsCombo->pAdapterInfo->displayModeList.size(); i++ )
		{
			D3DDISPLAYMODE displayMode = pDeviceSettingsCombo->pAdapterInfo->displayModeList[i];
			if( displayMode.Format != pDeviceSettingsCombo->AdapterFormat )
				continue; // Skip this display mode if it doesn't match the combo's adapter format

			if( displayMode.Width == pDeviceSettingsIn->pp.BackBufferWidth &&
				displayMode.Height == pDeviceSettingsIn->pp.BackBufferHeight )
			{
				bFound = true;
				break;
			}
		}

		// If the width and height are not supported by this combo, return FALSE
		if( !bFound )
			return FALSE;
	}

	//---------------------
	// Back buffer format
	//---------------------
	if( pMatchOptions->eBackBufferFormat == Choose_PreserveInput && 
		pDeviceSettingsCombo->BackBufferFormat != pDeviceSettingsIn->pp.BackBufferFormat )
		return FALSE;

	//---------------------
	// Back buffer count
	//---------------------
	// No caps for the back buffer count

	//---------------------
	// Multisample
	//---------------------
	if( pMatchOptions->eMultiSample == Choose_PreserveInput )
	{
		BOOL bFound = FALSE;
		for( int i=0; i<pDeviceSettingsCombo->multiSampleTypeList.size(); i++ )
		{
			D3DMULTISAMPLE_TYPE msType = pDeviceSettingsCombo->multiSampleTypeList[i];
			DWORD msQuality  = pDeviceSettingsCombo->multiSampleQualityList[i];

			if( msType == pDeviceSettingsIn->pp.MultiSampleType &&
				msQuality >= pDeviceSettingsIn->pp.MultiSampleQuality )
			{
				bFound = true;
				break;
			}
		}

		// If multisample type/quality not supported by this combo, then return FALSE
		if( !bFound )
			return FALSE;
	}

	//---------------------
	// Swap effect
	//---------------------
	// No caps for swap effects

	//---------------------
	// Depth stencil 
	//---------------------
	// If keep depth stencil format then check that the depth stencil format is supported by this combo
	if( pMatchOptions->eDepthFormat == Choose_PreserveInput &&
		pMatchOptions->eStencilFormat == Choose_PreserveInput )
	{
		if( pDeviceSettingsIn->pp.AutoDepthStencilFormat != D3DFMT_UNKNOWN)
		{
			int i;
			for (i=0;i<pDeviceSettingsCombo->depthStencilFormatList.size();i++)
			{
				if (pDeviceSettingsCombo->depthStencilFormatList[i]==pDeviceSettingsIn->pp.AutoDepthStencilFormat)
					break;
			}
			if (i>=pDeviceSettingsCombo->depthStencilFormatList.size())
				return FALSE;
		}
	}

	// If keep depth format then check that the depth format is supported by this combo
	if( pMatchOptions->eDepthFormat == Choose_PreserveInput &&
		pDeviceSettingsIn->pp.AutoDepthStencilFormat != D3DFMT_UNKNOWN )
	{
		BOOL bFound = FALSE;
		UINT dwDepthBits = D3DDepthBits( pDeviceSettingsIn->pp.AutoDepthStencilFormat );
		for( int i=0; i<pDeviceSettingsCombo->depthStencilFormatList.size(); i++ )
		{
			D3DFORMAT depthStencilFmt = pDeviceSettingsCombo->depthStencilFormatList[i];
			UINT dwCurDepthBits = D3DDepthBits( depthStencilFmt );
			if( dwCurDepthBits - dwDepthBits == 0)
				bFound = true;
		}

		if( !bFound )
			return FALSE;
	}

	// If keep depth format then check that the depth format is supported by this combo
	if( pMatchOptions->eStencilFormat == Choose_PreserveInput &&
		pDeviceSettingsIn->pp.AutoDepthStencilFormat != D3DFMT_UNKNOWN )
	{
		BOOL bFound = FALSE;
		UINT dwStencilBits = D3DStencilBits( pDeviceSettingsIn->pp.AutoDepthStencilFormat );
		for( int i=0; i<pDeviceSettingsCombo->depthStencilFormatList.size(); i++ )
		{
			D3DFORMAT depthStencilFmt = pDeviceSettingsCombo->depthStencilFormatList[i];
			UINT dwCurStencilBits = D3DStencilBits( depthStencilFmt );
			if( dwCurStencilBits - dwStencilBits == 0)
				bFound = true;
		}

		if( !bFound )
			return FALSE;
	}

	//---------------------
	// Present flags
	//---------------------
	// No caps for the present flags

	//---------------------
	// Refresh rate
	//---------------------
	// If keep refresh rate then check that the resolution is supported by this combo
	if( pMatchOptions->eRefreshRate == Choose_PreserveInput )
	{
		BOOL bFound = FALSE;
		for( int i=0; i<pDeviceSettingsCombo->pAdapterInfo->displayModeList.size(); i++ )
		{
			D3DDISPLAYMODE displayMode = pDeviceSettingsCombo->pAdapterInfo->displayModeList[i];
			if( displayMode.Format != pDeviceSettingsCombo->AdapterFormat )
				continue;
			if( displayMode.RefreshRate == pDeviceSettingsIn->pp.FullScreen_RefreshRateInHz )
			{
				bFound = true;
				break;
			}
		}

		// If refresh rate not supported by this combo, then return FALSE
		if( !bFound )
			return FALSE;
	}

	//---------------------
	// Present interval
	//---------------------
	// If keep present interval then check that the present interval is supported by this combo
	if( pMatchOptions->ePresentInterval == Choose_PreserveInput)
	{
		int i;
		for (i=0;i<pDeviceSettingsCombo->presentIntervalList.size();i++)
		{
			if (pDeviceSettingsCombo->presentIntervalList[i]==pDeviceSettingsIn->pp.PresentationInterval)
				break;
		}

		if (i>=pDeviceSettingsCombo->presentIntervalList.size())
			return FALSE;
	}

	return TRUE;
}


//--------------------------------------------------------------------------------------
// Returns a ranking number that describes how closely this device 
// combo matches the optimal combo based on the match options and the optimal device settings
//--------------------------------------------------------------------------------------
float DeviceChooser::RankDeviceCombo( EnumDeviceSettingsCombo* pDeviceSettingsCombo, 
						  DeviceSettings* pOptimalDeviceSettings,
						  D3DDISPLAYMODE* pAdapterDesktopDisplayMode )
{
	float fCurRanking = 0.0f; 

	// Arbitrary weights.  Gives preference to the ordinal, device type, and windowed
	const float fAdapterOrdinalWeight   = 1000.0f;
	const float fDeviceTypeWeight       = 100.0f;
	const float fWindowWeight           = 10.0f;
	const float fAdapterFormatWeight    = 1.0f;
	const float fVertexProcessingWeight = 1.0f;
	const float fResolutionWeight       = 1.0f;
	const float fBackBufferFormatWeight = 1.0f;
	const float fMultiSampleWeight      = 1.0f;
	const float fDepthStencilWeight     = 1.0f;
	const float fRefreshRateWeight      = 1.0f;
	const float fPresentIntervalWeight  = 1.0f;

	//---------------------
	// Adapter ordinal
	//---------------------
	if( pDeviceSettingsCombo->AdapterOrdinal == pOptimalDeviceSettings->AdapterOrdinal )
		fCurRanking += fAdapterOrdinalWeight;

	//---------------------
	// Device type
	//---------------------
	if( pDeviceSettingsCombo->DeviceType == pOptimalDeviceSettings->DeviceType )
		fCurRanking += fDeviceTypeWeight;
	// Slightly prefer HAL 
	if( pDeviceSettingsCombo->DeviceType == D3DDEVTYPE_HAL )
		fCurRanking += 0.1f; 

	//---------------------
	// Windowed
	//---------------------
	if( pDeviceSettingsCombo->Windowed == pOptimalDeviceSettings->pp.Windowed )
		fCurRanking += fWindowWeight;

	//---------------------
	// Adapter format
	//---------------------
	if( pDeviceSettingsCombo->AdapterFormat == pOptimalDeviceSettings->AdapterFormat )
	{
		fCurRanking += fAdapterFormatWeight;
	}
	else
	{
		int nBitDepthDelta = abs( (long) D3DColorChannelBits(pDeviceSettingsCombo->AdapterFormat) -
			(long) D3DColorChannelBits(pOptimalDeviceSettings->AdapterFormat) );
		float fScale = max(0.9f - (float)nBitDepthDelta*0.2f, 0);
		fCurRanking += fScale * fAdapterFormatWeight;
	}

	if( !pDeviceSettingsCombo->Windowed )
	{
		// Slightly prefer when it matches the desktop format or is DEFAULT_FULLSCREEN_ADAPTERFORMAT
		bool bAdapterOptimalMatch;
		if( D3DColorChannelBits(pAdapterDesktopDisplayMode->Format) >= 8 )
			bAdapterOptimalMatch = (pDeviceSettingsCombo->AdapterFormat == pAdapterDesktopDisplayMode->Format);
		else
			bAdapterOptimalMatch = (pDeviceSettingsCombo->AdapterFormat == DEFAULT_FULLSCREEN_ADAPTERFORMAT);

		if( bAdapterOptimalMatch )
			fCurRanking += 0.1f;
	}

	//---------------------
	// Vertex processing
	//---------------------
	if( (pOptimalDeviceSettings->BehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING) != 0 || 
		(pOptimalDeviceSettings->BehaviorFlags & D3DCREATE_MIXED_VERTEXPROCESSING) != 0 )
	{
		if( (pDeviceSettingsCombo->pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0 )
			fCurRanking += fVertexProcessingWeight;
	}
	// Slightly prefer HW T&L
	if( (pDeviceSettingsCombo->pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0 )
		fCurRanking += 0.1f;

	//---------------------
	// Resolution
	//---------------------
	bool bResolutionFound = false;
	for( int idm = 0; idm < pDeviceSettingsCombo->pAdapterInfo->displayModeList.size(); idm++ )
	{
		D3DDISPLAYMODE displayMode = pDeviceSettingsCombo->pAdapterInfo->displayModeList[idm];
		if( displayMode.Format != pDeviceSettingsCombo->AdapterFormat )
			continue;
		if( displayMode.Width == pOptimalDeviceSettings->pp.BackBufferWidth &&
			displayMode.Height == pOptimalDeviceSettings->pp.BackBufferHeight )
			bResolutionFound = true;
	}
	if( bResolutionFound )
		fCurRanking += fResolutionWeight;

	//---------------------
	// Back buffer format
	//---------------------
	if( pDeviceSettingsCombo->BackBufferFormat == pOptimalDeviceSettings->pp.BackBufferFormat )
	{
		fCurRanking += fBackBufferFormatWeight;
	}
	else
	{
		int nBitDepthDelta = abs( (long) D3DColorChannelBits(pDeviceSettingsCombo->BackBufferFormat) -
			(long) D3DColorChannelBits(pOptimalDeviceSettings->pp.BackBufferFormat) );
		float fScale = max(0.9f - (float)nBitDepthDelta*0.2f, 0);
		fCurRanking += fScale * fBackBufferFormatWeight;
	}

	// Check if this back buffer format is the same as 
	// the adapter format since this is preferred.
	bool bAdapterMatchesBB = (pDeviceSettingsCombo->BackBufferFormat == pDeviceSettingsCombo->AdapterFormat);
	if( bAdapterMatchesBB )
		fCurRanking += 0.1f;

	//---------------------
	// Back buffer count
	//---------------------
	// No caps for the back buffer count

	//---------------------
	// Multisample
	//---------------------
	bool bMultiSampleFound = false;
	for( int i=0; i<pDeviceSettingsCombo->multiSampleTypeList.size(); i++ )
	{
		D3DMULTISAMPLE_TYPE msType = pDeviceSettingsCombo->multiSampleTypeList[i];
		DWORD msQuality  = pDeviceSettingsCombo->multiSampleQualityList[i];

		if( msType == pOptimalDeviceSettings->pp.MultiSampleType &&
			msQuality >= pOptimalDeviceSettings->pp.MultiSampleQuality )
		{
			bMultiSampleFound = true;
			break;
		}
	}
	if( bMultiSampleFound )
		fCurRanking += fMultiSampleWeight;

	//---------------------
	// Swap effect
	//---------------------
	// No caps for swap effects

	//---------------------
	// Depth stencil 
	//---------------------
	if (TRUE)
	{
		int i;
		for (i=0;i<pDeviceSettingsCombo->depthStencilFormatList.size();i++)
		{
			if (pDeviceSettingsCombo->depthStencilFormatList[i]==pOptimalDeviceSettings->pp.AutoDepthStencilFormat)
				break;
		}
		if (i<pDeviceSettingsCombo->depthStencilFormatList.size())
			fCurRanking += fDepthStencilWeight;
	}

	//---------------------
	// Present flags
	//---------------------
	// No caps for the present flags

	//---------------------
	// Refresh rate
	//---------------------
	bool bRefreshFound = false;
	for( int idm = 0; idm < pDeviceSettingsCombo->pAdapterInfo->displayModeList.size(); idm++ )
	{
		D3DDISPLAYMODE displayMode = pDeviceSettingsCombo->pAdapterInfo->displayModeList[idm];
		if( displayMode.Format != pDeviceSettingsCombo->AdapterFormat )
			continue;
		if( displayMode.RefreshRate == pOptimalDeviceSettings->pp.FullScreen_RefreshRateInHz )
			bRefreshFound = true;
	}
	if( bRefreshFound )
		fCurRanking += fRefreshRateWeight;

	//---------------------
	// Present interval
	//---------------------
	// If keep present interval then check that the present interval is supported by this combo
	if (TRUE)
	{
		int i;
		for (i=0;i<pDeviceSettingsCombo->presentIntervalList.size();i++)
		{
			if (pDeviceSettingsCombo->presentIntervalList[i]==pOptimalDeviceSettings->pp.PresentationInterval)
				break;
		}
		if (i<pDeviceSettingsCombo->presentIntervalList.size())
			fCurRanking += fPresentIntervalWeight;
	}

	return fCurRanking;
}


//--------------------------------------------------------------------------------------
// Builds valid device settings using the match options, the input device settings, and the 
// best device settings combo found.
//--------------------------------------------------------------------------------------
void DeviceChooser::BuildValidDeviceSettings( DeviceSettings* pValidDeviceSettings, 
								  EnumDeviceSettingsCombo* pBestDeviceSettingsCombo, 
								  DeviceSettings* pDeviceSettingsIn, 
								  ChooseOptions* pMatchOptions )
{
	XDirect3D* pD3D = m_pDeviceObject->_d3d;
	D3DDISPLAYMODE adapterDesktopDisplayMode;
	pD3D->GetAdapterDisplayMode( pBestDeviceSettingsCombo->AdapterOrdinal, &adapterDesktopDisplayMode );

	// For each setting pick the best, taking into account the match options and 
	// what's supported by the device

	//---------------------
	// Adapter Ordinal
	//---------------------
	// Just using pBestDeviceSettingsCombo->AdapterOrdinal

	//---------------------
	// Device Type
	//---------------------
	// Just using pBestDeviceSettingsCombo->DeviceType

	//---------------------
	// Windowed 
	//---------------------
	// Just using pBestDeviceSettingsCombo->Windowed

	//---------------------
	// Adapter Format
	//---------------------
	// Just using pBestDeviceSettingsCombo->AdapterFormat

	//---------------------
	// Vertex processing
	//---------------------
	DWORD dwBestBehaviorFlags = 0;
	if( pMatchOptions->eVertexProcessing == Choose_PreserveInput )   
	{
		dwBestBehaviorFlags = pDeviceSettingsIn->BehaviorFlags;
	}
	else if( pMatchOptions->eVertexProcessing == Choose_UseDefault )    
	{
		// The framework defaults to HWVP if available otherwise use SWVP
		if ((pBestDeviceSettingsCombo->pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0)
			dwBestBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
		else
			dwBestBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}
	else // if( pMatchOptions->eVertexProcessing == Choose_ClosestToInput )    
	{
		// Default to input, and fallback to SWVP if HWVP not available 
		dwBestBehaviorFlags = pDeviceSettingsIn->BehaviorFlags;
		if ((pBestDeviceSettingsCombo->pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0 && 
			( (dwBestBehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING) != 0 || 
			(dwBestBehaviorFlags & D3DCREATE_MIXED_VERTEXPROCESSING) != 0) )
		{
			dwBestBehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
			dwBestBehaviorFlags &= ~D3DCREATE_MIXED_VERTEXPROCESSING;
			dwBestBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		}

		// One of these must be selected
		if( (dwBestBehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING) == 0 &&
			(dwBestBehaviorFlags & D3DCREATE_MIXED_VERTEXPROCESSING) == 0 &&
			(dwBestBehaviorFlags & D3DCREATE_SOFTWARE_VERTEXPROCESSING) == 0 )
		{
			if ((pBestDeviceSettingsCombo->pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0)
				dwBestBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
			else
				dwBestBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		}
	}

	//---------------------
	// Resolution
	//---------------------
	D3DDISPLAYMODE bestDisplayMode;  
	if( pMatchOptions->eResolution == Choose_PreserveInput )   
	{
		bestDisplayMode.Width = pDeviceSettingsIn->pp.BackBufferWidth;
		bestDisplayMode.Height = pDeviceSettingsIn->pp.BackBufferHeight;
	}
	else 
	{
		D3DDISPLAYMODE displayModeIn;  
		if( pMatchOptions->eResolution == Choose_ClosestToInput &&
			pDeviceSettingsIn && (pDeviceSettingsIn->pp.BackBufferWidth != 0 && pDeviceSettingsIn->pp.BackBufferWidth != 0) )   
		{
			displayModeIn.Width = pDeviceSettingsIn->pp.BackBufferWidth;
			displayModeIn.Height = pDeviceSettingsIn->pp.BackBufferHeight;
		}
		else // if( pMatchOptions->eResolution == Choose_UseDefault )   
		{
			// The framework defaults to desktop resolution to try to avoid slow mode change
			if (pBestDeviceSettingsCombo->Windowed)
			{
				HWND hWnd;
				hWnd=m_pDeviceObject->_hwnd;
				RECT rc;
				GetWindowRect(hWnd,&rc);
				displayModeIn.Width = rc.right-rc.left;
				displayModeIn.Height = rc.bottom-rc.top;
			}
			else
			{
				displayModeIn.Width = adapterDesktopDisplayMode.Width;
				displayModeIn.Height = adapterDesktopDisplayMode.Height;
			}
		}

		// Call a helper function to find the closest valid display mode to the optimal 
		FindValidResolution( pBestDeviceSettingsCombo, displayModeIn, &bestDisplayMode );
	}

	//---------------------
	// Back Buffer Format
	//---------------------
	// Just using pBestDeviceSettingsCombo->BackBufferFormat

	//---------------------
	// Back buffer count
	//---------------------
	UINT bestBackBufferCount;
	if( pMatchOptions->eBackBufferCount == Choose_PreserveInput )   
	{
		bestBackBufferCount = pDeviceSettingsIn->pp.BackBufferCount;
	}
	else if( pMatchOptions->eBackBufferCount == Choose_UseDefault )   
	{
		// The framework defaults to triple buffering 
		bestBackBufferCount = 2;
	}
	else // if( pMatchOptions->eBackBufferCount == Choose_ClosestToInput )   
	{
		bestBackBufferCount = pDeviceSettingsIn->pp.BackBufferCount;
		if( bestBackBufferCount > 3 )
			bestBackBufferCount = 3;
		if( bestBackBufferCount < 1 )
			bestBackBufferCount = 1;
	}

	//---------------------
	// Multisample
	//---------------------
	D3DMULTISAMPLE_TYPE bestMultiSampleType;
	DWORD bestMultiSampleQuality;
	if( pDeviceSettingsIn && pDeviceSettingsIn->pp.SwapEffect != D3DSWAPEFFECT_DISCARD )
	{
		// Swap effect is not set to discard so multisampling has to off
		bestMultiSampleType = D3DMULTISAMPLE_NONE;
		bestMultiSampleQuality = 0;
	}
	else
	{
		if( pMatchOptions->eBackBufferCount == Choose_PreserveInput )   
		{
			bestMultiSampleType    = pDeviceSettingsIn->pp.MultiSampleType;
			bestMultiSampleQuality = pDeviceSettingsIn->pp.MultiSampleQuality;
		}
		else if( pMatchOptions->eBackBufferCount == Choose_UseDefault )   
		{
			// Default to no multisampling (always supported)
			bestMultiSampleType = D3DMULTISAMPLE_NONE;
			bestMultiSampleQuality = 0;
		}
		else if( pMatchOptions->eBackBufferCount == Choose_ClosestToInput )   
		{
			// Default to no multisampling (always supported)
			bestMultiSampleType = D3DMULTISAMPLE_NONE;
			bestMultiSampleQuality = 0;

			for( int i=0; i < pBestDeviceSettingsCombo->multiSampleTypeList.size(); i++ )
			{
				D3DMULTISAMPLE_TYPE type = pBestDeviceSettingsCombo->multiSampleTypeList[i];
				DWORD qualityLevels = pBestDeviceSettingsCombo->multiSampleQualityList[i];

				// Check whether supported type is closer to the input than our current best
				if( abs(type - pDeviceSettingsIn->pp.MultiSampleType) < abs(bestMultiSampleType - pDeviceSettingsIn->pp.MultiSampleType) )
				{
					bestMultiSampleType = type; 
					bestMultiSampleQuality = min( qualityLevels-1, pDeviceSettingsIn->pp.MultiSampleQuality );
				}
			}
		}
		else
		{
			// Error case
			bestMultiSampleType = D3DMULTISAMPLE_NONE;
			bestMultiSampleQuality = 0;
		}
	}

	//---------------------
	// Swap effect
	//---------------------
	D3DSWAPEFFECT bestSwapEffect;
	if( pMatchOptions->eSwapEffect == Choose_PreserveInput )   
	{
		bestSwapEffect = pDeviceSettingsIn->pp.SwapEffect;
	}
	else if( pMatchOptions->eSwapEffect == Choose_UseDefault )   
	{
		bestSwapEffect = D3DSWAPEFFECT_DISCARD;
	}
	else // if( pMatchOptions->eSwapEffect == Choose_ClosestToInput )   
	{
		bestSwapEffect = pDeviceSettingsIn->pp.SwapEffect;

		// Swap effect has to be one of these 3
		if( bestSwapEffect != D3DSWAPEFFECT_DISCARD &&
			bestSwapEffect != D3DSWAPEFFECT_FLIP &&
			bestSwapEffect != D3DSWAPEFFECT_COPY )
		{
			bestSwapEffect = D3DSWAPEFFECT_DISCARD;
		}
	}

	//---------------------
	// Depth stencil 
	//---------------------
	D3DFORMAT bestDepthStencilFormat;
	BOOL bestEnableAutoDepthStencil;

	std::vector< int > depthStencilRanking;
	depthStencilRanking.resize(pBestDeviceSettingsCombo->depthStencilFormatList.size());

	UINT dwBackBufferBitDepth = D3DColorChannelBits( pBestDeviceSettingsCombo->BackBufferFormat ); 
	UINT dwInputDepthBitDepth = 0;
	if( pDeviceSettingsIn )
		dwInputDepthBitDepth = D3DDepthBits( pDeviceSettingsIn->pp.AutoDepthStencilFormat );

	for( int i=0; i<pBestDeviceSettingsCombo->depthStencilFormatList.size(); i++ )
	{
		D3DFORMAT curDepthStencilFmt = pBestDeviceSettingsCombo->depthStencilFormatList[i];
		DWORD dwCurDepthBitDepth = D3DDepthBits( curDepthStencilFmt );
		int nRanking;

		if( pMatchOptions->eDepthFormat == Choose_PreserveInput )
		{                       
			// Need to match bit depth of input
			if(dwCurDepthBitDepth == dwInputDepthBitDepth)
				nRanking = 0;
			else
				nRanking = 10000;
		}
		else if( pMatchOptions->eDepthFormat == Choose_UseDefault )
		{
			// Prefer match of backbuffer bit depth
			nRanking = abs((int)dwCurDepthBitDepth - (int)dwBackBufferBitDepth*4);
		}
		else // if( pMatchOptions->eDepthFormat == Choose_ClosestToInput )
		{
			// Prefer match of input depth format bit depth
			nRanking = abs((int)dwCurDepthBitDepth - (int)dwInputDepthBitDepth);
		}

		depthStencilRanking.push_back( nRanking );
	}

	UINT dwInputStencilBitDepth = 0;
	if( pDeviceSettingsIn )
		dwInputStencilBitDepth = D3DStencilBits( pDeviceSettingsIn->pp.AutoDepthStencilFormat );

	for( int i=0; i<pBestDeviceSettingsCombo->depthStencilFormatList.size(); i++ )
	{
		D3DFORMAT curDepthStencilFmt = pBestDeviceSettingsCombo->depthStencilFormatList[i];
		int nRanking = depthStencilRanking[i];
		DWORD dwCurStencilBitDepth = D3DStencilBits( curDepthStencilFmt );

		if( pMatchOptions->eStencilFormat == Choose_PreserveInput )
		{                       
			// Need to match bit depth of input
			if(dwCurStencilBitDepth == dwInputStencilBitDepth)
				nRanking += 0;
			else
				nRanking += 10000;
		}
		else if( pMatchOptions->eStencilFormat == Choose_UseDefault )
		{
			// Prefer 0 stencil bit depth
			nRanking += dwCurStencilBitDepth;
		}
		else // if( pMatchOptions->eStencilFormat == Choose_ClosestToInput )
		{
			// Prefer match of input stencil format bit depth
			nRanking += abs((int)dwCurStencilBitDepth - (int)dwInputStencilBitDepth);
		}

		depthStencilRanking[i]=nRanking;
	}

	int nBestRanking = 100000;
	int nBestIndex = -1;
	for( int i=0; i<pBestDeviceSettingsCombo->depthStencilFormatList.size(); i++ )
	{
		int nRanking = depthStencilRanking[i];
		if( nRanking < nBestRanking )
		{
			nBestRanking = nRanking;
			nBestIndex = i;
		}
	}

	if( nBestIndex >= 0 )
	{
		bestDepthStencilFormat = pBestDeviceSettingsCombo->depthStencilFormatList[nBestIndex];
		bestEnableAutoDepthStencil = true;
	}
	else
	{
		bestDepthStencilFormat = D3DFMT_UNKNOWN;
		bestEnableAutoDepthStencil = false;
	}


	//---------------------
	// Present flags
	//---------------------
	DWORD dwBestFlags;
	if( pMatchOptions->ePresentFlags == Choose_PreserveInput )   
	{
		dwBestFlags = pDeviceSettingsIn->pp.Flags;
	}
	else if( pMatchOptions->ePresentFlags == Choose_UseDefault )   
	{
		dwBestFlags = 0;
		if( bestEnableAutoDepthStencil )
			dwBestFlags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;            
	}
	else // if( pMatchOptions->ePresentFlags == Choose_ClosestToInput )   
	{
		dwBestFlags = pDeviceSettingsIn->pp.Flags;
		if( bestEnableAutoDepthStencil )
			dwBestFlags |= D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	}

	//---------------------
	// Refresh rate
	//---------------------
	if( pBestDeviceSettingsCombo->Windowed )
	{
		// Must be 0 for windowed
		bestDisplayMode.RefreshRate = 0;
	}
	else
	{
		if( pMatchOptions->eRefreshRate == Choose_PreserveInput )   
		{
			bestDisplayMode.RefreshRate = pDeviceSettingsIn->pp.FullScreen_RefreshRateInHz;
		}
		else 
		{
			UINT refreshRateMatch;
			if( pMatchOptions->eRefreshRate == Choose_ClosestToInput )   
			{
				refreshRateMatch = pDeviceSettingsIn->pp.FullScreen_RefreshRateInHz;
			}
			else // if( pMatchOptions->eRefreshRate == Choose_UseDefault )   
			{
				refreshRateMatch = adapterDesktopDisplayMode.RefreshRate;
			}

			bestDisplayMode.RefreshRate = 0;

			if( refreshRateMatch != 0 )
			{
				int nBestRefreshRanking = 100000;
				std::vector<D3DDISPLAYMODE>* pDisplayModeList = &pBestDeviceSettingsCombo->pAdapterInfo->displayModeList;
				for( int iDisplayMode=0; iDisplayMode<pDisplayModeList->size(); iDisplayMode++ )
				{
					D3DDISPLAYMODE displayMode = (*pDisplayModeList)[iDisplayMode];  
					if( displayMode.Format != pBestDeviceSettingsCombo->AdapterFormat || 
						displayMode.Height != bestDisplayMode.Height ||
						displayMode.Width != bestDisplayMode.Width )
						continue; // Skip display modes that don't match 

					// Find the delta between the current refresh rate and the optimal refresh rate 
					int nCurRanking = abs((int)displayMode.RefreshRate - (int)refreshRateMatch);

					if( nCurRanking < nBestRefreshRanking )
					{
						bestDisplayMode.RefreshRate = displayMode.RefreshRate;
						nBestRefreshRanking = nCurRanking;

						// Stop if perfect match found
						if( nBestRefreshRanking == 0 )
							break;
					}
				}
			}
		}
	}

	//---------------------
	// Present interval
	//---------------------
	UINT bestPresentInterval;
	if( pMatchOptions->ePresentInterval == Choose_PreserveInput )   
	{
		bestPresentInterval = pDeviceSettingsIn->pp.PresentationInterval;
	}
	else if( pMatchOptions->ePresentInterval == Choose_UseDefault )   
	{
		if( pBestDeviceSettingsCombo->Windowed )
		{
			// For windowed, the framework defaults to D3DPRESENT_INTERVAL_IMMEDIATE
			// which will wait not for the vertical retrace period to prevent tearing, 
			// but may introduce tearing
			bestPresentInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		}
		else
		{
			// For full screen, the framework defaults to D3DPRESENT_INTERVAL_DEFAULT 
			// which will wait for the vertical retrace period to prevent tearing
			bestPresentInterval = D3DPRESENT_INTERVAL_DEFAULT;
		}
	}
	else // if( pMatchOptions->ePresentInterval == Choose_ClosestToInput )   
	{
		int i;
		for (i=0;i<pBestDeviceSettingsCombo->presentIntervalList.size();i++)
		{
			if (pBestDeviceSettingsCombo->presentIntervalList[i]==pDeviceSettingsIn->pp.PresentationInterval)
				break;
		}
		if (i<pBestDeviceSettingsCombo->presentIntervalList.size())
		{
			bestPresentInterval = pDeviceSettingsIn->pp.PresentationInterval;
		}
		else
		{
			if( pBestDeviceSettingsCombo->Windowed )
				bestPresentInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
			else
				bestPresentInterval = D3DPRESENT_INTERVAL_DEFAULT;
		}
	}

	// Fill the device settings struct
	ZeroMemory( pValidDeviceSettings, sizeof(DeviceSettings) );
	pValidDeviceSettings->AdapterOrdinal                 = pBestDeviceSettingsCombo->AdapterOrdinal;
	pValidDeviceSettings->DeviceType                     = pBestDeviceSettingsCombo->DeviceType;
	pValidDeviceSettings->AdapterFormat                  = pBestDeviceSettingsCombo->AdapterFormat;
	pValidDeviceSettings->BehaviorFlags                  = dwBestBehaviorFlags;
	pValidDeviceSettings->pp.BackBufferWidth             = bestDisplayMode.Width;
	pValidDeviceSettings->pp.BackBufferHeight            = bestDisplayMode.Height;
	pValidDeviceSettings->pp.BackBufferFormat            = pBestDeviceSettingsCombo->BackBufferFormat;
	pValidDeviceSettings->pp.BackBufferCount             = bestBackBufferCount;
	pValidDeviceSettings->pp.MultiSampleType             = bestMultiSampleType;  
	pValidDeviceSettings->pp.MultiSampleQuality          = bestMultiSampleQuality;
	pValidDeviceSettings->pp.SwapEffect                  = bestSwapEffect;
	pValidDeviceSettings->pp.hDeviceWindow               = m_pDeviceObject->_hwnd;
	pValidDeviceSettings->pp.Windowed                    = pBestDeviceSettingsCombo->Windowed;
	pValidDeviceSettings->pp.EnableAutoDepthStencil      = bestEnableAutoDepthStencil;  
	pValidDeviceSettings->pp.AutoDepthStencilFormat      = bestDepthStencilFormat;
	pValidDeviceSettings->pp.Flags                       = dwBestFlags;                   
	pValidDeviceSettings->pp.FullScreen_RefreshRateInHz  = bestDisplayMode.RefreshRate;
	pValidDeviceSettings->pp.PresentationInterval        = bestPresentInterval;
}



//--------------------------------------------------------------------------------------
// Internal helper function to find the closest allowed display mode to the optimal 
//--------------------------------------------------------------------------------------
BOOL DeviceChooser::FindValidResolution( EnumDeviceSettingsCombo* pBestDeviceSettingsCombo, 
								D3DDISPLAYMODE displayModeIn, D3DDISPLAYMODE* pBestDisplayMode )
{
	D3DDISPLAYMODE bestDisplayMode;
	ZeroMemory( &bestDisplayMode, sizeof(D3DDISPLAYMODE) );

	if( pBestDeviceSettingsCombo->Windowed )
	{
		// Get the desktop resolution of the current monitor to use to keep the window
		// in a reasonable size in the desktop's 
		// This isn't the same as the current resolution from GetAdapterDisplayMode
		// since the device might be fullscreen 
		DeviceEnum* pd3dEnum = m_pEnum;
		EnumAdapterInfo* pAdapterInfo = pd3dEnum->GetAdapterInfo( pBestDeviceSettingsCombo->AdapterOrdinal );
		DEVMODE devMode;
		ZeroMemory( &devMode, sizeof(DEVMODE) );
		devMode.dmSize = sizeof(DEVMODE);
		char strDeviceName[256];
		memcpy(strDeviceName,pAdapterInfo->AdapterIdentifier.DeviceName,sizeof(pAdapterInfo->AdapterIdentifier.DeviceName));
		strDeviceName[255] = 0;
		EnumDisplaySettings( strDeviceName, ENUM_REGISTRY_SETTINGS, &devMode);
		UINT nMonitorWidth = devMode.dmPelsWidth;
		UINT nMonitorHeight = devMode.dmPelsHeight;

		// For windowed mode, just keep it something reasonable within the size 
		// of the working area of the desktop
		if( displayModeIn.Width > nMonitorWidth)
			displayModeIn.Width = nMonitorWidth;
		if( displayModeIn.Height > nMonitorHeight)
			displayModeIn.Height = nMonitorHeight;

		*pBestDisplayMode = displayModeIn;
	}
	else
	{
		int nBestRanking = 100000;
		int nCurRanking;
		std::vector<D3DDISPLAYMODE>* pDisplayModeList = &pBestDeviceSettingsCombo->pAdapterInfo->displayModeList;
		for( int iDisplayMode=0; iDisplayMode<pDisplayModeList->size(); iDisplayMode++ )
		{
			D3DDISPLAYMODE displayMode = (*pDisplayModeList)[iDisplayMode];

			// Skip display modes that don't match the combo's adapter format
			if( displayMode.Format != pBestDeviceSettingsCombo->AdapterFormat )
				continue;

			// Find the delta between the current width/height and the optimal width/height
			nCurRanking = abs((int)displayMode.Width - (int)displayModeIn.Width) + 
				abs((int)displayMode.Height- (int)displayModeIn.Height);

			if( nCurRanking < nBestRanking )
			{
				bestDisplayMode = displayMode;
				nBestRanking = nCurRanking;

				// Stop if perfect match found
				if( nBestRanking == 0 )
					break;
			}
		}

		if( bestDisplayMode.Width == 0 )
		{
			*pBestDisplayMode = displayModeIn;
			return FALSE; // No valid display modes found
		}

		*pBestDisplayMode = bestDisplayMode;
	}

	return TRUE;
}
