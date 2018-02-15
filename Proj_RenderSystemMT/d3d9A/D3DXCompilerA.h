#pragma once
#include "d3d9A.h"

#include "devicequeue.h"

#include "D3DXBufferA.h"

class CD3DXEffectCompiler
{
public:
	DEFINE_D3D9A_CLASS(CD3DXEffectCompiler)

	CD3DXEffectCompiler()
	{
		_nRef=1;
		_core=NULL;
	}
	D3D9A_ADDREF()
	D3D9A_RELEASE(Compiler_Release)

	STDMETHOD(CompileEffect)(THIS_ DWORD Flags,
		CD3DXBuffer** ppEffect, CD3DXBuffer** ppErrorMsgs)
	{
		*ppEffect=NULL;
		if (ppErrorMsgs)
			*ppErrorMsgs=NULL;
		DQ_Freeze();
		
		HRESULT hr;
		ID3DXBuffer *buf1,*buf2=NULL;
		if (ppErrorMsgs)
			hr=_core->CompileEffect(Flags,&buf1,&buf2);
		else
			hr=_core->CompileEffect(Flags,&buf1,NULL);

		if (hr==D3D_OK)
		{
			*ppEffect=D3D9A_New(CD3DXBuffer);
			(*ppEffect)->_core=buf1;
		}
		if (buf2)
		{
			if (ppErrorMsgs)
			{
				*ppErrorMsgs=D3D9A_New(CD3DXBuffer);
				(*ppErrorMsgs)->_core=buf2;
			}
		}

		DQ_UnFreeze();

		return hr;
	}

public:
	int _nRef;
	ID3DXEffectCompiler* _core;
};

