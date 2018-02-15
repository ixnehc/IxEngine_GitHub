#pragma once

#include "RenderSystem/IRenderSystem.h"

#include "GLBase.h"
#include "ResourceBase.h"

#include "RenderSystem/IVertexBuffer.h"

#include "math/range.h"

#include <vector>
#include <map>

#define SIZE_INDEX 2


class VBInfoGL;
class CVertexMgrGL;
class CDeviceObjectGL;

class VBBaseGL
{
public:
	VBBaseGL();
	BOOL IsVB()	{		return _bVB;	}

	GLVertexBufHandle GetHandle()	{		return _handle;	}

	i_math::rangei rng;

protected:
	void _Zero();
	void _AddDirty(DWORD from,DWORD to)
	{
		i_math::rangei rng;
		rng.set((int)from,(int)to);
		_dirties.push_back(rng);
	}

	BOOL _bVB;//vb or ib
	DWORD _flags;//VBFlag_XXXX
	DWORD _count;//count of vertex/index
	DWORD _memcount;//内存数量

	GLVertexBufHandle _handle;
	std::vector<BYTE> _buf;

	//修改的起始
	std::vector<i_math::rangei>_dirties;

	int _cLock;//Lock count

friend class CVertexMgrGL;
};

class VBInfoGL:public IVertexBuffer,public CResource,public VBBaseGL
{
public:
	DECLARE_CLASS(VBInfoGL);
	VBInfoGL();

	virtual IRenderSystem *GetRS(){	return CResource::GetRS();}
	virtual int AddRef(){	return CResource::AddRef();}
	virtual int Release();
	virtual const char *GetPath(){	return CResource::GetPath();}
	virtual AResult Touch(){	return CResource::Touch();}
	virtual BOOL ForceTouch(){	return CResource::ForceTouch();}
	virtual DWORD GetVer(){	return CResource::GetVer();}							


	virtual int GetCount()	{		return _count;	}//get vertex count return -1 on failure
	virtual int GetFrameCount()	{		return 1;	}		//return -1 on failure
	virtual FVFEx GetFVF()	{		return _fvf;	}	//fvf of this vb
	virtual BOOL SetFVF(FVFEx fvf);
	
	virtual int GetSize(){	return _fvfsize*_count;}//vertex buffer size in byte,return -1 on failure 
	virtual DWORD GetStride()	{		return _fvfsize;	}//fvf size of this vb

	//Lock for DirectAccess VB
	virtual void *Lock(BOOL bDiscard,FVFEx fvf=0,DWORD iFrame=0);
	virtual void AddDirty(DWORD from,DWORD to);//标记哪些顶点被修改了,from/to都以顶点为单位
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

	friend class CVertexMgrGL;
	friend class CDeviceObjectGL;
};


class IBInfoGL:public IIndexBuffer,public CResource,public VBBaseGL
{
public:
	DECLARE_CLASS(IBInfoGL);
	IBInfoGL();

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
	virtual void Unlock();
	virtual void UnlockAll();//清除所有的lock的引用计数

private:
	void _Zero();

	virtual BOOL _OnTouch(IRenderSystem *pRS)	;
	virtual void _OnUnload();
	void _OnDeviceLost();
	void _OnDeviceReset();


	friend class CVertexMgrGL;
	friend class CDeviceObjectGL;
	friend class VBInfoGL;

private:
};

class CDeviceObjectGL;
class CVertexMgrGL:public IVertexMgr,public CResourceMgr
{
public:
	CVertexMgrGL();

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

	std::vector<VBInfoGL*>_vbs;
	std::vector<IBInfoGL *>_ibs;

	DWORD _iLastVB;
	DWORD _iLastIB;


private:

	friend class VBInfoGL;
	friend class IBInfoGL;
};


