
#include "RenderSystem/IPatchTools.h"

class CPatchBuilder :public IPatchBuilder
{
public:
	CPatchBuilder(void);

	~CPatchBuilder(void);

	virtual BOOL Create(IRenderSystem * pRS,const FVFEx &fvf,DWORD nVtx,DWORD nIBs);  //创建顶点缓冲区

	virtual void Destroy();

	virtual BOOL Begin(const FVFEx & fvf);
	
	virtual BOOL Append(DWORD nVtx,DWORD nIB,const void * pVBData,const WORD * pIBData);

	virtual BOOL Append(DWORD nVtx,DWORD nIB,const WORD * pIBData,void *&pVtxArray);

	virtual void Clear();

	virtual void End(void);

	virtual BOOL BindPatch(IShader * shader,VBBindArg * arg = NULL);
	
	virtual Result GetResult();

protected:	
	
	Result _Attempt(DWORD nVtx,DWORD nIB);//尝试添加 如果失败表明已经满 或 失败，本次是否可以添加

	void _SetBad(){_bBad = TRUE;}

	BOOL _IsBad(){return _bBad;}//是否发生不可恢复的错误

	void _Clean(void);

	void _Zero(void);

	void _Lock(void);

	void _UnLock(void);

private:
	IIndexBuffer * _ib;
	IVertexBuffer * _vb;

	BYTE * _pVB;  //Lock时返回的指针
	WORD * _pIB;

	DWORD _nVB2Used;	//已经使用的个数
	DWORD _nIB2Used;

	BOOL _bOnAppend;	//是否正在添加顶点

	BOOL _bBad;

	IRenderSystem *_pRS;
	FVFEx _fvf;
	
	Result _resultApp;
};


