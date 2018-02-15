#pragma once
#include "d3d9A.h"

#include "devicequeue.h"


class CD3DXMesh
{
public:
	DEFINE_D3D9A_CLASS(CD3DXMesh)

	CD3DXMesh()
	{
		_nRef=1;
		_core=NULL;
	}

	D3D9A_ADDREF()
	D3D9A_RELEASE(Mesh_Release)

	STDMETHOD(LockVertexBuffer)(THIS_ DWORD Flags, LPVOID *ppData)
	{
		HRESULT hr;
		DQ_Freeze();
		hr=_core->LockVertexBuffer(Flags,ppData);
		if (hr!=D3D_OK)
			DQ_UnFreeze();
		return hr;
	}
	STDMETHOD(UnlockVertexBuffer)(THIS)
	{
		_core->UnlockVertexBuffer();
		DQ_UnFreeze();

		return D3D_OK;
	}
	STDMETHOD(LockIndexBuffer)(THIS_ DWORD Flags, LPVOID *ppData)
	{
		HRESULT hr;
		DQ_Freeze();
		hr=_core->LockIndexBuffer(Flags,ppData);
		if (hr!=D3D_OK)
			DQ_UnFreeze();
		return hr;
	}
	STDMETHOD(UnlockIndexBuffer)(THIS)
	{
		DQ_Freeze();
		_core->UnlockIndexBuffer();
		DQ_UnFreeze();
		return D3D_OK;
	}
	STDMETHOD(GenerateAdjacency)(THIS_ FLOAT Epsilon, DWORD* pAdjacency)
	{
		HRESULT hr;
		DQ_Freeze();
		hr=_core->GenerateAdjacency(Epsilon,pAdjacency);
		DQ_UnFreeze();
		return hr;

	}
	STDMETHOD_(DWORD, GetNumFaces)(THIS)
	{
		DWORD n;
		DQ_Freeze();
		n=_core->GetNumFaces();
		DQ_UnFreeze();
		return n;
	}
	STDMETHOD_(DWORD, GetNumVertices)(THIS)
	{
		DWORD n;
		DQ_Freeze();
		n=_core->GetNumVertices();
		DQ_UnFreeze();
		return n;
	}

public:
	int _nRef;
	ID3DXMesh * _core;

};

