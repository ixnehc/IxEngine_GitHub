#pragma once
#include "d3d9A.h"

#include "devicequeue.h"

#include "Direct3DSurfaceA.h"
#include "Direct3DVertexDeclarationA.h"
#include "Direct3DVertexBufferA.h"
#include "Direct3DTextureA.h"
#include "Direct3DQueryA.h"



#define MAX_RT 4
#define MAX_STREAMSOURCE 8

#define ATTACH_DEVICE(p) (p)->_dev=this;

class CDirect3DSurface;
class CDirect3DVertexDeclaration;
class CDirect3DVertexBuffer;
class CDirect3DIndexBuffer;
class CDirect3DTexture;
class CDirect3DQuery;

class CDynVBPool
{
public:
	CDynVBPool()
	{
		_cVBs=0;
		_cIBs=0;
		_dev=NULL;
	}
	void Init(IDirect3DDevice9 *dev)	{		_dev=dev;	}
	void Clear();
	IDirect3DVertexBuffer9 *AllocVB(DWORD len,DWORD fvf);
	void Free(IDirect3DVertexBuffer9 *vb);
	IDirect3DIndexBuffer9 *AllocIB(DWORD len,DWORD fmt);
	void Free(IDirect3DIndexBuffer9 *ib);


protected:
	IDirect3DDevice9 *_dev;
	struct _VBInfo
	{
		DWORD len;
		DWORD fvf;
		IDirect3DVertexBuffer9 *vb;
		BOOL bAlloc;
	};
	_VBInfo _vbs[1024];
	DWORD _cVBs;
	struct _IBInfo
	{
		DWORD len;
		DWORD fmt;
		IDirect3DIndexBuffer9 *ib;
		BOOL bAlloc;
	};
	_IBInfo _ibs[1024];
	DWORD _cIBs;

};

class CDirect3DDevice
{
public:
	DEFINE_D3D9A_CLASS(CDirect3DDevice)
	IDirect3DDevice9 *_core;
	int _nRef;


	CDynVBPool _pool;

	//cache
	D3DVIEWPORT9 _vp;
	D3DCAPS9 _caps;
	CDirect3DSurface* _rt[MAX_RT];
	CDirect3DSurface* _ds;
	CDirect3DVertexDeclaration *_vdecl;
	CDirect3DVertexBuffer *_vb[MAX_STREAMSOURCE];
	CDirect3DIndexBuffer *_ib;


protected:
	void _ClearCache()
	{
		for (int i=0;i<ARRAY_SIZE(_rt);i++)
			SAFE_RELEASE(_rt[i]);
		SAFE_RELEASE(_ds);
		SAFE_RELEASE(_vdecl);

		for (int i=0;i<ARRAY_SIZE(_vb);i++)
			SAFE_RELEASE(_vb[i]);
		SAFE_RELEASE(_ib);
	}
	void _BuildCache()
	{
		_pool.Init(_core);

		IDirect3DSurface9 *surf=NULL;
		_core->GetDeviceCaps(&_caps);
		_core->GetViewport(&_vp);
		for (int i=0;i<MAX_RT;i++)
		{
			if (D3D_OK==_core->GetRenderTarget(i,&surf))
			{
				_rt[i]=D3D9A_New(CDirect3DSurface);
				ATTACH_DEVICE(_rt[i]);
				surf->GetDesc(&_rt[i]->_desc);

				_rt[i]->_core=surf;
			}
		}
		if (D3D_OK==_core->GetDepthStencilSurface(&surf))
		{
			_ds=D3D9A_New(CDirect3DSurface);
			ATTACH_DEVICE(_ds);
			surf->GetDesc(&_ds->_desc);

			_ds->_core=surf;
		}
	}
public:

	CDirect3DDevice()
	{
		_core=NULL;
		_nRef=1;

		memset(_rt,0,sizeof(_rt));
		_ds=NULL;
		_vdecl=NULL;
		memset(_vb,0,sizeof(_vb));
		_ib=NULL;
	}

	STDMETHOD_(ULONG,AddRef)(THIS)	{		return ++_nRef;	}
	STDMETHOD_(ULONG,Release)(THIS)
	{
		ULONG ret=--_nRef;
		if (_nRef<=0)
		{
			_ClearCache();
			DQ_Flush();

			_pool.Clear();
			SAFE_RELEASE(_core);

			D3D9A_Delete(this);
		}
		return ret;
	}

	STDMETHOD(CreateVertexBuffer)(UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,
								CDirect3DVertexBuffer** vb,HANDLE* pSharedHandle)
	{
		*vb=D3D9A_New(CDirect3DVertexBuffer);
		(*vb)->_dynpool=&_pool;
		(*vb)->_AssignDesc(Length,Usage,FVF,Pool);
		if (Usage&D3DUSAGE_DYNAMIC)
			(*vb)->_bStatic=FALSE;
		else
			(*vb)->_bStatic=TRUE;

		DQ_Freeze();
		if ((*vb)->_bStatic)
			_core->CreateVertexBuffer(Length,Usage,FVF,Pool,&(*vb)->_core,NULL);
		else
			(*vb)->_core=_pool.AllocVB(Length,FVF);
		DQ_UnFreeze();

		return D3D_OK;
	}

	STDMETHOD(CreateIndexBuffer)(UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,
							CDirect3DIndexBuffer**ib,HANDLE* pSharedHandle)
	{
		*ib=D3D9A_New(CDirect3DIndexBuffer);
		(*ib)->_dynpool=&_pool;
		(*ib)->_AssignDesc(Length,Usage,Format,Pool);
		if (Usage&D3DUSAGE_DYNAMIC)
			(*ib)->_bStatic=FALSE;
		else
			(*ib)->_bStatic=TRUE;

		DQ_Freeze();
		if ((*ib)->_bStatic)
			_core->CreateIndexBuffer(Length,Usage,Format,Pool,&(*ib)->_core,NULL);
		else
			(*ib)->_core=_pool.AllocIB(Length,(DWORD)Format);
		DQ_UnFreeze();

		return D3D_OK;
	}

	STDMETHOD(CreateRenderTarget)(UINT Width,UINT Height,D3DFORMAT Format,
								D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Lockable,
								CDirect3DSurface** surf,HANDLE* pSharedHandle)
	{
		*surf=D3D9A_New(CDirect3DSurface);
		ATTACH_DEVICE(*surf);
		(*surf)->_AssignDesc(Width,Height,Format);

		DQ_PushFuncType(Device_CreateRenderTarget);
		DQ_Push4(_core);
		DQ_Push4(Width);
		DQ_Push4(Height);
		DQ_Push4(Format);
		DQ_Push4(*surf);

		return D3D_OK;
	}

	STDMETHOD(CreateDepthStencilSurface)(UINT Width,UINT Height,D3DFORMAT Format,
								D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Discard,
								CDirect3DSurface** surf,HANDLE* pSharedHandle)
	{
		*surf=D3D9A_New(CDirect3DSurface);
		ATTACH_DEVICE(*surf);
		(*surf)->_AssignDesc(Width,Height,Format);

		DQ_PushFuncType(Device_CreateDepthStencilSurface);
		DQ_Push4(_core);
		DQ_Push4(Width);
		DQ_Push4(Height);
		DQ_Push4(Format);
		DQ_Push4(*surf);
		return D3D_OK;
	}

	STDMETHOD(GetRenderTargetData)(THIS_ CDirect3DSurface* pRenderTarget,CDirect3DSurface* pDestSurface)
	{
		DQ_Flush();
		if ((pRenderTarget->_core)&&(pDestSurface->_core))
			return _core->GetRenderTargetData(pRenderTarget->_core,pDestSurface->_core);
		return D3DERR_INVALIDCALL;
	}

	STDMETHOD(CreateOffscreenPlainSurface)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,
											D3DPOOL Pool,CDirect3DSurface** surf,HANDLE* pSharedHandle)
	{
		*surf=D3D9A_New(CDirect3DSurface);
		ATTACH_DEVICE(*surf);
		(*surf)->_AssignDesc(Width,Height,Format);

		DQ_PushFuncType(Device_CreateOffscreenPlainSurface);
		DQ_Push4(_core);
		DQ_Push4(Width);
		DQ_Push4(Height);
		DQ_Push4(Format);
		DQ_Push4(Pool);
		DQ_Push4(*surf);
		return D3D_OK;
	}

	STDMETHOD(BeginScene)(THIS)
	{
		DQ_PushFuncType(Device_BeginScene);
		DQ_Push4(_core);
		return D3D_OK;
	}

	STDMETHOD(EndScene)(THIS)
	{
		DQ_PushFuncType(Device_EndScene);
		DQ_Push4(_core);
		return D3D_OK;
	}

	STDMETHOD(SetTransform)(THIS_ D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix)
	{
		DQ_PushFuncType(Device_SetTransform);
		DQ_Push4(_core);
		DQ_Push4(State);
		DQ_Push(pMatrix,sizeof(*pMatrix));
		return D3D_OK;
	}

	STDMETHOD(Clear)(THIS_ DWORD Count,CONST D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil)
	{
		if(Flags&D3DCLEAR_STENCIL)//如果当前的ds buffer没有stencil,去掉D3DCLEAR_STENCIL的标志
		{
			if (_ds)
			{
				switch(_ds->_desc.Format)
				{
				case D3DFMT_D16_LOCKABLE:
				case D3DFMT_D32:
				case D3DFMT_D24X8:
				case D3DFMT_D16:
					Flags&=(~D3DCLEAR_STENCIL);
				}
			}
		}
		DQ_PushFuncType(Device_Clear);
		DQ_Push4(_core);
		DQ_Push4(Count);
		if (Count>0)
			DQ_Push(pRects,Count*sizeof(D3DRECT));
		DQ_Push4(Flags);
		DQ_Push4(Color);
		DQ_Push4(Z);
		DQ_Push4(Stencil);

		return D3D_OK;
	}


	STDMETHOD(TestCooperativeLevel)(THIS)
	{
		DQ_Flush();
		return _core->TestCooperativeLevel();
	}
	STDMETHOD(EvictManagedResources)(THIS)
	{
		DQ_PushFuncType(Device_EvictManagedResources);
		DQ_Push4(_core);

		return D3D_OK;
	}
	STDMETHOD(GetDeviceCaps)(THIS_ D3DCAPS9* pCaps)
	{
		*pCaps=_caps;

		return D3D_OK;
	}
	STDMETHOD(Reset)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters)
	{
		_ClearCache();
		DQ_Flush();
		g_dq.ClearLost();//清除所有的Lost标志(Flush过程中可能会产生新的lost标志)

		_pool.Clear();
		HRESULT hr=_core->Reset(pPresentationParameters);

		//make some cache
		_BuildCache();

		return hr;
	}

	STDMETHOD(Present)(THIS_ CONST RECT* pSourceRect,CONST RECT* pDestRect,
						HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion)
	{
		DQ_PushFuncType(Device_Present);
		DQ_Push4(_core);

		DQ_PushPValue(pSourceRect);
		DQ_PushPValue(pDestRect);
		DQ_Push4(hDestWindowOverride);

		return g_dq.FetchLost()?D3DERR_DEVICELOST:D3D_OK;
	}

	STDMETHOD(CreateTexture)(THIS_ UINT Width,UINT Height,UINT Levels,
														DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,
														CDirect3DTexture** ppTexture,HANDLE* pSharedHandle)
	{
		*ppTexture=D3D9A_New(CDirect3DTexture);

		DQ_PushFuncType(Device_CreateTexture);
		DQ_Push4(_core);
		DQ_Push4(Width);
		DQ_Push4(Height);
		DQ_Push4(Levels);
		DQ_Push4(Usage);
		DQ_Push4(Format);
		DQ_Push4(Pool);

		DQ_Push4(*ppTexture);

		return D3D_OK;
	}

	STDMETHOD(ColorFill)(THIS_ CDirect3DSurface* pSurface,CONST RECT* pRect,D3DCOLOR color)
	{
		DQ_PushFuncType(Device_ColorFill);
		DQ_Push4(_core);

		DQ_Push4(pSurface);
		DQ_PushPValue(pRect);
		DQ_Push4(color);

		return D3D_OK;
	}

	STDMETHOD(SetRenderTarget)(THIS_ DWORD RenderTargetIndex,CDirect3DSurface* pRenderTarget)
	{
		if (RenderTargetIndex>=MAX_RT)
			return D3DERR_INVALIDCALL;
		SAFE_ADDREF(pRenderTarget);
		SAFE_RELEASE(_rt[RenderTargetIndex]);
		_rt[RenderTargetIndex]=pRenderTarget;

		DQ_PushFuncType(Device_SetRenderTarget);
		DQ_Push4(_core);
		DQ_Push4(RenderTargetIndex);
		DQ_Push4(pRenderTarget);

		return D3D_OK;
	}

	STDMETHOD(GetRenderTarget)(THIS_ DWORD RenderTargetIndex,CDirect3DSurface** ppRenderTarget)
	{
		*ppRenderTarget=NULL;
		if (RenderTargetIndex>=MAX_RT)
			return D3DERR_INVALIDCALL;
		*ppRenderTarget=_rt[RenderTargetIndex];
		SAFE_ADDREF(*ppRenderTarget);
		return D3D_OK;
	}
	STDMETHOD(SetDepthStencilSurface)(THIS_ CDirect3DSurface* pNewZStencil)
	{
		SAFE_ADDREF(pNewZStencil);
		SAFE_RELEASE(_ds);
		_ds=pNewZStencil;

		DQ_PushFuncType(Device_SetDepthStencilSurface);
		DQ_Push4(_core);
		DQ_Push4(pNewZStencil);

		return D3D_OK;
	}

	STDMETHOD(GetDepthStencilSurface)(THIS_ CDirect3DSurface** ppZStencilSurface)
	{
		*ppZStencilSurface=_ds;
		SAFE_ADDREF(*ppZStencilSurface);
		return D3D_OK;
	}

	STDMETHOD(SetViewport)(THIS_ CONST D3DVIEWPORT9* pViewport)
	{
		_vp=*pViewport;

		DQ_PushFuncType(Device_SetViewPort);
		DQ_Push4(_core);
		DQ_Push(pViewport,sizeof(D3DVIEWPORT9));

		return D3D_OK;
	}

	STDMETHOD(GetViewport)(THIS_ D3DVIEWPORT9* pViewport)
	{
		*pViewport=_vp;
		return D3D_OK;
	}

	STDMETHOD(SetRenderState)(THIS_ D3DRENDERSTATETYPE State,DWORD Value)
	{
		DQ_PushFuncType(Device_SetRenderState);
		DQ_Push4(_core);

		DQ_Push4(State);
		DQ_Push4(Value);

		return D3D_OK;
	}

	STDMETHOD(DrawPrimitive)(THIS_ D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount)
	{
		DQ_PushFuncType(Device_DrawPrimitive);
		DQ_Push4(_core);
		DQ_Push4(PrimitiveType);
		DQ_Push4(StartVertex);
		DQ_Push4(PrimitiveCount);

		return D3D_OK;
	}

	STDMETHOD(DrawIndexedPrimitive)(THIS_ D3DPRIMITIVETYPE PrimitiveType,INT BaseVertexIndex,
									UINT MinVertexIndex,UINT NumVertices,UINT startIndex,UINT primCount)
	{
		DQ_PushFuncType(Device_DrawIndexedPrimitive);
		DQ_Push4(_core);
		DQ_Push4(PrimitiveType);
		DQ_Push4(BaseVertexIndex);
		DQ_Push4(MinVertexIndex);
		DQ_Push4(NumVertices);
		DQ_Push4(startIndex);
		DQ_Push4(primCount);

		return D3D_OK;
	}

	STDMETHOD(CreateVertexDeclaration)(THIS_ CONST D3DVERTEXELEMENT9* pVertexElements,
															CDirect3DVertexDeclaration** ppDecl)
	{
		DWORD c=1;
		D3DVERTEXELEMENT9*p=(D3DVERTEXELEMENT9*)pVertexElements;
		while(1)
		{
			if(p->Stream==0xff)
				break;
			p++;
			c++;
		}

		*ppDecl=D3D9A_New(CDirect3DVertexDeclaration);

		DQ_PushFuncType(Device_CreateVertexDeclaration);
		DQ_Push4(_core);
		DQ_Push4(c);
		DQ_Push(pVertexElements,sizeof(D3DVERTEXELEMENT9)*c);

		DQ_Push4(*ppDecl);

		return D3D_OK;
	}

	STDMETHOD(SetVertexDeclaration)(THIS_ CDirect3DVertexDeclaration* pDecl)
	{
		SAFE_ADDREF(pDecl);
		SAFE_RELEASE(_vdecl);
		_vdecl=pDecl;

		DQ_PushFuncType(Device_SetVertexDeclaration);
		DQ_Push4(_core);

		DQ_Push4(pDecl);

		return D3D_OK;
	}

	STDMETHOD(SetFVF)(THIS_ DWORD FVF)
	{
		DQ_PushFuncType(Device_SetFVF);
		DQ_Push4(_core);
		DQ_Push4(FVF);

		return D3D_OK;
	}

	STDMETHOD(SetStreamSource)(THIS_ UINT StreamNumber,CDirect3DVertexBuffer* pStreamData,
																	UINT OffsetInBytes,UINT Stride)
	{
		SAFE_ADDREF(pStreamData);
		SAFE_RELEASE(_vb[StreamNumber]);
		_vb[StreamNumber]=pStreamData;

		DQ_PushFuncType(Device_SetStreamSource);
		DQ_Push4(_core);
		DQ_Push4(StreamNumber);
		if (pStreamData)
		{
			DQ_Push4(pStreamData->_core);
		}
		else
		{
			DQ_Push4C(NULL);
		}
		DQ_Push4(OffsetInBytes);
		DQ_Push4(Stride);

		return D3D_OK;
	}

	STDMETHOD(SetIndices)(THIS_ CDirect3DIndexBuffer* pIndexData)
	{
		SAFE_ADDREF(pIndexData);
		SAFE_RELEASE(_ib);
		_ib=pIndexData;

		DQ_PushFuncType(Device_SetIndices);
		DQ_Push4(_core);
		if (pIndexData)
		{
			DQ_Push4(pIndexData->_core);
		}
		else
		{
			DQ_Push4C(NULL);
		}

		return D3D_OK;
	}

	STDMETHOD(CreateQuery)(THIS_ D3DQUERYTYPE Type,CDirect3DQuery** ppQuery)
	{
		*ppQuery=D3D9A_New(CDirect3DQuery);

		DQ_PushFuncType(Device_CreateQuery);
		DQ_Push4(_core);
		DQ_Push4(Type);
		DQ_Push4(*ppQuery);

		return D3D_OK;
	}


	//	STDMETHOD(SetCursorProperties)(THIS_ UINT XHotSpot,UINT YHotSpot,IDirect3DSurface9* pCursorBitmap);
	//	STDMETHOD_(void, SetCursorPosition)(THIS_ int X,int Y,DWORD Flags);
	//	STDMETHOD_(BOOL, ShowCursor)(THIS_ BOOL bShow);
	//	STDMETHOD(CreateAdditionalSwapChain)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DSwapChain9** pSwapChain);
	//	STDMETHOD(GetSwapChain)(THIS_ UINT iSwapChain,IDirect3DSwapChain9** pSwapChain);
	//	STDMETHOD_(UINT, GetNumberOfSwapChains)(THIS);
	//	STDMETHOD(GetRasterStatus)(THIS_ UINT iSwapChain,D3DRASTER_STATUS* pRasterStatus);
	//	STDMETHOD(SetDialogBoxMode)(THIS_ BOOL bEnableDialogs);
	//	STDMETHOD_(void, SetGammaRamp)(THIS_ UINT iSwapChain,DWORD Flags,CONST D3DGAMMARAMP* pRamp);
	//	STDMETHOD_(void, GetGammaRamp)(THIS_ UINT iSwapChain,D3DGAMMARAMP* pRamp);
	//	STDMETHOD(CreateVolumeTexture)(THIS_ UINT Width,UINT Height,UINT Depth,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DVolumeTexture9** ppVolumeTexture,HANDLE* pSharedHandle);
	//	STDMETHOD(CreateCubeTexture)(THIS_ UINT EdgeLength,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DCubeTexture9** ppCubeTexture,HANDLE* pSharedHandle);
	//	STDMETHOD(CreateVertexBuffer)(THIS_ UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer9** ppVertexBuffer,HANDLE* pSharedHandle);
	//	STDMETHOD(CreateIndexBuffer)(THIS_ UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer9** ppIndexBuffer,HANDLE* pSharedHandle);
	//	STDMETHOD(CreateRenderTarget)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Lockable,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle);
	//	STDMETHOD(CreateDepthStencilSurface)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Discard,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle);
	//	STDMETHOD(UpdateSurface)(THIS_ IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestinationSurface,CONST POINT* pDestPoint);
	//	STDMETHOD(UpdateTexture)(THIS_ IDirect3DBaseTexture9* pSourceTexture,IDirect3DBaseTexture9* pDestinationTexture);
	//	STDMETHOD(GetFrontBufferData)(THIS_ UINT iSwapChain,IDirect3DSurface9* pDestSurface);
	//	STDMETHOD(StretchRect)(THIS_ IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestSurface,CONST RECT* pDestRect,D3DTEXTUREFILTERTYPE Filter);
	//	STDMETHOD(GetTransform)(THIS_ D3DTRANSFORMSTATETYPE State,D3DMATRIX* pMatrix);
	//	STDMETHOD(MultiplyTransform)(THIS_ D3DTRANSFORMSTATETYPE,CONST D3DMATRIX*);
	//	STDMETHOD(SetMaterial)(THIS_ CONST D3DMATERIAL9* pMaterial);
	//	STDMETHOD(GetMaterial)(THIS_ D3DMATERIAL9* pMaterial);
	//	STDMETHOD(SetLight)(THIS_ DWORD Index,CONST D3DLIGHT9*);
	//	STDMETHOD(GetLight)(THIS_ DWORD Index,D3DLIGHT9*);
	//	STDMETHOD(LightEnable)(THIS_ DWORD Index,BOOL Enable);
	//	STDMETHOD(GetLightEnable)(THIS_ DWORD Index,BOOL* pEnable);
	//	STDMETHOD(SetClipPlane)(THIS_ DWORD Index,CONST float* pPlane);
	//	STDMETHOD(GetClipPlane)(THIS_ DWORD Index,float* pPlane);
	//	STDMETHOD(GetRenderState)(THIS_ D3DRENDERSTATETYPE State,DWORD* pValue);
	//	STDMETHOD(CreateStateBlock)(THIS_ D3DSTATEBLOCKTYPE Type,IDirect3DStateBlock9** ppSB);
	//	STDMETHOD(BeginStateBlock)(THIS);
	//	STDMETHOD(EndStateBlock)(THIS_ IDirect3DStateBlock9** ppSB);
	//	STDMETHOD(SetClipStatus)(THIS_ CONST D3DCLIPSTATUS9* pClipStatus);
	//	STDMETHOD(GetClipStatus)(THIS_ D3DCLIPSTATUS9* pClipStatus);
	//	STDMETHOD(GetTexture)(THIS_ DWORD Stage,IDirect3DBaseTexture9** ppTexture);
	//	STDMETHOD(SetTexture)(THIS_ DWORD Stage,IDirect3DBaseTexture9* pTexture);
	//	STDMETHOD(GetTextureStageState)(THIS_ DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD* pValue);
	//	STDMETHOD(GetSamplerState)(THIS_ DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD* pValue);
	//	STDMETHOD(ValidateDevice)(THIS_ DWORD* pNumPasses);
	//	STDMETHOD(SetPaletteEntries)(THIS_ UINT PaletteNumber,CONST PALETTEENTRY* pEntries);
	//	STDMETHOD(GetPaletteEntries)(THIS_ UINT PaletteNumber,PALETTEENTRY* pEntries);
	//	STDMETHOD(SetCurrentTexturePalette)(THIS_ UINT PaletteNumber);
	//	STDMETHOD(GetCurrentTexturePalette)(THIS_ UINT *PaletteNumber);
	//	STDMETHOD(SetScissorRect)(THIS_ CONST RECT* pRect);
	//	STDMETHOD(GetScissorRect)(THIS_ RECT* pRect);
	//	STDMETHOD(SetSoftwareVertexProcessing)(THIS_ BOOL bSoftware);
	//	STDMETHOD(SetNPatchMode)(THIS_ float nSegments);
	//	STDMETHOD_(float, GetNPatchMode)(THIS);
	//	STDMETHOD(GetIndices)(THIS_ IDirect3DIndexBuffer9** ppIndexData);
	//	STDMETHOD(CreatePixelShader)(THIS_ CONST DWORD* pFunction,IDirect3DPixelShader9** ppShader);
	//	STDMETHOD(SetPixelShader)(THIS_ IDirect3DPixelShader9* pShader);
	//	STDMETHOD(GetPixelShader)(THIS_ IDirect3DPixelShader9** ppShader);
	//	STDMETHOD(SetPixelShaderConstantF)(THIS_ UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount);
	//	STDMETHOD(GetPixelShaderConstantF)(THIS_ UINT StartRegister,float* pConstantData,UINT Vector4fCount);
	//	STDMETHOD(SetPixelShaderConstantI)(THIS_ UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount);
	//	STDMETHOD(GetPixelShaderConstantI)(THIS_ UINT StartRegister,int* pConstantData,UINT Vector4iCount);
	//	STDMETHOD(SetPixelShaderConstantB)(THIS_ UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount);
	//	STDMETHOD(GetPixelShaderConstantB)(THIS_ UINT StartRegister,BOOL* pConstantData,UINT BoolCount);
	//	STDMETHOD(DrawRectPatch)(THIS_ UINT Handle,CONST float* pNumSegs,CONST D3DRECTPATCH_INFO* pRectPatchInfo);
	//	STDMETHOD(DrawTriPatch)(THIS_ UINT Handle,CONST float* pNumSegs,CONST D3DTRIPATCH_INFO* pTriPatchInfo);
	//	STDMETHOD(DeletePatch)(THIS_ UINT Handle);
	//	STDMETHOD(DrawPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType,UINT PrimitiveCount,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride);
	//	STDMETHOD(DrawIndexedPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType,UINT MinVertexIndex,UINT NumVertices,UINT PrimitiveCount,CONST void* pIndexData,D3DFORMAT IndexDataFormat,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride);
	//	STDMETHOD(ProcessVertices)(THIS_ UINT SrcStartIndex,UINT DestIndex,UINT VertexCount,IDirect3DVertexBuffer9* pDestBuffer,IDirect3DVertexDeclaration9* pVertexDecl,DWORD Flags);
	//	STDMETHOD(GetVertexDeclaration)(THIS_ IDirect3DVertexDeclaration9** ppDecl);
	//	STDMETHOD(GetFVF)(THIS_ DWORD* pFVF);
	//	STDMETHOD(CreateVertexShader)(THIS_ CONST DWORD* pFunction,IDirect3DVertexShader9** ppShader);
	//	STDMETHOD(SetVertexShader)(THIS_ IDirect3DVertexShader9* pShader);
	//	STDMETHOD(GetVertexShader)(THIS_ IDirect3DVertexShader9** ppShader);
	//	STDMETHOD(SetVertexShaderConstantF)(THIS_ UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount);
	//	STDMETHOD(GetVertexShaderConstantF)(THIS_ UINT StartRegister,float* pConstantData,UINT Vector4fCount);
	//	STDMETHOD(SetVertexShaderConstantI)(THIS_ UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount);
	//	STDMETHOD(GetVertexShaderConstantI)(THIS_ UINT StartRegister,int* pConstantData,UINT Vector4iCount);
	//	STDMETHOD(SetVertexShaderConstantB)(THIS_ UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount);
	//	STDMETHOD(GetVertexShaderConstantB)(THIS_ UINT StartRegister,BOOL* pConstantData,UINT BoolCount);
	//	STDMETHOD(GetStreamSource)(THIS_ UINT StreamNumber,IDirect3DVertexBuffer9** ppStreamData,UINT* pOffsetInBytes,UINT* pStride);
	//	STDMETHOD(SetStreamSourceFreq)(THIS_ UINT StreamNumber,UINT Setting);
	//	STDMETHOD(GetStreamSourceFreq)(THIS_ UINT StreamNumber,UINT* pSetting);

	friend class CDirect3D;
};
