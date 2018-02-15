#pragma once
#include "d3d9A.h"

#include "devicequeue.h"


class CDirect3DBaseTexture;
class CD3DXEffect
{
public:
	DEFINE_D3D9A_CLASS(CD3DXEffect)
	CD3DXEffect()
	{
		_nRef=1;
		_core=NULL;
	}
	D3D9A_ADDREF()
	D3D9A_RELEASE(Effect_Release)

	STDMETHOD_(D3DXHANDLE, GetParameterByName)(THIS_ D3DXHANDLE hParameter, LPCSTR pName)
	{
		D3DXHANDLE h;
		DQ_Freeze();
		h=_core->GetParameterByName(hParameter,pName);
		DQ_UnFreeze();
		return h;
	}
	STDMETHOD_(D3DXHANDLE, GetParameterElement)(THIS_ D3DXHANDLE hParameter, UINT Index)
	{
		D3DXHANDLE h;
		DQ_Freeze();
		h=_core->GetParameterElement(hParameter,Index);
		DQ_UnFreeze();
		return h;
	}
	STDMETHOD(SetFloatArray)(THIS_ D3DXHANDLE hParameter, CONST FLOAT* pf, UINT Count)
	{
		DQ_PushFuncType(Effect_SetFloatArray);
		DQ_Push4(_core);
		DQ_Push4(hParameter);
		DQ_Push4(Count);
		DQ_Push(pf,Count*sizeof(FLOAT));
		return D3D_OK;
	}
	STDMETHOD(SetTexture)(THIS_ D3DXHANDLE hParameter, CDirect3DBaseTexture *pTexture)
	{
		DQ_PushFuncType(Effect_SetTexture);
		DQ_Push4(_core);
		DQ_Push4(hParameter);
		DQ_Push4(pTexture);
		return D3D_OK;
	}
	STDMETHOD(SetFloat)(THIS_ D3DXHANDLE hParameter, FLOAT f)
	{
		DQ_PushFuncType(Effect_SetFloat);
		DQ_Push4(_core);
		DQ_Push4(hParameter);
		DQ_Push4(f);
		return D3D_OK;
	}
	STDMETHOD(SetMatrix)(THIS_ D3DXHANDLE hParameter, CONST D3DXMATRIX* pMatrix)
	{
		DQ_PushFuncType(Effect_SetMatrix);
		DQ_Push4(_core);
		DQ_Push4(hParameter);
		DQ_Push(pMatrix,sizeof(D3DXMATRIX));
		return D3D_OK;
	}
	STDMETHOD(SetInt)(THIS_ D3DXHANDLE hParameter, INT n)
	{
		DQ_PushFuncType(Effect_SetInt);
		DQ_Push4(_core);
		DQ_Push4(hParameter);
		DQ_Push4(n);
		return D3D_OK;
	}
	STDMETHOD(SetIntArray)(THIS_ D3DXHANDLE hParameter, CONST INT* pn, UINT Count)
	{
		DQ_PushFuncType(Effect_SetIntArray);
		DQ_Push4(_core);
		DQ_Push4(hParameter);
		DQ_Push4(Count);
		DQ_Push(pn,Count*sizeof(INT));
		return D3D_OK;
	}
	STDMETHOD(SetVector)(THIS_ D3DXHANDLE hParameter, CONST D3DXVECTOR4* pVector)
	{
		DQ_PushFuncType(Effect_SetVector);
		DQ_Push4(_core);
		DQ_Push4(hParameter);
		DQ_Push(pVector,sizeof(D3DXVECTOR4));
		return D3D_OK;
	}
	STDMETHOD(Begin)(THIS_ UINT *pPasses, DWORD Flags)
	{
		*pPasses=1;//注意，这里返回1,因为目前engine里没有用到多pass的effect
		DQ_PushFuncType(Effect_Begin);
		DQ_Push4(_core);
		DQ_Push4(Flags);
		return D3D_OK;
	}
	STDMETHOD(BeginPass)(THIS_ UINT Pass)
	{
		DQ_PushFuncType(Effect_BeginPass);
		DQ_Push4(_core);
		DQ_Push4(Pass);
		return D3D_OK;
	}
	STDMETHOD(CommitChanges)(THIS)
	{
		DQ_PushFuncType(Effect_CommitChanges);
		DQ_Push4(_core);
		return D3D_OK;
	}
	STDMETHOD(EndPass)(THIS)
	{
		DQ_PushFuncType(Effect_EndPass);
		DQ_Push4(_core);
		return D3D_OK;
	}
	STDMETHOD(End)(THIS)
	{
		DQ_PushFuncType(Effect_End);
		DQ_Push4(_core);
		return D3D_OK;
	}

public:
	int _nRef;
	ID3DXEffect* _core;

};
