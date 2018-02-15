#pragma once

#include "RenderSystem/ISound.h"


#include "Base.h"
#include "ResourceBase.h"


#include <vector>
#include <string>
#include <map>

class CSoundPlay:public ISoundPlay
{
public:
	DEFINE_CLASS(CSoundPlay);
	IMPLEMENT_REFCOUNT_C
	CSoundPlay()
	{
		_sound=NULL;
	}
	~CSoundPlay();

protected:
	void *_sound;
	friend class CSound;
};


struct DO_WriteTex;
class CSound:public ISound,public CResource
{
public:
	DECLARE_CLASS(CSound);
	CSound();

	RESOURCE_CORE()

public://take it as protected

	ISoundPlay *Play2D(BOOL bLoop=FALSE);
	ISoundPlay *Play3D(i_math::vector3df &pos,BOOL bLoop=FALSE);


protected:

	virtual BOOL _OnTouch(IRenderSystem *pRS)	;
	virtual void _OnUnload();

	void *_src;
	std::vector<BYTE>_buf;

	friend class CSoundMgr;

};


class CSoundMgr:public ISoundMgr,public CResourceMgr
{
public:
	CSoundMgr();
	BOOL Init(IRenderSystem *pRS,IFileSystem *pFS,const char *name);
	virtual void UnInit();

	RESOURCEMGR_CORE

	virtual IResource *ObtainRes(const char *pathRes);
	virtual BOOL ReloadRes(const char *path);

	void *GetEngine()	{		return _ikengine;	}


protected:
	virtual BOOL _CanGC(CResource *p);

	IFileSystem *_pFS;

	void *_ikengine;
};

