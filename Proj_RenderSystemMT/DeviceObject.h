#pragma once

#include "RenderSystem/IRenderSystem.h"
#include "RenderSystem/IVertexBuffer.h"

#include "ResData/ResDataDefines.h"

#include "base.h"

#include "DeviceEnum.h"

#include "fvfex/fvfex_type.h"

#include <deque>
#include <map>


//--------------------------------------------------------------------------------------
// Structs
//--------------------------------------------------------------------------------------
class DeviceEnum;
class CRenderSystem;

#define MAX_RT_COUNT 4
#define MAX_BIND_VB 4

class VBInfo;
class IBInfo;
struct SurfHandle;

class CDeviceObject
{
	XDirect3D* _d3d;                     // the main D3D object
	XDirect3DDevice* _device;               // the D3D rendering device
	XDirect3DQuery* _event;
	DeviceSettings  _devicesetting;
	D3DCAPS9 _caps;// D3D caps for current device
	DeviceEnum _enum;
	HWND  _hwnd;                  // the main app focus window
	BOOL _bLost;//Device lost
	DeviceConfig _config;

	IRenderSystem *_pRS;

	//RT/DS buffer stack
	struct _MRT
	{
		_MRT()
		{
			memset(this,0,sizeof(*this));
		}
		XDirect3DSurface *surfs[MAX_RT_COUNT];
	};
	std::deque<_MRT>_stackRT;
	BOOL _bMRT;//这个标志指定是否使用了超过一个的RenderTarget
	std::deque<XDirect3DSurface *>_stackDSBuffer;
	DWORD _w,_h;

	//VB/IB bind/draw
	//For VB binding/drawing
	struct _DrawPrimArg
	{
		_DrawPrimArg()
		{
			_bValid=FALSE;
		}
		BOOL _bValid;
		BOOL _bIndexed;
		DWORD _nIndice;
		DWORD _nVertice;
		DWORD _iStreamOff;//In vertex
		int _primstart;
		int _primcount;
		int _ds;//a D3DFILLMODE value
		DWORD _dpt;//a D3DPRIMITIVETYPE value
	};
	struct _FVFExKey
	{
		FVFEx fvfs[MAX_BIND_VB],fvfDraw;
		void Zero()		{			memset(this,0,sizeof(*this));		}
	};
	VBInfo* _vbCur[MAX_BIND_VB+1];//current vb binding to the device
	IBInfo * _ibCur;//Current ib binding to the device
	_FVFExKey _fvfCur;
	_DrawPrimArg _drawarg;
	std::map<_FVFExKey,XDirect3DVertexDeclaration *>_mapDecl;
	VBBindArg _bindarg;



public:
	CDeviceObject();
	~CDeviceObject();
	void Zero();

	BOOL Init(CRenderSystem *pRS);
	BOOL UnInit();//if and leak found,return FALSE,otherwise return TRUE

	//interface overidden 
	void *GetDevice();//return XDirect3DDevice ptr
	BOOL TestConfig(DeviceConfig &config);
	BOOL ResetConfig(DeviceConfig &config);
	BOOL GetConfig(DeviceConfig &config);
	void Clean();
	BOOL Restore();//called when device is lost
	BOOL Present(i_math::recti *rcDest=NULL,i_math::recti *rcSrc=NULL,HWND hwndOverride=NULL);
	BOOL IsLost();
	//For testing
	BOOL TestDraw();


	//State management
	BOOL SetRenderState(D3DRENDERSTATETYPE state, DWORD value)
		{		return (D3D_OK==_device->SetRenderState(state, value));	}
	BOOL BeginScene();
	BOOL EndScene();
	BOOL SetViewport(ViewportInfo &v);

	BOOL PushRenderTarget(SurfHandle *rts,DWORD count=1);
	BOOL PushDSBuffer(SurfHandle &ds);
	void PopRenderTarget();
	void PopDSBuffer();

	BOOL BindVB(VBInfo** vbs,DWORD nVB,IBInfo*ib,VBBindArg *arg);//bind the vertex buffer to the device
	void ClearVBBind();
	BOOL SetVBInstanceCount(DWORD count);
	BOOL DrawPrim();


	void SetAlphaTest(AlphaTestMode mode,DWORD ref);
	void SetBlend(ShaderBlendMode mode);
	void SetStencilOp(StencilMode mode,WORD ref,WORD mask);
	void SetDepthMethod(DepthMode mode);
	void SetFacing(FacingMode mode);
	ShaderState &GetState()	{		return _state;	}
	DeviceCap &GetCap()	{		return _cap;	}

	//helpful functions
	DWORD Width();
	DWORD Height();
	BOOL ColorFill(DWORD color,DWORD iBackBuffer=0,RECT *pRect=NULL);
	BOOL ClearZBuffer(RECT *pRect=NULL);
	BOOL ClearBuffer(ClearBufferFlag flag,RECT *pRect=NULL,D3DCOLOR col=0,float z=1.0f,DWORD s=0);
	BOOL FlushCommand();


	//operations
	IRenderSystem *GetRS()const{return _pRS;}


protected:
	void _SetWindow( HWND hWnd);
	HRESULT _Reset();
	BOOL _ChangeDevice( DeviceSettings* pNewDeviceSettings);
	void _Error( HRESULT hr );
	BOOL _IsWindowed();
	virtual HRESULT _OnCreated(XDirect3DDevice *pDevice);
	virtual HRESULT _OnReset(XDirect3DDevice *pDevice);
	virtual void _OnLost();
	virtual void _OnDestroyed();

	void _TestDraw_SetRenderState(DWORD rs,DWORD v);

public:

//Helper functions
	D3DCOLORVALUE D3DCOLORVALUEFromD3DCOLOR(D3DCOLOR color);
	DWORD GetVertexProcessFlag();//HW,SW,or Mixed
	BOOL SupportVertexShader(DWORD verMajor=1,DWORD verMinor=1);
	BOOL SupportPixelShader(DWORD verMajor=1,DWORD verMinor=1);
	DWORD MaxIndexValue();
	DWORD MaxTextureCoord();
	BOOL CheckDepthStencilMatch(D3DFORMAT fmt,D3DFORMAT fmtDS);
	BOOL SupportDSTexture(D3DFORMAT fmtDS);//
	BOOL SupportNullRT();//

protected:
	//Enumeration callback
	virtual BOOL _OnGetDisplayModeSizeLimit(int &wMin,int &hMin,int &wMax,int &hMax);//return whether the limit is overriden
	virtual BOOL _OnGetDisplayModeRefreshLimit(int &rOefreshMin,int &refreshMax);//return whether the limit is overriden
	virtual BOOL _OnGetPostPixelShaderBlendingRequirement(BOOL &bRequire);//return whether the requirement is overiden
	virtual BOOL _OnGetDeviceAcceptable(D3DCAPS9 *pCaps,D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat, BOOL bWindowed,BOOL &bAcceptable);
	virtual BOOL _OnGetMultisampleQualityMax(UINT &nMultisampleQualityMax);
	//

	//Choose callback
	virtual void _OnChoose(DeviceChooser *pChooser);


	//
	void _CleanStreamSource(DWORD iStart)
	{
		while(_vbCur[iStart])
		{
			_vbCur[iStart]=NULL;
			_device->SetStreamSource(iStart,NULL,0,0);
			iStart++;
		}
	}

	ShaderState _state;
	DeviceCap _cap;



	friend class DeviceEnum;
	friend class DeviceChooser;

	friend bool operator<(const CDeviceObject::_FVFExKey&v1, const CDeviceObject::_FVFExKey&v2);
	friend bool operator==(const CDeviceObject::_FVFExKey&v1, const CDeviceObject::_FVFExKey&v2);

};




