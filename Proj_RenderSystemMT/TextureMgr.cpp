/********************************************************************
	created:	2008/07/15   14:17
	filename: 	d:\IxEngine\Proj_RenderSystem\TextureMgr_a.cpp
	author:		cxi
	
	purpose:	异步的texture manager
*********************************************************************/

#include "stdh.h"

#include "RenderSystem.h"

#include "TextureMgr.h"
#include "DeviceObject.h"

#include "resdata/TexData.h"

#include "pixelfmt/pixelfmt.h"
#include <assert.h>

#include <bitset>
//////////////////////////////////////////////////////////////////////////
//CTexture
IMPLEMENT_CLASS(CTexture);
CTexture::CTexture()
{
	_flag=0;
	_tex=NULL;

	_surfLocked=NULL;
}

int CTexture::Release()
{
	if (!(_flag&TEXINFOFLAG_RTCREATE))
		return CResource::Release();

	int c=CResource::Release();
	if (c<=0)
		_OnUnload();

	return c;
}


BOOL CTexture::_OnTouch(IRenderSystem *pRS)
{

	XDirect3DDevice *device=(XDirect3DDevice *)((CRenderSystem*)pRS)->GetDeviceObj()->GetDevice();
	if (!(_flag&TEXINFOFLAG_RTCREATE))
	{
		if (_typeData!=Res_Texture)
			return FALSE;

		if (_data.empty())
			return FALSE;

		TexData data;
		data.data.swap(_data);
		data.textype=(TexData::TexDataType)_textype;


		extern BOOL LoadTex(XDirect3DDevice *pDevice,TexData *td,TexInfo &ti,XDirect3DBaseTexture*&pTex);

		BOOL bOk=FALSE;
		if (LoadTex(device,&data,_info,_tex))
			bOk=TRUE;


		data.data.swap(_data);

		return TRUE;
	}

	extern BOOL CreateTex(XDirect3DDevice *pDevice,TexInfo &ti,XDirect3DBaseTexture*&pTex,BOOL bRT);
	if (FALSE==CreateTex(device,_info,_tex,(_flag&TEXINFOFLAG_RENDERTARGET)!=0))
		return FALSE;
	return TRUE;

}

void CTexture::_OnUnload()
{
	SAFE_RELEASE_D3DRES(_tex);
	_info.Zero();
}

const TexInfo *CTexture::GetInfo()
{
	return &_info;
}
DWORD CTexture::GetType()
{
	return _info.type;
}

DWORD CTexture::GetFlag()
{
	return _flag;
}

DWORD CTexture::GetWidth()
{
	return _info.width;
}
DWORD CTexture::GetHeight()
{
	return _info.height;
}
DWORD CTexture::GetDepth()
{
	if (_info.type==TEXTYPE_3D)
		return _info.depth;
	return 0;
}
DWORD CTexture::GetMipLevel()
{
	return _info.miplevel;
}
DWORD CTexture::GetFormat()
{
	return _info.fmt;
}
void* CTexture::GetTex()
{
	if (A_Ok!=Touch())
		return NULL;
	return _tex;
}

void* CTexture::GetSurf(DWORD iLevel,DWORD iFace)
{
	if (A_Ok!=Touch())
		return NULL;
	if (_info.type==TEXTYPE_3D)
		return NULL;//Not support yet,by now

	if (!_tex)
		return NULL;

	XDirect3DSurface *pSurface=NULL;
	if (_info.type==TEXTYPE_2D)
	{
		XDirect3DTexture *p;
		p=(XDirect3DTexture *)_tex;
		if (D3D_OK!=p->GetSurfaceLevel(iLevel,&pSurface))
			return NULL;
	}
	if (_info.type==TEXTYPE_CUBIC)
	{
		XDirect3DCubeTexture *p;
		p=(XDirect3DCubeTexture *)_tex;
		if (D3D_OK!=p->GetCubeMapSurface((D3DCUBEMAP_FACES)iFace,iLevel,&pSurface))
			return NULL;
	}

	return pSurface;
}


BOOL CTexture::Stretch(ITexture *texSrc,TexStretchArg &arg)
{
	if (!texSrc)
		return FALSE;
	if (A_Ok!=texSrc->Touch())
		return FALSE;
	if (A_Ok!=Touch())
		return FALSE;
	BOOL bRet=FALSE;
	XDirect3DSurface *surfDest=NULL,*surfSrc=NULL;
	surfDest=(XDirect3DSurface *)GetSurf(arg.iLvlDest,arg.iFaceDest);
	if (!surfDest)
		goto _final;
	surfSrc=(XDirect3DSurface *)texSrc->GetSurf(arg.iLvlSrc,arg.iFaceSrc);
	if (!surfSrc)
		goto _final;

	RECT *rcSrc=NULL,*rcDest=NULL;
	if (arg.rcSrc.isValid())
		rcSrc=(RECT*)&arg.rcSrc;

	if (arg.rcDest.isValid())
		rcDest=(RECT*)&arg.rcDest;

	if (D3D_OK==XD3DXLoadSurfaceFromSurface(surfDest,NULL,rcDest,surfSrc,NULL,
		rcSrc,(D3DTEXTUREFILTERTYPE)arg.filter,0))
		bRet=TRUE;


_final:
	SAFE_RELEASE(surfDest);
	SAFE_RELEASE(surfSrc);

	return bRet;
}

BOOL CTexture::Filter(int filter)
{
	if (A_Ok!=Touch())
		return FALSE;

	if (D3D_OK!=XD3DXFilterTexture((XDirect3DBaseTexture*)GetTex(),NULL,0,filter))
		return FALSE;
	return TRUE;
}

extern BOOL SaveTex(TexData *td,XDirect3DBaseTexture*pTex);

TexData g_tdDump;//for dumping

//the pointer returned should NOT be kept for later using
BOOL CTexture::DumpData(TexData *&data)
{
	data=NULL;
	if (A_Ok!=Touch())
		return FALSE;

	g_tdDump.textype=TexData::Tex_DDS;
	if (FALSE==SaveTex(&g_tdDump,_tex))
		return FALSE;
	data=&g_tdDump;
	return TRUE;
}

BOOL CTexture::DumpTga(TexData *&data)
{
	data=NULL;
	if (A_Ok!=Touch())
		return FALSE;

	g_tdDump.textype=TexData::Tex_TGA;
	if (FALSE==SaveTex(&g_tdDump,_tex))
		return FALSE;
	data=&g_tdDump;
	return TRUE;
}

BOOL CTexture::DumpTga(const char *fn)
{
	TexData*data;
	if (FALSE==DumpTga(data))
		return FALSE;
	FILE *fl=fopen(fn,"wb");
	if (!fl)
		return FALSE;
	fwrite(&data->data[0],data->data.size(),1,fl);
	fclose(fl);
	return TRUE;
}

VOID  WINAPI _FillTex(D3DXVECTOR4* pOut, CONST D3DXVECTOR2* pTexCoord, 
					  CONST D3DXVECTOR2* pTexelSize, 	LPVOID pData)
{
	memcpy(pOut,pData,sizeof(D3DXVECTOR4));
}

BOOL CTexture::Fill(DWORD color)
{
	if ((_info.type==TEXTYPE_3D)||(_info.type==TEXTYPE_CUBIC)) 
		return FALSE;//Not support yet,by now

	if (A_Ok!=Touch())
		return FALSE;

	if (!_tex)
		return FALSE;

	D3DXCOLOR v;
	D3DColorToValue(color,v);
	if (D3D_OK!=XD3DXFillTexture((XDirect3DTexture *)_tex,_FillTex,&v))
		return FALSE;
	return TRUE;
}

void *CTexture::Lock(DWORD &pitch,TexLockFlag flag,DWORD iLevel,DWORD iFace)
{
	i_math::recti rc;
	return Lock(rc,pitch,flag,iLevel,iFace);
}

void *CTexture::Lock(i_math::recti &rc,DWORD &pitch,TexLockFlag flag,DWORD iLevel,DWORD iFace)
{
	if (_surfLocked)
		return NULL;

//	assert((flag==TexLock_ReadOnly)||(flag==TexLock_WriteOnly));

	if ((!(_flag&TEXINFOFLAG_RTCREATE))&&(flag==TexLock_WriteOnly))
		return NULL;//从文件载入的贴图不能修改

	if ( (_flag&TEXINFOFLAG_RENDERTARGET)&&(flag==TexLock_WriteOnly))
		return NULL;//目前不支持对RenderTarget的写入操作

	BOOL bRet=FALSE;
	HRESULT hr;
	D3DLOCKED_RECT r;
	XDirect3DSurface *surf=NULL;
	if (_flag&TEXINFOFLAG_RENDERTARGET)
	{
		XDirect3DSurface *surfRT=(XDirect3DSurface *)GetSurf(iLevel,iFace);
		if (surfRT)
		{
			D3DSURFACE_DESC desc;
			if (D3D_OK==surfRT->GetDesc(&desc))
			{
				XDirect3DDevice *device=NULL;
				if (D3D_OK==surfRT->GetDevice(&device))
				{
					if (D3D_OK==device->CreateOffscreenPlainSurface(desc.Width,desc.Height,desc.Format,
						D3DPOOL_SYSTEMMEM,&surf,NULL))
					{
						if (D3D_OK!=device->GetRenderTargetData(surfRT,surf))
							SAFE_RELEASE(surf);
					}
				}
				SAFE_RELEASE(device);
			}
			SAFE_RELEASE(surfRT);
		}
	}
	else
		surf=(XDirect3DSurface *)GetSurf(iLevel,iFace);
	if (!surf)
		return NULL;

	DWORD lockflag=0;
	switch(flag)
	{
	case TexLock_ReadOnly:
		lockflag=D3DLOCK_READONLY;
		break;
	case TexLock_WriteOnly:
//		lockflag=D3DLOCK_DISCARD;
		break;
	}

	D3DSURFACE_DESC desc;
	surf->GetDesc(&desc);

	if (!rc.isValid())
		hr=surf->LockRect(&r,NULL,lockflag);
	else
	{
		lockflag&=~D3DLOCK_DISCARD;
		assert(i_math::recti(0,0,GetWidth(),GetHeight()).isRectInside(rc));
		hr=surf->LockRect(&r,(RECT*)&rc,lockflag);
	}

	if (hr!=D3D_OK)
	{
		SAFE_RELEASE(surf);
		return NULL;
	}

	_surfLocked=surf;
	pitch=r.Pitch;

	return r.pBits;
}

void CTexture::UnLock()
{
	if (_surfLocked)
	{
		_surfLocked->UnlockRect();
		SAFE_RELEASE(_surfLocked);
	}
}

DWORD CTexture::GetPixelColor(int x,int y,DWORD iLevel,DWORD iFace)
{
	if ((((DWORD)x)>=GetWidth())||(((DWORD)y)>=GetHeight()))
		return 0;

	int x0,y0;
	x0=x/4*4;
	y0=y/4*4;
	int xOff,yOff;
	xOff=x%4;
	yOff=y%4;

	i_math::recti rc;

	switch(_info.fmt)
	{
	case D3DFMT_DXT1:
	case D3DFMT_DXT3:
	case D3DFMT_DXT5:
		{
			DWORD w = GetWidth();
			DWORD h = GetHeight();
			rc.set(0,0,w,h);
			break;
		}
	default:
		{
			rc.set(x0,y0,x0+4,y0+4);
			break;
		}
	}

	DWORD pitch;
	BYTE *data=(BYTE*)Lock(rc,pitch,TexLock_ReadOnly,iLevel,iFace);
	if (!data)
		return 0;

	Pixel_A8R8G8B8 col;
	memset(&col,0,sizeof(col));

	DWORD dwColor = 0;

	switch (_info.fmt)
	{
		case D3DFMT_R5G6B5:
		{
			Pixel_R5G6B5*p=(Pixel_R5G6B5*)(data+yOff*pitch+xOff*2);

			col.r=((DWORD)p->r)<<3;
			col.g=((DWORD)p->g)<<2;
			col.b=((DWORD)p->b)<<3;

			dwColor = FORCE_TYPE(DWORD,col);

			break;
		}
		case D3DFMT_A8R8G8B8:
		{
			Pixel_A8R8G8B8*p=(Pixel_A8R8G8B8*)(data+yOff*pitch+xOff*4);
			col=*p;

			dwColor = FORCE_TYPE(DWORD,col);
			
			break;
		}
		case D3DFMT_DXT1:
		{
			data = data + (y/4*pitch) + (x/4)*8;
			dwColor = _DecodeDXT1(data,xOff,yOff);
			break;
		}
		case D3DFMT_DXT3:
		{
			data = data + (y/4*pitch) + (x/4)*16;
			dwColor = _DecodeDXT3(data,xOff,yOff);
			break;
		}
		case D3DFMT_DXT5:
		{
			data = data + (y/4*pitch) + (x/4)*16;
			dwColor = _DecodeDXT5(data,xOff,yOff);
			break;
		}
	}

	UnLock();

	return dwColor;
}
DWORD CTexture::_DecodeDXT3(void * block,int x,int y)
{
	BYTE * pColBlock = ((BYTE *)(block)) + 8;
	DWORD col565 = _DecodeDXTCol(pColBlock,x,y);
	
    WORD alphaWord = ((WORD *)(block))[x];
	int off = 4*y;

	WORD place_mask = (0x0f)<<off;
	
	WORD code = (place_mask&alphaWord)>>off;
	
	DWORD alpha = DWORD(code<<28);

	return col565|alpha;
}
DWORD  CTexture::_DecodeDXT5(void * block,int x,int y)
{
	BYTE * pColBlock = ((BYTE *)(block)) + 8;

	DWORD col565 = _DecodeDXTCol(pColBlock,x,y);

	BYTE  alpha0 = ((BYTE *)(block))[0];
	BYTE  alpha1 = ((BYTE *)(block))[1];
    
	int off = y*12 + 3*x;
	
	BYTE * pCode = ((BYTE *)(block)) + 2;

	std::bitset<48> place_holder,place_mask;
	place_holder |= ((WORD *)(pCode))[2];
	place_holder = place_holder<<32;
	place_holder |= ((DWORD *)(pCode))[0];

	place_mask.set(off);
	place_mask.set(off+1);
	place_mask.set(off+2);

	place_holder = (place_holder&place_mask)>>off;
	
	DWORD code = place_holder.to_ulong();

	DWORD alpha = 0;
	
	if(alpha0>alpha1)
	{
		switch(code)
		{
		case 0x000: alpha = alpha0; break;
		case 0x001: alpha = alpha1; break;
		case 0x002: alpha = (6*alpha0 + 1*alpha1 + 3)/7; break;
		case 0x003: alpha = (5*alpha0 + 2*alpha1 + 3)/7; break;
		case 0x004: alpha = (4*alpha0 + 3*alpha1 + 3)/7; break;
		case 0x005: alpha = (3*alpha0 + 4*alpha1 + 3)/7; break;
		case 0x006: alpha = (2*alpha0 + 5*alpha1 + 3)/7; break;
		case 0x007: alpha = (1*alpha0 + 6*alpha1 + 3)/7; break;
		default: break;
		}
	}
	else
	{
		switch(code)
		{
		case 0x000: alpha = alpha0; break;
		case 0x001: alpha = alpha1; break;
		case 0x002: alpha = (4*alpha0 + 1*alpha1 + 2)/5; break;
		case 0x003: alpha = (3*alpha0 + 2*alpha1 + 2)/5; break;
		case 0x004: alpha = (2*alpha0 + 3*alpha1 + 2)/5; break;
		case 0x005: alpha = (1*alpha0 + 4*alpha1 + 2)/5; break;
		case 0x006: alpha = 0;   break;
		case 0x007: alpha = 255; break;
		default : break;
		}
	}
	
	return col565|alpha<<24;
}
DWORD CTexture::_DecodeDXTCol(void * block,int x,int y)
{
	WORD c0 = ((WORD *)block)[0];
	WORD c1 = ((WORD *)block)[1];

	DWORD col0 = _C565AToA8R8G8B8(c0) ; // color0
	DWORD col1 = _C565AToA8R8G8B8(c1); // color1

	int off = y*8 + 2*x;
	DWORD place_holders = ((DWORD *)(block))[1];
	DWORD place_mask = (0x3<<off);
	DWORD code = (place_holders&place_mask)>>off;
	
	DWORD col = 0;
	
	Pixel_A8R8G8B8 * p0 = (Pixel_A8R8G8B8 *)&col0;
	Pixel_A8R8G8B8 * p1 = (Pixel_A8R8G8B8 *)&col1;
	Pixel_A8R8G8B8 * pr = (Pixel_A8R8G8B8 *)&col;
	switch(code)
	{
		case 0x00:  col = col0;break;
		case 0x01:  col = col1;break;
		case 0x02:  
			{
				pr->r = (2*p0->r + 1*p1->r + 1)/3;
				pr->g = (2*p0->g + 1*p1->g + 1)/3;
				pr->b = (2*p0->b + 1*p1->b + 1)/3;
				break;
			}
		case 0x03:
			{
				pr->r = (1*p0->r + 2*p1->r + 1)/3;
				pr->g = (1*p0->g + 2*p1->g + 1)/3;
				pr->b = (1*p0->b + 2*p1->b + 1)/3;
				break;
			}
			
		default: break;
	}
	
	return (col&0x00ffffff);
}
DWORD CTexture::_C565AToA8R8G8B8(WORD col)
{
	Pixel_R5G6B5 * pR5G6B5 = (Pixel_R5G6B5 *)(&col);

	DWORD colAlpha = 0;
	Pixel_A8R8G8B8 * p = (Pixel_A8R8G8B8 *)(&colAlpha);
	p->a = 0;
	p->r = DWORD(pR5G6B5->r)<<3;
	p->g = DWORD(pR5G6B5->g)<<2;
	p->b = DWORD(pR5G6B5->b)<<3;
	
	return colAlpha;
}
DWORD CTexture::_DecodeDXT1(void * data,int x,int y)
{
	WORD c0 = ((WORD *)(data))[0];
	WORD c1 = ((WORD *)(data))[1];
	
	DWORD col = 0;
	BYTE alpha = 255;
	if(c0>c1){  // opaque foramt
		col = _DecodeDXTCol(data,x,y);
	}
	else  // one bit transparent code
	{	
		DWORD col0 = _C565AToA8R8G8B8(c0); // color0
		DWORD col1 = _C565AToA8R8G8B8(c1); // color1

		int off = y*8+2*x;

		DWORD place_holders = ((DWORD *)(data))[1];
		DWORD place_mask = (0x3<<off);
		DWORD code = (place_holders&place_mask)>>off;

		switch(code)
		{
		case 0x00: col = col0; break;
		case 0x01: col = col1; break;
		case 0x02: col = (col0+col1)/2; break;
		case 0x03: alpha = 0; break;
		default: break;
		}
	}
	
	// R5G6B5--->A8R8G8 A:255;
	return DWORD(alpha)<<24|col ;
}

void *CTexture::GetPixel(int x,int y,DWORD iLevel,DWORD iFace)
{
	assert(FALSE);
	return NULL;
}



//////////////////////////////////////////////////////////////////////////
//CTextureMgr

CTextureMgr::CTextureMgr()
{
}


BOOL CTextureMgr::Init(IRenderSystem *pRS,const char *name)
{
	CResourceMgr::Init(pRS,name);

	return TRUE;
}

void CTextureMgr::UnInit()
{
	CResourceMgr::UnInit();
}



IResource *CTextureMgr::ObtainRes(const char *pathRes)
{
	return CResourceMgr::_ObtainResA<CTexture>(pathRes);
}

BOOL CTextureMgr::ReloadRes(const char *pathRes)
{
	return CResourceMgr::_ReloadRes<CTexture>(pathRes,'A');
}


void CTextureMgr::OnDeviceDestroy()
{
	std::hash_map<std::string,CResource *>::iterator it;
	for (it=_mapRes.begin();it!=_mapRes.end();it++)
	{
		CResource *p=(*it).second;
		p->_OnUnload();
		p->SetState(CResource::Abandoned);
	}
}

void CTextureMgr::OnDeviceCreate()
{
	std::hash_map<std::string,CResource *>::iterator it;
	for (it=_mapRes.begin();it!=_mapRes.end();it++)
	{
		ResDataOp op;
		op.res=(*it).second;
		op.res->SetState(CResource::Loading);

		if (!CResource::_bDisableMT)
			_thrdResData->GetQueue()->PostOp(op);
		else
			_thrdResData->DoOp(op);//不使用多线程,直接处理
	}

}



//////////////////////////////////////////////////////////////////////////
//CWTextureMgr

extern DWORD d3dfmtBitSize(D3DFORMAT fmt);

ITexture *CWTextureMgr::Create(DWORD w,DWORD h,DWORD fmt,DWORD level)
{
	TexInfo ti;
	memset(&ti,0,sizeof(ti));
	ti.type=TEXTYPE_2D;
	ti.width=(WORD)w;
	ti.height=(WORD)h;
	ti.miplevel=(WORD)level;
	ti.fmt=fmt;
	ti.fmtbit=d3dfmtBitSize((D3DFORMAT)fmt);

	return Create(ti);
}

ITexture *CWTextureMgr::CreateCube(DWORD w,DWORD h,DWORD fmt,DWORD level)
{
	TexInfo ti;
	memset(&ti,0,sizeof(ti));
	ti.type=TEXTYPE_CUBIC;
	ti.width=(WORD)w;
	ti.height=(WORD)h;
	ti.miplevel=(WORD)level;
	ti.fmt=fmt;
	ti.fmtbit=d3dfmtBitSize((D3DFORMAT)fmt);

	return Create(ti);
}

ITexture *CWTextureMgr::Create(const TexInfo &ti)
{
	if ((ti.type!=TEXTYPE_2D)&&(ti.type!=TEXTYPE_CUBIC))
		return NULL;

	CTexture *p=_ObtainRes<CTexture>();

	p->_info=ti;
	p->_flag=TEXINFOFLAG_RTCREATE|TEXINFOFLAG_WRITABLE;

	p->SetState(CResource::Loaded);
	return p;
}


ITexture *CWTextureMgr::Create(TexData &data)
{
	CTexture *p=_ObtainRes<CTexture>();

	p->_typeData=Res_Texture;
	p->_textype=data.textype;
	p->_data.swap(data.data);

	if (FALSE==p->_OnTouch(_pRS))
		p->SetState(CResource::Abandoned);
	else
		p->SetState(CResource::Touched);

	p->_data.swap(data.data);

	p->_flag=TEXINFOFLAG_RTCREATE|TEXINFOFLAG_WRITABLE;
	return p;
}

void CWTextureMgr::OnDeviceDestroy()
{
	for (int i=0;i<_vecRes.size();i++)
	{
		CResource *p=_vecRes[i];
		SAFE_RELEASE(((CTexture *)p->_owner)->_tex);
		p->SetState(CResource::Abandoned);
	}
}

void CWTextureMgr::OnDeviceCreate()
{
	for (int i=0;i<_vecRes.size();i++)
		_vecRes[i]->SetState(CResource::Loaded);

}




//////////////////////////////////////////////////////////////////////////
//CRTextureMgr

ITexture *CRTextureMgr::Create(const TexInfo &ti)
{
	if ((ti.type!=TEXTYPE_2D)&&(ti.type!=TEXTYPE_CUBIC))
		return NULL;

	CTexture *p=_ObtainRes<CTexture>();

	p->_info=ti;
	p->_flag=TEXINFOFLAG_RTCREATE|TEXINFOFLAG_RENDERTARGET;

	p->SetState(CResource::Loaded);
	return p;
}

ITexture *CRTextureMgr::Create(DWORD w,DWORD h,DWORD fmt,DWORD level)
{
	TexInfo ti;
	memset(&ti,0,sizeof(ti));
	ti.type=TEXTYPE_2D;
	ti.width=(WORD)w;
	ti.height=(WORD)h;
	ti.miplevel=(WORD)level;
	ti.fmt=fmt;
	ti.fmtbit=d3dfmtBitSize((D3DFORMAT)fmt);

	return Create(ti);
}

ITexture *CRTextureMgr::CreateCube(DWORD w,DWORD h,DWORD fmt,DWORD level)
{
	TexInfo ti;
	memset(&ti,0,sizeof(ti));
	ti.type=TEXTYPE_CUBIC;
	ti.width=(WORD)w;
	ti.height=(WORD)h;
	ti.miplevel=(WORD)level;
	ti.fmt=fmt;
	ti.fmtbit=d3dfmtBitSize((D3DFORMAT)fmt);

	return Create(ti);
}


void CRTextureMgr::OnDeviceLost()
{
	for (int i=0;i<_vecRes.size();i++)
	{
		CTexture *p=(CTexture *)(_vecRes[i]->_owner);
		if (p)
		{
			SAFE_RELEASE(p->_tex);
			p->SetState(CResource::Abandoned);
		}
	}
}

void CRTextureMgr::OnDeviceReset()
{
	for (int i=0;i<_vecRes.size();i++)
		_vecRes[i]->SetState(CResource::Loaded);
}
