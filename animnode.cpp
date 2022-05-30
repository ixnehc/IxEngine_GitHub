#include "stdh.h"
#include "animnode.h"
#include "class/class.h"

#include "anim/KeySet.h"

#include "RenderSystem/ITools.h"
#include "RenderSystem/IDummies.h"
#include "RenderSystem/IRenderSystem.h"

#include "WorldSystem/IAssetRenderer.h"
#include "WorldSystem/IAnimTreeCtrl.h"
#include "WorldSystem/IRagdollCtrl.h"

#include "WorldSystem/IAssetSystem.h"

#include "Mano.h"
#include "SklLinks.h"

#include "commondefines/general_stl.h"
#include "resdata/SkeletonInfo.h"


#include "Log/LogDump.h"
#include "log/LogFile.h"

#include "avtrstates/avtrstates.h"

//////////////////////////////////////////////////////////////////////////
//CAnimNodes

CAnimNodes::CAnimNodes(AssetSystemState *ss)
{
	_ss=ss;
}


//////////////////////////////////////////////////////////////////////////
//CAnimNodeMatOffset

i_math::matrix43f*CAnimNodeMatOffset::GetMat(AnimTick t)	
{		
	if (!_base)
		return &_cache;

	if (t==_tCache)
		return &_cache;

	i_math::matrix43f *matBase=_base->GetMat(t);
	if (!matBase)
		return NULL;

	_cache=_matOff*(*matBase);
	_tCache=t;
	return &_cache;	
}


//////////////////////////////////////////////////////////////////////////
//CAnimNodeDtr
BOOL CAnimNodeDtr::Init(AssetSystemState*ss)
{
	_cache=ss->pRS->CreateMatrice43();
	return TRUE;
}

void CAnimNodeDtr::Clear()
{
	SAFE_RELEASE(_cache);
	Zero();
}

void CAnimNodeDtr::Reset(i_math::xformf *xfms,DWORD count,i_math::matrix43f &matBase,AnimTick t)
{
	_keys[0].resize(count);
	VEC_SET_BUFFER(_keys[0],xfms,count);
	_keys[1]=_keys[0];
	_t[0]=t;
	_t[1]=t;
	_iCur=0;
	_tCache=ANIMTICK_INFINITE;
	_cache->SetCount(count);

	_matBase=matBase;
}


IMatrice43 *CAnimNodeDtr::GetSkinMats(AnimTick t)
{
	if (t==_tCache)
		return _cache;

	int iEdge;
	float r;
	_CalcLerpInfo(t,iEdge,r);

	DWORD c=_cache->GetCount();
	i_math::matrix43f *mats=_cache->QueryPtr();
	if (iEdge==-1)
	{
		for (int i=0;i<c;i++)
		{
			i_math::xformf xfm;
			xfm=_keys[1-_iCur][i].getInterpolated(_keys[_iCur][i],r);
			xfm.getMatrix(mats[i]);
		}
	}
	else
	{
		for (int i=0;i<c;i++)
			_keys[iEdge][i].getMatrix(mats[i]);
	}
	_tCache=t;

	return _cache;
}


//////////////////////////////////////////////////////////////////////////
//CAnimNodeNoRoll

i_math::matrix43f *CAnimNodeNoRoll::GetMat(AnimTick t)
{
	if (t==_tCache)
		return &_cache;

	if (!_base)
		return NULL;

	i_math::matrix43f *mat=_base->GetMat(t);

	if (!mat)
		return NULL;

	i_math::xformf xfm;
	xfm.fromMatrix(*mat);

	i_math::vector3df euler;
	xfm.rot.toEuler(euler);
	euler.z=0;//清掉roll
	xfm.rot.fromEuler(euler);
	xfm.getMatrix(_cache);

	_tCache=t;
	return &_cache;
}

//////////////////////////////////////////////////////////////////////////
//CAnimNodeSkeleton
BOOL CAnimNodeSkeleton::Init(AssetSystemState *ss)
{
	_matsCache=ss->pRS->CreateMatrice43();
	return TRUE;
}

void CAnimNodeSkeleton::Clear()
{
	SAFE_RELEASE(_ctrl);
	SAFE_RELEASE(_matsCache);
	SAFE_RELEASE(_dummies);
	Zero();
}


void CAnimNodeSkeleton::SetAnimTreeCtrl(IAnimTreeCtrl *ctrl)
{
	SAFE_REPLACE(_ctrl,ctrl);
}

void CAnimNodeSkeleton::SetBase(IAnimNode *an)
{
	SAFE_REPLACE(_base,an);
	_tCache=ANIMTICK_INFINITE;
}

void CAnimNodeSkeleton::SetBaseLink(IDummies *dummies,const char *name)
{
	SAFE_ADDREF(dummies);
	SAFE_RELEASE(_dummies);
	_dummies=dummies;
	if (dummies)
		_iDummy=dummies->FindDummy(name);
	else
		_iDummy=-1;
}

void CAnimNodeSkeleton::SetBaseLink(IDummies *dummies,DWORD iDummy)
{
	SAFE_ADDREF(dummies);
	SAFE_RELEASE(_dummies);
	_dummies=dummies;

	_iDummy=iDummy;
}

i_math::matrix43f *CAnimNodeSkeleton::_CalcBaseLinkMat(AnimTick t,i_math::matrix43f *mat)
{
	if (!_ctrl)
		return NULL;
	ProfilerStart_Recent(CalcAnimTree)
	if (FALSE==_ctrl->Calc(t,_matsCache))
	{
		ProfilerEnd();
		return NULL;
	}
	ProfilerEnd();

	i_math::matrix43f *matBase=NULL;
	if (_base)
	{
		matBase=_base->GetMat(t);
		if (!matBase)
			return NULL;//这个时刻,base没有有意义的值,返回失败的值
	}

	if (_dummies)
	{
		if (_dummies->CalcMatAtBones(_iDummy,_matsCache,*mat))
		{
			mat->makeInverse();
			if (matBase)
				*mat=(*mat)*(*matBase);
			matBase=mat;
		}
	}

	
	if (!matBase)
	{//找不到dummy,也没有base,返回identity的matrix
		mat->makeIdentity();
		matBase=mat;
	}

	return matBase;
}


BOOL CAnimNodeSkeleton::CalcBaseLinkMat(AnimTick t,i_math::matrix43f &matBase)
{
	i_math::matrix43f *mat=_CalcBaseLinkMat(t,&matBase);
	if (!mat)
		return FALSE;
	if (mat!=&matBase)
		matBase=*mat;
	return TRUE;
}


IMatrice43*CAnimNodeSkeleton::GetSklMats(AnimTick t)
{
	if (!_ctrl)
		return NULL;

	if (t==_tCache)
		return _matsCache;

	i_math::matrix43f mat;
	i_math::matrix43f *matBase=_CalcBaseLinkMat(t,&mat);

	if (!matBase)
		return NULL;

	ISkeleton *skl=_ctrl->GetSkeleton();
	assert(skl);

	BoneCtrls bcs;
	if(_ctrl)
	{
		static BoneCtrl buf[256];
		bcs.bcs=buf;
		bcs.nBC=skl->GetBoneCount();

		if (_ctrl->Calc(t,bcs))
			skl->CalcSkeletonMatrice(_matsCache,&bcs,matBase);
		else
			skl->CalcSkeletonMatrice(_matsCache,NULL,matBase);
	}
	else
		skl->CalcSkeletonMatrice(_matsCache,NULL,matBase);

	_tCache=t;
	return _matsCache;
}

BOOL CAnimNodeSkeleton::CheckBaseLink(IDummies *dummies,const char *name)
{
	if (_dummies==dummies)
	{
		if (!_dummies)
			return TRUE;
		if (strcmp(_dummies->GetDummyName(_iDummy),name)==0)
			return TRUE;
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
//CAnimNodeMatrice43
void CAnimNodeMatrice43::SetSkeleton(ISkeleton *skl)
{
	SAFE_REPLACE(_skl,skl);
}


BOOL CAnimNodeMatrice43::Init(AssetSystemState*ss)
{
	_cache=ss->pRS->CreateMatrice43();
	return TRUE;
}

void CAnimNodeMatrice43::Clear()
{
	SAFE_RELEASE(_cache);
	SAFE_RELEASE(_skl);
	VEC_EMPTY(i_math::xformf,_keys[0]);
	VEC_EMPTY(i_math::xformf,_keys[1]);
	Zero();
}


void CAnimNodeMatrice43::Reset(i_math::xformf *xfms,DWORD count,AnimTick t)
{
	_keys[0].resize(count);
	VEC_SET_BUFFER(_keys[0],xfms,count);
	_keys[1]=_keys[0];
	_t[0]=t;
	_t[1]=t;
	_iCur=0;
	_tCache=ANIMTICK_INFINITE;
	_cache->SetCount(count);
}


IMatrice43 *CAnimNodeMatrice43::GetSklMats(AnimTick t)
{
	if (t==_tCache)
		return _cache;

	if (_skl)
		return NULL;

	if (_skl->GetBoneCount()!=_cache->GetCount())
		return NULL;

	i_math::matrix43f *matBase=NULL;
	if (_base)
	{
		matBase=_base->GetMat(t);
		if (!matBase)
			return NULL;//这个时刻,base没有有意义的值,返回失败的值
	}

	int iEdge;
	float r;
	_CalcLerpInfo(t,iEdge,r);

	DWORD c=_cache->GetCount();
	i_math::matrix43f *mats=_cache->QueryPtr();
	if (iEdge==-1)
	{
		for (int i=0;i<c;i++)
		{
			i_math::xformf xfm;
			xfm=_keys[1-_iCur][i].getInterpolated(_keys[_iCur][i],r);
			xfm.getMatrix(mats[i]);
		}
	}
	else
	{
		for (int i=0;i<c;i++)
			_keys[iEdge][i].getMatrix(mats[i]);
	}

	SkeletonInfo*si=_skl->GetSkeletonInfo();

	int iParent=-1;
	for (int i=0;i<c;i++)
	{
		iParent=(*si)[i].iParent;
		if (iParent==-1)
			mats[i]=mats[i]*(*matBase);
		else
			mats[i]=mats[i]*mats[iParent];
	}
	_tCache=t;

	return _cache;
}



//////////////////////////////////////////////////////////////////////////
//CAnimNodeRagdoll
BOOL CAnimNodeRagdoll::Init(AssetSystemState *ss)
{
	_matsCache=ss->pRS->CreateMatrice43();
	return TRUE;
}

void CAnimNodeRagdoll::Clear()
{
	SAFE_RELEASE(_ctrl);
	SAFE_RELEASE(_matsCache);
	Zero();
}


void CAnimNodeRagdoll::SetRagdollCtrl(IRagdollCtrl*ctrl)
{
	SAFE_REPLACE(_ctrl,ctrl);
}

IMatrice43*CAnimNodeRagdoll::GetSklMats(AnimTick t)
{
	if (!_ctrl)
		return NULL;

	if (t==_tCache)
		return _matsCache;

	if (FALSE==_ctrl->Calc(t,_matsCache))
		return NULL;

	_tCache=t;
	return _matsCache;
}

i_math::matrix43f *CAnimNodeRagdoll::GetMat(AnimTick t)
{
	if (!_ctrl)
		return NULL;

	if (t==_tCache3)
		return &_matCache;

	if (FALSE==_ctrl->Calc(t,_matCache))
		return NULL;
	_tCache3=t;
	return &_matCache;
}


//////////////////////////////////////////////////////////////////////////
//CAnimNodeSkin
BOOL CAnimNodeSkin::Init(AssetSystemState *ss)
{
	_matsCache=ss->pRS->CreateMatrice43();
	return TRUE;
}

void CAnimNodeSkin::Clear()
{
	SAFE_RELEASE(_skeleton);
    SAFE_RELEASE(_ctrlCloth);
    SAFE_RELEASE(_matsCache);
	Zero();
}


void CAnimNodeSkin::SetBase(IAnimNode *an)
{
	SAFE_REPLACE(_base,an);

	_tCache=ANIMTICK_INFINITE;
}

void CAnimNodeSkin::SetSkeleton(ISkeleton *skl)
{
	SAFE_ADDREF(skl);
	SAFE_RELEASE(_skeleton);

	_skeleton=skl;
}

void CAnimNodeSkin::SetClothCtrl(IClothCtrl *ctrlCloth)
{
    SAFE_REPLACE(_ctrlCloth, ctrlCloth);
}



IMatrice43*CAnimNodeSkin::GetSkinMats(AnimTick t)
{
	if ((t==_tCache)&&(t==_tClothCache))
		return _matsCache;

    IMatrice43 *mats = NULL;
    if (t != _tCache)
    {
        mats = _base->GetSklMats(t);
        if (!mats)
            return NULL;
        _matsCache->Set(mats->GetPtr(), mats->GetCount());
        _skeleton->CalcSkinMatrice(_matsCache);

        _tCache = t;
    }

    if (_ctrlCloth)
    {
        BYTE affects[256];
        DWORD nAffects;

        if (FALSE == _ctrlCloth->FillMats(_matsCache, t, affects, nAffects))
            return _matsCache;

        _skeleton->CalcSkeletonMatrice(_matsCache, affects, nAffects);
    }
    _tClothCache = t;

	return _matsCache;
}

//////////////////////////////////////////////////////////////////////////
//CAnimNodePathNav

BOOL CAnimNodePathNav::Init(AssetSystemState *ss)
{
	_skllinks=Class_New2(CSklLinks);
	_skllinks->AddRef();
	_skllinks->SetBase(this);

	return TRUE;
}

void CAnimNodePathNav::Destroy()
{
	Clear();
	Release();
}


void CAnimNodePathNav::Clear()
{
	SAFE_DESTROY(_skllinks);
	SAFE_RELEASE(_ctrl);
	SAFE_RELEASE(_owner);
	Zero();
}

void CAnimNodePathNav::SetAnimTreeCtrl(IAnimTreeCtrl *ctrl)
{
	SAFE_REPLACE(_ctrl,ctrl);
}

void CAnimNodePathNav::SetBase(IAnimNode *an)
{
	SAFE_REPLACE(_base,an);
	_tCache=ANIMTICK_INFINITE;
}

i_math::matrix43f*CAnimNodePathNav::GetMat(AnimTick t)
{
	if (!_ctrl)
		return NULL;

	if (t==_tCache)
		return &_matCache;

	i_math::matrix43f *matBase=NULL;
	if (_base)
	{
		matBase=_base->GetMat(t);
		if (!matBase)
			return NULL;//这个时刻,base没有有意义的值,返回失败的值
	}

	i_math::xformf xfm;
	if (FALSE==_ctrl->Calc(t,xfm))
		return NULL;

	xfm.getMatrix(_matCache);

	if (matBase)
		_matCache=_matCache*(*matBase);

	_tCache=t;
	return &_matCache;
}


void CAnimNodePathNav::SetOwner(IAsset *ast)
{
	SAFE_REPLACE(_owner,ast);
}

IMano*CAnimNodePathNav::GetMano()
{
	if (_skllinks)
	{
		IMano *mano=_skllinks->GetBaseMano();
		if(mano)
			return mano;
	}
	if (_base)
		return _base->GetMano();
	return  NULL;
}



//////////////////////////////////////////////////////////////////////////
//CAnimNodeUV
i_math::matrix43f*CAnimNodeUV::GetMat(AnimTick t)
{
	if (!_player)
		return NULL;

	if (t==_tCache)
		return &_cache;

	Key_mapcoord * key=(Key_mapcoord *)_player->Calc(t);

	key->MakeMat(_cache);

	_tCache=t;
	return &_cache;
}

//////////////////////////////////////////////////////////////////////////
//CAnimNodePath
i_math::matrix43f*CAnimNodePath::GetMat(AnimTick t)
{
	if (!_player)
		return NULL;

	if (t==_tCache)
		return &_cache;

	i_math::matrix43f *matBase=NULL;
	if (_base)
	{
		matBase=_base->GetMat(t);
		if (!matBase)
			return NULL;
	}

	Key_xform *key=(Key_xform*)_player->Calc(t);
	if (!key)
		return NULL;

	if (_bLocalBase)
	{
		if (!_bBaseCalc)
		{
			IAnim *anim=_player->GetAnim();
			int idxAP=_player->GetAPIdx();
			Key_xform key;
			if (anim)
				anim->CalcKey(&key,idxAP,0);
			_posBase=key.v.pos;
			_bBaseCalc=1;
		}
		key->v.pos-=_posBase;
	}

	key->v.getMatrix(_cache);

	if (matBase)
		_cache=_cache*(*matBase);

	_tCache=t;
	return &_cache;
}





//////////////////////////////////////////////////////////////////////////
//CAnimNodeSklLinks

CAnimNodeSklLinks::~CAnimNodeSklLinks()
{
	_Clear();
}

void CAnimNodeSklLinks::_Clear()
{
	SAFE_DESTROY(_bv);
	SAFE_DESTROY(_skllinks);
	SAFE_RELEASE(_owner);
	SAFE_RELEASE(_avs);
	Safe_Class_Delete(_ltc);
}

void CAnimNodeSklLinks::Destroy()
{
	_Clear();
	Release();
}

BOOL CAnimNodeSklLinks::Init(AssetSystemState *ss)
{
	_skllinks=Class_New2(CSklLinks);
	_skllinks->AddRef();
	_skllinks->SetBase(this);

	_ss=ss;

	return TRUE;
}

void CAnimNodeSklLinks::SetRatomsBv(IRatomsBv *bv)
{
	SAFE_DESTROY(_bv);
	_bv=bv;
	SAFE_ADDREF(_bv);
}

void CAnimNodeSklLinks::SetOwner(IAsset *ast)
{
	SAFE_REPLACE(_owner,ast);
}

void CAnimNodeSklLinks::SetAvs(CAvtrStates*avs)
{
	SAFE_REPLACE(_avs,avs)
}

IMano*CAnimNodeSklLinks::GetMano()
{
	if (_skllinks)
	{
		IMano *mano=_skllinks->GetBaseMano();
		if(mano)
			return mano;
	}
	if (_base)
		return _base->GetMano();
	return  NULL;
}

void CAnimNodeSklLinks::EnableLocalTime()
{
	if (!_ltc)
	{
		_ltc=Class_New2(LocalTimeCache);
		_ltc->localtime.Reset(_ss->tLast);
		_ltc->localtime.Update(_ss->t);
		_ltc->t=_ss->t;
	}
}


CLocalTime *CAnimNodeSklLinks::GetLocalTime()
{
	if (_ltc)
	{
		if (_ltc->t!=_ss->t)
		{
			_ltc->localtime.Update(_ss->t);
			_ltc->t=_ss->t;
		}
		return &_ltc->localtime;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////

CAnimNodeAnimTreeCtrl::CAnimNodeAnimTreeCtrl(void)
{
	_ctrl = NULL;
}

CAnimNodeAnimTreeCtrl::~CAnimNodeAnimTreeCtrl(void)
{
	SAFE_RELEASE(_ctrl);
}

void CAnimNodeAnimTreeCtrl::SetAnimTreeCtrl(IAnimTreeCtrl * ctrl)
{
	SAFE_REPLACE(_ctrl,ctrl);
}

//////////////////////////////////////////////////////////////////////////
//CAnimNodeCamera
CAnimNodeCamera::~CAnimNodeCamera()
{
	SAFE_RELEASE(_cam);
}

void CAnimNodeCamera::SetCamera(ICamera *cam,BOOL bLab)
{
	SAFE_REPLACE(_cam,cam);
	_bLab=bLab;
}
