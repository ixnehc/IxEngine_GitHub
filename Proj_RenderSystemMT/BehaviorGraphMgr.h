#pragma once
#include "RenderSystem/IBehaviorGraph.h"
#include "ResourceBase.h"
#include <map>
#include <vector>
#include "resdata/ResData.h"
#include "resdata/BehaviorGraphData.h"
#include "class/class.h"


class CBehaviorGraphR:public CResource,public IBehaviorGraph
{
	friend class CBehaviorGraphMgr;
	friend class CDynBehaviorGraphMgr;

public:
	RESOURCE_CORE();

	DECLARE_CLASS(CBehaviorGraphR);

	CBehaviorGraphR();
	virtual  ~CBehaviorGraphR(void);
	virtual BehaviorGraphData *GetData()	{		return &_dataBgp;	}
protected:
	void _Clean();
	virtual BOOL _OnTouch(IRenderSystem *pRS);
	virtual void _OnUnload();

	BehaviorGraphData _dataBgp;
};


class CBehaviorGraphMgr :public IBehaviorGraphMgr,public CResourceMgr
{
public:
	RESOURCEMGR_CORE();
	CBehaviorGraphMgr(void);
	~CBehaviorGraphMgr(void);
	virtual IResource * ObtainRes(const char * path);
	virtual BOOL ReloadRes(const char *path);
	virtual IResource *ObtainRes(StringID nmBG);
	virtual void GarbageCollect(DWORD step)	{		return;	}//override to do nothing

	virtual void SetClasses(LinkPadClasses *clsses)	{		_clsses=clsses;	}
	virtual LinkPadClasses *GetClasses()	{		return _clsses;	}

protected:

	void _LoadAllRes();

	LinkPadClasses *_clsses;

};



class CDynBehaviorGraphMgr:public IDynBehaviorGraphMgr,public CResourceMgr
{
public:

	//interfaces
	RESOURCEMGR_CORE
	virtual IResource *ObtainRes(const char *pathRes)	{		return NULL;	}
	virtual IBehaviorGraph *Create(BehaviorGraphData *data);

};




