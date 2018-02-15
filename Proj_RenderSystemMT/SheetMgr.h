#pragma once

#include "RenderSystem/ISheet.h"


//#include "Base.h"
#include "ResourceBase.h"


#include <vector>
#include <string>
#include <hash_map>

class CSheetMgr;


//R代表Resource,主要为了避免和CSheet重名
class CSheetR:public ISheet,public CResource
{
public:
	DECLARE_CLASS(CSheetR);

	RESOURCE_CORE();
	CSheetR();

	//interfaces
	virtual CSheet *GetCore()	{		return _core;	}

protected:
	CSheet *_core;

	BOOL _Load(char *buf,DWORD szBuf);
	void _Clear();

	virtual BOOL _OnTouch(IRenderSystem *pRS)	;
	virtual void _OnUnload();
};



class CSheetMgr:public ISheetMgr,public CResourceMgr
{
public:
	CSheetMgr();

	RESOURCEMGR_CORE

	virtual IResource *ObtainRes(const char *pathRes);
	virtual BOOL ReloadRes(const char *path)	{		return FALSE;	}
	virtual BOOL ReloadSheets(const char *pathDB);


protected:


};
