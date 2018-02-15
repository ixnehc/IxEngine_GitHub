#pragma once
#include "d3d9A.h"

#include "devicequeue.h"

class CDynVBPool;

class CDirect3DIndexBuffer
{
public:
	DEFINE_D3D9A_CLASS(CDirect3DIndexBuffer)
	CDirect3DIndexBuffer()
	{
		_nRef=1;
		_core=NULL;
		_dynpool=NULL;
		_bStatic=TRUE;
	}
	D3D9A_ADDREF()
	D3D9A_RELEASE(IndexBuffer_Release)

	STDMETHOD(Lock)(THIS_ UINT OffsetToLock,UINT SizeToLock,void** ppbData,DWORD Flags);
	STDMETHOD(Unlock)(THIS)
	{
		_core->Unlock();
		DQ_UnFreeze();
		return D3D_OK;
	}

	void _AssignDesc(UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool)
	{
		_len=Length;
		_usage=Usage;
		_fmt=Format;
		_pool=Pool;
	}

	int _nRef;
	IDirect3DIndexBuffer9 *_core;

	CDynVBPool *_dynpool;

	DWORD _len;
	DWORD _usage;
	DWORD _fmt;
	DWORD _pool;

	BOOL _bStatic;

};

class CDirect3DVertexBuffer
{
public:
	DEFINE_D3D9A_CLASS(CDirect3DVertexBuffer)
	CDirect3DVertexBuffer()
	{
		_nRef=1;
		_core=NULL;
		_dynpool=NULL;
		_bStatic=TRUE;
	}
	D3D9A_ADDREF()
	D3D9A_RELEASE(VertexBuffer_Release)

	STDMETHOD(Lock)(THIS_ UINT OffsetToLock,UINT SizeToLock,void** ppbData,DWORD Flags);
	STDMETHOD(Unlock)(THIS)
	{
		_core->Unlock();
		DQ_UnFreeze();
		return D3D_OK;
	}

	void _AssignDesc(UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool)
	{
		_len=Length;
		_usage=Usage;
		_fvf=FVF;
		_pool=Pool;
	}

	int _nRef;
	IDirect3DVertexBuffer9 *_core;

	CDynVBPool *_dynpool;

	DWORD _len;
	DWORD _usage;
	DWORD _fvf;
	DWORD _pool;


	BOOL _bStatic;
};


