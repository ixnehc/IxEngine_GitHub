/********************************************************************
	created:	3:8:2006   15:21
	filename: 	e:\IxEngine\Proj_RenderSystem\MeshMgr.cpp
	author:		cxi
	
	purpose:	CMesh & CMeshMgr (implement of IMesh & IMeshMgr)
*********************************************************************/
#include "stdh.h"

#pragma warning(disable:4018)
#pragma warning(disable:4267)

#include "RenderSystem.h"

#include "RenderSystem/ITools.h"

#include "MeshMgr.h"
#include "DeviceObject.h"

#include "VertexMgr.h"
#include <assert.h>

#include "Log/LogFile.h"

#include "fvfex/fvfex.h"

#include "affinespace/affinespace.h"
#include "rasterize/rasterize.h"

#include "timer/profiler.h"
#include "spatialtester/spatialtester.h"

#include "ShaderLibMgr.h"

#include "stringparser/stringparser.h"

#include "trisampler/trisampler.h"

#define MESHFRAME_EQUAL(f1,f2) (fabs((f1)-(f2))<0.01f)

#define IMATRIX_VERSION(mat) (((mat)==NULL)?0:(mat)->GetVer())

//////////////////////////////////////////////////////////////////////////
//CMeshSnapshot
void CMeshSnapshot::Zero()
{
	_iCurCache=-1;

	_pMesh=NULL;
}

void CMeshSnapshot::Clean()
{
	_cache.clear();

	SAFE_RELEASE(_pMesh);

	Zero();
}


DWORD CMeshSnapshot::GetFrameCount()
{
	if (!_pMesh)
		return 0;
	return _pMesh->GetFrameCount();
}


void CMeshSnapshot::Attach(CMesh *pMesh)
{
	if (_pMesh==pMesh)
		return;

	pMesh->AddRef();
	SAFE_RELEASE(_pMesh);

	Clean();//Clean all the cache

	_pMesh=pMesh;
}

BOOL CMeshSnapshot::TakeSnapshot(IMatrice43 *mat,MeshSnapshotArg &msa)
{
	static matrix43f matIdentity;

	matrix43f *mats=&matIdentity;
	DWORD nMats=1;
	if (mat)
	{
		matrix43f *t=mat->GetPtr();
		if (t)
		{
			mats=t;
			nMats=mat->GetCount();
		}
		else
			return FALSE;
	}

	return _TakeSnapshot(mats,nMats,mat,msa);
}

BOOL CMeshSnapshot::TakeSnapshot(i_math::matrix43f &mat,MeshSnapshotArg &msa)
{
	return _TakeSnapshot(&mat,1,NULL,msa);
}


BOOL CMeshSnapshot::_TakeSnapshot(matrix43f *mats,DWORD nMats,IMatrice43 *matrice,MeshSnapshotArg &msa)
{
	if (!_pMesh)
		return FALSE;
	Attach(_pMesh);//attach to refresh,(this will add ref internally)

	int iCache=-1;
	if (matrice)
	{
		for (int i=0;i<_cache.size();i++)//find in existing cache
		{
			Cache *cch=&_cache[i];
			if (MESHFRAME_EQUAL(cch->fFrame,msa.fFrame))
			{
				if (cch->matrice==matrice)
				{
					if (cch->verMatrice==IMATRIX_VERSION(matrice))
					{//Cache hit
						_iCurCache=i;
						return TRUE;
					}
					else
					{
						iCache=i;
						break;//We need update this cache
					}
				}
			}
		}
	}

	if (A_Ok!=_pMesh->Touch())
		return FALSE;

	Cache *cch;
	if (iCache!=-1)
		_iCurCache=iCache;
	else
	{
		_cache.resize(_cache.size()+1);
		_iCurCache=_cache.size()-1;
	}
	cch=&_cache[_iCurCache];


	std::vector<BYTE> &buf=cch->data;
	DWORD nVtx;
	FVFEx fvf;
	if (TRUE)
	{
		BOOL bUsingSkeleton=FALSE;

		if (_pMesh->_nSkeletonBones>0)
		{
			if (nMats==_pMesh->_nSkeletonBones)
				bUsingSkeleton=TRUE;
			else
			{//Not using skeleton,should be a single matrix
				if (nMats!=1)
					return FALSE;
			}
		}

		DWORD stride;
		std::vector<matrix43f>vecBoneMats;
		int nWeights=0;
		if (TRUE)//copy the vb data out
		{
			DWORD iFrame;
			IVertexBuffer *vb;
			IIndexBuffer *ib;
			if (FALSE==_pMesh->_ResolveVB(msa.fFrame,msa.iLod,vb,ib,iFrame))
				return FALSE;
			if (!vb)
				return FALSE;

			buf.resize(vb->GetSize());
			if (TRUE)
			{
				vector3df *vbuf=(vector3df *)vb->Lock(FALSE,0,iFrame);
				if (!vbuf)
					return FALSE;
				memcpy(&buf[0],vbuf,buf.size());
				vb->Unlock();
			}
			fvf=vb->GetFVF();
			nVtx=vb->GetCount();
			stride=vb->GetStride();

			if (bUsingSkeleton)
			{
				vecBoneMats.resize(_pMesh->_bones.size());
				for (int i=0;i<vecBoneMats.size();i++)
				{
					if (_pMesh->_bones[i]>=nMats)
						return FALSE;
					vecBoneMats[i]=mats[_pMesh->_bones[i]];
				}

				nWeights=_pMesh->_nWeight;
			}
		}

		FVFEx fvfT[]=
		{
			FVFEX_XYZ0,
			FVFEX_XYZ1,
			FVFEX_NORMAL0,
			FVFEX_NORMAL1,
			FVFEX_TANGENT,
			FVFEX_BINORMAL,
			FVFEX_FLAG_TEX0,
			FVFEX_FLAG_TEX1,
			FVFEX_FLAG_TEX2,
			FVFEX_FLAG_TEX3,
			FVFEX_FLAG_TEX4,
			FVFEX_FLAG_TEX5,
			FVFEX_FLAG_TEX6,
			FVFEX_FLAG_TEX7,
		};
		//mode: 0,transform,1,rotate,2,no change
		int modeT[]=
		{
			1,
			1,
			0,
			0,
			0,
			0,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
			2,
		};

		assert(ARRAY_SIZE(fvfT)==ARRAY_SIZE(modeT));

		vector3df *vbuf=NULL;
		f32 *weight0=NULL,*weight;
		BYTE *boneindice0=NULL,*boneindice;

		if (bUsingSkeleton)
		{
			weight0=(f32*)(&buf[0]+fvfOffset(fvf,FVFEX_WEIGHT(nWeights)));
			if (((BYTE*)weight0)<&buf[0])
				weight0=NULL;
			boneindice0=&buf[0]+fvfOffset(fvf,FVFEX_BONEINDICE0);
			if (((BYTE*)boneindice0)<&buf[0])
				return FALSE;
		}

		vector3df t;
		for (int k=0;k<ARRAY_SIZE(fvfT);k++)
		{
			vbuf=(vector3df*)(&buf[0]+fvfOffset(fvf,fvfT[k]));
			if (((BYTE*)vbuf)<&buf[0])
				continue;//doesnot exist

			if (bUsingSkeleton&&(modeT[k]!=2))
			{
				weight=weight0;
				boneindice=boneindice0;
				//Do the final calculation
				f32 lastweight; 
				for (int i=0;i<nVtx;i++)
				{
					vector3df r(0,0,0);

					lastweight=0.0f;
					int j;
					for (j=0;j<nWeights-1;j++)
					{
						if (modeT[k])
							vecBoneMats[boneindice[j]].transformVect(*vbuf,t);
						else
							vecBoneMats[boneindice[j]].rotateVect(*vbuf,t);
						r+=t*weight[j];
						lastweight+=weight[j];
					}
					if (modeT[k])
						vecBoneMats[boneindice[j]].transformVect(*vbuf,t);
					else
						vecBoneMats[boneindice[j]].rotateVect(*vbuf,t);
					r+=t*(1.0f-lastweight);
					if (!modeT[k])
						r.normalize();//for normal,we normalize it
					*vbuf=r;

					//step forward
					(BYTE*&)vbuf+=stride;
					if (weight)
						(BYTE*&)weight+=stride;
					(BYTE*&)boneindice+=stride;
				}
			}
			else
			{
				if (modeT[k]==1)
				{
					for (int i=0;i<nVtx;i++)
					{
						mats[0].transformVect(*vbuf,t);
						*vbuf=t;
						(BYTE*&)vbuf+=stride;
					}
				}
				if (modeT[k]==0)
				{
					for (int i=0;i<nVtx;i++)
					{
						mats[0].rotateVect(*vbuf,t);
						*vbuf=t;
						(BYTE*&)vbuf+=stride;
					}
				}
			}
		}
	}


	cch->fFrame=msa.fFrame;
	cch->matrice=matrice;
	cch->verMatrice=IMATRIX_VERSION(matrice);

	cch->fvf=fvf;
	cch->count=nVtx;

	if (TRUE)//The UVAtlas info
	{
		memset(cch->ai,0,sizeof(cch->ai));
		for (int i=0;i<_pMesh->_atlases.size();i++)
			cch->ai[_pMesh->_atlases[i].channel]=_pMesh->_atlases[i];
	}


	return TRUE;
}

DWORD CMeshSnapshot::GetVBCount()
{
	if (_iCurCache==-1)
		return 0;
	return _cache[_iCurCache].count;
}

IVertexBuffer *CMeshSnapshot::_GetCurVB()
{
	if (_iCurCache==-1)
		return NULL;
	return _pMesh->_vb;
}


DWORD CMeshSnapshot::GetIBCount(DWORD idx)
{
	if (_iCurCache==-1)
		return 0;
	if(idx>=_pMesh->_lods.size())
		return 0;

	if (!_pMesh->_lods[idx].ib)
		return 0;

	return _pMesh->_lods[idx].ib->GetCount();
}

FVFEx CMeshSnapshot::GetFVF()
{
	if (_iCurCache==-1)
		return 0;
	return _cache[_iCurCache].fvf;
}

WORD *CMeshSnapshot::GetIndices(DWORD idx)
{
	static std::vector<BYTE>buf;
	if (_iCurCache==-1)
		return NULL;
	if (idx>=_pMesh->_lods.size()&&
		!_pMesh->_lods[idx].ib)
		return NULL;

	void *data=_pMesh->_lods[idx].ib->Lock(FALSE);
	if(!data)
		return NULL;

	buf.resize(_pMesh->_lods[idx].ib->GetSize());

	memcpy(&buf[0],data,buf.size());

	_pMesh->_lods[idx].ib->Unlock();

	return (WORD*)&buf[0];
}

void *CMeshSnapshot::GetVertices(FVFEx fvf,DWORD *stride)
{
	static std::vector<BYTE>buf;
	if (_iCurCache==-1)
		return 0;

	Cache *cch=&_cache[_iCurCache];

	if (fvf==0)
	{
		if (stride)
			*stride=fvfSize(cch->fvf);
		return &cch->data[0];
	}

	if ((fvf&cch->fvf)!=fvf)
		return NULL;
	DWORD sz=fvfSize(fvf);
	buf.resize(cch->count*sz);

	fvfCopy(cch->count,&buf[0],fvf,&cch->data[0],cch->fvf);
	if (stride)
		*stride=sz;
	return &buf[0];
}

i_math::vector3df *CMeshSnapshot::GetPos()
{
	return (i_math::vector3df *)GetVertices(FVFEX_XYZ0,NULL);
}

i_math::vector3df *CMeshSnapshot::GetNormal()
{
	return (i_math::vector3df *)GetVertices(FVFEX_NORMAL0,NULL);
}

BOOL CMeshSnapshot::GetUVAtlasInfo(DWORD channel,UVAtlasInfo &ai)
{
	if (channel>=MAX_TEXTURE)
		return FALSE;
	if (_iCurCache==-1)
		return FALSE;

	Cache *cch=&_cache[_iCurCache];

	if ((cch->ai[channel].w==0)&&(cch->ai[channel].h==0))
		return FALSE;
	assert(FVFEX_FLAG_TEX(channel)&cch->fvf);
	ai=cch->ai[channel];
	return TRUE;
}


//find the first uv atlas channel available
//search from start(including start)
//return -1 on failure
int CMeshSnapshot::FindUVAtlasChannel(DWORD start)
{
	if (_iCurCache==-1)
		return -1;

	Cache *cch=&_cache[_iCurCache];

	for (int i=start;i<MAX_TEXTURE;i++)
	{
		if ((cch->ai[i].w!=0)||(cch->ai[i].h!=0))
			return i;
	}
	return -1;
}



TriSample *CMeshSnapshot::GetMeshSamples(MeshSampleArg &arg,DWORD &nSamples)
{
	static CTriSampler sampler;

	nSamples=0;

	int channel=arg.channel;
	DWORD w,h;
	w=arg.w;
	h=arg.h;

	if (channel==-1)
		channel=FindUVAtlasChannel();

	if (channel==-1)
		return NULL;

	if (!(GetFVF()&FVFEX_FLAG_TEX(channel)))
		return NULL;

	if ((w==0)||(h==0))
	{
		UVAtlasInfo ai;
		if (!GetUVAtlasInfo(channel,ai))
			return NULL;
		w=ai.w;
		h=ai.h;
	}

	FVFEx fvfDest;
	if (!arg.bTsi)
		fvfDest=FVFEX_XYZ0|FVFEX_NORMAL0|FVFEX_FLAG_TEX(channel);
	else
		fvfDest=FVFEX_XYZ0|FVFEX_NORMAL0|FVFEX_BINORMAL|FVFEX_TANGENT|FVFEX_FLAG_TEX(channel);

	if ((fvfDest&GetFVF())!=fvfDest)
		return NULL;//缺少必须的顶点格式

	void *vertices=GetVertices(fvfDest,NULL);

	return sampler.Build(vertices,GetVBCount(),fvfDest,FVFEX_FLAG_TEX(channel),
										GetIndices(0),GetIBCount(0),
										w,h,nSamples,_pMesh->GetPath());
}


BOOL CMeshSnapshot::HitTest(i_math::line3df &ray,float &dist)
{
	dist=0.0f;
	DWORD n=GetIBCount(0);

	WORD *indices=GetIndices(0);

	assert(n%3==0);
	i_math::vector3df *vertices=GetPos();

	BOOL bHit=FALSE;
	i_math::triangle3df tri;
	float dist2Min=1000000000000.0f;
	for (int i=0;i<n;i+=3)
	{
		tri.set(vertices[indices[i]],vertices[indices[i+1]],vertices[indices[i+2]]);

		if ((tri.pointA==tri.pointB)||(tri.pointB==tri.pointC)||(tri.pointA==tri.pointC))
			continue;//如果triangle为一条线段或者一个点
		i_math::vector3df out;
		if (tri.getSafeIntersectionWithLimitedLine(ray,out))
		{
			float dist2=(float)((out-ray.start).getLengthSQ());
			if (dist2<dist2Min)
				dist2Min=dist2;

			bHit=TRUE;
		}
	}

	if (bHit)
		dist=sqrt(dist2Min);
	return bHit;
}


//////////////////////////////////////////////////////////////////////////
//CMesh
IMPLEMENT_CLASS(CMesh);

CMesh::CMesh()
{
	_Zero();
}


void CMesh::_Zero()
{
	_nSkeletonBones=NULL;

	_nWeight=0;
	_flag=0;
	_vbCache=_vb=NULL;
	_ibCache=NULL;
	_lods.clear();
	_fpCache.setEmpty();

	_aabb.reset(0,0,0);
}

void CMesh::_Clean()
{
	_frames.clear();
	SAFE_RELEASE(_vb);
	SAFE_RELEASE(_vbCache);
	for(int i = 0;i<_lods.size();i++){
		SAFE_RELEASE(_lods[i].ib);
	}
	SAFE_RELEASE(_ibCache);
	_bones.clear();
	_lods.clear();
	_atlases.clear();
	_Zero();
}

extern void fvfCopyByStride(DWORD nVertice,FVFEx fvfSrc,void *pDest,DWORD nStrideDest,void *pSrc,DWORD nStrideSrc);
BOOL FillVtxElement(IVertexBuffer *vb,FVFEx fvf,void *src,DWORD strideSrc,DWORD nVtx,DWORD iFrame)
{

	if (!src)
		return FALSE;
	DWORD strideDest=vb->GetStride();
	void *pDest=vb->Lock(TRUE,fvf,iFrame);
	if(!pDest)
		return FALSE;

	fvfCopyByStride(nVtx,fvf,pDest,strideDest,src,strideSrc);

	vb->Unlock();
	return TRUE;
}

#define TRY_FillVtxElement(fvfElem,src)\
{\
	if (src)\
	{\
		if (FALSE==FillVtxElement(_vb,fvfElem,src,sizeof(src[0]),meshdata.vtxframes.m_nVtx,k))\
			goto _fail;\
	}\
}

#define ACCUM_FVF(fvfElem,src) \
{\
	if (src)\
		fvf|=(fvfElem);\
}

BOOL CMesh::_OnTouchHeader(IRenderSystem *pRS)
{
	VALIDATE_RES_TYPE(Res_Mesh);
	MeshData meshdata;
	meshdata.LoadHeaderData(_data);

	_aabb=meshdata.aabb;
	_nWeight=meshdata.nWeight;
	_flag=meshdata.flag;
	_frames=meshdata.frames;
	_nSkeletonBones=meshdata.nSkeletonBones;

	return TRUE;
}

BOOL CMesh::_OnTouch(IRenderSystem *pRS)
{
	ProfilerStart_Recent(Mesh_OnTouch);
	VALIDATE_RES_TYPE(Res_Mesh);

	MeshData meshdata;
	meshdata.LoadData(_data);
	CVertexMgr *vtxmgr=(CVertexMgr *)((CRenderSystem*)pRS)->GetVertexMgr();

	assert(vtxmgr);

	_aabb=meshdata.aabb;
	_nWeight=meshdata.nWeight;
	_flag=meshdata.flag;
	_frames=meshdata.frames;
	_nSkeletonBones=meshdata.nSkeletonBones;

	_bones=meshdata.bones;
	_segbones = meshdata.segbones;

	//LOD information
	_lods.resize(meshdata.lodInfos.size());
	for(int i = 0;i<_lods.size();i++){
		_lods[i].clean();
		_lods[i].segs.swap(meshdata.lodInfos[i].segs);
		_lods[i].dist = meshdata.lodInfos[i].dist;
	}

	_atlases.resize(meshdata.atlases.size());
	for (int i=0;i<meshdata.atlases.size();i++)
	{
		_atlases[i].channel=meshdata.atlases[i].channel;
		_atlases[i].w=meshdata.atlases[i].w;
		_atlases[i].h=meshdata.atlases[i].h;
		_atlases[i].gutter=meshdata.atlases[i].gutter;
	}

	FVFEx fvf=0;
	if (TRUE)//decide fvf
	{
		MeshData::VtxData *p;
		assert(meshdata.vtxframes.size()>0);
		p=&meshdata.vtxframes[0];

		ACCUM_FVF(FVFEX_XYZ0,p->pos);
		ACCUM_FVF(FVFEX_NORMAL0,p->normal);
		ACCUM_FVF(FVFEX_BINORMAL,p->binormal);
		ACCUM_FVF(FVFEX_TANGENT,p->tangent);
		if (p->colorF)
		{
			ACCUM_FVF(FVFEX_DIFFUSE_F,p->colorF);
		}
		else
		{
			ACCUM_FVF(FVFEX_DIFFUSE,p->color);
		}
		ACCUM_FVF(FVFEX_WEIGHT(_nWeight),p->weight);
		ACCUM_FVF(FVFEX_BONEINDICE0,p->boneindex0);
		ACCUM_FVF(FVFEX_BONEINDICE1,p->boneindex1);
		for (int m=0;m<ARRAY_SIZE(p->tex);m++)
			ACCUM_FVF(FVFEX_FLAG_TEX(m),p->tex[m]);
		//XXXXX:More VtxData Element
	}
	
	//Index buffer
	for(int i = 0;i<meshdata.lodInfos.size();i++){
		_lods[i].ib =vtxmgr->CreateIB((DWORD)meshdata.lodInfos[i].indice.size(),0);
		if (!_lods[i].ib)
			goto _fail;
		//copy index data to video memory
		void *pDest = NULL;
		pDest = _lods[i].ib->Lock(TRUE);
		if (!pDest)
			goto _fail;
		memcpy(pDest,&(meshdata.lodInfos[i].indice[0]),sizeof(WORD)*(meshdata.lodInfos[i].indice.size()));
		_lods[i].ib->Unlock();
	}

	//Now fill vertex data into surfMesh->vb
	if(TRUE){
		_vb=vtxmgr->CreateVB(meshdata.vtxframes.m_nVtx,fvf,
			(DWORD)meshdata.vtxframes.size(),0);
		if (!_vb)
			goto _fail;

		for (int k=0;k<meshdata.vtxframes.size();k++){
			MeshData::VtxData *p;
			p=&meshdata.vtxframes[k];

			TRY_FillVtxElement(FVFEX_XYZ0,p->pos);
			TRY_FillVtxElement(FVFEX_NORMAL0,p->normal);
			TRY_FillVtxElement(FVFEX_BINORMAL,p->binormal);
			TRY_FillVtxElement(FVFEX_TANGENT,p->tangent);
			TRY_FillVtxElement(FVFEX_DIFFUSE,p->color);
			TRY_FillVtxElement(FVFEX_DIFFUSE_F,p->colorF);

			if (_nWeight>1)
				TRY_FillVtxElement(FVFEX_WEIGHT(_nWeight),p->weight);

			TRY_FillVtxElement(FVFEX_BONEINDICE0,p->boneindex0); 
			TRY_FillVtxElement(FVFEX_BONEINDICE1,p->boneindex1);
			for (int m=0;m<ARRAY_SIZE(p->tex);m++)
				TRY_FillVtxElement(FVFEX_FLAG_TEX(m),p->tex[m]);
			//XXXXX:More VtxData Element
		}
	}

	ProfilerEnd();

	return TRUE;

_fail:

	_Clean();
	return FALSE;

}

void CMesh::_OnUnload()
{
	_Clean();
}


DWORD CMesh::GetFrameCount()
{
	return _frames.size();
}

FramePtr CMesh::_CalcFP(f32 fFrame)
{
	int f1,f2;
	f32 r;
	if (TRUE)
	{
		f1=(int)fFrame;
		f2=f1+1;
		if (f2>=_frames.size())
			f2=_frames.size()-1;
		r=fFrame-(f32)f1;
	}

	FramePtr fp;
	fp.f1=_frames[f1];
	fp.f2=_frames[f2];
	fp.r=r;

	return fp;
}

//find the actual frame in given surface,if need interpolation,make a cache internally
BOOL CMesh::_ResolveVB(f32 fFrame,int iLod,IVertexBuffer *&vb,IIndexBuffer *&ib,DWORD &iFrame)
{
	vb=NULL;
	ib=NULL;
	iFrame=0;
	
	FramePtr fp;
	fp=_CalcFP(fFrame);

	iLod = (iLod>=_lods.size())?_lods.size()-1:iLod;

	vb=_vb;
	ib= _lods[iLod].ib;

	if (fp.f1==fp.f2)
	{
		iFrame=fp.f1;
		return TRUE;
	}
	if (i_math::equals(fp.r,0.0f))
	{
		iFrame=fp.f1;
		return TRUE;
	}
	if (i_math::equals(fp.r,1.0f))
	{
		iFrame=fp.f2;
		return TRUE;
	}

#pragma message("Need more cache")
	if ((fp.equals(_fpCache))&&(_vbCache))//cache hit
	{
		vb=_vbCache;
		ib=_ibCache;
		iFrame=0;
		return TRUE;
	}

	BYTE *q,*p,*q1,*q2;
	IVertexMgr *vtxmgr;

	if (!_vbCache)//construct _vbCache/_ibCache if not exists
	{
		vtxmgr=(CVertexMgr*)_pMgr->GetRS()->GetVertexMgr();
		if (!vtxmgr)
			return FALSE;

		_vbCache=vtxmgr->CreateVB(_vb->GetCount(),_vb->GetFVF(),1);
		if (!_vbCache)
			goto _fail;

		if (_lods[iLod].ib)
		{
			_ibCache=vtxmgr->CreateIB(_lods[iLod].ib->GetCount());
			if (!_ibCache)
				goto _fail;

			//Copy the index
			q=(BYTE*)_lods[iLod].ib->Lock(TRUE);
			if (!q)
				goto _fail;
			p=(BYTE*)_ibCache->Lock(TRUE);
			if (!p)
			{
				_lods[iLod].ib->Unlock();
				goto _fail;
			}
			memcpy(p,q,_ibCache->GetSize());
			_lods[iLod].ib->Unlock();
			_ibCache->Unlock();
		}
	}

	q1=(BYTE*)_vb->Lock(TRUE,0,fp.f1);
	if(!q1)
		goto _fail;
	q2=(BYTE*)_vb->Lock(TRUE,0,fp.f2);
	assert(q2);//我们假定第一次lock成功后,第二次lock必能成功

	p=(BYTE*)_vbCache->Lock(TRUE);
	if (!p)
	{
		_vb->Unlock();
		_vb->Unlock();
		goto _fail;
	}

	//need interpolation between the 2 frames:
	extern void fvfInterpolate(DWORD nVertice,FVFEx fvf,void *pDest,void *pSrc1,void *pSrc2,float r);
	fvfInterpolate(_vb->GetCount(),_vb->GetFVF(),p,q1,q2,fp.r);

	_vbCache->Unlock();

	_vb->Unlock();
	_vb->Unlock();

	vb=_vbCache;
	ib=_ibCache;
	iFrame=0;
	return TRUE;

_fail:	
	SAFE_RELEASE(_vbCache);
	SAFE_RELEASE(_ibCache);
	_fpCache.setEmpty();
	return FALSE;
}

inline i_math::matrix43f *GetIndexedMatrice(i_math::matrix43f *mats,DWORD cMats,WORD *indice,DWORD &cIndices)
{
	static matrix43f buffer[MAX_BONE];
	if (indice)
	{
		for (int i=0;i<cIndices;i++)
		{
			DWORD idx=indice[i];
			if (idx>=cMats)
				return NULL;
			buffer[i]=mats[idx];
		}
		return buffer;
	}
	cIndices=cMats;
	return mats;
}


BOOL CMesh::_Draw(IShader *shader0,i_math::matrix43f *mats0,DWORD cMats0,DrawMeshArg &dmg)
{
	dmg.iLod = 0;

	if (A_Ok!=Touch())
		return FALSE;

	CShader *shader=(CShader *)shader0;	

	BOOL bUsingSkeleton=FALSE;
	if ((_nSkeletonBones>1)&&(cMats0==_nSkeletonBones))
		bUsingSkeleton=TRUE;//support skeleton ,and bone count matches,use skeleton
	else
	{//not using skeleton,should be a single matrix or no matrix
		if (cMats0>1)
		{
			static BOOL bLogged=FALSE;
			if (!bLogged)
			{
				LOG_DUMP_3P("Mesh",Log_Error,"绘制骨骼动画Mesh(%s)时,发现Mesh的骨骼数量(%d)与骨架系统骨骼数量(%d)不一致!",GetPath(),_nSkeletonBones,cMats0);
				bLogged=TRUE;
			}
			return FALSE;
		}
	}

	//check whether the shader could match the weight count requirement
	if (bUsingSkeleton&&(_nWeight>shader->GetCap_weightcount()))
		return FALSE;

	assert(_vb);
	_vb->Touch();

	int lod = (_lods.size()<=dmg.iLod)?(_lods.size()-1):dmg.iLod;
	assert(_lods[lod].ib);
	_lods[lod].ib->Touch();

	//decide which vb's which frame to use
	IVertexBuffer *vbWork;
	IIndexBuffer *ibWork;
	VBBindArg arg;
	if (!_ResolveVB(dmg.fFrame,lod,vbWork,ibWork,arg.iFrame))
		return FALSE;


	arg.fvfDraw=vbWork->GetFVF();

	//fill mode
	arg.fillmode=dmg.fillmode;

	matrix43f *mats;
	DWORD nMats;
	matrix43f matIdentity;

#pragma message("PROBLEM: when IMesh bind itself to the shader it may re-set the matrice to the shader for a lot of time,any optimizations?")

	//now set the world matrice and draw
	if (bUsingSkeleton)//this mesh attaches a skeleton
	{
		if (shader->GetCap_maxbones()>=_bones.size())//the shader can support so many bones
		{
			assert(_bones.size()>0);

			//use first set of bone indice
			FVFEx_Mod(arg.fvfDraw,0,FVFEX_BONEINDICE1);//remove second set

			mats=GetIndexedMatrice(mats0,cMats0,&_bones[0],nMats=_bones.size());

			if (EPFail==shader->SetEP_World(mats,nMats))
				return FALSE;
				
			if (!shader->BindVB(vbWork,ibWork,&arg))
				return FALSE;
			return shader->DoShadeRaw();
		}
		else
		{//hardware could not support so many bones,use segments to try

			BOOL bOk=TRUE;
			//use second set of bone indice
			FVFEx_Mod(arg.fvfDraw,0,FVFEX_BONEINDICE0);//remove first set
			
			for (int i= 0;i<_lods[lod].segs.size();i++)
			{
				MeshData::SegInfo *si = NULL;
				si=&(_lods[lod].segs[i]);
				if(shader->GetCap_maxbones()<si->bc)//this segment's bone count is still too many for the hardware
				{
					bOk=FALSE;
					continue;
				}

				mats=GetIndexedMatrice(mats0,cMats0,&_segbones[si->bs],nMats=si->bc);
				
				if (EPFail==shader->SetEP_World(mats,nMats))
				{
					bOk=FALSE;
					continue;
				}

				arg.primstart = si->ps;
				arg.primcount = si->pc;
				if (!shader->BindVB(vbWork,ibWork,&arg))
				{
					bOk=FALSE;
					continue;
				}
				bOk&=shader->DoShadeRaw();
			}

			return bOk;
		}
	}
	else
	{
		//Not using skeleton,remove all the bone-related vertex elements
		FVFEx_Mod(arg.fvfDraw,0,FVFEX_WEIGHT0|FVFEX_WEIGHT01|
													FVFEX_WEIGHT012|FVFEX_WEIGHT0123|
													FVFEX_BONEINDICE0|FVFEX_BONEINDICE1);
		if ((!mats0)||(cMats0==0))
			mats0=&matIdentity;
		if (EPFail==shader->SetEP_World(mats0,1))
			return FALSE;

		if (!shader->BindVB(vbWork,ibWork,&arg))
			return FALSE;
		return shader->DoShadeRaw();
	}

	return FALSE;//should not reach here
}



FeatureCode CMesh::GetFeature()
{
	switch(_nWeight)
	{
		case 0:
			return FC_none;
		case 1:
			return FC_bones1;
		case 2:
			return FC_bones2;
		case 3:
			return FC_bones3;
		case 4:
			return FC_bones4;
		default:
			assert(FALSE);
	}
	return FC_none;
}

IMeshSnapshot *CMesh::ObtainSnapshot()
{
	CMeshSnapshot *ss=(CMeshSnapshot*)_pMgr->GetRS()->CreateMeshSnapshot();
	ss->Attach(this);
	return (IMeshSnapshot *)ss;
}


//////////////////////////////////////////////////////////////////////////
//CMeshMgr
CMeshMgr::CMeshMgr()
{
}

IResource *CMeshMgr::ObtainRes(const char *path)
{
	return _ObtainResH<CMesh>(path);
}

BOOL CMeshMgr::ReloadRes(const char *pathRes)
{
	return CResourceMgr::_ReloadRes<CMesh>(pathRes,'H');
}
