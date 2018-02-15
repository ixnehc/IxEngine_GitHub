/********************************************************************
	created:	2008/08/14   10:10
	filename: 	d:\IxEngine\Proj_RenderSystemMT\d3d9A\D3DFunctionsA.cpp
	author:		cxi
	
	purpose:	D3D/D3DX functions
*********************************************************************/

#include "stdh.h"

#include "D3DFunctionsA.h"


#include "Direct3DDeviceA.h"
#include "Direct3DA.h"

#include "Direct3DTextureA.h"

#include "D3DXEffectA.h"
#include "D3DXBufferA.h"
#include "D3DXCompilerA.h"
#include "D3DXMeshA.h"




CDirect3D * Direct3DCreateA(UINT SDKVersion)
{
	DQ_Flush();

	IDirect3D9 *p=Direct3DCreate9(SDKVersion);
	if (!p)
		return NULL;

	CDirect3D *d3d=D3D9A_New(CDirect3D);
	d3d->_core=p;

	return d3d;
}

HRESULT D3DXCreateCubeTextureA(CDirect3DDevice *pDevice,UINT Size,
							   UINT MipLevels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,
							   CDirect3DCubeTexture**ppCubeTexture)
{
	*ppCubeTexture=D3D9A_New(CDirect3DCubeTexture);

	(*ppCubeTexture)->_BuildSurfs(6,pDevice,Size,Size,Format);

	DQ_PushFuncType(Func_D3DXCreateCubeTexture);
	DQ_Push4(pDevice);
	DQ_Push4(Size);
	DQ_Push4(MipLevels);
	DQ_Push4(Usage);
	DQ_Push4(Format);
	DQ_Push4(Pool);
	DQ_Push4(*ppCubeTexture);

	return D3D_OK;
}


HRESULT D3DXCreateVolumeTextureA(CDirect3DDevice *pDevice,UINT Width,UINT Height,
								 UINT Depth,UINT MipLevels,DWORD Usage,
								 D3DFORMAT Format,D3DPOOL Pool,
								 CDirect3DVolumeTexture** ppVolumeTexture)
{
	*ppVolumeTexture=D3D9A_New(CDirect3DVolumeTexture);

	DQ_PushFuncType(Func_D3DXCreateVolumeTexture);
	DQ_Push4(pDevice);
	DQ_Push4(Width);
	DQ_Push4(Height);
	DQ_Push4(Depth);
	DQ_Push4(MipLevels);
	DQ_Push4(Usage);
	DQ_Push4(Format);
	DQ_Push4(Pool);
	DQ_Push4(*ppVolumeTexture);

	return D3D_OK;
}

HRESULT D3DXCreateTextureA(CDirect3DDevice *pDevice,UINT Width,UINT Height,
						   UINT MipLevels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,
						   CDirect3DTexture** ppTexture)
{
	*ppTexture=D3D9A_New(CDirect3DTexture);
	(*ppTexture)->_BuildSurfs(1,pDevice,Width,Height,Format);

	DQ_PushFuncType(Func_D3DXCreateTexture);
	DQ_Push4(pDevice);
	DQ_Push4(Width);
	DQ_Push4(Height);
	DQ_Push4(MipLevels);
	DQ_Push4(Usage);
	DQ_Push4(Format);
	DQ_Push4(Pool);
	DQ_Push4(*ppTexture);

	return D3D_OK;

}


HRESULT D3DXCreateTextureFromFileInMemoryExA(
																		CDirect3DDevice *pDevice,
																		LPCVOID                   pSrcData,
																		UINT                      SrcDataSize,
																		UINT                      Width,
																		UINT                      Height,
																		UINT                      MipLevels,
																		DWORD                     Usage,
																		D3DFORMAT                 Format,
																		D3DPOOL                   Pool,
																		DWORD                     Filter,
																		DWORD                     MipFilter,
																		D3DCOLOR                  ColorKey,
																		D3DXIMAGE_INFO*           pSrcInfo,
																		PALETTEENTRY*             pPalette,
																		CDirect3DTexture**      ppTexture)
{
	HRESULT hr;
	IDirect3DTexture9 *tex;
	DQ_Freeze();
	hr=D3DXCreateTextureFromFileInMemoryEx(pDevice->_core,pSrcData,
							SrcDataSize,Width,Height,MipLevels,Usage,Format,Pool,
							Filter,MipFilter,ColorKey,pSrcInfo,pPalette,&tex);

// 	D3DSURFACE_DESC desc;
// 	switch(ti.type)
// 	{
// 	case TEXTYPE_2D:
// 		{
// 			((XDirect3DTexture*)pTex)->GetLevelDesc(0,&desc);
// 			break;
// 		}
// 	case TEXTYPE_CUBIC:
// 		{
// 			((XDirect3DCubeTexture*)pTex)->GetLevelDesc(0,&desc);
// 			break;
// 		}
// 	default:
// 		assert(FALSE);//Ä¿Ç°²»¿¼ÂÇvolume texture
// 		return TRUE;
// 	}

	DQ_UnFreeze();

	if (hr!=D3D_OK)
		return hr;

	*ppTexture=D3D9A_New(CDirect3DTexture);
	(*ppTexture)->_BuildSurfs(1,pDevice,Width,Height,Format);

	(*ppTexture)->_core=tex;
	(*ppTexture)->_FillSurfs(0);

	return hr;
}


HRESULT D3DXSaveTextureToFileInMemoryA(
									   CD3DXBuffer**ppDestBuf,
									   D3DXIMAGE_FILEFORMAT      DestFormat,
									   CDirect3DBaseTexture*  pSrcTexture,
									   CONST PALETTEENTRY*       pSrcPalette)
{
	DQ_Flush();

	HRESULT hr=D3DERR_INVALIDCALL;
	ID3DXBuffer *data;
	if (pSrcTexture->_core)
	{
		hr=D3DXSaveTextureToFileInMemory(&data,DestFormat,pSrcTexture->_core,pSrcPalette);
		if (hr==D3D_OK)
		{
			*ppDestBuf=D3D9A_New(CD3DXBuffer);
			(*ppDestBuf)->_core=data;
		}
	}

	return hr;
}

HRESULT D3DXLoadSurfaceFromSurfaceA(CDirect3DSurface *pDestSurface,CONST PALETTEENTRY*pDestPalette,
									CONST RECT*pDestRect,
									CDirect3DSurface *pSrcSurface,CONST PALETTEENTRY*pSrcPalette,
									CONST RECT*               pSrcRect,
									DWORD                     Filter,
									D3DCOLOR                  ColorKey)
{
	DQ_PushFuncType(Func_D3DXLoadSurfaceFromSurface);
	DQ_Push4(pDestSurface);
	DQ_PushPValue(pDestRect);
	DQ_Push4(pSrcSurface);
	DQ_PushPValue(pSrcRect);
	DQ_Push4(Filter);
	DQ_Push4(ColorKey);

	return D3D_OK;
}


HRESULT D3DXFilterTextureA(CDirect3DBaseTexture*pBaseTexture,CONST PALETTEENTRY*pPalette,
																	   UINT SrcLevel,DWORD Filter)
{
	DQ_PushFuncType(Func_D3DXFilterTexture);
	DQ_Push4(pBaseTexture);
	DQ_Push4(SrcLevel);
	DQ_Push4(Filter);

	return D3D_OK;
}

HRESULT D3DXCreateEffectA(CDirect3DDevice*               pDevice,
													LPCVOID                         pSrcData,
													UINT                            SrcDataLen,
													CONST D3DXMACRO*                pDefines,
													LPD3DXINCLUDE                   pInclude,
													DWORD                           Flags,
													LPD3DXEFFECTPOOL                pPool,
													CD3DXEffect**                   ppEffect,
													CD3DXBuffer**                   ppErrors)
{
	(*ppEffect)=NULL;
	if (ppErrors)
		(*ppErrors)=NULL;

	HRESULT hr;
	DQ_Freeze();

	ID3DXEffect *effect;
	ID3DXBuffer *err=NULL;

	if (ppErrors)
		hr=D3DXCreateEffect(pDevice->_core,pSrcData,SrcDataLen,pDefines,pInclude,Flags,pPool,&effect,&err);
	else
		hr=D3DXCreateEffect(pDevice->_core,pSrcData,SrcDataLen,pDefines,pInclude,Flags,pPool,&effect,NULL);
	if (hr==D3D_OK)
	{
		*ppEffect=D3D9A_New(CD3DXEffect);
		(*ppEffect)->_core=effect;
	}

	if (err)
	{
		if (ppErrors)
		{
			*ppErrors=D3D9A_New(CD3DXBuffer);
			(*ppErrors)->_core=err;
		}
	}


	DQ_UnFreeze();

	return hr;
}

HRESULT D3DXCreateEffectCompilerA(LPCSTR  pSrcData,
																UINT                            SrcDataLen,
																CONST D3DXMACRO*                pDefines,
																LPD3DXINCLUDE                   pInclude,
																DWORD                           Flags,
																CD3DXEffectCompiler**           ppCompiler,
																CD3DXBuffer**                   ppErrors)
{
	(*ppCompiler)=NULL;
	if (ppErrors)
		(*ppErrors)=NULL;

	HRESULT hr;
	DQ_Freeze();

	ID3DXEffectCompiler *compiler;
	ID3DXBuffer *err=NULL;

	if (ppErrors)
		hr=D3DXCreateEffectCompiler(pSrcData,SrcDataLen,pDefines,pInclude,Flags,&compiler,&err);
	else
		hr=D3DXCreateEffectCompiler(pSrcData,SrcDataLen,pDefines,pInclude,Flags,&compiler,NULL);
	if (hr==D3D_OK)
	{
		*ppCompiler=D3D9A_New(CD3DXEffectCompiler);
		(*ppCompiler)->_core=compiler;
	}

	if (err)
	{
		if (ppErrors)
		{
			*ppErrors=D3D9A_New(CD3DXBuffer);
			(*ppErrors)->_core=err;
		}
	}

	DQ_UnFreeze();

	return hr;

}

HRESULT D3DXDisassembleEffectA(CD3DXEffect* pEffect,
									BOOL EnableColorCode,CD3DXBuffer**ppDisassembly)
{
	*ppDisassembly=NULL;
	if (!pEffect->_core)
		return D3DERR_INVALIDCALL;

	HRESULT hr;
	DQ_Freeze();
	ID3DXBuffer *data;
	hr=D3DXDisassembleEffect(pEffect->_core,EnableColorCode,&data);

	if (hr==D3D_OK)
	{
		(*ppDisassembly)=D3D9A_New(CD3DXBuffer);
		(*ppDisassembly)->_core=data;
	}

	DQ_UnFreeze();

	return hr;
}

HRESULT D3DXCreateMeshFVFA(DWORD NumFaces, 
														DWORD NumVertices, 
														DWORD Options, 
														DWORD FVF, 
														CDirect3DDevice* pD3DDevice, 
														CD3DXMesh** ppMesh)
{
	*ppMesh=NULL;
	HRESULT hr;
	DQ_Freeze();

	ID3DXMesh *mesh;
	hr=D3DXCreateMeshFVF(NumFaces,NumVertices,Options,FVF,pD3DDevice->_core,&mesh);
	if (hr==D3D_OK)
	{
		*ppMesh=D3D9A_New(CD3DXMesh);
		(*ppMesh)->_core=mesh;
	}


	DQ_UnFreeze();
	return hr;
}


HRESULT D3DXComputeIMTFromPerVertexSignalA(CD3DXMesh* pMesh,
																CONST FLOAT *pfVertexSignal, // uSignalDimension floats per vertex
																UINT uSignalDimension,
																UINT uSignalStride,         // stride of signal in bytes
																DWORD dwOptions,            // reserved for future use
																LPD3DXUVATLASCB pStatusCallback,
																LPVOID pUserContext,
																CD3DXBuffer**ppIMTData)
{
	*ppIMTData=NULL;
	if (!pMesh->_core)
		return D3DERR_INVALIDCALL;

	HRESULT hr;
	DQ_Freeze();

	ID3DXBuffer *data;
	hr=D3DXComputeIMTFromPerVertexSignal(pMesh->_core,pfVertexSignal,
													uSignalDimension,uSignalStride,dwOptions,
													pStatusCallback,pUserContext,&data);
	if (hr==D3D_OK)
	{
		*ppIMTData=D3D9A_New(CD3DXBuffer);
		(*ppIMTData)->_core=data;
	}

	DQ_UnFreeze();
	return hr;
}


HRESULT D3DXUVAtlasCreateA(CD3DXMesh* pMesh,
														UINT uMaxChartNumber,
														FLOAT fMaxStretch,
														UINT uWidth,
														UINT uHeight,
														FLOAT fGutter,
														DWORD dwTextureIndex,
														CONST DWORD *pdwAdjacency,
														CONST DWORD *pdwFalseEdgeAdjacency,
														CONST FLOAT *pfIMTArray,
														LPD3DXUVATLASCB pStatusCallback,
														FLOAT fCallbackFrequency,
														LPVOID pUserContext,
														CD3DXMesh* *ppMeshOut,
														CD3DXBuffer* *ppFacePartitioning,
														CD3DXBuffer* *ppVertexRemapArray,
														FLOAT *pfMaxStretchOut,
														UINT *puNumChartsOut)
{
	return D3DERR_INVALIDCALL;
// 	*ppMeshOut=NULL;
// 	*ppFacePartitioning=NULL;
// 	*ppVertexRemapArray=NULL;
// 	if (!pMesh->_core)
// 		return D3DERR_INVALIDCALL;
// 
// 	HRESULT hr;
// 	DQ_Freeze();
// 
// 	ID3DXMesh *mesh;
// 	ID3DXBuffer *data1,*data2;
// 	hr=D3DXUVAtlasCreate(pMesh->_core,uMaxChartNumber,fMaxStretch,
// 													uWidth,uHeight,fGutter,dwTextureIndex,
// 													pdwFalseEdgeAdjacency,pdwFalseEdgeAdjacency,
// 													pfIMTArray,pStatusCallback,fCallbackFrequency,pUserContext,
// 													&mesh,&data1,&data2,pfMaxStretchOut,puNumChartsOut);
// 	if (hr==D3D_OK)
// 	{
// 		*ppMeshOut=D3D9A_New(CD3DXMesh);
// 		(*ppMeshOut)->_core=mesh;
// 		*ppFacePartitioning=D3D9A_New(CD3DXBuffer);
// 		(*ppFacePartitioning)->_core=data1;
// 		*ppVertexRemapArray=D3D9A_New(CD3DXBuffer);
// 		(*ppVertexRemapArray)->_core=data2;
// 	}
// 
// 	DQ_UnFreeze();
// 	return hr;
}

BOOL D3DXCheckVersionA(UINT D3DSdkVersion, UINT D3DXSdkVersion)
{
	BOOL b;
	DQ_Freeze();
	b=D3DXCheckVersion(D3DSdkVersion,D3DXSdkVersion);
	DQ_UnFreeze();
	return b;
}

HRESULT D3DXFillTextureA(CDirect3DTexture*pTexture,LPD3DXFILL2D pFunction,LPVOID pData)
{
	DQ_Flush();
	if (!pTexture->_core)
		return D3DERR_INVALIDCALL;

	return D3DXFillTexture((IDirect3DTexture9*)(pTexture->_core),pFunction,pData);
}
