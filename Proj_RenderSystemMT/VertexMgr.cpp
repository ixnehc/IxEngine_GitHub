/********************************************************************
	created:	2006/06/17
	created:	17:6:2006   15:05
	filename: 	d:\IxEngine\Proj_RenderSystem\VertexMgr.cpp
	author:		cxi
	
	purpose:	IVertexBuffer,IVertexMgr implement
*********************************************************************/
#include "stdh.h"

#include "fvfex/fvfex.h"

#include "RenderSystem.h"

#include "VertexMgr.h"


#include "DeviceObject.h"

#include "timer/profiler.h"

#include <assert.h>

#pragma warning (disable: 4018)

//////////////////////////////////////////////////////////////////////////
//VBBase
VBBase::VBBase()
{
}


void VBBase::_Zero()
{
	_bVB=TRUE;
	_flags=0;//VBFlag_XXXX

	_d3dhandle=NULL;
	_count=0;
	_memcount=0;

	_cLock=0;
	_dataLocked=NULL;
}




//////////////////////////////////////////////////////////////////////////
//VBInfo
IMPLEMENT_CLASS(VBInfo);
void VBInfo::_Zero()
{
	VBBase::_Zero();
	_fvf=0;
	_fvfsize=0;
	_nFrames=0;

	_state=CResource::Abandoned;
}


VBInfo::VBInfo()
{
	_Zero();
}

int VBInfo::Release()
{
	int c=CResource::Release();
	if (c<=0)
	{
		assert(_cLock==0);
		_OnUnload();
	}

	return c;
}


BOOL VBInfo::_OnTouch(IRenderSystem *pRS)
{
	XDirect3DDevice *pDevice=(XDirect3DDevice *)((CRenderSystem *)pRS)->GetDeviceObj()->GetDevice();

	D3DPOOL pool;
	DWORD usage=0;
	if (_flags&VBFlag_Dynamic)
	{
		pool=D3DPOOL_DEFAULT;
		usage=D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC;
	}
	else
		pool=D3DPOOL_MANAGED;

	HRESULT hr;

	if (!_d3dhandle)
	{
		hr=pDevice->CreateVertexBuffer(GetSize(),usage,
			0,pool,(XD3DVertexBuffer**)&_d3dhandle,NULL);
		if (hr!=D3D_OK)
			return FALSE;
		_memcount=GetSize();
	}
	return TRUE;
}

void VBInfo::_OnUnload()
{
	SAFE_RELEASE_D3DRES((XD3DVertexBuffer*&)_d3dhandle);
	_Zero();
}

void VBInfo::_OnDeviceLost()
{
	if (_flags&VBFlag_Dynamic)
	{
		SAFE_RELEASE_D3DRES((XD3DVertexBuffer*&)_d3dhandle);
		SetState(CResource::Abandoned);
	}
}

void VBInfo::_OnDeviceReset()
{
	SetState(CResource::Loaded);
}





void *VBInfo::Lock(BOOL bDiscard,FVFEx fvf,DWORD iFrame)
{
	if (iFrame>=_nFrames)
		return NULL;

	int nOff=0;
	if (fvf!=0)
		nOff=fvfOffset(_fvf,fvf);
	if (nOff<0)
		return NULL;//fvf invalid

	if (A_Ok!=Touch())
		return NULL;
	if (_cLock<=0)
	{
		assert(!_dataLocked);

		XD3DVertexBuffer *pVB=(XD3DVertexBuffer *)_d3dhandle;

		HRESULT hr;
		if (_flags&VBFlag_Dynamic)
		{
			if (bDiscard)
				hr=pVB->Lock(0,0,(void**)&_dataLocked,D3DLOCK_DISCARD);
			else
				hr=pVB->Lock(0,0,(void**)&_dataLocked,D3DLOCK_NOOVERWRITE);
		}
		else
		{
			if (bDiscard)
				hr=pVB->Lock(0,0,(void**)&_dataLocked,0);
			else
				hr=pVB->Lock(0,0,(void**)&_dataLocked,D3DLOCK_READONLY);
		}

		if (hr!=D3D_OK)
			return NULL;
		_cLock=0;
	}

	_cLock++;
	return (void *)((unsigned char*)_dataLocked+iFrame*_fvfsize*_count+nOff);
}


void VBInfo::Unlock()
{

	if (_cLock>0)
		_cLock--;

	if (_cLock<=0)
	{
		if ((_dataLocked)&&(_d3dhandle))
			((XD3DVertexBuffer*)_d3dhandle)->Unlock();
		_dataLocked=NULL;
	}

}

void VBInfo::UnlockAll()
{
	_cLock=0;
	Unlock();
}


BOOL VBInfo::SetFVF(FVFEx fvf)
{
	if (_nFrames!=1)
	{
		assert(FALSE);//目前不支持frame count大于1的vb
		return FALSE;
	}
	DWORD fvfsize=fvfSize(fvf);
	if (fvfsize<=0)
		return FALSE;
	if (_memcount==0)
	{
		//D3D资源尚未创建,不用更新count
		assert(!_d3dhandle);
		_fvf=fvf;
		_fvfsize=fvfsize;
	}
	else
	{
		_count=_memcount/fvfsize;
		_fvf=fvf;
		_fvfsize=fvfsize;
	}
	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//IBInfo
IMPLEMENT_CLASS(IBInfo);
void IBInfo::_Zero()
{
	VBBase::_Zero();

	_bVB=FALSE;//mark myself as an IB
}


IBInfo::IBInfo()
{
	_Zero();
}

int IBInfo::Release()
{
	int c=CResource::Release();
	if (c<=0)
	{
		assert(_cLock==0);
		_OnUnload();
	}

	return c;
}


BOOL IBInfo::_OnTouch(IRenderSystem *pRS)
{

	XDirect3DDevice *pDevice=(XDirect3DDevice *)((CRenderSystem *)pRS)->GetDeviceObj()->GetDevice();

	D3DPOOL pool;
	DWORD usage=0;
	if (_flags&VBFlag_Dynamic)
	{
		pool=D3DPOOL_DEFAULT;
		usage=D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC;
	}
	else
		pool=D3DPOOL_MANAGED;

	if (!_d3dhandle)
	{
		HRESULT hr=pDevice->CreateIndexBuffer(GetSize(),usage,D3DFMT_INDEX16,
			pool,(XD3DIndexBuffer**)&_d3dhandle,NULL);
		if (hr!=D3D_OK)
			return FALSE;
	}

	return TRUE;
}

void IBInfo::_OnUnload()
{
	SAFE_RELEASE_D3DRES((XD3DIndexBuffer*&)_d3dhandle);
	_Zero();
}

void IBInfo::_OnDeviceLost()
{
	if (_flags&VBFlag_Dynamic)
	{
		SAFE_RELEASE_D3DRES((XD3DIndexBuffer*&)_d3dhandle);
		SetState(CResource::Loaded);
	}
}

void IBInfo::_OnDeviceReset()
{
	SetState(CResource::Loaded);
}

void *IBInfo::Lock(BOOL bDiscard)
{
	if (A_Ok!=CResource::Touch())
		return NULL;

	if (_cLock<=0)
	{
		assert(!_dataLocked);

		XD3DIndexBuffer *pIB=(XD3DIndexBuffer *)_d3dhandle;

		HRESULT hr;
		if (_flags&VBFlag_Dynamic)
		{
			if (bDiscard)
				hr=pIB->Lock(0,0,(void**)&_dataLocked,D3DLOCK_DISCARD);
			else
				hr=pIB->Lock(0,0,(void**)&_dataLocked,D3DLOCK_NOOVERWRITE);
		}
		else
		{
			if (bDiscard)
				hr=pIB->Lock(0,0,(void**)&_dataLocked,0);
			else
				hr=pIB->Lock(0,0,(void**)&_dataLocked,D3DLOCK_READONLY);
		}
		if (hr!=D3D_OK)
			return NULL;
		_cLock=0;
	}

	_cLock++;
	return (void *)_dataLocked;
}

void IBInfo::Unlock()
{
	if (_cLock>0)
		_cLock--;

	if (_cLock<=0)
	{
		if ((_dataLocked)&&(_d3dhandle))
			((XD3DIndexBuffer*)_d3dhandle)->Unlock();
		_dataLocked=NULL;
	}
}

void IBInfo::UnlockAll()
{
	_cLock=0;
	Unlock();
}


//////////////////////////////////////////////////////////////////////////
//CVertexMgr
CVertexMgr::CVertexMgr()
{
}

BOOL CVertexMgr::Init(IRenderSystem *pRS,const char *name)
{
	if (FALSE==CResourceMgr::Init(pRS,name))
		return FALSE;

	_iLastVB=0;
	_iLastIB=0;

	return TRUE;
}

void CVertexMgr::UnInit()
{
	CResourceMgr::UnInit();//Will call UnloadAll() internally
}


//flags: VBFlag_XXXX
IVertexBuffer* CVertexMgr::CreateVB(DWORD nVertice,FVFEx fvf,
									DWORD nFrames,DWORD flags)
{
	if (nVertice<=0)
		return NULL;
	if (nFrames<=0)
		return NULL;
	if (!fvfCheck(fvf))
		return NULL;

	VBInfo *p=NewRes<VBInfo>();
	_vbs.push_back(p);

	p->AddRef();

	p->_fvf=fvf;
	p->_count=nVertice;
	p->_nFrames=nFrames;
	p->_flags=flags;

	p->_fvfsize=fvfSize(fvf);

	p->SetState(CResource::Loaded);

	return p;
}

//flags: VBFlag_XXXX
IIndexBuffer* CVertexMgr::CreateIB(DWORD nIndice,DWORD flags)
{
	if (nIndice<=0)
		return NULL;

	IBInfo *p=NewRes<IBInfo>();
	_ibs.push_back(p);

	p->AddRef();

	p->_count=nIndice;
	p->_flags=flags;

	p->SetState(CResource::Loaded);

	return p;
}


void CVertexMgr::_UnloadAll()
{
	for (int i=0;i<_vbs.size();i++)
	{
		_vbs[i]->_OnUnload();
		Class_Delete(_vbs[i]);
	}
	_vbs.clear();

	for (int i=0;i<_ibs.size();i++)
	{
		_ibs[i]->_OnUnload();
		Class_Delete(_ibs[i]);
	}
	_ibs.clear();
}


void CVertexMgr::OnDeviceLost()
{
	for (int i=0;i<_vbs.size();i++)
		_vbs[i]->_OnDeviceLost();
	for (int i=0;i<_ibs.size();i++)
		_ibs[i]->_OnDeviceLost();
}

void CVertexMgr::OnDeviceReset()
{
	for (int i=0;i<_vbs.size();i++)
		_vbs[i]->_OnDeviceReset();
	for (int i=0;i<_ibs.size();i++)
		_ibs[i]->_OnDeviceReset();
}

void CVertexMgr::GarbageCollect(DWORD step)
{
	_gc_vec<VBInfo>(_vbs,step,_iLastVB);
	_gc_vec<IBInfo>(_ibs,step,_iLastIB);
}