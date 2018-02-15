/********************************************************************
	created:	2008/08/11   12:45
	filename: 	d:\IxEngine\Proj_RenderSystemMT\d3d9A\Direct3DDeviceA.cpp
	author:		cxi
	
	purpose:	CDirect3DDevice
*********************************************************************/

#include "stdh.h"

#include "Direct3DDeviceA.h"

#include "timer/profiler.h"

//////////////////////////////////////////////////////////////////////////
//CDynVBPool

void CDynVBPool::Clear()
{
	for (int i=0;i<_cVBs;i++)
	{
		SAFE_RELEASE(_vbs[i].vb);
	}
	_cVBs=0;

	for (int i=0;i<_cIBs;i++)
	{
		SAFE_RELEASE(_ibs[i].ib);
	}
	_cIBs=0;

	_dev=NULL;
}

IDirect3DVertexBuffer9 *CDynVBPool::AllocVB(DWORD len,DWORD fvf)
{
	DWORD c=_cVBs;
	for (int i=0;i<c;i++)
	{
		if (_vbs[i].bAlloc)
			continue;
		if ((_vbs[i].len==len)&&(_vbs[i].fvf==fvf))
		{
			_vbs[i].bAlloc=TRUE;
			return _vbs[i].vb;
		}
	}
	if (_cVBs>=ARRAY_SIZE(_vbs))
		return NULL;

	IDirect3DVertexBuffer9 *vb;
	if (D3D_OK!=_dev->CreateVertexBuffer(len,D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY,
																		fvf,D3DPOOL_DEFAULT,&vb,NULL))
		return NULL;
	_vbs[_cVBs].fvf=fvf;
	_vbs[_cVBs].len=len;
	_vbs[_cVBs].vb=vb;
	_vbs[_cVBs].bAlloc=TRUE;
	_cVBs++;

	return vb;
}

void CDynVBPool::Free(IDirect3DVertexBuffer9 *vb)
{
	DWORD c=_cVBs;
	for (int i=0;i<c;i++)
	{
		if (!_vbs[i].bAlloc)
			continue;
		if (_vbs[i].vb==vb)
		{
			_vbs[i].bAlloc=FALSE;
			break;
		}
	}
}

IDirect3DIndexBuffer9 *CDynVBPool::AllocIB(DWORD len,DWORD fmt)
{
	DWORD c=_cIBs;
	for (int i=0;i<c;i++)
	{
		if (_ibs[i].bAlloc)
			continue;
		if ((_ibs[i].len==len)&&(_ibs[i].fmt==fmt))
		{
			_ibs[i].bAlloc=TRUE;
			return _ibs[i].ib;
		}
	}
	if (_cIBs>=ARRAY_SIZE(_ibs))
		return NULL;

	IDirect3DIndexBuffer9 *ib;
	if (D3D_OK!=_dev->CreateIndexBuffer(len,D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY,
		(D3DFORMAT)fmt,D3DPOOL_DEFAULT,&ib,NULL))
		return NULL;
	_ibs[_cIBs].fmt=fmt;
	_ibs[_cIBs].len=len;
	_ibs[_cIBs].ib=ib;
	_ibs[_cIBs].bAlloc=TRUE;
	_cIBs++;

	return ib;
}

void CDynVBPool::Free(IDirect3DIndexBuffer9 *ib)
{
	DWORD c=_cIBs;
	for (int i=0;i<c;i++)
	{
		if (!_ibs[i].bAlloc)
			continue;
		if (_ibs[i].ib==ib)
		{
			_ibs[i].bAlloc=FALSE;
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//CDirect3DDevice



