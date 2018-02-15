

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
	//ע��:��������û��ʹ�ö�̬��vb,�����þ�̬��vb,��Ϊ�����Dynamic��vb�Ļ�,�����֡�����ȶ�������(��ĳЩ�����,֡����
	//��úܵ�,��֪��Ϊʲô),��ʹ�þ�̬��vb��,�Ͳ��������������
	//��Ȼ���ʹ�þ�̬��vb�Ļ�,���ܻ�����������,��Ҫ����Ĳ���
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

	//��������״̬
	DWORD nVtxTotal = _vb->GetCount();
	DWORD nIBToatal = _ib->GetCount();
	if(nVtxTotal-_nVB2Used<nVtx||nIBToatal-_nIB2Used<nIB){
		_UnLock();

		//����vb��ib�Ĵ�С�����������һ��Patchʱ
		if(_nVB2Used==0)
		{
			DWORD nVtx2Adj = max(nVtxTotal,nVtx);
			DWORD nIB2Adj = max(nIBToatal,nIB);
			_Clean();

			//����ʧ��
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
	_nIB2Used = _nVB2Used = 0;	//����Ѿ�ʹ�õĿռ䣬���Դ�Ŀ�ʼλ�ÿ�ʼ���
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



