#pragma once

#include "RenderSystem/IAnim.h"

//#include "Base.h"
#include "ResourceBase.h"


#include "resdata/AnimData.h"
#include "class/class.h"


#include <vector>
#include <string>
#include <map>

class CBoneAnim:public IBoneAnim,public CResource
{
public:
	DECLARE_CLASS(CBoneAnim);
	CBoneAnim();
	virtual ~CBoneAnim()	{Zero();}
	void Zero();
	void Clean();
	//Interfaces:
	RESOURCE_CORE()

	virtual BOOL _OnTouchHeader(IRenderSystem *pRS)	{		return TRUE;	}
	
	// mats:temp pointer
	virtual BOOL CalcKey(AnimTick t,i_math::xformf *xfms,DWORD nXfm,DWORD idxAP,Bitset<8>*mask);	
	
	//access animpiece
	virtual DWORD GetAnimPieceCount();
	virtual StringID GetAnimPieceName(DWORD idx);//if fail,return ""
	virtual int FindAnimPiece(StringID name);//if not found,return -1
	virtual BOOL GetAPRange(DWORD idxAP,AnimTick &tStart,AnimTick &tEnd);
	virtual AnimEvent *GetEvents(DWORD idxAP,DWORD &count);

	virtual BOOL GetAPParam(DWORD idxAP,DWORD iParam,float &v);
	virtual BOOL GetAnimRange(AnimTick &tStart,AnimTick &tEnd);
	//for Anim_Bones
	virtual ISkeleton*GetSkeleton()	{return _skeleton;}

	
	typedef std::map<StringID,DWORD>::iterator itMapStr2Idx;
protected:

	virtual BOOL _OnTouch(IRenderSystem *pRS)	;
	virtual void _OnUnload();
	
	BOOL _GetBonePos1(DWORD s,DWORD e,AnimTick t,i_math::vector3df &pos);
	BOOL _GetBonePos2(DWORD s,DWORD e,AnimTick t,i_math::vector3df &pos);
	BOOL _GetBoneScale1(DWORD s,DWORD e,AnimTick t,float &scale);
	BOOL _GetBoneScale2(DWORD s,DWORD e,AnimTick t,float &scale);
	BOOL _GetBoneRot1(DWORD s,DWORD e,AnimTick t,i_math::quatf &rot);
	BOOL _GetBoneRot2(DWORD s,DWORD e,AnimTick t,i_math::quatf &rot);

	BonesData2 _boneData2;
	ISkeleton * _skeleton;
	std::map<StringID,DWORD> _mapStr2Idx;
	DWORD _nBones;

friend class CBoneAnimMgr;
friend class CDynBoneAnimMgr;
friend BOOL AnimFromBoneData2(CBoneAnim *anim,IRenderSystem *pRS);
};

class CBoneAnimMgr:public IBoneAnimMgr,public CResourceMgr
{
public:
	CBoneAnimMgr();

	RESOURCEMGR_CORE
	virtual IResource *ObtainRes(const char *path);
	virtual BOOL ReloadRes(const char *path);
};


class CDynBoneAnimMgr:public IDynBoneAnimMgr,public CResourceMgr
{
public:
	CDynBoneAnimMgr();

	//interfaces
	RESOURCEMGR_CORE
	virtual IResource *ObtainRes(const char *path)	{		return NULL;	}
	virtual IBoneAnim *Create(ResData *data);
private:
};


