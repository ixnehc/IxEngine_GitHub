
/************************************************************************/
/* 
e:\IxEngine\Proj_RenderSystem\DummiesMgr.cpp
author: star
purpose: Dummies resource Manager
date: 2008-01-07
*/
/************************************************************************/

#include "stdh.h"
#include ".\dummiesmgr.h"
#include "assert.h"
#include "../common/Log/LogFile.h"
#include "resdata/DummiesData.h"
#include "commondefines/general.h"
#include "spatialtester/spatialtester.h"

#include "Matrice43.h"
#include <stack>

#define VECTORSIZE_CHECK(idx,vec) \
	if(idx>=vec.size()) \
	return FALSE;

#define MAPSEEK_FROMID(idx,map) \
	for(it=map.begin();it!=map.end();it++)	 \
	{														\
		if(idx==(*it).second)								\
			break;											\
	}

#define MAPSEEK_FROMKEY(key,map)		\
	it=map.find(key);    


IMPLEMENT_CLASS(CDummies);

CDummies::CDummies()
{	
	_boneindices.clear();
	_vecDummies.clear();
	_mpNameIdx.clear();
	_bInit = FALSE;
}
CDummies::~CDummies(void)
{
}


DWORD  CDummies::GetCount()
{
	return _vecDummies.size();
}

const char * CDummies::GetDummyName(DWORD idx)
{
	std::hash_map<std::string,WORD>::iterator it;				
	
	MAPSEEK_FROMID(idx,_mpNameIdx);
	if(it!=_mpNameIdx.end()) 
		return it->first.c_str();

	return NULL;
}

DummyInfo * CDummies::GetDummyInfo(DWORD &count)
{
	count=_vecDummies.size();
	return &_vecDummies[0];
}

int CDummies::FindDummy(const char *name)
{
	std::hash_map<std::string,WORD>::iterator it=_mpNameIdx.find(std::string(name));

	if(it==_mpNameIdx.end())
		return -1;
	return (int)((*it).second);

}

BOOL CDummies::CalcMat(const char *nameDummy,i_math::matrix43f &matBase,i_math::matrix43f &mat)
{
	int idx=FindDummy(nameDummy);
	if (idx==-1)
		return FALSE;
	return CalcMat((DWORD)idx,matBase,mat);
}

BOOL CDummies::CalcMat(DWORD iDummy,i_math::matrix43f &matBase,i_math::matrix43f &mat)
{
	if (iDummy>=_vecDummies.size())
		return FALSE;
	DummyInfo *di=&_vecDummies[iDummy];

	mat=di->matOff;
	mat*=matBase;

	return TRUE;
}

BOOL CDummies::CalcMatAtSkeleton(const char *nameDummy,IMatrice43 *sklmats,i_math::matrix43f &mat)
{
	int idx=FindDummy(nameDummy);
	if (idx==-1)
		return FALSE;
	return CalcMatAtSkeleton((DWORD)idx,sklmats,mat);
}

BOOL CDummies::CalcMatAtSkeleton(DWORD iDummy,IMatrice43 *sklmats,i_math::matrix43f &mat)
{
	if (!sklmats)
		return FALSE;
	if (iDummy>=_vecDummies.size())
		return FALSE;
	DummyInfo *di=&_vecDummies[iDummy];

	mat=di->matOff;

	DWORD nMats=sklmats->GetCount();
	i_math::matrix43f *mats=sklmats->GetPtr();

	int iBone=di->idxBone;
	if (iBone>=nMats)
		return FALSE;

	mat*=mats[iBone];

	return TRUE;
}


BOOL CDummies::CalcMatAtBones(const char *nameDummy,IMatrice43 *bonemats,i_math::matrix43f &mat)
{
	int idx=FindDummy(nameDummy);
	if (idx==-1)
		return FALSE;
	return CalcMatAtBones((DWORD)idx,bonemats,mat);
}

BOOL CDummies::CalcMatAtBones(DWORD iDummy,IMatrice43 *bonemats,i_math::matrix43f &mat)
{

	if (iDummy>=_vecDummies.size())
		return FALSE;
	DummyInfo *di=&_vecDummies[iDummy];

	mat=di->matOff;

	if (!bonemats)
		return FALSE;

	DWORD nMats=bonemats->GetCount();
	i_math::matrix43f *mats=bonemats->GetPtr();

	int iBone=di->idxBone;
	if (iBone>=nMats)
		return FALSE;
	while(iBone!=-1)
	{
		mat*=mats[iBone];
		iBone=_boneindices[iBone];
	}

	return TRUE;
}

BOOL CDummies::CalcAabb(i_math::aabbox3df &aabb,IMatrice43 *sklmats,i_math::matrix43f *mat)
{
	if ((!mat)&&(!sklmats))
		return FALSE;

	aabb.resetInvalid();

	i_math::matrix43f *mats=mat;
	DWORD nMats=1;

	if (sklmats)
	{
		mats=sklmats->GetPtr();
		nMats=sklmats->GetCount();
	}

// 	if (_boneindices.size()!=nMats)
// 		return FALSE;//骨骼数量不匹配


	i_math::aabbox3df aabbT;
	i_math::spheref sph;
	i_math::capsulef cap;
	for (int i=0;i<_vecDummies.size();i++)
	{
		DummyInfo *info=&_vecDummies[i];

		if (info->idxBone>=nMats)
			continue;

		switch(info->getBoundType())
		{
			case DummyInfo::BoundType_Point:
				sph.setZero();//0,0,0为中心,半径为0
				break;
			case DummyInfo::BoundType_Sphere:
			{
				sph=*info->getSphere();
				break;
			}
			case DummyInfo::BoundType_AABB:
			{
				sph.fromAABB(*info->getAAbb());
				break;
			}
			case DummyInfo::BoundType_Capsule:
			{
				info->matOff.transformCapsule(*info->getCapsule(),cap);
				mats[info->idxBone].transformCapsule(cap,cap);

				float radius=cap.radius;
				aabb.addInternalPoint(cap.start.x-radius,cap.start.y-radius,cap.start.z-radius);
				aabb.addInternalPoint(cap.start.x+radius,cap.start.y+radius,cap.start.z+radius);
				aabb.addInternalPoint(cap.end.x-radius,cap.end.y-radius,cap.end.z-radius);
				aabb.addInternalPoint(cap.end.x+radius,cap.end.y+radius,cap.end.z+radius);

				continue;
			}
			default:
				continue;
		}

		info->matOff.transformSphere(sph,sph);
		mats[info->idxBone].transformSphere(sph,sph);

		sph.toAABB(aabbT);

		aabb.addInternalBox(aabbT);
	}

	return TRUE;
}

BOOL CDummies::HitTest(i_math::vector3df &posRet,i_math::line3df &line,float radius,IMatrice43 *sklmats,i_math::matrix43f *matBase)
{
	if ((!matBase)&&(!sklmats))
		return FALSE;

	i_math::matrix43f *mats=matBase;
	DWORD nMats=1;

	if (sklmats)
	{
		mats=sklmats->GetPtr();
		nMats=sklmats->GetCount();
	}

	if (_boneindices.size()!=nMats)
		return FALSE;//骨骼数量不匹配

	assert(nMats>0);

	BOOL bRet=FALSE;

	float dist2Min=10000000.0f;
	float dist2;
	i_math::spheref sph;
	i_math::aabbox3df aabb;
	i_math::vector3df posHit,posTemp;
	i_math::matrix43f mat;

	if (nMats==1)
	{
		//先把line转换到dummies空间
		i_math::line3df lineDummies;
		if (TRUE)
		{
			i_math::matrix43f matInv=mats[0];
			matInv.makeInverse();
			matInv.transformLine(line,lineDummies);
			radius*=matInv.getScaleX();
		}

		i_math::line3df lineDummy;
		for (int i=0;i<_vecDummies.size();i++)
		{
			DummyInfo *info=&_vecDummies[i];

			if (info->idxBone>=nMats)
				return FALSE;

			BOOL bHit=FALSE;

			switch(info->getBoundType())
			{
				case DummyInfo::BoundType_Sphere:
				{
					info->matOff.transformSphere(*info->getSphere(),sph);
					sph.radius+=radius;
					if (sph.getIntersectionWithLine(lineDummies,posHit))
						bHit=TRUE;
					break;
				}
				case DummyInfo::BoundType_AABB:
				{
					info->matOffInv.transformLine(lineDummies,lineDummy);

					float radius2=radius;
					radius2*=info->matOffInv.getScaleX();
					aabb=*info->getAAbb();

					aabb.inflate(radius2,radius2,radius2);

					if (aabb.intersectsWithLine(lineDummy))
					{
						i_math::vector3df lineVect=(lineDummy.end-lineDummy.start);
						if (aabb.calcIntersectionWithLine(lineDummy.start,lineVect,posHit,posTemp))
						{
							info->matOff.transformVect(posHit,posHit);//转回dummies空间
							bHit=TRUE;
						}
					}
					break;
				}
				case DummyInfo::BoundType_Capsule:
				{
					//目前不支持
					assert(FALSE);
					break;
				}
				default:
					continue;
			}

			if (bHit)
			{
				dist2=(float)((posHit-lineDummies.start).getLengthSQ());
				if (dist2<dist2Min)
				{
					dist2Min=dist2;
					posRet=posHit;
					bRet=TRUE;
				}
			}
		}

		if (bRet)
			mats[0].transformVect(posRet,posRet);//转回世界空间
	}
	else
	{
		i_math::matrix43f matInv;
		i_math::line3df lineDummy;
		for (int i=0;i<_vecDummies.size();i++)
		{
			DummyInfo *info=&_vecDummies[i];

			if (info->idxBone>=nMats)
				return FALSE;

			BOOL bHit=FALSE;

			switch(info->getBoundType())
			{
				case DummyInfo::BoundType_Sphere:
				{
					info->matOff.transformSphere(*info->getSphere(),sph);
					mats[info->idxBone].transformSphere(sph,sph);
					sph.radius+=radius;
					if (sph.getIntersectionWithLine(line,posHit))
						bHit=TRUE;
					break;
				}
				case DummyInfo::BoundType_AABB:
				{
					mat=info->matOff*mats[info->idxBone];
					matInv=mat;
					matInv.makeInverse();

					matInv.transformLine(line,lineDummy);//转到dummy空间
					float radius2=radius;
					radius2*=matInv.getScaleX();

					i_math::aabbox3df aabb=*info->getAAbb();
					aabb.inflate(radius2,radius2,radius2);

					if (aabb.intersectsWithLine(lineDummy))
					{
						i_math::vector3df lineVect=(lineDummy.end-lineDummy.start);
						if (aabb.calcIntersectionWithLine(lineDummy.start,lineVect,posHit,posTemp))
						{
							mat.transformVect(posHit,posHit);//转回世界空间
							bHit=TRUE;
						}
					}
					break;
				}
				case DummyInfo::BoundType_Capsule:
				{
					//目前不支持
					assert(FALSE);
					break;
				}
				default:
					continue;
			}

			if (bHit)
			{
				dist2=(float)((posHit-line.start).getLengthSQ());
				if (dist2<dist2Min)
				{
					dist2Min=dist2;
					posRet=posHit;
					bRet=TRUE;
				}
			}
		}

	}

	return bRet;
}




BOOL CDummies::_AddDummy(DummyInfo &dummy,const char * name)
{
	std::string strName=name;
	std::hash_map<std::string,WORD>::iterator it;

	MAPSEEK_FROMKEY(strName,_mpNameIdx);
	if(it!=_mpNameIdx.end()) 
		return FALSE;

	_vecDummies.push_back(dummy);
	WORD nIndex=_vecDummies.size()-1;
	_mpNameIdx[strName]=nIndex;

	return TRUE;
}
void CDummies::_Clean()
{

	_vecDummies.clear();
	_mpNameIdx.clear();

}
BOOL CDummies::_OnTouch(IRenderSystem *pRS)
{
	VALIDATE_RES_TYPE(Res_Dummies);

	ResData *data = ResData_New(_typeData);
	data->LoadData(_data);
	
	BOOL bOk = FALSE;
	bOk = _FillBody(this,data);
	
	ResData_Delete(data);

	return bOk;
}
BOOL CDummies::_FillBody(CDummies * pDummies,ResData * pRes)
{
	DummiesData *pDummiesRes=(DummiesData *)pRes;
	pDummies->_Clean();
	for(int i=0;i<pDummiesRes->dummies.size();i++)
	{
		Dummy * p=&(pDummiesRes->dummies[i]);
		pDummies->_AddDummy(*p,p->name);
	}

	pDummies->_boneindices.resize(pDummiesRes->skeletonInfo.size());
	for(int i=0;i<pDummiesRes->skeletonInfo.size();i++)
	{
		BoneInfo * boneinfo = &(pDummiesRes->skeletonInfo[i]);
		pDummies->_boneindices[i]=boneinfo->iParent;
	}

	i_math::matrix43f mat;
	CalcAabb(_aabbDef,NULL,&mat);

	return TRUE;
}
void CDummies::_OnUnload()
{
	_Clean();
}

//////////////////////////////////////////////////////////////////////////
IResource * CDummiesMgr::ObtainRes(const char * path)
{	
    return _ObtainResS<CDummies>(path);	
}

BOOL CDummiesMgr::ReloadRes(const char *pathRes)
{
	return CResourceMgr::_ReloadRes<CDummies>(pathRes,'S');
}


//////////////////////////////////////////////////////////////////////////
IDummies * CDynDummiesMgr::Create(const DummiesData * dummies)
{
	CDummies * dum = _ObtainRes<CDummies>();

	int n = dummies->dummies.size();
	dum->_vecDummies.resize(n);

	for(int i = 0;i<n;i++)
		dum->_vecDummies[i] = *(DummyInfo*)(&dummies->dummies[i]);

	dum->_typeData = Res_Dummies;
	dum->SetState(CResource::Touched);

	return dum;
}



