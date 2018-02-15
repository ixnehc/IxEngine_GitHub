#pragma once

#include "RenderSystem/IRenderSystem.h"

#include "Base.h"
#include "ResourceBase.h"

#include <vector>
#include <map>

#define SIZE_INDEX 2


class VBInfo;
class CVertexMgr;
class CDeviceObject;

class VBBase
{
public:
	VBBase();
	BOOL IsVB()	{		return _bVB;	}
protected:
	void _Zero();
	BOOL _bVB;//vb or ib
	DWORD _flags;//VBFlag_XXXX
	DWORD _count;//count of vertex/index
	DWORD _memcount;//D3D 资源所用的内存数量

	void * _d3dhandle;//XD3DVertexBuffer/XD3DIndexBuffer

	BYTE *_dataLocked;
	int _cLock;//Lock count

friend class CVertexMgr;
};

class VBInfo:public IVertexBuffer,public CResource,public VBBase
{
public:
	DECLARE_CLASS(VBInfo);
	VBInfo();

	virtual IRenderSystem *GetRS(){	return CResource::GetRS();}
	virtual int AddRef(){	return CResource::AddRef();}
	virtual int Release();
	virtual const char *GetPath(){	return CResource::GetPath();}
	virtual AResult Touch(){	return CResource::Touch();}
	virtual BOOL ForceTouch(){	return CResource::ForceTouch();}
	virtual DWORD GetVer(){	return CResource::GetVer();}							


	virtual int GetCount()	{		return _count;	}//get vertex count return -1 on failure
	virtual int GetFrameCount()	{		return _nFrames;	}		//return -1 on failure
	virtual FVFEx GetFVF()	{		return _fvf;	}	//fvf of this vb
	virtual BOOL SetFVF(FVFEx fvf);
	
	virtual int GetSize(){	return _fvfsize*_count*_nFrames;}//vertex buffer size in byte,return -1 on failure 
	virtual DWORD GetStride()	{		return _fvfsize;	}//fvf size of this vb

	//Lock for DirectAccess VB
	virtual void *Lock(BOOL bDiscard,FVFEx fvf=0,DWORD iFrame=0);
	virtual void AddDirty(DWORD from,DWORD to){}
	virtual void Unlock();
	virtual void UnlockAll();//清除所有的lock的引用计数

private:
	void _Zero();

	virtual BOOL _OnTouch(IRenderSystem *pRS)	;
	virtual void _OnUnload();
	void _OnDeviceLost();
	void _OnDeviceReset();


	FVFEx _fvf;
	DWORD _fvfsize;
	DWORD _nFrames;

	friend class CVertexMgr;
	friend class CDeviceObject;
};


class IBInfo:public IIndexBuffer,public CResource,public VBBase
{
public:
	DECLARE_CLASS(IBInfo);
	IBInfo();

	//interfacea
	virtual IRenderSystem *GetRS(){	return CResource::GetRS();}
	virtual int AddRef(){	return CResource::AddRef();}
	virtual int Release();
	virtual const char *GetPath(){	return CResource::GetPath();}
	virtual AResult Touch(){	return CResource::Touch();}
	virtual BOOL ForceTouch(){	return CResource::ForceTouch();}
	virtual DWORD GetVer(){	return CResource::GetVer();}							
	virtual int GetCount()	{		return _count;	}//get vertex count return -1 on failure
	virtual int GetSize()	{		return _count*SIZE_INDEX;	}//index buffer size in byte,return -1 on failure 
	virtual DWORD GetStride()	{		return SIZE_INDEX;	}//size of each index
	virtual void *Lock(BOOL bDiscard);
	virtual void AddDirty(DWORD from,DWORD to){}
	virtual void Unlock();
	virtual void UnlockAll();//清除所有的lock的引用计数

private:
	void _Zero();

	virtual BOOL _OnTouch(IRenderSystem *pRS)	;
	virtual void _OnUnload();
	void _OnDeviceLost();
	void _OnDeviceReset();


	friend class CVertexMgr;
	friend class CDeviceObject;
	friend class VBInfo;

private:
};

class CDeviceObject;
class CVertexMgr:public IVertexMgr,public CResourceMgr
{
public:
	CVertexMgr();

	RESOURCEMGR_CORE

	virtual BOOL Init(IRenderSystem *pRS,const char *name);
	virtual void UnInit();
	virtual void GarbageCollect(DWORD step);
	void OnDeviceLost();
	void OnDeviceReset();

	//overiding

	//interfaces
	virtual IResource *ObtainRes(const char *path)	{		return NULL;	}
	virtual IVertexBuffer* CreateVB(DWORD nVertice,FVFEx fvf,DWORD nFrames,DWORD flags);//flags: VBFlag_XXXX
	virtual IIndexBuffer* CreateIB(DWORD nIndice,DWORD flags);//flags: VBFlag_XXXX

protected:
	virtual void _UnloadAll();

	std::vector<VBInfo*>_vbs;
	std::vector<IBInfo *>_ibs;

	DWORD _iLastVB;
	DWORD _iLastIB;


private:

	friend class VBInfo;
	friend class IBInfo;
};


