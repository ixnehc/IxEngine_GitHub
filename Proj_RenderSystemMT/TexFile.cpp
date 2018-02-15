/********************************************************************
	created:	2006/08/01
	created:	1:8:2006   14:17
	filename: 	e:\IxEngine\Proj_RenderSystem\TexFile.cpp
	file base:	TexFile
	file ext:	cpp
	author:		cxi
	
	purpose:	functions for reading/writing a tex file
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "interface/interface.h"

#include "FileSystem/IFileSystem.h"
#include "RenderSystem/IRenderSystemDefines.h"
#include "RenderSystem/ITexture.h"


#include "assert.h"

#include "Log/LastError.h"
#include "Log/LogFile.h"

#include "datapacket/DataPacket.h"

#include "stringparser/stringparser.h"
#include "ijlApi/ijlApi.h"
#include "texCompress/3dcDecode.h"
#include "resdata/ResDataDefines.h"
#include "resdata/TexData.h"

#include <d3d9.h>
#include <d3dx9.h>

#pragma warning(disable:4018)

LogFile g_logFile("TexFile");



DWORD d3dfmtBitSize(D3DFORMAT fmt)
{
	switch(fmt)
	{
	case D3DFMT_UNKNOWN: return 0;
	case D3DFMT_A4L4: return 8;
	case D3DFMT_A8: return 8;
	case D3DFMT_L8: return 8;
	case D3DFMT_P8: return 8;
	case D3DFMT_R3G3B2: return 8;
	case D3DFMT_A1R5G5B5: return 16;
	case D3DFMT_A4R4G4B4: return 16;
	case D3DFMT_A8L8: return 16;
	case D3DFMT_A8P8: return 16;
	case D3DFMT_A8R3G3B2: return 16;
	case D3DFMT_L16: return 16;
	case D3DFMT_R5G6B5: return 16;
	case D3DFMT_X1R5G5B5: return 16;
	case D3DFMT_X4R4G4B4: return 16;
	case D3DFMT_R8G8B8: return 24;
	case D3DFMT_A2B10G10R10: return 32;
	case D3DFMT_A2R10G10B10: return 32;
	case D3DFMT_A8B8G8R8: return 32;
	case D3DFMT_A8R8G8B8: return 32;
	case D3DFMT_G16R16: return 32;
	case D3DFMT_X8B8G8R8: return 32;
	case D3DFMT_X8R8G8B8: return 32;
	case D3DFMT_A16B16G16R16: return 64;

	case D3DFMT_CxV8U8: return 16;
	case D3DFMT_V8U8: return 16;
	case D3DFMT_Q8W8V8U8: return 32;
	case D3DFMT_V16U16: return 32;
	case D3DFMT_Q16W16V16U16: return 64;

	case D3DFMT_L6V5U5: return 16;
	case D3DFMT_A2W10V10U10: return 32;
	case D3DFMT_X8L8V8U8: return 32;

	case D3DFMT_DXT1: return 4;
	case D3DFMT_DXT2: return 8;
	case D3DFMT_DXT3: return 8;
	case D3DFMT_DXT4: return 8;
	case D3DFMT_DXT5: return 8;
	case D3DFMT_G8R8_G8B8: return 16;
	case D3DFMT_R8G8_B8G8: return 16;
	case D3DFMT_UYVY: return 16;
	case D3DFMT_YUY2: return 16;

	case D3DFMT_R16F: return 16;
	case D3DFMT_G16R16F: return 32;
	case D3DFMT_A16B16G16R16F: return 64;

	case D3DFMT_R32F: return 32;
	case D3DFMT_G32R32F: return 64;
	case D3DFMT_A32B32G32R32F: return 128;

	default: return 0;
	}
}

DWORD CalcTexInfoSize(TexInfo &ti)
{
	DWORD nSize;
	nSize=ti.width*ti.height*d3dfmtBitSize((D3DFORMAT)ti.fmt)/8;
	nSize*=2;
	if (ti.miplevel!=0)
		nSize-=nSize/2/(ti.miplevel);
	else
		nSize-=nSize/2/1;


	if (ti.type==TEXTYPE_CUBIC)
		nSize*=6;
	if (ti.type==TEXTYPE_3D)
		nSize*=ti.depth;

	return nSize;
}

DWORD CalcFullMipLevel(DWORD w,DWORD h)
{
	if (w<h)
		w=h;
	return i_math::fastlog2(w);
}



struct TGAHeader 
{
	// byte descriptionlen;
	byte  cmaptype;
	byte  imagetype;
	WORD cmapstart;
	WORD cmapentries;
	byte  cmapbits;
	WORD xoffset;
	WORD yoffset;
	WORD width;
	WORD height;
	byte  bpp;
	byte  attrib;
};

struct BMPHeader 
{
	WORD bmpIdentifier;
	byte  junk[16];
	WORD biWidth;
	WORD  junk1;
	WORD biHeight;
	DWORD junk2;
	WORD bpp;
	WORD compression;
	byte  junk3[22];
};
struct DDSHeader 
{
	DWORD dwMagic;
	DWORD dwSize;
	DWORD dwFlags;
	DWORD dwHeight;
	DWORD dwWidth;
	DWORD dwPitchOrLinearSize;
	DWORD dwDepth; 
	DWORD dwMipMapCount;
	DWORD dwReserved[11];

	struct 
	{
		DWORD dwSize;
		DWORD dwFlags;
		DWORD dwFourCC;
		DWORD dwRGBBitCount;
		DWORD dwRBitMask;
		DWORD dwGBitMask;
		DWORD dwBBitMask;
		DWORD dwRGBAlphaBitMask; 
	} ddpfPixelFormat;

	struct 
	{
		DWORD dwCaps1;
		DWORD dwCaps2;
		DWORD Reserved[2];
	} ddsCaps;
	DWORD dwReserved2;
};

BOOL CreateRefDevice(HWND hWnd,XDirect3D *&pD3D,XDirect3DDevice *&pDevice)
{
	pD3D= XDirect3DCreate(D3D_SDK_VERSION);
	if (pD3D== NULL)
	{
		return FALSE;
	}

	HRESULT hr;
	D3DPRESENT_PARAMETERS presentParams;
	D3DDEVTYPE devType;

	ZeroMemory(&presentParams, sizeof(presentParams));
	presentParams.Windowed = TRUE;
	presentParams.SwapEffect = D3DSWAPEFFECT_COPY;
	presentParams.BackBufferWidth = 8;
	presentParams.BackBufferHeight = 8;
	presentParams.BackBufferFormat = D3DFMT_UNKNOWN;

	devType = D3DDEVTYPE_REF; 

	hr = pD3D->CreateDevice(D3DADAPTER_DEFAULT, devType, hWnd, 
		D3DCREATE_SOFTWARE_VERTEXPROCESSING, &presentParams, &pDevice);
	if (FAILED(hr))
	{
		MessageBox(NULL,"无法创建Ref Device,请确认安装了DirectX SDK!","Error",MB_OK);
		pD3D->Release();
		return FALSE;
	}

	D3DCAPS9 Caps;
	pDevice->GetDeviceCaps(&Caps);
	if (Caps.PrimitiveMiscCaps & D3DPMISCCAPS_NULLREFERENCE)
	{
		MessageBox(NULL,"SHIT3","SHIT3",MB_OK);
		pD3D->Release();
		pDevice->Release();
		return FALSE;
	}

	return TRUE;
}


BOOL CreateTex(XDirect3DDevice *pDevice,TexInfo &ti,XDirect3DBaseTexture*&pTex,BOOL bRT)
{
	pTex=NULL;

	extern DWORD CalcFullMipLevel(DWORD w,DWORD h);
	if (ti.miplevel==0xffff)
		ti.miplevel=(WORD)CalcFullMipLevel(ti.width,ti.height);

	DWORD usage=0;
	if (ti.miplevel==0)
		usage|=D3DUSAGE_AUTOGENMIPMAP;

	int pool=D3DPOOL_MANAGED;
	if (bRT)
	{
		if ((ti.fmt==D3DFMT_D24S8)||(ti.fmt==D3DFMT_D16))
			usage|=D3DUSAGE_DEPTHSTENCIL;
		else
			usage|=D3DUSAGE_RENDERTARGET;

		pool=D3DPOOL_DEFAULT;
	}

	if (ti.type==TEXTYPE_CUBIC)
	{
		if (D3D_OK!=XD3DXCreateCubeTexture(pDevice,ti.width,ti.miplevel,usage,
												(D3DFORMAT)ti.fmt,(D3DPOOL)pool,
												(XDirect3DCubeTexture **)&pTex))
			return FALSE;
	}
	else
	if (ti.type==TEXTYPE_3D)
	{
		if (D3D_OK!=XD3DXCreateVolumeTexture(pDevice,ti.width,ti.height,ti.depth,
											ti.miplevel,usage,(D3DFORMAT)ti.fmt,(D3DPOOL)pool,
											(XDirect3DVolumeTexture **)&pTex))
			return FALSE;
	}
	else
	{
		if (ti.fmt==D3DFMT_ATI2N)
		{
			if(D3D_OK!=pDevice->CreateTexture(ti.width,ti.height,ti.miplevel,usage,D3DFMT_ATI2N,
						(D3DPOOL)pool,(XDirect3DTexture **)&pTex,NULL))
				return FALSE;
		}
		else
		{
			if (D3D_OK!=XD3DXCreateTexture(pDevice,ti.width,ti.height,ti.miplevel,
												usage,(D3DFORMAT)ti.fmt,(D3DPOOL)pool,
												(XDirect3DTexture **)&pTex))
				return FALSE;
		}

	}


	if (ti.miplevel==0)//an autogen-mipmap texture,set the filter type
		pTex->SetAutoGenFilterType((D3DTEXTUREFILTERTYPE)ti.filterAutoGenMipmap);

	return TRUE;
}

#define DDSCAPS2_CUBEMAP 0x00000200

//#pragma disable(warning:4244)
//edit by star
void LoadTexInfo(TexInfo &ti,IFile * pFile)
{
	std::string extName=pFile->GetSuffix();

	if(0==StringCmpNoCase(extName.c_str(),"tga"))
	{
		pFile->Reset();
		TGAHeader  tagHeader;
		pFile->Read(&tagHeader,sizeof(tagHeader));	
		ti.height=tagHeader.height;
		ti.width=tagHeader.width;
		if(tagHeader.bpp/8==3)
			ti.fmt=D3DFMT_R8G8B8;
		else
			ti.fmt=D3DFMT_A8R8G8B8;
	}
	else if(0==StringCmpNoCase(extName.c_str(),"bmp"))
	{
		pFile->Reset();
		BMPHeader  bmpHeader;
		pFile->Read(&bmpHeader,sizeof(bmpHeader));
		ti.height=bmpHeader.biHeight;
		ti.width=bmpHeader.biWidth;
		if(bmpHeader.bpp/8==3)
			ti.fmt=D3DFMT_R8G8B8;
		else
			ti.fmt=D3DFMT_A8R8G8B8;
	}
	else if(0==StringCmpNoCase(extName.c_str(),"dds"))
	{
		pFile->Reset();
		DDSHeader ddsHeader;
		pFile->Read(&ddsHeader,sizeof(ddsHeader));
		ti.height=(WORD)ddsHeader.dwHeight;
		ti.width=(WORD)ddsHeader.dwWidth;
		ti.miplevel=(WORD)ddsHeader.dwMipMapCount;
		ti.fmt=ddsHeader.ddpfPixelFormat.dwFourCC;
		if (ddsHeader.ddsCaps.dwCaps2&DDSCAPS2_CUBEMAP)
			ti.type=TEXTYPE_CUBIC;
	} 
	else if(0==StringCmpNoCase(extName.c_str(),"jpg"))
	{
		pFile->Reset();
		DWORD w,h,ch;
		extern BOOL IJL_ReadImageInfo(DWORD &w,DWORD &h,DWORD &channel,const char * name);
		std::string filename=pFile->GetPath();
		IJL_ReadImageInfo(w,h,ch,filename.c_str());
		ti.height=(WORD)h;
		ti.width=(WORD)w;
		if(ch==3)
			ti.fmt=D3DFMT_R5G6B5;
		else
			ti.fmt=D3DFMT_A8R8G8B8;
	} 

	ti.fmtbit=d3dfmtBitSize((D3DFORMAT)ti.fmt);

}

//load texture information from memory
BOOL LoadTexInfo(TexInfo &ti,TexData*data0)
{
	BYTE *data=data0->GetDataPtr();
	switch(data0->GetDataType())
	{
			case TexData::Tex_TGA:
			{
				TGAHeader  tagHeader;
				memcpy(&tagHeader,data,sizeof(tagHeader));
				ti.height=tagHeader.height;
				ti.width=tagHeader.width;
				if(tagHeader.bpp/8==3)
					ti.fmt=D3DFMT_R8G8B8;
				else
					ti.fmt=D3DFMT_A8R8G8B8;
				break;
			}
		case TexData::Tex_BMP:
			{
				 BMPHeader bmpHeader;
				 memcpy(&bmpHeader,data,sizeof(bmpHeader));
				 ti.height=bmpHeader.biHeight;
				 ti.width=bmpHeader.biWidth;
				 if(bmpHeader.bpp/8==3)
					 ti.fmt=D3DFMT_R8G8B8;
				 else
					 ti.fmt=D3DFMT_A8R8G8B8;
				 break;
			}
		case TexData::Tex_DDS:
			{
				DDSHeader ddsHeader;
				memcpy(&ddsHeader,data,sizeof(ddsHeader));
				ti.height=(WORD)ddsHeader.dwHeight;
				ti.width=(WORD)ddsHeader.dwWidth;
				ti.miplevel=(WORD)ddsHeader.dwMipMapCount;
				ti.fmt=ddsHeader.ddpfPixelFormat.dwFourCC;
				if (ddsHeader.ddsCaps.dwCaps2&DDSCAPS2_CUBEMAP)
					ti.type=TEXTYPE_CUBIC;
				break;
			}
		case TexData::Tex_JPG :
			{
				DWORD w,h,ch;
				extern BOOL IJL_ReadImageInfo(DWORD &w,DWORD &h,DWORD &channel,BYTE *data,DWORD szData);
				IJL_ReadImageInfo(w,h,ch,data,data0->GetDataSize());
				ti.height=(WORD)h;
				ti.width=(WORD)w;
				if(ch==3)
					ti.fmt=D3DFMT_R5G6B5;
				else
					ti.fmt=D3DFMT_A8R8G8B8;

				break;
			}
		default:
			break;
	}
	ti.fmtbit=d3dfmtBitSize((D3DFORMAT)ti.fmt);
	return TRUE;
}


BOOL LoadTexData(IFile *fl,TexData *data)
{
	data->Clean();
	data->data.resize(fl->GetSize());
	fl->Read(&data->data[0],fl->GetSize());

	std::string extName=fl->GetSuffix();

	if(0==StringCmpNoCase(extName.c_str(),"tga"))
		data->textype=TexData::Tex_TGA;
	else if(0==StringCmpNoCase(extName.c_str(),"bmp"))
		data->textype=TexData::Tex_BMP;
	else if(0==StringCmpNoCase(extName.c_str(),"dds"))
		data->textype=TexData::Tex_DDS;
	else if(0==StringCmpNoCase(extName.c_str(),"jpg"))
		data->textype=TexData::Tex_JPG;

	if (data->textype==TexData::Tex_UNKNOWN)
	{
		data->Clean();
		return FALSE;
	}

	return TRUE;
}

BOOL SaveTexData(IFile *fl,TexData *data)
{
	fl->Write(&data->data[0],data->data.size());
	return TRUE;
}


//  [12/21/2007]
BOOL CreateHdSupportTex(XDirect3DDevice *pDevice,TexInfo &ti,void *data,int szData,XDirect3DBaseTexture *&pTex)
{
	pTex=NULL;
	 if(!pDevice) 
		 return FALSE;
	 if(ti.width%4||ti.height%4) 
		 return FALSE;
	 DDSHeader *ddsHeader=(DDSHeader *)data;
	 //support 3dc format
	 int headerLen=sizeof(DDSHeader);
	 if(ddsHeader->ddpfPixelFormat.dwFourCC==D3DFMT_ATI2N)
	 {
		 if(D3D_OK==pDevice->CreateTexture(ti.width,ti.height,1,0,
											D3DFMT_ATI2N,D3DPOOL_MANAGED,
											(XDirect3DTexture **)&pTex,NULL))
		 {
			 D3DLOCKED_RECT  lockRect;
			 RECT  srcRect;
			 srcRect.left=srcRect.top=0;srcRect.right=ti.width; srcRect.bottom=ti.height;
			 if ((!i_math::ispower2(ti.width))||(!i_math::ispower2(ti.height)))
			 {
				 //Size of image isn't power of 2, convert it to R5G6B5 format
				 SAFE_RELEASE(pTex);
			 }
			 else
			 {
				 ((XDirect3DTexture *)pTex)->LockRect(0,&lockRect,&srcRect,D3DLOCK_DISCARD);
				 assert(lockRect.Pitch==ti.width);  //assert the card
				 memcpy(lockRect.pBits,(byte*)data+headerLen,ti.width*ti.height);
				 ((XDirect3DTexture *)pTex)->UnlockRect(0);
				 return TRUE;
			 }
		 }

		 pDevice->CreateTexture(ti.width,ti.height,1,0,D3DFMT_R5G6B5,
											D3DPOOL_MANAGED,
											(XDirect3DTexture **)&pTex,NULL);
		 std::vector<byte> buf;
		 buf.resize((szData-headerLen)*2);
		 Convert3DcToR5G6B5((byte*)data+headerLen,&buf[0],ti.width,ti.height);
		 D3DLOCKED_RECT  lockRect;
		 RECT  srcRect;
		 srcRect.left=srcRect.top=0;srcRect.right=ti.width; srcRect.bottom=ti.height;
		 ((XDirect3DTexture *)pTex)->LockRect(0,&lockRect,&srcRect,D3DLOCK_DISCARD);
		 for(int i=0;i<ti.height;i++)
			 memcpy((byte *)lockRect.pBits+i*lockRect.Pitch,&buf[0]+ti.width*2*i,ti.width*2);
		 ((XDirect3DTexture *)pTex)->UnlockRect(0);

		 return TRUE;		
	 }
	 return FALSE;
}

BOOL LoadTex(XDirect3DDevice *pDevice,TexData *td,TexInfo &ti,XDirect3DBaseTexture*&pTex)
{
	pTex=NULL;

	if (FALSE==LoadTexInfo(ti,td))
		return FALSE;

	assert((ti.type==TEXTYPE_2D)||(ti.type==TEXTYPE_CUBIC));//目前还不支持volume map

	BOOL bLoaded=FALSE;
	if ((td->GetDataType()==TexData::Tex_DDS)&&(ti.type==TEXTYPE_2D))
	{
		if(CreateHdSupportTex(pDevice,ti,&td->data[0],td->data.size(),pTex))
			bLoaded=TRUE;
	}

	if (!bLoaded)
	{
		DWORD usage=0;
		if (ti.miplevel==0)
			usage|=D3DUSAGE_AUTOGENMIPMAP;


		if (ti.type==TEXTYPE_2D)
		{
			if(D3D_OK!= XD3DXCreateTextureFromFileInMemoryEx(pDevice,
				&td->data[0],td->data.size(),ti.width,ti.height,ti.miplevel,0,
				(D3DFORMAT)ti.fmt,D3DPOOL_MANAGED,
				D3DX_FILTER_TRIANGLE | D3DX_FILTER_DITHER,
				D3DX_FILTER_BOX,0,NULL,NULL,
				(XDirect3DTexture**)&pTex))
				return FALSE;
			bLoaded=TRUE;
		}
#ifndef MULTITHREAD_D3D
		if (ti.type==TEXTYPE_CUBIC)
		{
			if(D3D_OK!= XD3DXCreateCubeTextureFromFileInMemoryEx(pDevice,
				&td->data[0],td->data.size(),ti.width,ti.miplevel,0,
				(D3DFORMAT)ti.fmt,D3DPOOL_MANAGED,
				D3DX_FILTER_TRIANGLE | D3DX_FILTER_DITHER,
				D3DX_FILTER_BOX,0,NULL,NULL,
				(XDirect3DCubeTexture**)&pTex))
				return FALSE;
			bLoaded=TRUE;
		}
#endif

	}


	//贴图的一些格式有些显卡会不支持,载入后某些格式会发生变化,要repair一下
#ifndef MULTITHREAD_D3D

	D3DSURFACE_DESC desc;
	switch(ti.type)
	{
		case TEXTYPE_2D:
		{
			((XDirect3DTexture*)pTex)->GetLevelDesc(0,&desc);
			break;
		}
		case TEXTYPE_CUBIC:
		{
			((XDirect3DCubeTexture*)pTex)->GetLevelDesc(0,&desc);
			break;
		}
		default:
			assert(FALSE);//目前不考虑volume texture
			return TRUE;
	}

	ti.width=desc.Width;
	ti.height=desc.Height;

#endif

	return TRUE;
}


BOOL SaveTex(TexData *td,XDirect3DBaseTexture*pTex)
{
	D3DXIMAGE_FILEFORMAT fmt;
	switch(td->textype)
	{
		case TexData::Tex_DDS:
			fmt=D3DXIFF_DDS;
			break;
		case TexData::Tex_TGA:
			fmt=D3DXIFF_TGA;
			break;
		case TexData::Tex_JPG:
			fmt=D3DXIFF_JPG;
			break;
		case TexData::Tex_BMP:
			fmt=D3DXIFF_BMP;
			break;
		default:
			return FALSE;
	} 

	XD3DXBuffer* p;
	if (D3D_OK==XD3DXSaveTextureToFileInMemory(&p,fmt,pTex,NULL))
	{
		td->data.clear();
		VEC_APPEND_BUFFER(td->data,(BYTE*)p->GetBufferPointer(),p->GetBufferSize());
		SAFE_RELEASE(p);
		return TRUE;
	}
	return FALSE;

}
