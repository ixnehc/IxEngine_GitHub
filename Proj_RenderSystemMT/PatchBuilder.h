
#include "RenderSystem/IPatchTools.h"

class CPatchBuilder :public IPatchBuilder
{
public:
	CPatchBuilder(void);

	~CPatchBuilder(void);

	virtual BOOL Create(IRenderSystem * pRS,const FVFEx &fvf,DWORD nVtx,DWORD nIBs);  //�������㻺����

	virtual void Destroy();

	virtual BOOL Begin(const FVFEx & fvf);
	
	virtual BOOL Append(DWORD nVtx,DWORD nIB,const void * pVBData,const WORD * pIBData);

	virtual BOOL Append(DWORD nVtx,DWORD nIB,const WORD * pIBData,void *&pVtxArray);

	virtual void Clear();

	virtual void End(void);

	virtual BOOL BindPatch(IShader * shader,VBBindArg * arg = NULL);
	
	virtual Result GetResult();

protected:	
	
	Result _Attempt(DWORD nVtx,DWORD nIB);//������� ���ʧ�ܱ����Ѿ��� �� ʧ�ܣ������Ƿ�������

	void _SetBad(){_bBad = TRUE;}

	BOOL _IsBad(){return _bBad;}//�Ƿ������ɻָ��Ĵ���

	void _Clean(void);

	void _Zero(void);

	void _Lock(void);

	void _UnLock(void);

private:
	IIndexBuffer * _ib;
	IVertexBuffer * _vb;

	BYTE * _pVB;  //Lockʱ���ص�ָ��
	WORD * _pIB;

	DWORD _nVB2Used;	//�Ѿ�ʹ�õĸ���
	DWORD _nIB2Used;

	BOOL _bOnAppend;	//�Ƿ�������Ӷ���

	BOOL _bBad;

	IRenderSystem *_pRS;
	FVFEx _fvf;
	
	Result _resultApp;
};


