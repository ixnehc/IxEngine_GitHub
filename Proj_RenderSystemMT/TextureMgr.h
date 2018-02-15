#pragma once

#include "RenderSystem/ITexture.h"


#include "Base.h"
#include "ResourceBase.h"


#include <vector>
#include <string>
#include <map>

class CDeviceObject;
class CTextureMgr;



struct DO_WriteTex;
class CTexture:public ITexture,public CResource
{
public:
	DECLARE_CLASS(CTexture);
	CTexture();

	virtual IRenderSystem *GetRS(){	return CResource::GetRS();}
	virtual int AddRef(){	return CResource::AddRef();}
	virtual int Release();
	virtual const char *GetPath(){	return CResource::GetPath();}
	virtual AResult Touch(){	return CResource::Touch();}
	virtual BOOL ForceTouch(){	return CResource::ForceTouch();}
	virtual DWORD GetVer(){	return CResource::GetVer();}							

	const TexInfo *GetInfo();
	DWORD GetType();
	DWORD GetFlag();
	DWORD GetWidth();
	DWORD GetHeight();
	DWORD GetDepth();
	DWORD GetMipLevel();
	DWORD GetFormat();
	void *GetTex();
	void* GetSurf(DWORD iLevel=0,DWORD iFace=0);
	BOOL Stretch(ITexture *texSrc,TexStretchArg &arg);
	BOOL Filter(int filter=4);//D3DX_FILTER_TRIANGLE
	BOOL DumpData(TexData *&data);//the pointer returned should NOT be kept for later using
	BOOL DumpTga(TexData *&data);
	BOOL DumpTga(const char *fn);
	BOOL Fill(DWORD color);

	void *Lock(DWORD &pitch,TexLockFlag flag,DWORD iLevel=0,DWORD iFace=0);
	void *Lock(i_math::recti &rc,DWORD &pitch,TexLockFlag flag,DWORD iLevel=0,DWORD iFace=0);
	void UnLock();

	virtual void *GetPixel(int x,int y,DWORD iLevel=0,DWORD iFace=0);
	DWORD GetPixelColor(int x,int y,DWORD iLevel,DWORD iFace);

public://take it as protected


	void *_GetTex()	{		return _tex;	}
	void *_GetSurf(DWORD iLevel,DWORD iFace);
	
	DWORD  _DecodeDXT1(void * block,int x,int y);
	
	DWORD  _DecodeDXT3(void * block,int x,int y);
	
	DWORD  _DecodeDXT5(void * block,int x,int y);

	DWORD _DecodeDXTCol(void * block,int x,int y);//R5G6B5
	
	DWORD _C565AToA8R8G8B8(WORD col);
protected:

	virtual BOOL _OnTouch(IRenderSystem *pRS)	;
	virtual void _OnUnload();
	

	TexInfo _info;
	DWORD _flag;
	XDirect3DBaseTexture * _tex;

	XDirect3DSurface *_surfLocked;


	friend class CTextureMgr;
	friend class CWTextureMgr;
	friend class CRTextureMgr;
};


class CTextureMgr:public ITextureMgr,public CResourceMgr
{
public:
	CTextureMgr();
	BOOL Init(IRenderSystem *pRS,const char *name);
	virtual void UnInit();

	//TODO:当Device重新创建时,要保证资源的内容维持原样
	void OnDeviceCreate();
	void OnDeviceDestroy();

	RESOURCEMGR_CORE

	virtual IResource *ObtainRes(const char *pathRes);
	virtual BOOL ReloadRes(const char *path);


protected:
};

class CWTextureMgr:public IWTextureMgr,public CResourceMgr
{
public:
	RESOURCEMGR_CORE

	//TODO:当Device重新创建时,要保证资源的内容维持原样
	void OnDeviceCreate();
	void OnDeviceDestroy();

	virtual IResource *ObtainRes(const char *pathRes)	{		return NULL;	}

	virtual ITexture *Create(const TexInfo &ti);
	virtual ITexture *Create(DWORD w,DWORD h,DWORD fmt,DWORD level=1);
	virtual ITexture *CreateCube(DWORD w,DWORD h,DWORD fmt,DWORD level=1);
	virtual ITexture *Create(TexData &data);
};

class CRTextureMgr:public IRTextureMgr,public CResourceMgr
{
public:
	RESOURCEMGR_CORE

	virtual IResource *ObtainRes(const char *pathRes)	{		return NULL;	}

	virtual ITexture *Create(const TexInfo &ti);
	virtual ITexture *Create(DWORD w,DWORD h,DWORD fmt,DWORD level);
	virtual ITexture *CreateCube(DWORD w,DWORD h,DWORD fmt,DWORD level);

	void OnDeviceLost();
	void OnDeviceReset();
};


class CD3D9Base;
class CD3D9TextureManager;

