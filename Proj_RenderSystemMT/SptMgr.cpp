/********************************************************************
	created:	2008/07/08
	created:	1:7:2008   18:08
	filename: 	e:\IxEngine\Proj_RenderSystem\SptMgr.cpp
	file path:	e:\IxEngine\Proj_RenderSystem
	file base:	SptMgr
	file ext:	cpp
	author:		star
	
	purpose:	spt resource manager
*********************************************************************/

#include "stdh.h"

#pragma warning(disable:4018)
#pragma warning(disable:4267)


#include "SptMgr.h"
#include "DeviceObject.h"

#include "RenderSystem/ITexture.h"
#include "RenderSystem/IDummies.h"
#include <assert.h>

#include "Log/LogFile.h"

#include "stringparser/stringparser.h"

#include "shaderlib/SLDefines.h"

#include "fvfex/fvfex.h"

#include "resdata/DummiesData.h"

extern const char *ResolveRefPath(const char *path,CResource *owner);
//////////////////////////////////////////////////////////////////////////

BOOL FillResource(IVertexMgr * pVertexMgr,ITextureMgr * pTexMgr,IDynDummiesMgr * pDummiesMgr,CResource * pRes,ResData * resdata)
{
	CSpt * pSpt = static_cast<CSpt *>(pRes);
	SptData * pSptData = static_cast<SptData *>(resdata);

	if(TRUE) // branch
	{
		DWORD stride = fvfSize(pSptData->fvfBranch);
		DWORD nVtx = (stride)?pSptData->branchVB.size()/stride:0;
		if(nVtx>0)
		{
			pSpt->_branchVB = pVertexMgr->CreateVB(nVtx,pSptData->fvfBranch,1);
			assert(pSpt->_branchVB);
			//
			void * buf = NULL;
			buf = pSpt->_branchVB->Lock(TRUE);
			memcpy(buf,&(pSptData->branchVB[0]),pSptData->branchVB.size());	
			pSpt->_branchVB->Unlock();
			
			//
			assert(pSptData->numBranchLods);
			for(int i = 0;i<pSptData->numBranchLods;i++)
			{
				DWORD nIndices = pSptData->branchLODs[i+1] - pSptData->branchLODs[i];

				pSpt->_branchIBs[i] =  pVertexMgr->CreateIB(nIndices);
				buf = pSpt->_branchIBs[i]->Lock(TRUE);
				stride = pSpt->_branchIBs[i]->GetStride();

				memcpy(buf,&(pSptData->sptIB[pSptData->branchLODs[i]]),nIndices*stride);
				pSpt->_branchIBs[i]->Unlock();
			}
		}
	}

	if(TRUE) //frond
	{
		DWORD stride = fvfSize(pSptData->fvfFrond);
		DWORD nVtx = (stride)?pSptData->frondVB.size()/stride:0;
		
		if(nVtx>0)
		{
			pSpt->_frondVB = pVertexMgr->CreateVB(nVtx,pSptData->fvfFrond,1);
			assert(pSpt->_frondVB);

			void * buf = NULL;
			buf = pSpt->_frondVB->Lock(TRUE);
			memcpy(buf,&(pSptData->frondVB[0]),pSptData->frondVB.size());	
			pSpt->_frondVB->Unlock();

			for(int i = 0;i<pSptData->numFrondLods;i++)
			{
				DWORD nIndices = pSptData->frondLODs[i+1] - pSptData->frondLODs[i];
				pSpt->_frondIBs[i] =  pVertexMgr->CreateIB(nIndices);
				buf = pSpt->_frondIBs[i]->Lock(TRUE);
				stride = pSpt->_frondIBs[i]->GetStride();
				memcpy(buf,&(pSptData->sptIB[pSptData->frondLODs[i]]),nIndices*stride);
				pSpt->_frondIBs[i]->Unlock();
			}
		}
	}

	if(TRUE)  //leaf card
	{
		DWORD stride = fvfSize(pSptData->fvfLeafCard);
		DWORD nVtx = (stride)?pSptData->leafCardVB.size()/stride:0;
		if(nVtx)
		{
			void * buf = NULL;
			pSpt->_leafCardVBs = pVertexMgr->CreateVB(nVtx,pSptData->fvfLeafCard,1);
			buf = pSpt->_leafCardVBs->Lock(TRUE);
			memcpy(buf,&(pSptData->leafCardVB[0]),pSptData->leafCardVB.size());
			pSpt->_leafCardVBs->Unlock();

			int vbase = 0;
			for(int i = 0;i< pSptData->numLeafCardLods;i++)
			{
				DWORD size  = pSptData->leafCardLODs[i+1] - pSptData->leafCardLODs[i];
				DWORD nVtx = size/stride;

				pSpt->vBaseLeaf[i] = vbase;
				pSpt->nPrimLeaf[i] = nVtx/2;
				vbase += nVtx;
			}

			nVtx = (pSptData->leafCardLODs[1] - pSptData->leafCardLODs[0])/stride;
			DWORD nLeaf = nVtx/4;
			//
			std::vector<WORD> indices;
			DWORD nIndices = nLeaf*6;
			indices.resize(nIndices);
			int corner[6] = {0,3,1,2,1,3};
			for(int i = 0;i<nLeaf;i++)
				for(int j =0;j<6;j++)
					indices[6*i+j] = 4*i+corner[j];

			pSpt->_leafIB = pVertexMgr->CreateIB(nIndices);
			buf = pSpt->_leafIB->Lock(TRUE);
			stride = pSpt->_leafIB->GetStride();
			memcpy(buf,&indices[0],nIndices*stride);
			pSpt->_leafIB->Unlock();
		}
	}

	if(TRUE) //leaf mesh
	{
		DWORD stride = fvfSize(pSptData->fvfLeafMesh);
		DWORD nVtx = (stride)?pSptData->leafMeshVB.size()/stride:0;
		if(nVtx)
		{
			void *buf = NULL;
			pSpt->_leafMeshVB = pVertexMgr->CreateVB(nVtx,pSptData->fvfLeafMesh,1);
			buf = pSpt->_leafMeshVB->Lock(TRUE);
			memcpy(buf,&pSptData->leafMeshVB[0],pSptData->leafMeshVB.size());
			pSpt->_leafMeshVB->Unlock();

			int nIndex = pSptData->leafMeshLODs[pSptData->numLeafMeshLods] - pSptData->leafMeshLODs[0];

			pSpt->_leafMeshIB = pVertexMgr->CreateIB(nIndex);
			buf = pSpt->_leafMeshIB->Lock(TRUE);
			stride = pSpt->_leafMeshIB->GetStride();
			void * src = &(pSptData->sptIB[0])+pSptData->leafMeshLODs[0];
			memcpy(buf,src,nIndex*stride);
			pSpt->_leafMeshIB->Unlock();

			for(int i = 0 ;i<pSptData->numLeafMeshLods;i++)		
			{
				// different lod has a different index start point , primitive count.
				pSpt->_leafMeshPrimStart[i] = (pSptData->leafMeshLODs[i] - pSptData->leafMeshLODs[0])/3;
				pSpt->_leafMeshNumPrim[i] = (pSptData->leafMeshLODs[i+1] - pSptData->leafMeshLODs[i])/3;
			}
		}
	}

	if(TRUE)  //global
	{
		pSpt->nLodBranch = pSptData->numBranchLods;
		pSpt->nLodFrond  = pSptData->numFrondLods;
		pSpt->nLodLeafCard = pSptData->numLeafCardLods;
		pSpt->nLodLeafMesh = pSptData->numLeafMeshLods;
		pSpt->nLods = pSptData->numLods;
		pSpt->_sptLod.leaveCards.fRockScale = pSptData->fLeafRockScale;
		pSpt->_sptLod.leaveCards.fRustleScale = pSptData->fLeafRusltScale;

		memcpy(pSpt->fTransitionDist,pSptData->transitionDists,MAX_LOD_LEVEL*sizeof(float));
		memcpy(pSpt->fTransitionPrecent,pSptData->transitionPrecent,MAX_LOD_LEVEL*sizeof(float));
		pSpt->_winds.resize(pSptData->cfgwinds.size());
		memcpy(&(pSpt->_winds[0]),&(pSptData->cfgwinds[0]),pSptData->cfgwinds.size()*sizeof(SptWndCfg));

		pSpt->fSlide = 1.0f/pSpt->nLods;
	} 


	if(TRUE) // texture load
	{
		if(!pSptData->mapBranchDif.empty()){
			const char * path = ResolveRefPath(pSptData->mapBranchDif.c_str(),pSpt);
			pSpt->_maps.trunk = (ITexture *)pTexMgr->ObtainRes(path);
		}
		if(!pSptData->mapBranchNor.empty()){
			const char * path = ResolveRefPath(pSptData->mapBranchNor.c_str(),pSpt);
			pSpt->_maps.trunkNormal = (ITexture *)pTexMgr->ObtainRes(path);
		}
		if(!pSptData->mapCompisiteDif.empty()){
			const char * path = ResolveRefPath(pSptData->mapCompisiteDif.c_str(),pSpt);
			pSpt->_maps.composite = (ITexture *)pTexMgr->ObtainRes(path);
		}
		if(!pSptData->mapCompisiteNor.empty()){
			const char * path = ResolveRefPath(pSptData->mapCompisiteNor.c_str(),pSpt);
			pSpt->_maps.compositeNormal = (ITexture *)pTexMgr->ObtainRes(path);
		}
	}

	if(TRUE) // bb
	{
//		pSpt->_sptLod.billboard.fwVertBB = pSptData->fwVertBB;
//		pSpt->_sptLod.billboard.fhVertBB = pSptData->fhVertBB;
//		pSpt->_sptLod.billboard.fwHorizBB = pSptData->fwHorizBB;
//		pSpt->_sptLod.billboard.fhHorizBB = pSptData->fhHorizBB;
//		pSpt->_sptLod.billboard.fhHorizHeight = pSptData->fhHorizHeight;

//		memcpy(pSpt->_sptLod.billboard.texHorzMap,pSptData->texHorizMap,sizeof(pSptData->texHorizMap));
//		memcpy(pSpt->_sptLod.billboard.texVertMap,pSptData->texVertMap,MAX_BB_IMAGES*4*sizeof(float));

//		pSpt->_sptLod.billboard.nImages = pSptData->nImages;
	}


	if(TRUE) //bounding box
	{
		pSpt->_aabb = pSptData->aabb;

		pSpt->_caps.swap(pSptData->capus);
		pSpt->_obs.swap(pSptData->obbs);
		pSpt->_sphs.swap(pSptData->sphs);
		pSpt->_sphleaf = pSptData->leafboundSph;
	}
	
	// create the dummies object provided to physical system
	if(pSpt->GetNumberOfCollisionObjects()>0)
			pSpt->_CreateDummies(pDummiesMgr);

	//初始化 光照贴图计算工具
	pSpt->_mapSizeCalc.Set(pSptData->szBr,pSptData->szFr,
						   pSptData->nPixel);
	
	return TRUE;
}
//////////////////////////////////////////////////////////////////////////
//CSpt
IMPLEMENT_CLASS(CSpt);

CSptTriSampAdapter CSpt::_triSampleAdapter;

ISptTriSampleAdapter * CSpt::GetTriSampleAdapter()
{
	_triSampleAdapter.SetOwner(this);
	return &_triSampleAdapter;
}
void CSpt::GetMapSize(i_math::size2di &szBr,i_math::size2di &szFr,int lvl)
{
	float scale = GetMapScale(lvl);
	_mapSizeCalc.GetMapSize(szBr,szFr,scale);
}
CSpt::CSpt()
{
	Zero();
}
CSpt::~CSpt()
{
	Clean();
}
float CSpt::GetMapScale(int lvl) const
{
	assert(lvl<=4&&lvl>=0);
	const float scale[] = {0,1.0f,1.5f,2.0f,3.0f};
	return scale[lvl];
}
IDummies * CSpt::GetCollisionDummies()
{
	return _dummies;
}
void CSpt::_CreateDummies(IDynDummiesMgr * mgr)
{
	SAFE_RELEASE(_dummies);

	int n = GetNumberOfCollisionObjects();

	//Added By Chenxi
	if (n<=0)
		return;
	//
	
	DummiesData data;
	data.dummies.resize(n);

	int k = 0;
	for(int i = 0;i<_obs.size();i++)
	{
		Dummy &dum = data.dummies[k++];
		dum.setType(DummyInfo::BoundType_AABB);
		i_math::aabbox3df * abb = dum.getAAbb();
		*abb = _obs[i];
	}

	for(int i = 0;i<_sphs.size();i++)
	{
		Dummy &dum = data.dummies[k++];
		dum.setType(DummyInfo::BoundType_Sphere);
		i_math::spheref * sph = dum.getSphere();
		*sph = _sphs[i];
	}

	for(int i = 0;i<_caps.size();i++)
	{
		Dummy &dum = data.dummies[k++];
		dum.setType(DummyInfo::BoundType_Capsule);
		i_math::capsulef * cap = dum.getCapsule();
		*cap = _caps[i];
	}

	_dummies = mgr->Create(&data);
}

void CSpt::_BuildVtxInfo()
{
	//已经创建不再创建
	if(_infoVtx)
		return;
	
	_infoVtx = Class_New2(SptVtxInfo);

	SptLod & lod = GetLod(0);

	//创建Branch 顶点信息
	if(lod.branch.vb.vb&&lod.branch.vb.ib){
		DWORD nVtx = lod.branch.vb.vb->GetCount();
		_infoVtx->SetBrVtxCount(nVtx);
		_GetVtxInfoFromVB(lod.branch.vb.vb,
						 &(_infoVtx->brvtxs[0]),
						 &(_infoVtx->brnors[0]),
						 &(_infoVtx->brbinors[0]),
						 &(_infoVtx->brtans[0]),
						 &(_infoVtx->bruvs[0]));
		_GetIndexFromIB(lod.branch.vb.ib,&(_infoVtx->brvtxs[0]),_infoVtx->brIndices);
	}

	//创建Frond 顶点信息
	if(lod.frond.vb.vb&&lod.frond.vb.ib){
		DWORD nVtx = lod.frond.vb.vb->GetCount();
		_infoVtx->SetFrVtxCount(nVtx);
		_GetVtxInfoFromVB(lod.frond.vb.vb,
						&(_infoVtx->frvtxs[0]),
						&(_infoVtx->frnors[0]),
						&(_infoVtx->frbinors[0]),
						&(_infoVtx->frtans[0]),
						&(_infoVtx->fruvs[0]));
		_GetIndexFromIB(lod.frond.vb.ib,&(_infoVtx->frvtxs[0]),_infoVtx->frIndices);
	}

	//创建树叶位点信息 
	_GetLeafCenterVtxs(); 
	_GetLeafMesh();
}

void CSpt::_GetLeafCenterVtxs()
{
	SptLod &lod = GetLod(0);
	if(lod.leaveCards.vb.vb){
		DWORD count = lod.leaveCards.vb.vb->GetCount();
		FVFEx fvf = lod.leaveCards.vb.vb->GetFVF();
		BYTE * pByte = (BYTE *)lod.leaveCards.vb.vb->Lock(FALSE);

		if(pByte){
			DWORD stride = lod.leaveCards.vb.vb->GetStride();
			BYTE * pXYZ = pByte + fvfOffset(fvf,FVFEX_XYZW0);
			BYTE * pWH = pByte + fvfOffset(fvf,FVFEX_FLAG_QUX1);
			float w = 0 , h =0;

			//4个顶点为一片叶子具有同一个中心
			for(int i = 0;i<count;i+=4){
				i_math::vector3df * p = (i_math::vector3df *)(pXYZ);
				i_math::vector4df * pwhf = (i_math::vector4df *)(pWH);
				_infoVtx->leavevtxs.push_back(*p);
				pXYZ += 4*stride;
				pWH += 4*stride;
				w = max(w,pwhf->x);
				h = max(h,pwhf->y);
			}

			_infoVtx->radius = max(w,h)/2.0f;
		}

		lod.leaveCards.vb.vb->Unlock();
	}
}
void CSpt::_GetLeafMesh()
{
	SptLod &lod = GetLod(0);

	_infoVtx->lmvtx.clear();
	_infoVtx->lmhookpos.clear();
	_infoVtx->lminstmat.clear();
	_infoVtx->lmindices.clear();

	if(lod.leaveMeshs.vb.vb){
		DWORD count = lod.leaveMeshs.vb.vb->GetCount();
		DWORD stride = lod.leaveMeshs.vb.vb->GetStride();
		BYTE * pStart = (BYTE *)lod.leaveMeshs.vb.vb->Lock(FALSE);
		FVFEx fvf = lod.leaveMeshs.vb.vb->GetFVF();

		if(pStart){
			BYTE * pXY = pStart + fvfOffset(fvf,FVFEX_FLAG_TEX3);
			BYTE * poslm = pStart + fvfOffset(fvf,FVFEX_XYZW0);
			BYTE * nor = pStart + fvfOffset(fvf,FVFEX_FLAG_QUX1);
			BYTE * tan = pStart + fvfOffset(fvf,FVFEX_FLAG_QUX2); //.xyz: tan .xyz , .w:local pos .z

			BOOL bMeetMeshEnd = FALSE;
			i_math::vector2df * pfXY = (i_math::vector2df*)(pXY);
			i_math::vector3df * norvec = (i_math::vector3df *)(nor);
			i_math::vector4df * tanvec = (i_math::vector4df *)(tan);
			i_math::vector3df binvec = norvec->crossProduct(i_math::vector3df(tanvec->x,tanvec->y,tanvec->z));
			binvec.normalize();
			i_math::matrix43f mat;
			mat.set(tanvec->x,binvec.x,norvec->x,
					tanvec->y,binvec.y,norvec->y,
					tanvec->z,binvec.z,norvec->z,
					0,0,0);
			
			i_math::vector3df poslast(pfXY->x,pfXY->y,tanvec->w),posCur;
			_infoVtx->lminstmat.push_back(mat);
			_infoVtx->lmhookpos.push_back(poslast);
			for(int i = 0;i<count;i++){
				pfXY = (i_math::vector2df*)(pXY);
				tanvec = (i_math::vector4df *)(tan);
				posCur.set(pfXY->x,pfXY->y,tanvec->w);
				
				//一个新的实例
				if(posCur!=poslast){
					norvec = (i_math::vector3df *)(nor);
					binvec = norvec->crossProduct(i_math::vector3df(tanvec->x,tanvec->y,tanvec->z));
					binvec.normalize();
					i_math::matrix43f mat;
					mat.set(tanvec->x,binvec.x,norvec->x,
							tanvec->y,binvec.y,norvec->y,
							tanvec->z,binvec.z,norvec->z,
							0,0,0);
					
					poslast = posCur;
					_infoVtx->lminstmat.push_back(mat);
					_infoVtx->lmhookpos.push_back(posCur);					
					bMeetMeshEnd = TRUE;		
				}

				if(!bMeetMeshEnd){
					i_math::vector3df *posLocal = (i_math::vector3df *)poslm;
					_infoVtx->lmvtx.push_back(*posLocal);
				}

				pXY += stride;
				nor += stride;
				tan += stride;
				poslm += stride;
			}
		}

		lod.leaveMeshs.vb.vb->Unlock();
	}
	
	DWORD nVtx = _infoVtx->lmvtx.size();
	if(lod.leaveMeshs.vb.ib){
		DWORD count = lod.leaveMeshs.vb.ib->GetCount();
		WORD * p =(WORD *)lod.leaveMeshs.vb.ib->Lock(FALSE);
		if(p){
			for(int i = 0;i<count;i++){
				if(p[i]>nVtx)
					break;
				_infoVtx->lmindices.push_back(p[i]);
			}
		}
		lod.leaveMeshs.vb.ib->Unlock();
	}
}

void CSpt::_GetVtxInfoFromVB(IVertexBuffer * vb,i_math::vector3df *pos,
					   i_math::vector3df * nor,i_math::vector3df * binor,
					   i_math::vector3df * tan,i_math::vector2df * uv)
{

	FVFEx fvfVB = vb->GetFVF();
	BYTE * pVtx = (BYTE *)vb->Lock(FALSE);
	if(pVtx){
		BYTE * pVer = pVtx + fvfOffset(fvfVB,FVFEX_XYZW0);//.xyz pos
		BYTE * pNor = pVtx + fvfOffset(fvfVB,FVFEX_NORMAL0);
		BYTE * pTan = pVtx + fvfOffset(fvfVB,FVFEX_TANGENT);
		BYTE * pUV  = pVer + fvfOffset(fvfVB,FVFEX_FLAG_QUX0) + 2*sizeof(float);

		DWORD stride = vb->GetStride();

		for(int i = 0;i<vb->GetCount();i++){
			pos[i] = *((i_math::vector3df*)pVer);
			uv[i]  = *((i_math::vector2df*)pUV);
			nor[i] = *((i_math::vector3df*)pNor);
			tan[i] = *((i_math::vector3df*)pTan);
			binor[i] = nor[i].crossProduct(tan[i]);
			binor[i].normalize();

			pVer += stride;
			pUV  += stride;
			pNor += stride;
			pTan += stride;
		}
	}
	vb->Unlock();
}

void CSpt::_GetIndexFromIB(IIndexBuffer * ib,i_math::vector3df * pos,std::vector<WORD> &vecIB)
{
	//展开索引
	vecIB.clear();
	DWORD nIB = ib->GetCount();
	WORD * pSampIB = &vecIB[0];
	
	DWORD  nValid = 0;
	WORD * pIB = (WORD *)ib->Lock(FALSE);
	if(pIB){
		i_math::triangle3df tri;
		for(int i = 0;i<nIB-2;i++){
			tri.set(pos[pIB[0]],pos[pIB[1]],pos[pIB[2]]);
			if(tri.getArea()>0.0f){
				vecIB.push_back(pIB[0]);
				vecIB.push_back(pIB[1]);
				vecIB.push_back(pIB[2]);
			}	
			pIB++;
		}
	}
	ib->Unlock();
}

int  CSpt::GetNumberOfCollisionObjects()
{
	int numObjects = _caps.size() + _obs.size() + _sphs.size();
	return numObjects;
}
i_math::aabbox3df & CSpt::GetBoundingBox()
{
	return _aabb;
}
void * CSpt::GetCollisionObject(int idx ,CollisionObjectType & coType)
{
	int nbs = _obs.size();
	int nsphs = _sphs.size();
	int ncaps = _caps.size();

	void * pObj = NULL;

	// capsule --> shpere ---> ob
	if(idx < ncaps)
	{
		coType = CO_CAPSULE;
		pObj = &_caps[idx];	
	}
	else if(idx<ncaps+nsphs)
	{
		coType = CO_SPHERE;
		pObj = &_sphs[idx - ncaps];
	}
	else if(idx<ncaps+nsphs+nbs)
	{
		coType = CO_BOX;
		pObj = & _obs[idx-ncaps-nsphs];
	}
	return pObj;
}
BOOL CSpt::TestCollision(const i_math::line3df & rayHit,i_math::vector3df &pos,float scale,float rotY,
						 i_math::vector3df &outIntersec,SptColType cTpye/* = SPTCT_ALL*/)
{
	
	BOOL ret = FALSE;
	
	_BuildVtxInfo();

	i_math::matrix43f mat;
	mat.setScale(scale,scale,scale);
	mat.addRotationY(rotY);
	mat.addTranslation(pos);
	std::vector<i_math::vector3df> posWorld;	
	
	i_math::vector3df linePoint,linevec;
	linePoint = rayHit.start;
	linevec = rayHit.end - rayHit.start;
	linevec.normalize();

	//测试树干
	if((cTpye&SPTCT_BRANCH)&&!_infoVtx->brIndices.empty()){
		posWorld.resize(_infoVtx->brvtxs.size());
		for(int i = 0;i<posWorld.size();i++)
			mat.transformVect(_infoVtx->brvtxs[i],posWorld[i]);
		
		i_math::triangle3df triTest;	
		WORD * pIB = &_infoVtx->brIndices[0];
		for(int i = 0;i<_infoVtx->brIndices.size();i+=3){
			triTest.set(posWorld[pIB[0]],
						posWorld[pIB[1]],
						posWorld[pIB[2]]);
			
			if(triTest.getSafeIntersectionWithLimitedLine(rayHit,outIntersec))
				return TRUE;

			pIB += 3;
		}
	}

	//测试树枝
	if((cTpye&SPTCT_FROND)&&!_infoVtx->frIndices.empty()){
		posWorld.resize(_infoVtx->frvtxs.size());
		for(int i = 0;i<posWorld.size();i++)
			mat.transformVect(_infoVtx->frvtxs[i],posWorld[i]);

		i_math::triangle3df triTest;
		WORD * pIB = &_infoVtx->frIndices[0];

		int nPrim = 0;
		for(int i = 0;i<_infoVtx->frIndices.size();i+=3){
			i_math::vector3df p[3];
			for(int k = 0;k<3;k++)
				p[k] = posWorld[pIB[k]];
			
			if(nPrim%2==1)
				triTest.set(p[0],p[1],p[2]);
			else
				triTest.set(p[2],p[1],p[0]);
			nPrim++;

			if(triTest.getSafeIntersectionWithLimitedLine(rayHit,outIntersec))
				return TRUE;
	
			pIB += 3;
		}
	}
	
	//测试 面片叶子
	if((cTpye&SPTCT_LEAF)&&!_infoVtx->leavevtxs.empty()){
		posWorld.resize(_infoVtx->leavevtxs.size());
		for(int i = 0;i<posWorld.size();i++)
			mat.transformVect(_infoVtx->leavevtxs[i],posWorld[i]);
		
		i_math::spheref sph;
		i_math::vector3df out;
		float radius = _infoVtx->radius * scale;
		float rSQ = radius*radius;
		for(int i = 0;i<posWorld.size();i++){
			sph.set(posWorld[i],radius);
			i_math::vector3df v0 = sph.center - linePoint;
			float d = v0.dotProduct(linevec);
			i_math::vector3df v1(d*linevec.x,d*linevec.y,d*linevec.z);
			i_math::vector3df projv = linePoint + v1;
			if((projv - sph.center).getLengthSQ()<=rSQ){
				v0 = projv - sph.center;
				v0.setLength(radius);
				outIntersec = sph.center + v0;
				if(rayHit.isPointBetweenStartAndEnd(outIntersec))
					return TRUE;
			}
		}
	}

	//测试 模型叶子
	if((cTpye&SPTCT_LEAF)&&!_infoVtx->lminstmat.empty()){
		i_math::matrix43f matWorld;
		i_math::vector3df pos,out;
		i_math::triangle3df triTest;
		for(int i = 0;i<_infoVtx->lminstmat.size();i++){
			mat.transformVect(_infoVtx->lmhookpos[i],pos);
			matWorld.setScale(scale,scale,scale);
			matWorld = matWorld*_infoVtx->lminstmat[i];
			matWorld.addTranslation(pos);
			
			posWorld.resize(_infoVtx->lmvtx.size());
			for(int i = 0;i<_infoVtx->lmvtx.size();i++)
				matWorld.transformVect(_infoVtx->lmvtx[i],posWorld[i]);

			WORD * pIB = &(_infoVtx->lmindices[0]);
			for(int i = 0;i<_infoVtx->lmindices.size();i+=3){
				triTest.set(posWorld[pIB[i+0]],
							posWorld[pIB[i+1]],
							posWorld[pIB[i+2]]);
				if(triTest.getSafeIntersectionWithLimitedLine(rayHit,out)){
					outIntersec = out;
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

SptLod & CSpt::GetLod(DWORD iLod)
{
	int lod = min(iLod,nLods-1);
	float   f2Dist = 0;	
	for(int i = 0;i<iLod;i++)
		f2Dist  += fTransitionDist[i];
	return GetLodByDistance(f2Dist+0.0001f);
}

DWORD CSpt:: GetNumberOfLod()
{
	return nLods;
}

SptLod & CSpt::GetLodByDistance(float fDist)
{
	_sptLod.fLodValue = 0.0f;
	float	ratio = 1.0f;

	int		lodLevel = 0;
	float   f2Dist = 0;		//start of lodlevel distance
	for(;fDist > f2Dist+fTransitionDist[lodLevel]&&lodLevel< nLods;f2Dist+=fTransitionDist[lodLevel++]); //find current lod level

	f2Dist += fTransitionDist[lodLevel];	
	if(lodLevel>=nLods) 
	{
		_sptLod.bPureBB = TRUE;
		_sptLod.bHasBB = TRUE;
		_sptLod.billboard.fFadeOut = 87.0f/255.0f;
		return _sptLod;
	}
	_sptLod.bPureBB = FALSE;

	ratio = (f2Dist - fDist)/fTransitionDist[lodLevel];
	_sptLod.fLodValue  = 1.0f -(lodLevel+1.0f - ratio)*fSlide;


	BOOL bTransition  = (ratio < fTransitionPrecent[lodLevel]);
	if(TRUE) //leaf card lod
	{
		int nLodLeaf = max(nLodLeafCard,nLodLeafMesh);

		int last =  min(nLodLeaf - 1,lodLevel);
		int next =  min(nLodLeaf - 1,lodLevel+1);

		BOOL bOverflow = (last==next);
		float transRot = ratio / fTransitionPrecent[lodLevel];

		int lods[2];
		lods[0] = last ;
		
		BOOL bLeafTransition = (bOverflow)?FALSE:bTransition;

		lods[1] = bLeafTransition ? next:-1;

		float fAplhaTest[2];
		fAplhaTest[0] =bLeafTransition&&(!bOverflow)||(bTransition&&lodLevel==nLods-1)? (255.0f - transRot*(255.0f - 84.0f)) : 84.0f;
		if(bTransition) fAplhaTest[0] -= 74.0f;
		
		//[84,255] opaque-->transparent  decrease 74.0f ,in order to delay transparent. 
		fAplhaTest[1] =bLeafTransition? ((84.0f - 74.0f) + transRot*(255.0f - 84.0f)) :255.0f;
//		fAplhaTest[1] =bLeafTransition?255.0f:fAplhaTest[1]; 

		_sptLod.leaveCards.vb.vb = _leafCardVBs;
		_sptLod.leaveCards.vb.ib = _leafIB;
		_sptLod.leaveCards.bNeedMerge = bLeafTransition;

		_sptLod.leaveCards.arg[0].dpt = D3DPT_TRIANGLELIST;
		_sptLod.leaveCards.arg[0].vbase = vBaseLeaf[lods[0]];
		_sptLod.leaveCards.arg[0].primstart = 0;
		_sptLod.leaveCards.arg[0].primcount = nPrimLeaf[lods[0]];

		_sptLod.leaveCards.bNeedMerge = FALSE;

		if(lods[1]>0)
		{
			_sptLod.leaveCards.arg[1].dpt = D3DPT_TRIANGLELIST;
			_sptLod.leaveCards.arg[1].vbase = vBaseLeaf[lods[1]];
			_sptLod.leaveCards.arg[1].primstart = 0;
			_sptLod.leaveCards.arg[1].primcount = nPrimLeaf[lods[1]];
			_sptLod.leaveCards.bNeedMerge = TRUE;
		}

		_sptLod.leaveCards.fAlphaTest[0] = fAplhaTest[0]/255.0f;
		_sptLod.leaveCards.fAlphaTest[1] = fAplhaTest[1]/255.0f;

		if(TRUE) // for leaf Mesh
		{
			_sptLod.leaveMeshs.vb.vb = _leafMeshVB;
			_sptLod.leaveMeshs.vb.ib = _leafMeshIB;
			_sptLod.leaveMeshs.arg[0].dpt = D3DPT_TRIANGLELIST;
			_sptLod.leaveMeshs.arg[0].primstart = _leafMeshPrimStart[lods[0]];
			_sptLod.leaveMeshs.arg[0].primcount = _leafMeshNumPrim[lods[0]];

			_sptLod.leaveMeshs.bNeedMerge = FALSE;
			if(_sptLod.leaveCards.bNeedMerge)
			{
				_sptLod.leaveMeshs.bNeedMerge = TRUE;
				_sptLod.leaveMeshs.arg[1].dpt = D3DPT_TRIANGLELIST;
				_sptLod.leaveMeshs.arg[1].primstart = _leafMeshPrimStart[lods[1]];
				_sptLod.leaveMeshs.arg[1].primcount = _leafMeshNumPrim[lods[1]];
			}
			_sptLod.leaveMeshs.fAlphaTest[0] = fAplhaTest[0]/255.0f;
			_sptLod.leaveMeshs.fAlphaTest[1] = fAplhaTest[1]/255.0f;
		}
	}

	if(TRUE) // branch lod
	{
		BOOL bOverflow = lodLevel >= nLodBranch;
		int lod = bOverflow ?(nLodBranch - 1):lodLevel;
		float transRot = ratio/fTransitionPrecent[nLods-1];

		float fAplhaTest = bTransition&&(lodLevel==nLods-1)?(255.0f - transRot*(255.0f - 84.0f)):84.0f;

		_sptLod.branch.arg.dpt = D3DPT_TRIANGLESTRIP;
		_sptLod.branch.vb.vb = _branchVB;
		_sptLod.branch.vb.ib = _branchIBs[lod];
		_sptLod.branch.fAlphaTest = fAplhaTest/255.0f;
	}

	if(TRUE) // frond lod
	{
		BOOL bOverflow = lodLevel >= nLodFrond;
		int lod = bOverflow ?(nLodFrond - 1):lodLevel;
		float transRot = ratio/fTransitionPrecent[nLods - 1];

		float fAplhaTest = bTransition&&(lodLevel==nLods-1)?(255.0f - transRot*(255.0f - 84.0f)):84.0f;

		_sptLod.frond.arg.dpt = D3DPT_TRIANGLESTRIP;
		_sptLod.frond.vb.vb = _frondVB;
		_sptLod.frond.vb.ib = _frondIBs[lod];
		_sptLod.frond.fAlphaTest = fAplhaTest/255.0f;
	}

	if(TRUE) //bb
	{
		float transRot = ratio/fTransitionPrecent[nLods - 1];
		BOOL bFade = bTransition&&(lodLevel==nLods- 1);
		float fFadeOut = bFade?(84.0f +23.0f + transRot*(255.0f - 84.0f)):255.0f;//(1.0f-transRot):0.0f;
		_sptLod.bHasBB = bFade;
		_sptLod.billboard.fFadeOut = fFadeOut/255.0f;
	}

	return _sptLod;	
}

void CSpt::Zero()
{
	_branchVB = NULL;
	_frondVB = NULL;
	_leafCardVBs = NULL;
	_leafIB = NULL;
	_leafMeshVB = NULL;
	_leafMeshIB = NULL;
	_infoVtx = NULL;
	_dummies = NULL;

	memset(_leafMeshVBs,0,MAX_LOD_LEVEL*sizeof(IVertexBuffer *));
	memset(_branchIBs,0,MAX_LOD_LEVEL*sizeof(IIndexBuffer *));
	memset(_frondIBs,0,MAX_LOD_LEVEL*sizeof(IIndexBuffer *));

	_maps.trunk = NULL;
	_maps.trunkNormal = NULL;
	_maps.composite = NULL;
	_maps.compositeNormal = NULL;
	_maps.shadowmap = NULL;
}

void CSpt::Clean()
{
	SAFE_RELEASE(_branchVB);
	SAFE_RELEASE(_frondVB);
	SAFE_RELEASE(_leafCardVBs);
	SAFE_RELEASE(_leafIB)
	SAFE_RELEASE(_leafMeshVB);
	SAFE_RELEASE(_leafMeshIB);

	Safe_Class_Delete(_infoVtx);

	for(int i = 0;i<MAX_LOD_LEVEL;i++)
	{
		SAFE_RELEASE(_leafMeshVBs[i]);
		SAFE_RELEASE(_branchIBs[i]);
		SAFE_RELEASE(_frondIBs[i]);
	}

	SAFE_RELEASE(_maps.trunk);
	SAFE_RELEASE(_maps.trunkNormal);
	SAFE_RELEASE(_maps.composite);
	SAFE_RELEASE(_maps.compositeNormal);
	SAFE_RELEASE(_maps.shadowmap);
	SAFE_RELEASE(_dummies);

	Zero();
}

BOOL CSpt::_OnTouch(IRenderSystem *pRS)
{
	VALIDATE_RES_TYPE(Res_Spt);
	

	SptData sptdata;
	sptdata.LoadData(_data);
	
	if (FALSE==FillResource(pRS->GetVertexMgr(),pRS->GetTexMgr(),pRS->GetDynDummiesMgr(),this,&sptdata))
	{
		Clean();
		return FALSE;
	}

	return TRUE;
}

void CSpt::_OnUnload()
{
	Clean();
}

//////////////////////////////////////////////////////////////////////////
//CSptMgr
CSptMgr::CSptMgr()
{
}

IResource *CSptMgr::ObtainRes(const char *pathRes)
{
	return _ObtainResA<CSpt>(pathRes);
}

BOOL CSptMgr::ReloadRes(const char *pathRes)
{
	return CResourceMgr::_ReloadRes<CSpt>(pathRes,'A');
}


//////////////////////////////////////////////////////////////////////////
//CDynSptMgr

ISpt *CDynSptMgr::Create(SptData *data,const char * pathRes)
{
	CSpt *spt=_ObtainRes<CSpt>();
	data->SaveData(spt->_data);
	spt->_typeData=Res_Spt;
	spt->SetPath(pathRes);
	spt->SetState(CResource::Loaded);

	return spt;
}
