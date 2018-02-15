#pragma once
#include "d3d9A.h"

#include "devicequeue.h"


class CD3DXBuffer
{
public:
	DEFINE_D3D9A_CLASS(CD3DXBuffer)
	CD3DXBuffer()
	{
		_nRef=1;
		_core=NULL;
	}

	D3D9A_ADDREF()
	D3D9A_RELEASE(Buffer_Release)

	STDMETHOD_(LPVOID, GetBufferPointer)(THIS)
	{
		LPVOID p;
		DQ_Freeze();
		p=_core->GetBufferPointer();
		DQ_UnFreeze();
		return p;
	}
	STDMETHOD_(DWORD, GetBufferSize)(THIS)
	{
		DWORD sz;
		DQ_Freeze();
		sz=_core->GetBufferSize();
		DQ_UnFreeze();
		return sz;
	}

public:
	int _nRef;
	ID3DXBuffer* _core;

};
