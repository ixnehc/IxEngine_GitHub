// ***************************************************************
//  PatchGeom   version:  1.0   ? date: 03/14/2007
//  -------------------------------------------------------------
//  author:		ixnehc
//  Purpose:  a class  to manage a geometry containing multiple patches
// ***************************************************************
#include "stdh.h"

#include "RenderSystem/IRenderSystem.h"
#include "RenderSystem/IRenderPort.h"

#include "commondefines/general_stl.h"

#include "PatchGeom.h"

#include "timer/profiler.h"


//////////////////////////////////////////////////////////////////////////
//CPatchGeom

BOOL CPatchGeom::Init(IRenderSystem *pRS)
{
	_pRS=pRS;

	Reset((std::vector<DWORD>*)NULL);

	return TRUE;
}


void CPatchGeom::Clear()
{
	Reset(TRUE);

	_pool.Clear();

	Zero();
}

void CPatchGeom::_ClearSubSurf(PGSubSurf *sfSub,BOOL bReleaseVB)
{
	if (!sfSub)
		return;

	if (bReleaseVB)
	{
		SAFE_RELEASE(sfSub->vb);
		SAFE_RELEASE(sfSub->ib);
	}
	for (int i=0;i<sfSub->poolPatch.GetCount();i++)
	{
		PGPatch *pa=sfSub->poolPatch.Get(i);
		if (pa)
		{
			assert(pa->hData);
			_pool.Free(pa->hData);
		}
	}

	sfSub->poolPatch.Reset();
	sfSub->nIdx=0;
	sfSub->nVtx=0;
	sfSub->nIdxActual=0;
	sfSub->nVtxActual=0;
	sfSub->nAppendingVtx=0;
	sfSub->flag|=VBDirty|IBDirty;
}

//clean the sub surf that doesnot contain any patches
void CPatchGeom::_FlushSurf(PGSurf *sf)
{
	int i=0;
	while(i<sf->subs.size())
	{
		PGSubSurf *sfSub=sf->subs[i];
		if ((sfSub->nVtxActual==0)||(sfSub->nIdxActual==0))
		{
			_ClearSubSurf(sfSub);
			sf->subs.erase(sf->subs.begin()+i);
			_subsurfs[sfSub->iSerial]=NULL;
			Safe_Class_Delete(sfSub);
			continue;
		}
		i++;
	}
}



//clear all the surfaces' content(but keep the surfaces themselves)
void CPatchGeom::Reset(BOOL bCleanSurface)
{
	for (int i=0;i<_subsurfs.size();i++)
	{
		_ClearSubSurf(_subsurfs[i]);
		Safe_Class_Delete(_subsurfs[i]);
	}

	_subsurfs.clear();

	for (int i=1;i<_surfs.size();i++)
	{
		if (_surfs[i])
			_surfs[i]->subs.clear();
	}

	if (bCleanSurface)
	{
		for (int i=0;i<_surfs.size();i++)
			Safe_Class_Delete(_surfs[i]);
		_surfs.clear();
	}

	_pool.Reset();
}

void CPatchGeom::Reset(std::vector<DWORD>*PatchPossibleSize,BOOL bCleanSurface)
{
	Reset(bCleanSurface);
	if (PatchPossibleSize)
		_pool.Reset(*PatchPossibleSize);
	else
		_pool.Reset(128,15);//use default sizes(128 byte to 4M)
}

//re-allocate big enough vb/ib to contain the nVtx/nIdx
BOOL CPatchGeom::_ReAllocSubSurf(PGSubSurf *subsurf,DWORD nVtx,DWORD nIdx)
{
	assert(nVtx<=0xffff);
	DWORD delta;
	if (subsurf->maxvtx<nVtx)
	{
		delta=subsurf->surf->deltavtx;
		subsurf->maxvtx+=((nVtx-subsurf->maxvtx)+delta-1)/delta*delta;
		if (subsurf->maxvtx>0xffff)
			subsurf->maxvtx=0xffff;
		SAFE_RELEASE(subsurf->vb);
		subsurf->vb=_pRS->GetVertexMgr()->CreateVB(subsurf->maxvtx,
								subsurf->surf->fvf,1,VBFlag_Dynamic);
		subsurf->flag|=VBDirty;
		subsurf->nVtx=0;
		subsurf->nAppendingVtx=0;
	}
	if (subsurf->maxidx<nIdx)
	{
		delta=subsurf->surf->deltaidx;
		subsurf->maxidx+=((nVtx-subsurf->maxidx)+delta-1)/delta*delta;
		SAFE_RELEASE(subsurf->ib);
		subsurf->ib=_pRS->GetVertexMgr()->CreateIB(subsurf->maxidx,
																		VBFlag_Dynamic);
		subsurf->flag|=IBDirty;
		subsurf->nIdx=0;
	}

	return (subsurf->vb&&subsurf->ib);
}


PGSubSurf* CPatchGeom::_AddSubSurf(FVFEx fvf,DWORD maxvtx,DWORD maxidx)
{
	assert(_pRS);

	IVertexBuffer *vb=_pRS->GetVertexMgr()->CreateVB(maxvtx,fvf,1,VBFlag_Dynamic);
	IIndexBuffer *ib=_pRS->GetVertexMgr()->CreateIB(maxidx,VBFlag_Dynamic);
	if (!ib)
		SAFE_RELEASE(vb);

	if (!vb)
		return NULL;

	PGSubSurf *subsurf=Class_New2(PGSubSurf);

	subsurf->vb=vb;
	subsurf->ib=ib;
	subsurf->stride=vb->GetStride();
	subsurf->maxvtx=maxvtx;
	subsurf->maxidx=maxidx;
	subsurf->nVtx=0;
	subsurf->nIdx=0;
	subsurf->nIdxActual=0;
	subsurf->nVtxActual=0;
	subsurf->nAppendingVtx=0;
	subsurf->flag=(SurfFlag)0;
	subsurf->surf=NULL;
	subsurf->iSerial=-1;

	for (int i=0;i<_subsurfs.size();i++)//find an empty slot
	{
		if (_subsurfs[i]==NULL)
		{
			_subsurfs[i]=subsurf;
			subsurf->iSerial=i;
			return subsurf;
		}
	}
	_subsurfs.push_back(subsurf);
	subsurf->iSerial=_subsurfs.size()-1;
	return subsurf;
}

PGSurfHandle CPatchGeom::AddSurf(FVFEx fvf,DWORD maxvtx,DWORD maxidx,BOOL bMultiVB,DWORD dpt)
{
	if (_surfs.size()<=0)
		_surfs.push_back(NULL);

	PGSurf *surf=Class_New2(PGSurf);
	surf->fvf=fvf;
	surf->dpt=dpt;
	surf->maxvtx=maxvtx;
	surf->maxidx=maxidx;
	surf->bSingleVB=!bMultiVB;
	surf->deltavtx=i_math::clampup_u(surf->maxvtx/2,16);
	surf->deltaidx=i_math::clampup_u(surf->maxidx/2,16);

	surf->iSerial=-1;

	for (int i=1;i<_surfs.size();i++)
	{
		if (!_surfs[i])
		{//an empty slot
			_surfs[i]=surf;
			surf->iSerial=i;
			return (PGSurfHandle)i;
		}
	}
	_surfs.push_back(surf);
	surf->iSerial=_surfs.size()-1;
	return (PGSurfHandle)(surf->iSerial);
}


BOOL CPatchGeom::RemoveSurf(PGSurfHandle surf)
{
	if (surf==NULL)
		return FALSE;
	assert((surf<_surfs.size())&&(surf>0));
	PGSurf* sf=	_surfs[surf];
	for (int i=0;i<sf->subs.size();i++)
	{
		_ClearSubSurf(sf->subs[i]);
		_subsurfs[sf->subs[i]->iSerial]=NULL;
		Safe_Class_Delete(sf->subs[i]);
	}

	sf->subs.clear();
	Safe_Class_Delete(sf);
	_surfs[surf]=NULL;
	return TRUE;
}

void CPatchGeom::_ChangePatch(PGSubSurf *sfSub,MemHandle hPatch,MemHandle hData,int nVtx,int nIdx,DWORD group)
{
	//Fill the patch data
	PGPatch *pa=sfSub->poolPatch.ObtainPtr(hPatch);
	pa->vbase=-1;
	pa->hData=hData;
	pa->cIdx=nIdx;
	pa->cVtx=(WORD)nVtx;
	pa->group=(WORD)group;
	pa->flag=0;

	//Update the surface state
	sfSub->flag|=IBDirty;//content changed,we need rebuild IB
	if (!(sfSub->flag&VBDirty))
	{
		if (nVtx+sfSub->nVtx+sfSub->nAppendingVtx>sfSub->maxvtx)
		{
			sfSub->flag|=VBDirty;//overflow,we need re-build VB
			sfSub->nAppendingVtx=0;
		}
		else
		{
			pa->flag|=Appending;
			sfSub->nAppendingVtx+=nVtx;
		}
	}
	sfSub->nIdxActual+=nIdx;
	sfSub->nVtxActual+=nVtx;

	if (group>=sfSub->priminfo.size())
		sfSub->priminfo.resize(group+1);
}

//find a sub surf that could contain the patch
PGSubSurf *CPatchGeom::_FindSubSurf(PGSurf *sf,DWORD nVtx,DWORD nIdx)
{
	assert(sf);
	int i;
	for (i=0;i<sf->subs.size();i++)
	{
		PGSubSurf *subsurf=sf->subs[i];
		if ((subsurf->nIdxActual+nIdx<subsurf->maxidx)&&
			(subsurf->nVtxActual+nVtx<subsurf->maxvtx))
			break;
		else
		{
			if (sf->bSingleVB)
			{
				if (subsurf->nVtxActual+nVtx>0xffff)
					return NULL;

				if (FALSE==_ReAllocSubSurf(subsurf,subsurf->nVtxActual+nVtx,subsurf->nIdxActual+nIdx))
					return NULL;

				return subsurf;
			}
		}
	}
	if (i>=sf->subs.size())
	{
		PGSubSurf *sfSub=_AddSubSurf(sf->fvf,sf->maxvtx,sf->maxidx);
		if (!sfSub)
			return NULL;
		sf->subs.push_back(sfSub);
		sfSub->surf=sf;
	}

	return sf->subs[i];
}


PGPatchHandle CPatchGeom::AddPatch(PGSurfHandle surf,
								   void *vertice,DWORD nVtx,WORD *indice,DWORD nIdx,DWORD group)
{
	assert((nVtx>0)&&(nIdx>0));
	assert((surf<_surfs.size())&&(surf>0));

	PGSubSurf* sfSub=_FindSubSurf(_surfs[surf],nVtx,nIdx);
	if (!sfSub)
		return NULL;

	//The handle for return
	PatchHandle ph;
	ph.iSubSurf=sfSub->iSerial;
	ph.hPatch=sfSub->poolPatch.Alloc();
	assert(ph.hPatch);

	//Accept the input data into a mem handle
	MemHandle hData;
	if (TRUE)
	{
		DWORD szData=nVtx*sfSub->stride+nIdx*sizeof(WORD);
		hData=_pool.Alloc(szData);
		assert(hData);
		BYTE *p=(BYTE*)_pool.ObtainPtr(hData);
		memcpy(p,vertice,nVtx*sfSub->stride);
		memcpy(p+nVtx*sfSub->stride,indice,nIdx*sizeof(WORD));
	}

	_ChangePatch(sfSub,ph.hPatch,hData,nVtx,nIdx,group);
	return *(PGPatchHandle*)&ph;
}

void CPatchGeom::_FetchPatch(PGSubSurf *sfSub,MemHandle hPatch,PGPatch &paRet)
{
	PGPatch *pa=sfSub->poolPatch.ObtainPtr(hPatch);
	paRet=*pa;
	if (!(sfSub->flag&VBDirty))
	{
		if (pa->flag&Appending)
			sfSub->nAppendingVtx-=pa->cVtx;
	}

	sfSub->nVtxActual-=pa->cVtx;
	sfSub->nIdxActual-=pa->cIdx;

	sfSub->flag|=IBDirty;//content changed,we need rebuild IB

	sfSub->poolPatch.Free(hPatch);

}


BOOL CPatchGeom::RemovePatch(PGPatchHandle patch)
{
	if (patch==NULL)
		return TRUE;
	PatchHandle ph=*(PatchHandle*)&patch;

	assert((ph.iSubSurf<_subsurfs.size())&&(ph.iSubSurf>=0));
	PGSubSurf* sfSub=_subsurfs[ph.iSubSurf];

	//Free the patch data
	PGPatch pa;
	_FetchPatch(sfSub,ph.hPatch,pa);
	_pool.Free(pa.hData);

	return TRUE;
}

BOOL CPatchGeom::RemoveAllPatches(PGSurfHandle surf)
{
	if (surf==NULL)
		return FALSE;
	assert((surf<_surfs.size())&&(surf>0));
	PGSurf* sf=	_surfs[surf];
	for (int i=0;i<sf->subs.size();i++)
		_ClearSubSurf(sf->subs[i],FALSE);
	return TRUE;
}


PGPatchHandle CPatchGeom::UpdatePatch(PGPatchHandle patch,void *vertice,DWORD nVtx,WORD *indice,DWORD nIdx,DWORD group)
{
	PatchHandle ph=*(PatchHandle*)&patch;
	assert((ph.iSubSurf<_subsurfs.size())&&(ph.iSubSurf>=0));
	PGSubSurf* sfSub=_subsurfs[ph.iSubSurf];
	PGSurfHandle surf=(PGSurfHandle)(sfSub->surf->iSerial);

	RemovePatch(patch);

	assert((nVtx>0)&&(nIdx>0));
	if (!((nVtx>0)&&(nIdx>0)))
		return NULL;

	return AddPatch(surf,vertice,nVtx,indice,nIdx,group);
}


PGPatchHandle CPatchGeom::AssignPatch(PGPatchHandle patch,PGSurfHandle surf)
{
	if (patch==NULL)
		return patch;
	PGSubSurf* sfSub;

	PGPatch pa;
	//remove in the old and get the data out
	if (TRUE)
	{
		PatchHandle ph=*(PatchHandle*)&patch;
		assert((ph.iSubSurf<_subsurfs.size())&&(ph.iSubSurf>=0));
		sfSub=_subsurfs[ph.iSubSurf];
		if (sfSub->surf==_surfs[surf])
			return patch;//No change

		_FetchPatch(sfSub,ph.hPatch,pa);
	}

	//Add to new surface
	sfSub=_FindSubSurf(_surfs[surf],pa.cVtx,pa.cIdx);
	if (!sfSub)
		return NULL;

	//The handle for return
	PatchHandle ph;
	ph.iSubSurf=sfSub->iSerial;
	ph.hPatch=sfSub->poolPatch.Alloc();
	assert(ph.hPatch);

	_ChangePatch(sfSub,ph.hPatch,pa.hData,pa.cVtx,pa.cIdx,pa.group);

	return *(PGPatchHandle*)&ph;
}

BOOL CPatchGeom::SetPatchGroup(PGPatchHandle patch,DWORD group)
{
	if (patch==NULL)
		return FALSE;
	PGSubSurf* sfSub;

	PGPatch *pa;
	PatchHandle ph=*(PatchHandle*)&patch;
	assert((ph.iSubSurf<_subsurfs.size())&&(ph.iSubSurf>=0));
	sfSub=_subsurfs[ph.iSubSurf];
	pa=sfSub->poolPatch.ObtainPtr(ph.hPatch);
	if(pa->group==group)
		return TRUE;//no change

	pa->group=(WORD)group;
	sfSub->flag|=IBDirty;
	if (group>=sfSub->priminfo.size())
		sfSub->priminfo.resize(group+1);

	return TRUE;

}


VBHandles CPatchGeom::_GetVB(PGSubSurf *sfSub,DWORD &ps,DWORD &pc,DWORD group,DWORD dpt)
{
	static std::vector<WORD>temp;
	static std::vector<PGPatch*>temp2;

	if (sfSub->flag&(VBDirty|IBDirty))
	{
//		ProfilerStart_Recent(_GetVB_Core)

		BYTE *pv=NULL;
		WORD*pi;
		pi=(WORD*)sfSub->ib->Lock(TRUE);
		if (sfSub->flag&VBDirty)
		{
			//Make a total build
			pv=(BYTE*)sfSub->vb->Lock(TRUE);
			sfSub->nVtx=0;
		}
		sfSub->nIdx=0;

		PGPatch*pa;
		PGPatch **patches;
		DWORD c;
		if (TRUE)
		{
			temp2.resize(sfSub->poolPatch.GetCount());
			c=0;
			DWORD n=sfSub->poolPatch.GetCount();
			for (int i=0;i<n;i++)
			{
				pa=sfSub->poolPatch.Get(i);
				if (!pa)
					continue;

				temp2[c]=pa;
				c++;
			}
			temp2.resize(c);
		}

		patches=&temp2[0];

		VEC_SET(sfSub->priminfo,0);

		int i=0;
		while(sfSub->nIdx<sfSub->nIdxActual)
		{
			DWORD curgroup=0xffffffff;

			i=0;

			for (;i<c;i++)
			{
				pa=patches[i];

				if (curgroup==0xffffffff)
				{
					if (sfSub->priminfo[pa->group].ic!=0)//processed
						continue;
					curgroup=pa->group;
					sfSub->priminfo[curgroup].is=sfSub->nIdx;
					assert(pa->cIdx>0);
				}
				else
				{
					if (pa->group!=curgroup)
						continue;
				}


				if (sfSub->nIdx+pa->cIdx>sfSub->maxidx)
				{
					assert(FALSE);
					break;
				}

				BYTE *data=(BYTE*)_pool.ObtainPtr(pa->hData);
				assert(data);
				DWORD szVtx=pa->cVtx*sfSub->stride;

				if ((pa->flag&Appending)||(sfSub->flag&VBDirty))
				{
					if (sfSub->nVtx+pa->cVtx>sfSub->maxvtx)
					{
						assert(FALSE);
						break;
					}

					if (!pv)
					{
						pv=(BYTE*)sfSub->vb->Lock(NULL,FALSE);//with No-Overwrite flag
						pv+=sfSub->nVtx*sfSub->stride;
					}

					memcpy(pv,data,szVtx);
					pv+=szVtx;

					pa->vbase=sfSub->nVtx;
					sfSub->nVtx+=pa->cVtx;
				}

				if (TRUE)
				{
					temp.resize(pa->cIdx);
					if (TRUE)
					{
						WORD *p=&temp[0];
						WORD *q=(WORD*)(data+szVtx);

						for (int j=0;j<pa->cIdx;j++)
						{
							assert(pa->vbase!=-1);
							p[j]=q[j]+pa->vbase;
						}
					}
					memcpy(pi,&temp[0],pa->cIdx*sizeof(WORD));
					pi+=pa->cIdx;

					sfSub->nIdx+=pa->cIdx;
				}

				pa->flag=0;
			}
			assert(curgroup!=0xffffffff);
			if (curgroup==0xffffffff)
				break;
			sfSub->priminfo[curgroup].ic=sfSub->nIdx-sfSub->priminfo[curgroup].is;
		}

		if (pv)
			sfSub->vb->Unlock();

		sfSub->ib->Unlock();

		sfSub->nAppendingVtx=0;
		sfSub->flag=0;

//		ProfilerEnd();
	}

	assert(sfSub->nIdx==sfSub->nIdxActual);

	ps=pc=0;
	if (group>=sfSub->priminfo.size())
		return VBHandles();
	if (sfSub->priminfo[group].ic==0)
		return VBHandles();
	switch(dpt)
	{
		case D3DPT_TRIANGLELIST:
			ps=sfSub->priminfo[group].is/3;
			pc=sfSub->priminfo[group].ic/3;
			break;
		case D3DPT_TRIANGLESTRIP:
			ps=sfSub->priminfo[group].is;
			pc=sfSub->priminfo[group].ic-2;
			break;
		default:
			assert(FALSE);
	}
	return VBHandles(sfSub->vb,sfSub->ib);
}

DWORD CPatchGeom::GetVBCount(PGSurfHandle surf)
{
	if (surf==NULL)
		return 0;
	assert((surf<_surfs.size())&&(surf>0));
	_FlushSurf(_surfs[surf]);
	return _surfs[surf]->subs.size();
}

FVFEx CPatchGeom::GetFVF(PGSurfHandle surf)
{
	if (surf==NULL)
		return 0;
	return _surfs[surf]->fvf;
}



VBHandles CPatchGeom::GetVB(PGSurfHandle surf,DWORD idx,DWORD &ps,DWORD &pc,DWORD group)
{
	if (surf==NULL)
		return VBHandles();
	assert((surf<_surfs.size())&&(surf>0));
	PGSurf* sf=	_surfs[surf];
	if (idx>=sf->subs.size())
		return VBHandles();
	return _GetVB(sf->subs[idx],ps,pc,group,sf->dpt);
}

BOOL CPatchGeom::DrawSurf(IRenderer *rdr,PGSurfHandle surf,DWORD group,BOOL bWireframe)
{
// 	rdr->GetDevice()->FlushCommand();
// 	ProfilerStart(ttt);

	DWORD c=GetVBCount(surf);
	VBHandles vbh;
	VBBindArg arg;
	arg.SetFillMode(bWireframe?2:3);
	BOOL bRet=TRUE;
	for (int i=0;i<c;i++)
	{

		vbh=GetVB(surf,i,(DWORD&)arg.primstart,(DWORD&)arg.primcount,group);
		if (vbh.IsEmpty())
			continue;


		if (FALSE==rdr->BindVB(vbh,arg))
		{
			bRet=FALSE;
			continue;
		}


		if (FALSE==rdr->Render())
			bRet=FALSE;

	}
// 	rdr->GetDevice()->FlushCommand();
// 	ProfilerEnd();

	return bRet;
}




void CPatchGeom::CollectStats(PGStat &stat)
{
	memset(&stat,0,sizeof(stat));

	_pool.CollectStat(stat.PhysMemUsed,stat.PhysMemAlloc);

	for (int i=0;i<_subsurfs.size();i++)
	{
		PGSubSurf *sfSub=_subsurfs[i];
		if (sfSub)
		{
			stat.nSubSurf++;
			stat.VBMemActual+=sfSub->nVtxActual*sfSub->stride;
			stat.VBMemUsed+=sfSub->nVtx*sfSub->stride;
			stat.VBMemAlloc+=sfSub->maxvtx*sfSub->stride;
			stat.IBMemActual+=sfSub->nIdxActual*sizeof(WORD);
			stat.IBMemUsed+=sfSub->nIdx*sizeof(WORD);
			stat.IBMemAlloc+=sfSub->maxidx*sizeof(WORD);

			DWORD u,a;
			sfSub->poolPatch.CollectStat(u,a);
			stat.PhysMemUsed+=u;
			stat.PhysMemAlloc+=a;
		}
	}



	for (int i=0;i<_surfs.size();i++)
	{
		PGSurf *sf=_surfs[i];
		if (sf)
		{
			stat.nSurf++;
			if (sf->subs.size()>stat.MaxSubSurfInSurf)
				stat.MaxSubSurfInSurf=sf->subs.size();
		}
	}
}

void CPatchGeom::OnDeviceLost()
{
	//全部标记为dirty
	for (int i=0;i<_subsurfs.size();i++)
	{
		PGSubSurf *sfSub=_subsurfs[i];
		if (!sfSub)
			continue;
		sfSub->flag|=VBDirty|IBDirty;
	}
}
