
/********************************************************************
	created:	2008/07/15   14:17
	filename: 	d:\IxEngine\Proj_RenderSystem\SheetMgr_a.cpp
	author:		cxi
	
	purpose:	sheet
*********************************************************************/

#include "stdh.h"

#include "SheetMgr.h"

#include "stringparser/stringparser.h"

#include "timer/profiler.h"

#include <assert.h>




//////////////////////////////////////////////////////////////////////////
//CSheetR
IMPLEMENT_CLASS(CSheetR);
CSheetR::CSheetR()
{
	_core=NULL;
}


//这个buffer包含了一个字符串以及它的结束符\0
BOOL CSheetR::_Load(char *buf,DWORD szBuf)
{
	_core=Class_New2(CSheet);
	_core->AddRef();
	BOOL bRet=_core->Load(buf,szBuf);
	_core->SetPath(_path.c_str());

	std::string err;
	err=_core->GetErrorStr();
	if (!err.empty())
	{
		std::vector<std::string>buf;
		SplitStringBy("\n",err,&buf);

		for (int i=0;i<buf.size();i++)
		{
			LOG_DUMP_2P("SheetMgr",Log_Error,"%s(%s)",buf[i].c_str(),_path.c_str());
		}
	}

	return TRUE;

}


void CSheetR::_Clear()
{
	if (_core)
	{
		_core->Clear();
	}
	SAFE_RELEASE(_core);
}





BOOL CSheetR::_OnTouch(IRenderSystem *pRS)
{
	VALIDATE_RES_TYPE(Res_Sheet);

	ProfilerStart(SheetLoad);
	if (!_Load((char*)&_data[0],_data.size()))
		return FALSE;

	ProfilerEnd();
	return TRUE;
}

void CSheetR::_OnUnload()
{
	_Clear();
}



//////////////////////////////////////////////////////////////////////////
//CSheetMgr

CSheetMgr::CSheetMgr()
{

}




IResource *CSheetMgr::ObtainRes(const char *pathRes)
{
	return CResourceMgr::_ObtainResS<CSheetR>(pathRes);
}


BOOL CSheetMgr::ReloadSheets(const char *pathDB0)
{
	if (!CheckFileSuffix(pathDB0,"xls"))
		return FALSE;

	std::hash_map<std::string,CResource*>::iterator it;
	std::vector<std::string> reloads;
	for (it=_mapRes.begin();it!=_mapRes.end();it++)
	{
		if (StringEqualNoCase((*it).first.c_str(),pathDB0))
			reloads.push_back((*it).first);
	}

	for (int i=0;i<reloads.size();i++)
		CResourceMgr::_ReloadRes<CSheetR>(reloads[i].c_str(),'S');

	return reloads.size()>0;
}
