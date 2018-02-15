#pragma once

#include "RenderSystem/IRagdoll.h"


//#include "Base.h"
#include "ResourceBase.h"




class CRagdoll:public IRagdoll,public CResource
{
public:
	DECLARE_CLASS(CRagdoll);
	CRagdoll();

	RESOURCE_CORE()

public://take it as protected
	BYTE *GetData(DWORD &szData)
	{
		szData=_buf.size();
		return &_buf[0];
	}

protected:

	virtual BOOL _OnTouch(IRenderSystem *pRS)	;
	virtual void _OnUnload();

	std::vector<BYTE>_buf;

	friend class CRagdollMgr;

};


class CRagdollMgr:public IRagdollMgr,public CResourceMgr
{
public:
	CRagdollMgr();

	RESOURCEMGR_CORE

	virtual IResource *ObtainRes(const char *pathRes);
	virtual BOOL ReloadRes(const char *path);
};

