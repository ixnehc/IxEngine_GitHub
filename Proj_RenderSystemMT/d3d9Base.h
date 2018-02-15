#pragma once


typedef IDirect3D9 XDirect3D;
typedef IDirect3DDevice9 XDirect3DDevice;

typedef IDirect3DIndexBuffer9 XD3DIndexBuffer;
typedef IDirect3DVertexBuffer9 XD3DVertexBuffer;

typedef IDirect3DBaseTexture9 XDirect3DBaseTexture;

typedef IDirect3DSurface9 XDirect3DSurface;

typedef IDirect3DCubeTexture9 XDirect3DCubeTexture;
typedef IDirect3DVolumeTexture9 XDirect3DVolumeTexture;
typedef IDirect3DTexture9 XDirect3DTexture;

typedef IDirect3DVertexDeclaration9 XDirect3DVertexDeclaration;
typedef IDirect3DQuery9 XDirect3DQuery;

typedef ID3DXEffect XD3DXEffect;
typedef ID3DXEffectCompiler XD3DXEffectCompiler;
typedef ID3DXBuffer XD3DXBuffer;
typedef ID3DXMesh XD3DXMesh;

#define XDirect3DCreate Direct3DCreate9

#define XD3DXCreateCubeTexture D3DXCreateCubeTexture
#define XD3DXCreateVolumeTexture D3DXCreateVolumeTexture 
#define XD3DXCreateTexture D3DXCreateTexture 
#define XD3DXCreateTextureFromFileInMemoryEx D3DXCreateTextureFromFileInMemoryEx 
#define XD3DXCreateCubeTextureFromFileInMemoryEx D3DXCreateCubeTextureFromFileInMemoryEx 
#define XD3DXSaveTextureToFileInMemory D3DXSaveTextureToFileInMemory 
#define XD3DXLoadSurfaceFromSurface D3DXLoadSurfaceFromSurface 
#define XD3DXFilterTexture D3DXFilterTexture 
#define XD3DXCreateEffect D3DXCreateEffect 
#define XD3DXCreateEffectCompiler D3DXCreateEffectCompiler 
#define XD3DXDisassembleEffect D3DXDisassembleEffect 
#define XD3DXCreateMeshFVF D3DXCreateMeshFVF 
#define XD3DXComputeIMTFromPerVertexSignal D3DXComputeIMTFromPerVertexSignal 
#define XD3DXUVAtlasCreate D3DXUVAtlasCreate 
#define XD3DXCheckVersion D3DXCheckVersion 
#define XD3DXFillTexture D3DXFillTexture 


#define D3D_FLIP()
#define D3D_FLUSH()

#define DEFINE_DQ()