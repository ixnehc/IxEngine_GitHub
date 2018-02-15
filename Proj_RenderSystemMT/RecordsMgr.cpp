/********************************************************************
	created:	2012/03/19
	file base:	RecordsMgr
	author:		cxi
	
	purpose:	Records Manager
*********************************************************************/

#include "stdh.h"
#include "assert.h"
#include "../common/Log/LogFile.h"
#include "commondefines/general.h"

#include "RecordsMgr.h"

IMPLEMENT_CLASS(CRecordsR)



BOOL CRecordsR::_OnTouch(IRenderSystem *pRS)
{
	VALIDATE_RES_TYPE(Res_Records);

	std::string name=GetFileTitle(_path);
	CClass *clss=((CRecordsMgr*)_pMgr)->FindRecordClass(name.c_str());
	if (!clss)
	{
		LOG_DUMP_1P("CRecordsMgr",Log_Error,"不支持的Records格式:%s!",_path.c_str());
		return FALSE;
	}

	_dataRecords.records.Init(clss);
	_dataRecords.LoadData(_data);

	return TRUE;

}

void CRecordsR::_OnUnload()
{
	_dataRecords.Clear();
}

//////////////////////////////////////////////////////////////////////////
IResource * CRecordsMgr::ObtainRes(const char * path)
{
    return _ObtainResS<CRecordsR>(path);	
}

BOOL CRecordsMgr::ReloadRes(const char *pathRes)
{
	return CResourceMgr::_ReloadRes<CRecordsR>(pathRes,'S');
}

void CRecordsMgr::BindRecordClass(const char *nameRes,CClass *clssRecord)
{
	std::string name=GetFileTitle(std::string(nameRes));
	StringLower(name);

	_clsses[name]=clssRecord;
}


CClass *CRecordsMgr::FindRecordClass(const char *nameRes)
{
	std::hash_map<std::string,CClass*>::iterator it;
	std::string name=nameRes;
	StringLower(name);

	it=_clsses.find(name);
	if (it==_clsses.end())
		return NULL;

	return (*it).second;
}
