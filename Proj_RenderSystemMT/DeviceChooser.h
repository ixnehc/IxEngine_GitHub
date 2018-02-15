#pragma once
#include "Base.h"
#include <vector>


class CDeviceObject;

enum ChooseType
{
	Choose_UseDefault= 0,  // Use the closest valid value to a default 
	Choose_PreserveInput,    // Use input without change, but may cause no valid device to be found
	Choose_ClosestToInput   // Use the closest valid value to the input 
};



struct ChooseOptions
{
	ChooseType eAdapterOrdinal;
	ChooseType eDeviceType;
	ChooseType eWindowed;
	ChooseType eAdapterFormat;
	ChooseType eVertexProcessing;
	ChooseType eResolution;
	ChooseType eBackBufferFormat;
	ChooseType eBackBufferCount;
	ChooseType eMultiSample;
	ChooseType eSwapEffect;
	ChooseType eDepthFormat;
	ChooseType eStencilFormat;
	ChooseType ePresentFlags;
	ChooseType eRefreshRate;
	ChooseType ePresentInterval;
};



class EnumDeviceInfo;
struct EnumDSMSConflict;
class EnumAdapterInfo;
struct EnumDeviceSettingsCombo;
struct EnumDSMSConflict
{
	D3DFORMAT DSFormat;
	D3DMULTISAMPLE_TYPE MSType;
};
//--------------------------------------------------------------------------------------
// A class describing a Direct3D device that contains a 
//       unique supported device type 
//--------------------------------------------------------------------------------------
class EnumDeviceInfo
{
public:
	~EnumDeviceInfo();

	UINT AdapterOrdinal;
	D3DDEVTYPE DeviceType;
	D3DCAPS9 Caps;

	// List of EnumDeviceSettingsCombo* with a unique set 
	// of AdapterFormat, BackBufferFormat, and Windowed
	std::vector<EnumDeviceSettingsCombo*> deviceSettingsComboList; 
};

//--------------------------------------------------------------------------------------
// A class describing an adapter which contains a unique adapter ordinal 
// that is installed on the system
//--------------------------------------------------------------------------------------
class EnumAdapterInfo
{
public:
	~EnumAdapterInfo();

	UINT AdapterOrdinal;
	D3DADAPTER_IDENTIFIER9 AdapterIdentifier;
	char szUniqueDescription[MAX_DEVICE_IDENTIFIER_STRING+32];

	std::vector<D3DDISPLAYMODE> displayModeList; // Array of supported D3DDISPLAYMODEs
	std::vector<EnumDeviceInfo*> deviceInfoList; // Array of EnumDeviceInfo* with unique supported DeviceTypes
};



struct EnumDeviceSettingsCombo
{
	UINT AdapterOrdinal;
	D3DDEVTYPE DeviceType;
	D3DFORMAT AdapterFormat;
	D3DFORMAT BackBufferFormat;
	BOOL Windowed;

	std::vector<D3DFORMAT> depthStencilFormatList; // List of D3DFORMATs
	std::vector<D3DMULTISAMPLE_TYPE> multiSampleTypeList; // List of D3DMULTISAMPLE_TYPEs
	std::vector<DWORD> multiSampleQualityList; // List of number of quality levels for each multisample type
	std::vector<UINT> presentIntervalList; // List of D3DPRESENT flags
	std::vector<EnumDSMSConflict> DSMSConflictList; // List of EnumDSMSConflict

	EnumAdapterInfo* pAdapterInfo;
	EnumDeviceInfo* pDeviceInfo;
};






class CDeviceObject;
class DeviceEnum;
struct DeviceSettings;


class DeviceChooser
{
public:
	//The request on choice
	void SetRequest_AdapterOrdinal(DWORD ordinalAdapter,BOOL bForce=FALSE);
	void SetRequest_DeviceType(D3DDEVTYPE typeDevice,BOOL bForce=FALSE);
	void SetRequest_AdapterFormat(D3DFORMAT format,BOOL bForce=FALSE);
	void SetRequest_FullScreen(BOOL bFullScreen=TRUE);
	void SetRequest_VertexProcessingType(DWORD typeVertexProcessing,BOOL bForce=FALSE);
	void SetRequest_Resolution(DWORD w,DWORD h);
	void SetRequest_BackBufferFormat(D3DFORMAT format,BOOL bForce=FALSE);
	void SetRequest_BackBufferCount(int n,BOOL bForce=FALSE);
	void SetRequest_MultiSample(D3DMULTISAMPLE_TYPE type,DWORD quality,BOOL bForce=FALSE);
	void SetRequest_SwapEffect(D3DSWAPEFFECT swapeffect,BOOL bForce=FALSE);
	void SetRequest_DepthStencilFormat(D3DFORMAT format,BOOL bForce=FALSE);
	void SetRequest_PresentFlags(DWORD flag,BOOL bForce=FALSE);
	void SetRequest_PresentInterval(DWORD interval,BOOL bForce=FALSE);
	void SetRequest_RefreshRate(DWORD RefreshRate);

private:
	DeviceChooser()
	{
		Reset();
	}
	void Reset()
	{
		memset(&m_ChooseOption,0,sizeof(m_ChooseOption));//All set to default
		memset(&m_RequestDeviceSetting,0,sizeof(m_RequestDeviceSetting));

		m_pDeviceObject=NULL;
		m_pEnum=NULL;
	}
	BOOL Choose(CDeviceObject *pD3D9Base,DeviceEnum *pEnum,DeviceSettings &devicesetting);//Return whether successfully choose one device setting.
	ChooseOptions m_ChooseOption;
	DeviceSettings m_RequestDeviceSetting;

	CDeviceObject *m_pDeviceObject;
	DeviceEnum *m_pEnum;

	void BuildOptimalDeviceSettings( DeviceSettings* pOptimalDeviceSettings, 
		DeviceSettings* pDeviceSettingsIn, 
		ChooseOptions* pMatchOptions );
	BOOL DoesDeviceComboMatchPreserveOptions( EnumDeviceSettingsCombo* pDeviceSettingsCombo, 
		DeviceSettings* pDeviceSettingsIn, 
		ChooseOptions* pMatchOptions );
	float RankDeviceCombo( EnumDeviceSettingsCombo* pDeviceSettingsCombo, 
		DeviceSettings* pOptimalDeviceSettings,
		D3DDISPLAYMODE* pAdapterDesktopDisplayMode );

	BOOL FindValidResolution( EnumDeviceSettingsCombo* pBestDeviceSettingsCombo, 
		D3DDISPLAYMODE displayModeIn, D3DDISPLAYMODE* pBestDisplayMode );

	void BuildValidDeviceSettings( DeviceSettings* pValidDeviceSettings, 
		EnumDeviceSettingsCombo* pBestDeviceSettingsCombo, 
		DeviceSettings* pDeviceSettingsIn, 
		ChooseOptions* pMatchOptions );


	friend class CDeviceObject;
	friend class DeviceEnum;

};

