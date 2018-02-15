#pragma once

#include "Base.h"

#include <vector>

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
class EnumAdapterInfo;
class EnumDeviceInfo;
struct EnumDeviceSettingsCombo;
struct EnumDSMSConflict;

class CDeviceObject;

//--------------------------------------------------------------------------------------
// Enumerates available Direct3D adapters, devices, modes, etc.
// Use D3D9GetEnumeration() to access global instance
//--------------------------------------------------------------------------------------
class DeviceEnum
{
private:
	// These should be called before Enumerate(). 
	//
	// Use these calls and the IsDeviceAcceptable to control the contents of 
	// the enumeration object, which affects the device selection and the device settings dialog.
	void SetRequirePostPixelShaderBlending( bool bRequire ) { m_bRequirePostPixelShaderBlending = bRequire; }
	void SetResolutionMinMax( UINT nMinWidth, UINT nMinHeight, UINT nMaxWidth, UINT nMaxHeight );  
	void SetRefreshMinMax( UINT nMin, UINT nMax );
	void SetMultisampleQualityMax( UINT nMax );    
	void GetPossibleVertexProcessingList( bool* pbSoftwareVP, bool* pbHardwareVP, bool* pbPureHarewareVP, bool* pbMixedVP );
	void SetPossibleVertexProcessingList( bool bSoftwareVP, bool bHardwareVP, bool bPureHarewareVP, bool bMixedVP );
	std::vector<D3DFORMAT>* GetPossibleDepthStencilFormatList();   
	std::vector<D3DMULTISAMPLE_TYPE>* GetPossibleMultisampleTypeList();   
	std::vector<UINT>* GetPossiblePresentIntervalList();
	void ResetPossibleDepthStencilFormats();
	void ResetPossibleMultisampleTypeList();
	void ResetPossiblePresentIntervalList();

	// Call Enumerate() to enumerate available D3D adapters, devices, modes, etc.
	HRESULT Enumerate(CDeviceObject *pD3D9Base);

	// These should be called after Enumerate() is called
	std::vector<EnumAdapterInfo*>*   GetAdapterInfoList();  
	EnumAdapterInfo*                    GetAdapterInfo( UINT AdapterOrdinal );  
	EnumDeviceInfo*                     GetDeviceInfo( UINT AdapterOrdinal, D3DDEVTYPE DeviceType );    
	EnumDeviceSettingsCombo*            GetDeviceSettingsCombo( DeviceSettings* pDeviceSettings ) { return GetDeviceSettingsCombo( pDeviceSettings->AdapterOrdinal, pDeviceSettings->DeviceType, pDeviceSettings->AdapterFormat, pDeviceSettings->pp.BackBufferFormat, pDeviceSettings->pp.Windowed ); }
	EnumDeviceSettingsCombo*            GetDeviceSettingsCombo( UINT AdapterOrdinal, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL Windowed );  

	~DeviceEnum();

	void Reset();


	DeviceEnum();

	XDirect3D* m_pD3D;                                    
	CDeviceObject *m_pDeviceObject;

	bool m_bRequirePostPixelShaderBlending;
	std::vector<D3DFORMAT> m_DepthStecilPossibleList;
	std::vector<D3DMULTISAMPLE_TYPE> m_MultiSampleTypeList;
	std::vector<UINT> m_PresentIntervalList;

	bool m_bSoftwareVP;
	bool m_bHardwareVP;
	bool m_bPureHarewareVP;
	bool m_bMixedVP;

	UINT m_nMinWidth;
	UINT m_nMaxWidth;
	UINT m_nMinHeight;
	UINT m_nMaxHeight;
	UINT m_nRefreshMin;
	UINT m_nRefreshMax;
	UINT m_nMultisampleQualityMax;

	// Array of EnumAdapterInfo* with unique AdapterOrdinals
	std::vector<EnumAdapterInfo*> m_AdapterInfoList;  

	HRESULT EnumerateDevices( EnumAdapterInfo* pAdapterInfo, std::vector<D3DFORMAT>* pAdapterFormatList );
	HRESULT EnumerateDeviceCombos( EnumAdapterInfo* pAdapterInfo, EnumDeviceInfo* pDeviceInfo, std::vector<D3DFORMAT>* pAdapterFormatList );
	void BuildDepthStencilFormatList( EnumDeviceSettingsCombo* pDeviceCombo );
	void BuildMultiSampleTypeList( EnumDeviceSettingsCombo* pDeviceCombo );
	void BuildDSMSConflictList( EnumDeviceSettingsCombo* pDeviceCombo );
	void BuildVertexProcessingTypeList( EnumDeviceInfo* pDeviceInfo, EnumDeviceSettingsCombo* pDeviceCombo );
	void BuildPresentIntervalList( EnumDeviceInfo* pDeviceInfo, EnumDeviceSettingsCombo* pDeviceCombo );
	void ClearAdapterInfoList();

	friend class CDeviceObject;
	friend class DeviceChooser;
};


