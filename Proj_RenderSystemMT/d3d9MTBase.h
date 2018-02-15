#pragma once

class CDirect3D;
class CDirect3DDevice;

class CDirect3DIndexBuffer;
class CDirect3DVertexBuffer;

class CDirect3DBaseTexture;

class CDirect3DSurface;

class CDirect3DCubeTexture;
class CDirect3DVolumeTexture;
class CDirect3DTexture;

class CDirect3DVertexDeclaration;
class CDirect3DQuery;

class CD3DXEffect;
class CD3DXEffectCompiler;
class CD3DXBuffer;
class CD3DXMesh;



#include "d3d9a/Direct3DSurfaceA.h"

#include "d3d9a/Direct3DVertexBufferA.h"

#include "d3d9a/Direct3DTextureA.h"

#include "d3d9a/Direct3DVertexDeclarationA.h"

#include "d3d9a/Direct3DQueryA.h"

#include "d3d9a/Direct3DDeviceA.h"

#include "d3d9a/Direct3DA.h"

#include "d3d9a/D3DXEffectA.h"

#include "d3d9a/D3DXBufferA.h"

#include "d3d9A/D3DXCompilerA.h"

#include "d3d9a/D3DXMeshA.h"



typedef CDirect3D XDirect3D;
typedef CDirect3DDevice XDirect3DDevice;

typedef CDirect3DIndexBuffer XD3DIndexBuffer;
typedef CDirect3DVertexBuffer XD3DVertexBuffer;

typedef CDirect3DBaseTexture XDirect3DBaseTexture;

typedef CDirect3DSurface XDirect3DSurface;

typedef CDirect3DCubeTexture XDirect3DCubeTexture;
typedef CDirect3DVolumeTexture XDirect3DVolumeTexture;
typedef CDirect3DTexture XDirect3DTexture;

typedef CDirect3DVertexDeclaration XDirect3DVertexDeclaration;
typedef CDirect3DQuery XDirect3DQuery;

typedef CD3DXEffect XD3DXEffect;
typedef CD3DXEffectCompiler XD3DXEffectCompiler;
typedef CD3DXBuffer XD3DXBuffer;
typedef CD3DXMesh XD3DXMesh;

#include "d3d9a/D3DFunctionsA.h"


#define XDirect3DCreate Direct3DCreateA
#define XD3DXCreateCubeTexture D3DXCreateCubeTextureA
#define XD3DXCreateVolumeTexture D3DXCreateVolumeTextureA
#define XD3DXCreateTexture D3DXCreateTextureA
#define XD3DXCreateTextureFromFileInMemoryEx D3DXCreateTextureFromFileInMemoryExA
#define XD3DXCreateCubeTextureFromFileInMemoryEx D3DXCreateCubeTextureFromFileInMemoryExA 
#define XD3DXSaveTextureToFileInMemory D3DXSaveTextureToFileInMemoryA
#define XD3DXLoadSurfaceFromSurface D3DXLoadSurfaceFromSurfaceA
#define XD3DXFilterTexture D3DXFilterTextureA
#define XD3DXCreateEffect D3DXCreateEffectA
#define XD3DXCreateEffectCompiler D3DXCreateEffectCompilerA
#define XD3DXDisassembleEffect D3DXDisassembleEffectA
#define XD3DXCreateMeshFVF D3DXCreateMeshFVFA
#define XD3DXComputeIMTFromPerVertexSignal D3DXComputeIMTFromPerVertexSignalA
#define XD3DXUVAtlasCreate D3DXUVAtlasCreateA
#define XD3DXCheckVersion D3DXCheckVersionA
#define XD3DXFillTexture D3DXFillTextureA


#include "d3d9a/devicequeue.h"
#define D3D_FLIP() g_dq.Flip()
#define D3D_FLUSH() g_dq.Flush()

#define DEFINE_DQ() CDeviceQueue g_dq;

//当阻塞调用时,会出现的情形:
//					logic					render
//						o							o
//						|							|
//						|							|
//						|							|
//						o							|
//													|
//													|
//													|
//													|
//													o
//						o							o
//						|							|
//						.							|
//													|
//													|
//													|
//													|
//													|
//													|
//													o
//													o
//													|
//													|
//													.
//						|							
//						o
//						o							|									<--这一桢变慢了
//						|							|
//						|							|
//						|							|
//						o							|
//													o
//						o							o
//						|							|
//						|							|
//						|							|
//						o							|
//													|
//													|
//													|
//													|
//													o

