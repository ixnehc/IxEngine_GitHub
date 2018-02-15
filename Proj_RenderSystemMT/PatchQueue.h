#pragma once

#include "RenderSystem/IPatchTools.h"
#include "RenderSystem/IShader.h"

#include "mempool/mempool.h"

//#include "ShaderLibMgr.h"

#include "class/class.h"


#include <assert.h>

class CShader;
class CPatchQueue:public IPatchQueue
{
public:
	DEFINE_CLASS_MP(CPatchQueue)
	CPatchQueue()
	{
		Zero();
		_pRS=NULL;
	}
	~CPatchQueue()
	{
		Clear();
	}
	void Zero();
	void Clear();

	void Init(IRenderSystem *pRS)	{		_pRS=pRS;	}

	IMPLEMENT_REFCOUNT_C;
	virtual BOOL Reset(FVFEx fvf,DWORD maxvertex=0xffff,
												DWORD maxindice=0xffff*3);//if maxindice is 0,do not use index buffer

	virtual FVFEx GetFVF()	{		return _fvf;	}

	virtual PQPatch AddPatch(void *vertice,DWORD nVtx,
							WORD *indice,DWORD nIdx,DWORD primtype=4);//return 0 if fail,
																													//4 is D3DPT_TRIANGLELIST
	virtual void RemovePatch(PQPatch patch);
	virtual void RemoveAllPatches();//clean all the patches
	//IMPORTANT:the patch passed in is not garranteed valid after calling this function,
	//you need replace it with the returned one
	virtual PQPatch UpdatePatch(PQPatch patch,void *vertice,DWORD nVtx,
												WORD *indice,DWORD nIdx,DWORD primtype=4);

	virtual BOOL BindPatch(IShader *shader,PQPatch pa)
	{
#pragma warning(disable:4312)
		_Patch*ph=(_Patch*)pa;
#pragma warning(default:4312)

		_bindarg.SetDPT(ph->primtype);
		_bindarg.SetPrimRange(ph->ps,ph->pc);
		_bindarg.SetVertexBase(ph->vbase);

		return shader->BindVB(_vb,_ib,&_bindarg);
	}

	void OnDeviceReset();


protected:
	struct _Patch
	{
		//in vb
		DWORD vbase;
		WORD ps;
		WORD pc;//prim count
		WORD primtype;
		WORD nVtx;
		WORD nIdx;
		MemHandle hData;
	};

	BOOL _AddPatch(void *vertice,DWORD nVtx,WORD *indice,DWORD nIdx);
	DWORD _ResolvePrimCount(DWORD nVtx,DWORD nIdx,DWORD primtype);
	BOOL _Squeeze();
	FVFEx _fvf;
	DWORD _szFVF;

	CMemPool<_Patch> _poolPatch;
	CMemPoolEx _pool;

	DWORD _nMaxVtx;
	DWORD _nMaxIdx;
	DWORD _nActualVtx;//实际的vertex数量(扣除那些作废的vertex,也就是squeeze后的vertex数量)
	DWORD _nActualIdx;//同上

	DWORD _nVtx;//current vertex count in _vb
	DWORD _nIdx;//current index count in _ib

	IRenderSystem *_pRS;

	IVertexBuffer *_vb;
	IIndexBuffer *_ib;

	VBBindArg _bindarg;


};