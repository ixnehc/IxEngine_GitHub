#pragma once
#include "d3d9A.h"

#include "devicequeue.h"

class CDirect3D;
class CDirect3DDevice;
class CDirect3DBaseTexture;
class CDirect3DCubeTexture;
class CDirect3DVolumeTexture;
class CDirect3DTexture;
class CDirect3DSurface;
class CD3DXEffectCompiler;
class CD3DXEffect;
class CD3DXBuffer;
class CD3DXMesh;


CDirect3D * Direct3DCreateA(UINT SDKVersion);

HRESULT D3DXCreateCubeTextureA(CDirect3DDevice *pDevice,UINT Size,
							   UINT MipLevels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,
							   CDirect3DCubeTexture**ppCubeTexture);

HRESULT D3DXCreateVolumeTextureA(CDirect3DDevice *pDevice,UINT Width,UINT Height,
								 UINT Depth,UINT MipLevels,DWORD Usage,
								 D3DFORMAT Format,D3DPOOL Pool,
								 CDirect3DVolumeTexture** ppVolumeTexture);

HRESULT D3DXCreateTextureA(CDirect3DDevice *pDevice,UINT Width,UINT Height,
						   UINT MipLevels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,
						   CDirect3DTexture** ppTexture);

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
	CDirect3DTexture**      ppTexture);

HRESULT D3DXSaveTextureToFileInMemoryA(
									   CD3DXBuffer**ppDestBuf,
									   D3DXIMAGE_FILEFORMAT      DestFormat,
									   CDirect3DBaseTexture*  pSrcTexture,
									   CONST PALETTEENTRY*       pSrcPalette);


HRESULT D3DXLoadSurfaceFromSurfaceA(CDirect3DSurface *pDestSurface,CONST PALETTEENTRY*pDestPalette,
									CONST RECT*pDestRect,
									CDirect3DSurface *pSrcSurface,CONST PALETTEENTRY*pSrcPalette,
									CONST RECT*               pSrcRect,
									DWORD                     Filter,
									D3DCOLOR                  ColorKey);

HRESULT D3DXFilterTextureA(
						   CDirect3DBaseTexture*    pBaseTexture,
						   CONST PALETTEENTRY*       pPalette,
						   UINT                      SrcLevel,
						   DWORD                     Filter);

HRESULT D3DXCreateEffectA(
						  CDirect3DDevice*               pDevice,
						  LPCVOID                         pSrcData,
						  UINT                            SrcDataLen,
						  CONST D3DXMACRO*                pDefines,
						  LPD3DXINCLUDE                   pInclude,
						  DWORD                           Flags,
						  LPD3DXEFFECTPOOL                pPool,
						  CD3DXEffect**                   ppEffect,
						  CD3DXBuffer**                   ppCompilationErrors);

HRESULT D3DXCreateEffectCompilerA(
								  LPCSTR                          pSrcData,
								  UINT                            SrcDataLen,
								  CONST D3DXMACRO*                pDefines,
								  LPD3DXINCLUDE                   pInclude,
								  DWORD                           Flags,
								  CD3DXEffectCompiler**           ppCompiler,
								  CD3DXBuffer**                   ppParseErrors);

HRESULT D3DXDisassembleEffectA(
							   CD3DXEffect* pEffect, 
							   BOOL EnableColorCode, 
							   CD3DXBuffer* *ppDisassembly);

HRESULT D3DXCreateMeshFVFA(
						   DWORD NumFaces, 
						   DWORD NumVertices, 
						   DWORD Options, 
						   DWORD FVF, 
						   CDirect3DDevice* pD3DDevice, 
						   CD3DXMesh** ppMesh);

HRESULT D3DXComputeIMTFromPerVertexSignalA(
	CD3DXMesh* pMesh,
	CONST FLOAT *pfVertexSignal, // uSignalDimension floats per vertex
	UINT uSignalDimension,
	UINT uSignalStride,         // stride of signal in bytes
	DWORD dwOptions,            // reserved for future use
	LPD3DXUVATLASCB pStatusCallback,
	LPVOID pUserContext,
	CD3DXBuffer* *ppIMTData);

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
						   UINT *puNumChartsOut);


BOOL D3DXCheckVersionA(UINT D3DSdkVersion, UINT D3DXSdkVersion);

HRESULT D3DXFillTextureA(
						 CDirect3DTexture*        pTexture,
						 LPD3DXFILL2D              pFunction,
						 LPVOID                    pData);
