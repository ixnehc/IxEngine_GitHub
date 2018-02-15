#pragma once

#include "concurrent/thread.h"
#include "concurrent/semaphore.h"
#include "concurrent/mutexes.h"
#include "concurrent/tbbinc.h"

#include <vector>


#define DQ_PushFuncType(v) g_dq.Push<FuncType>((FuncType)((DWORD)v));
//#define DQ_PushFuncType(v) g_dq.Push<FuncType>((FuncType)((DWORD)v|0xff7e0000));
#define DQ_Push4(v) g_dq.Push<DWORD>(*(DWORD*)&(v));
#define DQ_Push4C(v) g_dq.Push<DWORD>((DWORD)(v));
#define DQ_Push(p,sz) g_dq.Push((BYTE*)(p),sz);
#define DQ_PushPValue(pv)																			\
									if (!pv)																			\
										g_dq.Push<DWORD>(0);										\
									else																				\
									{																					\
										g_dq.Push<DWORD>(1);										\
										g_dq.Push((BYTE*)pv,sizeof(*pv));							\
									}

#define DQ_States() g_dq.GetStates()

#define DQ_Flip() g_dq.Flip();
#define DQ_Flush() g_dq.Flush();
#define DQ_Freeze() g_dq.Freeze();
#define DQ_UnFreeze() g_dq.UnFreeze();





enum FuncType
{
	FuncType_Null=0,

	Device_CreateVertexBuffer,
	Device_CreateIndexBuffer,
	Device_CreateRenderTarget,
	Device_CreateDepthStencilSurface,
	Device_CreateOffscreenPlainSurface,
	Device_BeginScene,
	Device_EndScene,
	Device_SetTransform,
	Device_Clear,
	Device_EvictManagedResources,
	Device_Present,
	Device_CreateTexture,
	Device_ColorFill,
	Device_SetRenderTarget,
	Device_SetDepthStencilSurface,
	Device_SetViewPort,
	Device_SetRenderState,
	Device_DrawPrimitive,
	Device_DrawIndexedPrimitive,
	Device_CreateVertexDeclaration,
	Device_SetVertexDeclaration,
	Device_SetFVF,
	Device_SetStreamSource,
	Device_SetIndices,
	Device_CreateQuery,

	Surface_Release,

	IndexBuffer_Release,
	VertexBuffer_Release,
	IndexBuffer_ReleaseDyn,
	VertexBuffer_ReleaseDyn,

	VertexDeclaration_Release,

	Texture_Release,
	Texture_SetAutoGenFilterType,

	Query_Release,
	Query_Flush,

	Effect_Release,
	Effect_SetFloatArray,
	Effect_SetTexture,
	Effect_SetFloat,
	Effect_SetMatrix,
	Effect_SetInt,
	Effect_SetIntArray,
	Effect_SetVector,
	Effect_Begin,
	Effect_BeginPass,
	Effect_CommitChanges,
	Effect_EndPass,
	Effect_End,

	Buffer_Release,

	Compiler_Release,

	Mesh_Release,

	Func_D3DXCreateCubeTexture,
	Func_D3DXCreateVolumeTexture,
	Func_D3DXCreateTexture,
	Func_D3DXLoadSurfaceFromSurface,
	Func_D3DXFilterTexture,

	Test_0,
	Test_1,

};



class CDeviceQueue:public CThread
{
public:
	enum QueueState
	{
		Running,
		Questing,
		Freezing,
	};

	CDeviceQueue();
	~CDeviceQueue();

	template<typename T>
	void Push(T v)
	{
		if (sizeof(T)+_sz>=_capacity)
		{
			_in.resize(_capacity=_sz+sizeof(T));
			_pIn0=&_in[0];
		}
		memcpy(_pIn0+_sz,&v,sizeof(T));
		_sz+=sizeof(T);
	}
	void Push(BYTE *data,DWORD szData)
	{
		if (szData+_sz>=_capacity)
		{
			_in.resize(_capacity=_sz+szData);
			_pIn0=&_in[0];
		}
		memcpy(_pIn0+_sz,data,szData);
		_sz+=szData;
	}

	BOOL FetchLost()	{		BOOL b=_bLostOut;_bLostOut=FALSE;return b;	}
	void ClearLost()	{		_bLostOut=_bLostIn=FALSE;	}

	void Flip();
	void Flush();

	void Freeze()
	{
		if (TRUE)
		{
			tbb::spin_mutex::scoped_lock lock(_mutex);
			if (!_bProcessing)
				_qs=Freezing;
			else
			{
				if (_qs==Running)
					_qs=Questing;
			}
		}
		while(_qs!=Freezing)
			Sleep(0);
	}

	void UnFreeze()
	{
		tbb::spin_mutex::scoped_lock lock(_mutex);
		_qs=Running;
	}

	void Finish()
	{
		Flush();
		_finish=1;

		_smphOut.Post();
		while(_finish!=2);
	}

	virtual int OnDo();

protected:



	BOOL _DoProcess();

	std::vector<BYTE>_out;
	BOOL _bLostOut;


	Semaphore _smphOut;
	Semaphore _smphIn;

	tbb::spin_mutex _mutex;

	tbb::atomic<int>_finish;

	int _qs;
	tbb::atomic<int>_bProcessing;


	int patch[2048];
	BOOL _bLostIn;
	std::vector<BYTE> _in;
	BYTE *_pIn0;
	DWORD _capacity;
	DWORD _sz;

	DWORD _ttt;


};

extern CDeviceQueue g_dq;
