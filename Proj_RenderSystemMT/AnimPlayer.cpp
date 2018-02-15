/********************************************************************
	created:	2010/12/01   21:55
	filename: 	d:\IxEngine\Proj_RenderSystem\AnimPlayer.cpp
	author:		ixnehc
	
	purpose:	简单的动画播放器
*********************************************************************/
#include "stdh.h"
#include "Base.h"
#include "interface/interface.h"

#include "AnimPlayer.h"

void CAnimPlayer::Clear()
{
	SAFE_RELEASE(_anim);
	_result.Clean();
	Zero();
}

void CAnimPlayer::Reset(IAnim *anim,DWORD idxAP,AnimTick tStart,BOOL bLoop)
{
	Clear();
	SAFE_REPLACE(_anim,anim);
	_tStart=tStart;
	_bLoop=bLoop?1:0;
	_idxAP=idxAP;
	KeySet_Define(&_result,_kt=anim->GetKeyType());
	_result.SetKeyCount(1);
}

Key* CAnimPlayer::Calc(AnimTick t)
{
	if (!_anim)
		return FALSE;

	if (t>_tStart)
		t-=_tStart;
	else
		t=0;

	AnimTick tAnim=(AnimTick)(_speedrate*((float)t));

	if (_bLoop)
	{
		AnimTick tStart,tEnd;
		if (FALSE==_anim->GetAPRange(_idxAP,tStart,tEnd))
			return NULL;
		tAnim%=(tEnd-tStart+1);
	}

	Key *k=_result.GetKey(0);

	if (FALSE==_anim->CalcKey(k,_idxAP,tAnim))
		return NULL;

	return k;
}

