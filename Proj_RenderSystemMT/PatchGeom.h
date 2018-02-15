#pragma once

#include "RenderSystem/IPatchTools.h"

#include "mempool/mempool.h"

#include <assert.h>


#include "class/class.h"

struct PGPrimInfo
{
	int is;
	int ic;
};

struct PGPatch
{
	int vbase;//if -1,the patch does not has its vertex data filled into its surface's vb
	MemHandle hData;
	DWORD cIdx;
	WORD cVtx;
	WORD flag:2;
	WORD group:14;
};


struct PGSurf;
struct PGSubSurf
{
	DEFINE_CLASS(PGSubSurf);
	IVertexBuffer *vb;
	IIndexBuffer *ib;
	DWORD stride;//VB stride
	DWORD maxvtx;
	DWORD maxidx;
	DWORD nIdx;//count of indice in vb
	DWORD nVtx;//count of vertice in vb
	DWORD nIdxActual;//Actual indice count contained in all the patches
	DWORD nVtxActual;//Actual vertice count contained in all the patches

	DWORD nAppendingVtx;

	DWORD flag;

	std::vector<PGPrimInfo>priminfo;

	CMemPool_v<PGPatch> poolPatch;

	int iSerial;//index in _subsurfs
	PGSurf *surf;
};

struct PGSurf
{
	DEFINE_CLASS(PGSurf);
	std::vector<PGSubSurf *> subs;
	DWORD maxvtx;
	DWORD maxidx;
	FVFEx fvf;
	int iSerial;//index in _surfs
	BOOL bSingleVB;//whether the surf could only contain at most 1 sub surf,in which case
	//the vb/ib would be re-allocated when vertex/index count
	//exceeds the capacity of the buffer size
	DWORD deltavtx;//the amount step to increase when re-allocating vertex buffer
	DWORD deltaidx;//the amount step to increase when re-allocating index buffer
	DWORD dpt;
};



class CPatchGeom:public IPatchGeom
{
public:
	IMPLEMENT_REFCOUNT_C;
	DEFINE_CLASS_MP(CPatchGeom)
	CPatchGeom()
	{
		Zero();
	}
	~CPatchGeom()
	{
		Clear();
	}
	void Zero()
	{
		_pRS=NULL;
	}

	BOOL Init(IRenderSystem *pRS);

	void Clear();
	virtual void Reset(BOOL bCleanSurface=FALSE);

	virtual void Reset(std::vector<DWORD>*PatchPossibleSize,BOOL bCleanSurface=FALSE);

	virtual PGSurfHandle AddSurf(FVFEx fvf,DWORD maxvtx,DWORD maxidx,BOOL bMultipleVB=TRUE,DWORD dpt=4);//4 is D3DPT_TRIANGLELIST
	virtual BOOL RemoveSurf(PGSurfHandle surf);
	virtual PGPatchHandle AddPatch(PGSurfHandle surf,void *vertice,DWORD nVtx,WORD *indice,DWORD nIdx,DWORD group);//return -1 if fail
	virtual BOOL RemovePatch(PGPatchHandle patch);
	virtual BOOL RemoveAllPatches(PGSurfHandle surf);
	virtual PGPatchHandle UpdatePatch(PGPatchHandle patch,void *vertice,DWORD nVtx,WORD *indice,DWORD nIdx,DWORD group);
	virtual PGPatchHandle AssignPatch(PGPatchHandle patch,PGSurfHandle surf);
	virtual BOOL SetPatchGroup(PGPatchHandle patch,DWORD group);
	virtual DWORD GetVBCount(PGSurfHandle surf);
	virtual VBHandles GetVB(PGSurfHandle surf,DWORD idx,DWORD &ps,DWORD &pc,DWORD group);
	virtual FVFEx GetFVF(PGSurfHandle surf);

	virtual BOOL DrawSurf(IRenderer *rdr,PGSurfHandle surf,DWORD group=0,BOOL bWireframe=FALSE);


	virtual void CollectStats(PGStat &stat);

	void OnDeviceLost();

protected:
	enum SurfFlag
	{
		IBDirty=1,		
		VBDirty=2,
	};
	enum PatchFlag
	{
		Appending=1,//this patch is in the appending queue
	};

	struct PatchHandle
	{
		DWORD hPatch:20;
		DWORD iSubSurf:12;
	};



	PGSubSurf* _AddSubSurf(FVFEx fvf,DWORD maxvtx,DWORD maxidx);
	PGSubSurf *_FindSubSurf(PGSurf *sf,DWORD nVtx,DWORD nIdx);//find a sub surf that could contain the patch
	BOOL _ReAllocSubSurf(PGSubSurf *sf,DWORD nVtx,DWORD nIdx);//re-allocate big enough vb/ib to contain the nVtx/Idx
	void _ClearSubSurf(PGSubSurf *sf,BOOL bReleaseVB=TRUE);
	void _ChangePatch(PGSubSurf *sf,MemHandle hPatch,MemHandle hData,int nVtx,int nIdx,DWORD group);
	void _FetchPatch(PGSubSurf *sf,MemHandle hPatch,PGPatch &pa);

	void _FlushSurf(PGSurf *sf);//clean the sub surf that doesnot contain any patches

	VBHandles _GetVB(PGSubSurf *sfSub,DWORD &ps,DWORD &pc,DWORD group,DWORD dpt);

	std::vector<PGSubSurf*>_subsurfs;

	std::vector<PGSurf *>_surfs;

	CMemPoolEx _pool;//for all the cached vtx/idx data

	IRenderSystem *_pRS;
};
