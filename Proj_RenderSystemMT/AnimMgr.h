#pragma once

#include "RenderSystem/IAnim.h"


#include "ResourceBase.h"


#include "resdata/AnimData.h"
#include "class/class.h"


#include <vector>
#include <string>

class CDeviceObject;
class CAnimMgr;

class CAnim:public IAnim,public CResource
{
public:
	DECLARE_CLASS(CAnim);
	CAnim();
	virtual ~CAnim()	{	}
	void Zero();
	virtual void Clean();

	//Interfaces:
	RESOURCE_CORE()
	virtual KeyType GetKeyType();
	virtual DWORD GetAnimPieceCount();
	virtual StringID GetAnimPieceName(DWORD idx);
	virtual int FindAnimPiece(StringID name);//if not found,return -1

	virtual BOOL CalcKey(Key *key,DWORD idxAP,AnimTick t);

	virtual KeySet *GetKeySet()	{		return _keyset;	}

	virtual AnimEvent *GetEvents(DWORD idxAP,DWORD &count);
	virtual BOOL GetAPRange(DWORD idxAP,AnimTick &tStart,AnimTick &tEnd);
	virtual BOOL GetAPKeyRange(DWORD idxAP,DWORD&iStart,DWORD&iEnd);//[iStart,iEnd)
	virtual BOOL GetAPParam(DWORD idxAP,DWORD iParam,float &v);

	virtual BOOL GetKeys(Key *key,DWORD &nKeys,DWORD idxAP);


protected:

	virtual BOOL _OnTouch(IRenderSystem *pRS)	;
	virtual BOOL _OnTouchHeader(IRenderSystem *pRS);
	virtual void _OnUnload();

	KeyType _kt;
	KeySet *_keyset;
	std::vector<AnimPiece> _animpieces;

friend class CAnimMgr;
friend class CDynAnimMgr;
friend BOOL AnimFromData(CAnim *anim,ResData *data,IRenderSystem* sklmgr);
};

class CAnimMgr:public IAnimMgr,public CResourceMgr
{
public:
	CAnimMgr();

	RESOURCEMGR_CORE
	virtual IResource *ObtainRes(const char *path);
	virtual BOOL ReloadRes(const char *path);

};


struct AnimData;

class CDynAnimMgr:public IDynAnimMgr,public CResourceMgr
{
public:
	CDynAnimMgr();

	//interfaces
	RESOURCEMGR_CORE
	virtual IResource *ObtainRes(const char *path)	{		return NULL;	}
	virtual IAnim *Create(AnimData *data);


private:

};


