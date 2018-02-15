/********************************************************************
	created:	2006/8/12   14:17
	filename: 	d:\IxEngine\Proj_RenderSystem\AnimMgr.cpp
	author:		ixnehc
	
	purpose:	IAnim,IAnimMgr implement
*********************************************************************/
#include "stdh.h"

#pragma warning(disable:4018)
#pragma warning(disable:4267)


#include "AnimMgr.h"
#include <assert.h>

#include "Log/LogFile.h"


extern KeyType KeyTypeFromResType(ResType t);


BOOL AnimFromData(CAnim *anim,ResData *data0, IRenderSystem *pRS)
{
	assert(anim->GetKeyType()==KeyTypeFromResType(data0->GetType()));

	AnimData *data=(AnimData*)data0;

	anim->_animpieces=data->animpieces;

	anim->_keyset=KeySet_Clone(data->GetKeySet());

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//CAnim
IMPLEMENT_CLASS(CAnim)
CAnim::CAnim()
{
	Zero();
}

void CAnim::Zero()
{
	_kt=KT_None;
	_keyset=NULL;
}

void CAnim::Clean()
{
	KeySet_Delete(_keyset);
	_animpieces.clear();
	Zero();
}

BOOL CAnim::_OnTouchHeader(IRenderSystem *pRS)
{
	_kt=KeyTypeFromResType(_typeData);
	if (_kt==KT_None)
	{
		LOG_DUMP_1P("ResourceMgr",Log_Error,"资源类型不匹配(资源\"%s\"不是一个动画资源!",_path.c_str());
		return FALSE;
	}
	return TRUE;
}


BOOL CAnim::_OnTouch(IRenderSystem *pRS)
{
	_kt=KeyTypeFromResType(_typeData);
	if (_kt==KT_None)
	{
		LOG_DUMP_1P("ResourceMgr",Log_Error,"资源类型不匹配(资源\"%s\"不是一个动画资源!",_path.c_str());
		return FALSE;
	}

	BOOL bOk;
	ResData *data=ResData_New(_typeData);
	data->LoadData(_data);

	bOk=AnimFromData(this,data,pRS);

	ResData_Delete(data);

	return bOk;
}

void CAnim::_OnUnload()
{
	Clean();
}


KeyType CAnim::GetKeyType()
{
	return _kt;
}


DWORD CAnim::GetAnimPieceCount()
{
	return _animpieces.size();
}

StringID CAnim::GetAnimPieceName(DWORD idx)
{
	if (idx>=_animpieces.size())
		return StringID_Invalid;
	return (StringID)_animpieces[idx].name;
}

//if not found,return -1
//name is caps sensitive
int CAnim::FindAnimPiece(StringID name)
{
	int idx;
	VEC_FIND_BY_ELEMENT(_animpieces,name,name,idx);
	return idx;
}

BOOL CAnim::GetAPRange(DWORD idxAP,AnimTick &tStart,AnimTick &tEnd)
{
	if (idxAP>=_animpieces.size())
		return FALSE;

	tStart=_animpieces[idxAP].tStart;
	tEnd=_animpieces[idxAP].tEnd;
	if (tEnd==ANIMTICK_INFINITE)
	{
		if (_keyset)
			tEnd=_keyset->GetEndTick();
		else
			tEnd=tStart;
	}
	return TRUE;
}

BOOL CAnim::GetAPKeyRange(DWORD idxAP,DWORD&iStart,DWORD&iEnd)
{
	if (idxAP>=_animpieces.size())
		return FALSE;

	iStart=_animpieces[idxAP].iStart;
	iEnd=_animpieces[idxAP].iEnd;
	return TRUE;
}



AnimEvent *CAnim::GetEvents(DWORD idxAP,DWORD &count)
{
	count=0;
	if (idxAP>=_animpieces.size())
		return NULL;
	count=_animpieces[idxAP].events.size();
	return &_animpieces[idxAP].events[0];
}

BOOL CAnim::GetAPParam(DWORD idxAP,DWORD iParam,float &v)
{
	if (idxAP>=_animpieces.size())
		return FALSE;

	if (iParam>=sizeof(_animpieces[idxAP].params))
		return FALSE;
	v=_animpieces[idxAP].params[iParam];
	return TRUE;
	
}


BOOL CAnim::CalcKey(Key *key,DWORD idxAP,AnimTick t)
{
	AnimTick tStart,tEnd;
	if (!GetAPRange(idxAP,tStart,tEnd))
		return FALSE;
	DWORD iStart,iEnd;
	if (!GetAPKeyRange(idxAP,iStart,iEnd))
		return FALSE;

	KeySet_CalcKey(_keyset,key,iStart,iEnd,tStart+t);
	return TRUE;
}

BOOL CAnim::GetKeys(Key *key,DWORD &nKeys,DWORD idxAP)
{
	DWORD iStart,iEnd;
	if (!GetAPKeyRange(idxAP,iStart,iEnd))
		return FALSE;

	DWORD nActualKey=iEnd-iStart;

	if (nActualKey<nKeys)
		nKeys=nActualKey;

	Key *k=_keyset->GetKey(iStart);
	memcpy(key,k,nKeys*_keyset->GetKeySize());

	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//CAnimMgr
CAnimMgr::CAnimMgr()
{
}

IResource *CAnimMgr::ObtainRes(const char *path)
{
	return _ObtainResH<CAnim>(path);
}

BOOL CAnimMgr::ReloadRes(const char *pathRes)
{
	return CResourceMgr::_ReloadRes<CAnim>(pathRes,'H');
}



//////////////////////////////////////////////////////////////////////////
//CDynAnimMgr

CDynAnimMgr::CDynAnimMgr()
{
}

IAnim *CDynAnimMgr::Create(AnimData *data)
{
	KeyType kt=KeyTypeFromResType(data->GetType());
	if (kt==KT_None)
		return NULL;
	CAnim *anim=_ObtainRes<CAnim>();
	anim->_kt=kt;
	
	if (FALSE==AnimFromData(anim,data,_pRS))
	{
		anim->SetState(CResource::Abandoned);
		return anim;
	}
	anim->SetState(CResource::Touched);

	return (IAnim*)anim;
}

