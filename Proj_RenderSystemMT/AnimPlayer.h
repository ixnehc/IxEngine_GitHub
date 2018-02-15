#pragma once

#include "RenderSystem/IAnim.h"

#include "base.h"

#include "anim/animbase.h"
#include "anim/KeySet.h"

#include "bitset/bitset.h"

#include "class/class.h"

#include <deque>

class CAnimPlayer:public IAnimPlayer
{
public:
	DEFINE_CLASS(CAnimPlayer);
	CAnimPlayer()
	{
		Zero();
	}
	~CAnimPlayer()
	{
		Clear();
	}
	void Zero()
	{
		_kt=KT_None;
		_bIgnoreEvent=0;
		_bLoop=0;
		_anim=NULL;
		_idxAP=-1;
		_speedrate=1.0f;

		_tStart=0;
	}
	void Clear();
	//Interfaces
	IMPLEMENT_REFCOUNT_C;

	virtual void Reset(IAnim *anim,DWORD idxAP,AnimTick tStart,BOOL bLoop);
	
	virtual void Reset(AnimTick tStart)	{		_tStart=tStart;	}

	virtual IAnim *GetAnim()	{		return _anim;	}
	virtual int GetAPIdx()	{		return _idxAP;	}

	virtual void SetSpeedRate(float rate)	{		_speedrate=rate;	}

	virtual BOOL Tick(AnimTick t)	{		return FALSE;	}//目前不支持
	virtual Key* Calc(AnimTick t);

	//if event is ignored,no met event will be add to queue during IncTick()/DecTick()
	virtual void SetIgnoreEvent(BOOL bIgnore=TRUE)	{		_bIgnoreEvent=bIgnore?1:0;	}
	virtual BOOL FetchEvent(AnimEvent &e)	{		return FALSE;	}//暂时不支持event

protected:

	KeyType _kt;

	WORD _bIgnoreEvent:1;
	WORD _bLoop:1;
	WORD _bTicked:1;

	IAnim *_anim;
	int _idxAP;
	AnimTick _tStart;
	float _speedrate;

	KeySet _result;//the result holder
};