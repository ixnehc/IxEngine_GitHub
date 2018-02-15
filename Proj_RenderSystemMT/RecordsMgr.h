#pragma once
#include "RenderSystem/IRecords.h"
#include "ResourceBase.h"
#include <hash_map>
#include <vector>
#include "resdata/ResData.h"
#include "resdata/RecordsData.h"
#include "class/class.h"

//R代表Resource,主要为了避免和CRecords重名
class CRecordsR:public CResource,public IRecords
{
	friend class CRecordsMgr;
public:

	DECLARE_CLASS(CRecordsR);

	virtual CRecords *GetRecords()	
	{		
		if (Touch()!=A_Ok)
			return NULL;
		return &_dataRecords.records;	
	}

	RESOURCE_CORE();

protected:	
	
	virtual BOOL _OnTouch(IRenderSystem *pRS)	;
	virtual void _OnUnload();

	RecordsData _dataRecords;

};




class CRecordsMgr :public IRecordsMgr,public CResourceMgr
{
public:
	RESOURCEMGR_CORE();
	virtual IResource * ObtainRes(const char * path);
	virtual BOOL ReloadRes(const char *path);

	virtual void BindRecordClass(const char *nameRes,CClass *clssRecord);//将资源文件名和一个class绑定起来
	virtual CClass *FindRecordClass(const char *nameRes);


protected:

	std::hash_map<std::string,CClass*> _clsses;

};

