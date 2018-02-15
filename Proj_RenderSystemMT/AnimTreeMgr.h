#pragma once
#include "RenderSystem/IAnimTree.h"
#include "ResourceBase.h"
#include <map>
#include <vector>
#include "resdata/ResData.h"
#include "resdata/AnimTreeData.h"
#include "class/class.h"


class CAnimTree:public CResource,public IAnimTree
{
	friend class CAnimTreeMgr;
	friend class CDynAnimTreeMgr;

public:
	RESOURCE_CORE();

	DECLARE_CLASS(CAnimTree);

	CAnimTree();
	virtual  ~CAnimTree(void);
	virtual AnimTreeData *GetData()	{		return &_atdata;	}
protected:
	void _Clean();
	virtual BOOL _OnTouch(IRenderSystem *pRS);
	virtual void _OnUnload();

	AnimTreeData _atdata;
};


class CAnimTreeMgr :public IAnimTreeMgr,public CResourceMgr
{
public:
	RESOURCEMGR_CORE();
	CAnimTreeMgr(void);
	~CAnimTreeMgr(void);
	virtual IResource * ObtainRes(const char * path);
	virtual BOOL ReloadRes(const char *path);

protected:

};



class CDynAnimTreeMgr:public IDynAnimTreeMgr,public CResourceMgr
{
public:

	//interfaces
	RESOURCEMGR_CORE
	virtual IResource *ObtainRes(const char *pathRes)	{		return NULL;	}
	virtual IAnimTree *Create(AnimTreeData *data);

};




