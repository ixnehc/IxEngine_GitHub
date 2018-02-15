
#pragma  once

#include "RenderSystem/ISpeedTree.h"


#include "fvfex/fvfex.h"

#include "trisampler/trisampler.h"

class CSptTriSampAdapter :public ISptTriSampleAdapter
{
public:
	CSptTriSampAdapter(){_pSpt = NULL;}

	virtual TriSample * BuildBranchTriSamples(DWORD &nSample,int lvl);
	virtual TriSample * BuildFrondTriSamples(DWORD &nSample,int lvl);
	virtual TriSample * BuildEnvSamples(DWORD &nSample,DWORD &w);

	//call after BranchTriSampVtx or FrondTriSampVtx 
	virtual i_math::vector3df * GetVtxPos(DWORD & nVtx);
	virtual void Release();

	void SetOwner(ISpt *pSpt){_pSpt = pSpt;}
protected:
	// the pointer can't be retain for later using
	BOOL BranchTriSampVtx(void * &pVtx,DWORD & nVtx,WORD * &pIB,DWORD &nIBs);
	BOOL FrondTriSampVtx(void * &pVtx,DWORD & nVtx,WORD * &pIB,DWORD &nIBs);
	FVFEx GetUvFVF()  { return FVFEX_FLAG_TEX0;}
	FVFEx GetVtxFVF(){return FVFEX_FLAG_TEX0|FVFEX_XYZ0|FVFEX_NORMAL0|FVFEX_BINORMAL|FVFEX_TANGENT;}

	void _TriSampVtxFromVB(IVertexBuffer * vb);
	void _TriSampIndexFromIB(IIndexBuffer * ib);
	void _InitTrans();
	BOOL _CheckUv(i_math::vector2df * uvs);
	std::vector<BYTE> _sampBufferVB;
	std::vector<WORD> _sampBufferIB;
	std::vector<i_math::vector3df> _posVtxs;
	std::vector<i_math::vector2df> _uvVtxs;
	std::vector<TriSample> _triSamples;
	DWORD _nVtx,_nIBs;
	ISpt * _pSpt;
};