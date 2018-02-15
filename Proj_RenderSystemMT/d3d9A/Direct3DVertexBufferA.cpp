/********************************************************************
	created:	2008/08/12   18:12
	filename: 	d:\IxEngine\Proj_RenderSystemMT\d3d9A\Direct3DVertexBufferA.cpp
	author:		cxi
	
	purpose:	VertexBuffer/IndexBuffer
*********************************************************************/
#include "stdh.h"

#include "Direct3DVertexBufferA.h"

#include "Direct3DDeviceA.h"




HRESULT CDirect3DIndexBuffer::Lock(THIS_ UINT OffsetToLock,UINT SizeToLock,void** ppbData,DWORD Flags)
{
	if (!_core)
		return D3DERR_INVALIDCALL;
	HRESULT hr=D3DERR_INVALIDCALL;

	if (_bStatic)
	{
		DQ_Flush();
		DQ_Freeze();
		hr=_core->Lock(OffsetToLock,SizeToLock,ppbData,Flags);
		if (hr!=D3D_OK)
			DQ_UnFreeze();
		return hr;
	}


	if (Flags&D3DLOCK_NOOVERWRITE)
	{
		DQ_Freeze();
		hr=_core->Lock(OffsetToLock,SizeToLock,ppbData,Flags);
		if (hr!=D3D_OK)
			DQ_UnFreeze();
		return hr;
	}

	//丢弃原来的
	DQ_PushFuncType(IndexBuffer_ReleaseDyn);
	DQ_Push4(_dynpool);
	DQ_Push4(_core);

	DQ_Freeze();

	_core=_dynpool->AllocIB(_len,_fmt);

	if (_core)
		hr=_core->Lock(OffsetToLock,SizeToLock,ppbData,Flags);

	if (hr!=D3D_OK)
		DQ_UnFreeze();
	return hr;

}


HRESULT CDirect3DVertexBuffer::Lock(UINT OffsetToLock,UINT SizeToLock,void** ppbData,DWORD Flags)
{
	if (!_core)
		return D3DERR_INVALIDCALL;
	HRESULT hr=D3DERR_INVALIDCALL;

	if (_bStatic)
	{
		DQ_Flush();
		DQ_Freeze();
		hr=_core->Lock(OffsetToLock,SizeToLock,ppbData,Flags);
		if (hr!=D3D_OK)
			DQ_UnFreeze();
		return hr;
	}

	if (Flags&D3DLOCK_NOOVERWRITE)
	{
		DQ_Freeze();
		hr=_core->Lock(OffsetToLock,SizeToLock,ppbData,Flags);
		if (hr!=D3D_OK)
			DQ_UnFreeze();
		return hr;
	}

	//丢弃原来的
	DQ_PushFuncType(VertexBuffer_ReleaseDyn);
	DQ_Push4(_dynpool);
	DQ_Push4(_core);

	DQ_Freeze();

	_core=_dynpool->AllocVB(_len,_fvf);

	if (_core)
		hr=_core->Lock(OffsetToLock,SizeToLock,ppbData,Flags);

	if (hr!=D3D_OK)
		DQ_UnFreeze();
	return hr;
}
