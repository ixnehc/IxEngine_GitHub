#pragma once
#include "d3d9A.h"

#include "devicequeue.h"


class CDirect3DVertexDeclaration
{
public:
	DEFINE_D3D9A_CLASS(CDirect3DVertexDeclaration)
	CDirect3DVertexDeclaration()
	{
		_nRef=1;
		_core=NULL;
	}
	D3D9A_ADDREF()
	D3D9A_RELEASE(VertexDeclaration_Release)

public:
	int _nRef;
	IDirect3DVertexDeclaration9 * _core;

};
