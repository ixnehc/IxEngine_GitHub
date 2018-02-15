#pragma once

#include "RenderSystem/IMtrlExt.h"

#include "Base.h"
#include "ResourceBase.h"


#include "resdata/MtrlExtData.h"

#include "shaderlib/SLDefines.h"


#include <vector>
#include <string>
#include <map>

class CDeviceObject;
class CMtrlExtMgr;

class IShader;
class CMtrlExt:public IMtrlExt,public CResource
{
public:
	DECLARE_CLASS(CMtrlExt);

	CMtrlExt();
	~CMtrlExt();

	void Zero();
	void Clean();

	//interfaces
	RESOURCE_CORE();

	virtual MteID GetID();

	virtual MtrlExtData *GetData()	{		if (A_Ok!=Touch()) return NULL;return &_dataMTE;	}

protected:
	virtual BOOL _OnTouch(IRenderSystem *pRS)	;
	virtual void _OnUnload();

	MtrlExtData _dataMTE;
	MteID _id;
	friend class CMtrlExtMgr;

};

class CMtrlExtMgr:public IMtrlExtMgr,public CResourceMgr
{
public:
	CMtrlExtMgr();
	BOOL Init(IRenderSystem *pRS,const char *name);

	RESOURCEMGR_CORE
	virtual IResource *ObtainRes(const char *pathRes);
	virtual BOOL ReloadRes(const char *path);
	virtual IMtrlExt *Create(MtrlExtData *data,const char *pathOverride);



	virtual void GarbageCollect(DWORD step)	{	}//载入后的资源,我们就不释放了

	MteID AllocID()	{		return _idMteSeed++;	}


protected:

	MteID _idMteSeed;

	AnimTick _tPaint;

};


