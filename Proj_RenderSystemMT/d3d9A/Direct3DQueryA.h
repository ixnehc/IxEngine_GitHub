#pragma once
#include "d3d9A.h"

#include "devicequeue.h"


class CDirect3DQuery
{
public:
	DEFINE_D3D9A_CLASS(CDirect3DQuery)
	CDirect3DQuery()
	{
		_nRef=1;
		_core=NULL;
	}
	D3D9A_ADDREF()
	D3D9A_RELEASE(Query_Release)

	STDMETHOD(Flush)()
	{
		DQ_PushFuncType(Query_Flush);
		DQ_Push4C(this);
		return D3D_OK;
	}


public:
	int _nRef;
	IDirect3DQuery9 * _core;

};
