/********************************************************************
	created:	2006/12/11   17:51
	filename: 	e:\IxEngine\Proj_RenderSystem\VertexQueue.cpp
	author:		cxi
	
	purpose:	a vertex queue for easily rendering dynamic-length patches
*********************************************************************/
#include "stdh.h"
#include "RenderSystem/IRenderSystem.h"
#include "PatchQueue.h"

#include "fvfex/fvfex.h"

#pragma warning (disable:4311)
#pragma warning (disable:4312)


void CPatchQueue::Zero()
{
	_fvf=0;
	_szFVF=0;

	_vb=NULL;
	_ib=NULL;

	_nMaxVtx=0;
	_nMaxIdx=0;
	_nVtx=0;
	_nIdx=0;
	_nActualVtx=0;
	_nActualIdx=0;
}

void CPatchQueue::Clear()
{
	_pool.Clear();
	_poolPatch.FreeAll();
	_poolPatch.Reset();

	SAFE_RELEASE(_vb);
	SAFE_RELEASE(_ib);

	Zero();
}


BOOL CPatchQueue::Reset(FVFEx fvf,DWORD maxvertex,DWORD maxindice)
{
	Clear();

	_fvf=fvf;
	_szFVF=fvfSize(fvf);

	_pool.Reset(64,15);//use default sizes(64 byte to 2M)
	_poolPatch.Reset();

	_vb=_pRS->GetVertexMgr()->CreateVB(maxvertex,_fvf,1,VBFlag_Dynamic);
	if (!_vb)
		goto _fail;
	if (maxindice>0)
	{
		_ib=_pRS->GetVertexMgr()->CreateIB(maxindice,VBFlag_Dynamic);
		if (!_ib)
			goto _fail;
	}

	_nMaxVtx=maxvertex;
	_nMaxIdx=maxindice;

	return TRUE;

_fail:
	Clear();
	return FALSE;
}

BOOL CPatchQueue::_Squeeze()
{
	BOOL bRet=FALSE;

	BYTE *p,*q=NULL;
	p=(BYTE*)_vb->Lock(TRUE);
	if (!p)
		goto _fail;
	if (_ib)
	{
		q=(BYTE*)_ib->Lock(TRUE);
		if (!q)
			goto _fail;
	}

	_nActualVtx=0;
	_nActualIdx=0;
	_nVtx=0;
	_nIdx=0;

	DWORD c=_poolPatch.GetCount();
	for (int i=0;i<c;i++)
	{
		_Patch *pa=_poolPatch.Get(i);
		if (!pa)
			continue;

		BYTE *data=(BYTE *)_pool.ObtainPtr(pa->hData);
		assert(data);
		BYTE *vtx,*idx;
		vtx=data;
		idx=data+_szFVF*pa->nVtx;

		memcpy(p,data,_szFVF*pa->nVtx);
		data+=_szFVF*pa->nVtx;
		p+=_szFVF*pa->nVtx;
		if (q)
		{
			memcpy(q,data,sizeof(WORD)*pa->nIdx);
			q+=sizeof(WORD)*pa->nIdx;
		}

		pa->vbase=_nVtx;
		pa->ps=(WORD)_ResolvePrimCount(_nVtx,_nIdx,pa->primtype);

		_nVtx+=pa->nVtx;
		_nIdx+=pa->nIdx;
	}

	_nActualVtx=_nVtx;
	_nActualIdx=_nIdx;

	bRet=TRUE;

_fail:
	if (p)
		_vb->Unlock();
	if (q)
		_ib->Unlock();

	return bRet;

}


BOOL CPatchQueue::_AddPatch(void *vertice,DWORD nVtx,WORD *indice,DWORD nIdx)
{
	if (nVtx+_nActualVtx>=_nMaxVtx)
		return FALSE;
	if (nIdx+_nActualIdx>=_nMaxIdx)
		return FALSE;

	if ((nVtx+_nVtx>_nMaxVtx)||(nIdx+_nIdx>_nMaxIdx))
	{//the buffer overflow,we need squeeze the vb/ib
		if (FALSE==_Squeeze())
			return FALSE;
	}

	BOOL bOk=TRUE;
	BYTE *p=(BYTE *)_vb->Lock(FALSE);
	if (p)
	{
		memcpy(&p[_nVtx*_szFVF],vertice,nVtx*_szFVF);
		_vb->Unlock();
	}
	else
		bOk=FALSE;

	if (_ib)
	{
		BYTE *p=(BYTE *)_ib->Lock(FALSE);
		if (p)
		{
			memcpy(&p[_nIdx*sizeof(WORD)],indice,nIdx*sizeof(WORD));
			_ib->Unlock();
		}
		else
			bOk=FALSE;
	}
	return bOk;
}

DWORD CPatchQueue::_ResolvePrimCount(DWORD nVtx,DWORD nIdx,DWORD primtype)
{
	switch(primtype)
	{
		case 4:
		{
			if (nIdx>0)
				return nIdx/3;
			return nVtx/3;
		}
		assert(FALSE);
	}
	return 0;
}


PQPatch CPatchQueue::AddPatch(void *vertice,DWORD nVtx,
							 WORD *indice,DWORD nIdx,DWORD primtype)
{
	if ((nIdx>0)&&(!_ib))
		return NULL;

	if (!_AddPatch(vertice,nVtx,indice,nIdx))
		return NULL;
	assert(nVtx<=0xffff);
	assert(nIdx<=0xffff);

	DWORD szData=nVtx*_szFVF+nIdx*sizeof(WORD);
	PQPatch h=_pool.Alloc(szData);
	if (!h)
		return NULL;

	BYTE *p=(BYTE *)_pool.ObtainPtr(h);
	assert(p);

	_Patch*pa=_poolPatch.Alloc();

	if (TRUE)//fill the patch info
	{
		pa->vbase=_nVtx;
		pa->ps=(WORD)_ResolvePrimCount(_nVtx,_nIdx,primtype);
		pa->pc=(WORD)_ResolvePrimCount(nVtx,nIdx,primtype);
		pa->primtype=(WORD)primtype;
		pa->nVtx=(WORD)nVtx;
		pa->nIdx=(WORD)nIdx;
		pa->hData=h;
	}

	//the patch data
	memcpy(p,vertice,nVtx*_szFVF);
	memcpy(p+nVtx*_szFVF,indice,nIdx*sizeof(WORD));

	_nVtx+=nVtx;
	_nIdx+=nIdx;

	_nActualVtx+=nVtx;
	_nActualIdx+=nIdx;

	return (PQPatch)pa;
}

//4 is D3DPT_TRIANGLELIST
void CPatchQueue::RemovePatch(PQPatch patch)
{
	if (!patch)
		return;
	_Patch*ph=(_Patch*)patch;
	_pool.Free(ph->hData);

	_nActualVtx-=ph->nVtx;
	_nActualIdx-=ph->nIdx;

	_poolPatch.Free(ph);
}

void CPatchQueue::RemoveAllPatches()
{
	_pool.Reset();
	_poolPatch.Reset(FALSE);
	_nActualVtx=0;
	_nActualIdx=0;
}

PQPatch CPatchQueue::UpdatePatch(PQPatch patch,void *vertice,DWORD nVtx,
											WORD *indice,DWORD nIdx,DWORD primtype)
{
	RemovePatch(patch);
	return AddPatch(vertice,nVtx,indice,nIdx,primtype);
}


void CPatchQueue::OnDeviceReset()
{
	_Squeeze();
}