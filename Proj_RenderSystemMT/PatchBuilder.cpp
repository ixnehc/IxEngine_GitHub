

#include "stdh.h"

#include "RenderSystem/IRenderSystem.h"
#include "RenderSystem/IShader.h"

#include "PatchBuilder.h"

#include "fvfex/fvfex.h"

#include <assert.h>

CPatchBuilder::CPatchBuilder(void)
{
	_Zero();
	_bOnAppend = FALSE;
}

CPatchBuilder::~CPatchBuilder(void)
{
	_Clean();
}

void CPatchBuilder::_Clean(void)
{
	SAFE_RELEASE(_ib);
	SAFE_RELEASE(_vb);
	_Zero();
}

void CPatchBuilder::_Zero(void)
{
	_vb = NULL;
	_ib = NULL;
	_pVB = NULL;
	_pIB = NULL;
	Clear();
}

BOOL CPatchBuilder::Create(IRenderSystem * pRS,const FVFEx &fvf,DWORD szVB,DWORD szIB)
{
	if(!pRS)
		return FALSE;
	
	_Clean();
	//注意:这里我们没有使用动态的vb,而是用静态的vb,因为如果用Dynamic的vb的话,会出现帧数不稳定的现象(在某些情况下,帧数会
	//变得很低,不知道为什么),而使用静态的vb后,就不会出现这种现象
	//当然如果使用静态的vb的话,可能会有其它问题,需要更多的测试
	if (FALSE)
	{
		_vb = pRS->GetVertexMgr()->CreateVB(szVB,fvf,1,VBFlag_Dynamic);
		_ib = pRS->GetVertexMgr()->CreateIB(szIB,VBFlag_Dynamic);
	}
	else
	{
		_vb = pRS->GetVertexMgr()->CreateVB(szVB,fvf,1,0);
		_ib = pRS->GetVertexMgr()->CreateIB(szIB,0);
	}
	if(!_vb||!_ib){
		_Clean();
		return FALSE;
	}
	
	_fvf = fvf;
	_pRS = pRS;

	return TRUE;
}

void CPatchBuilder::Destroy(void)
{
	_Clean();
}

void CPatchBuilder::_Lock(void)
{
	assert((!_pVB)&&(!_pIB));

	if(_vb)
		_pVB = (BYTE *)_vb->Lock(TRUE);
	if(_ib)
		_pIB = (WORD *)_ib->Lock(TRUE);

	if(!_pVB||!_pIB){
		if(_vb)
			_vb->Unlock();
		if(_ib)
			_ib->Unlock();

		_pVB = NULL;
		_pIB = NULL;
	}
}

void CPatchBuilder::_UnLock(void)
{
	if(_pVB)
		_vb->Unlock();
	if(_pIB)
		_ib->Unlock();

	_pVB = NULL;
	_pIB = NULL;
}

BOOL CPatchBuilder::Begin(const FVFEx &fvf)
{
	if(!_vb||!_ib)
		return FALSE;
	
	_UnLock();
	Clear();
	_fvf = fvf;
	_vb->SetFVF(_fvf);
	assert((!_pVB)&&(!_pIB));
	_bOnAppend = TRUE; //
	
	return TRUE;
}

IPatchBuilder::Result CPatchBuilder::_Attempt(DWORD nVtx,DWORD nIB)
{
	if(!_pVB||!_pIB){
		if(_bOnAppend)
			_Lock();
	}
	
	if(!_pVB||!_pIB)
		return IPatchBuilder::Bad;

	//处于锁定状态
	DWORD nVtxTotal = _vb->GetCount();
	DWORD nIBToatal = _ib->GetCount();
	if(nVtxTotal-_nVB2Used<nVtx||nIBToatal-_nIB2Used<nIB){
		_UnLock();

		//调整vb和ib的大小，当不足绘制一个Patch时
		if(_nVB2Used==0)
		{
			DWORD nVtx2Adj = max(nVtxTotal,nVtx);
			DWORD nIB2Adj = max(nIBToatal,nIB);
			_Clean();

			//创建失败
			if(!Create(_pRS,_fvf,nVtx2Adj,nIB2Adj))
			{
				_Clean();
				_bBad = TRUE;
				return IPatchBuilder::Bad;
			}
		}

		return IPatchBuilder::Overflow;
	}

	return IPatchBuilder::Success;
}

BOOL CPatchBuilder::Append(DWORD nVtx,DWORD nIB,const void * pVBData,const WORD * pIBData)
{
	_resultApp = _Attempt(nVtx,nIB);

	if(IPatchBuilder::Success!=_resultApp)
		return FALSE;
	
	assert(pVBData&&pIBData&&_bOnAppend);

	assert(_pVB&&_pIB);

	DWORD stride = _vb->GetStride();
	memcpy(_pVB + stride*_nVB2Used,pVBData,nVtx*stride);
	WORD * pDst = _pIB + _nIB2Used;
	for(int i = 0;i<nIB;i++){
		DWORD idx = pIBData[i] + _nVB2Used;
		*pDst = WORD(idx);
		pDst++;
	}
	_nVB2Used += nVtx;
	_nIB2Used += nIB;
		
	return TRUE;
}

BOOL CPatchBuilder::Append(DWORD nVtx,DWORD nIB,const WORD * pIBData,void *&pVtxArray)
{
	_resultApp = _Attempt(nVtx,nIB);
	if(IPatchBuilder::Success!=_resultApp)
		return FALSE;

	pVtxArray  = NULL;
	assert(_bOnAppend);	
	assert(_pVB&&_pIB);
	
	DWORD stride = _vb->GetStride();
	WORD * pDst = _pIB + _nIB2Used;
	for(int i = 0;i<nIB;i++){
		DWORD idx = pIBData[i] + _nVB2Used;
		*pDst = WORD(idx);
		pDst++;
	}

	pVtxArray = _pVB + stride*_nVB2Used;	
	_nVB2Used += nVtx;
	_nIB2Used += nIB;
	
	return TRUE;
}

CPatchBuilder::Result CPatchBuilder::GetResult()
{
	return _resultApp;
}

void CPatchBuilder::Clear()
{
	_nIB2Used = _nVB2Used = 0;	//清除已经使用的空间，从显存的开始位置开始填充
}

void CPatchBuilder::End(void)
{
	_bOnAppend = FALSE;
	_UnLock();
}

BOOL CPatchBuilder::BindPatch(IShader * shader,VBBindArg * arg/* = NULL*/)
{
	if(shader&&_nIB2Used>0){
		VBBindArg argBind;
		if(arg)
			argBind = *arg;
		
		assert(!(_pVB||_pIB));
		argBind.primcount = _nIB2Used/3;
		argBind.primstart = 0;
		shader->BindVB(_vb,_ib,&argBind);
		
		return TRUE;
	}

	return FALSE;
}



