/********************************************************************
	created:	2006/07/07
	created:	7:7:2006   15:39
	filename: 	e:\IxEngine\Proj_RenderSystem\ResourceBase.cpp
	file path:	e:\IxEngine\Proj_RenderSystem
	file base:	ResourceBase
	file ext:	cpp
	author:		cxi
	
	purpose:	Resource and Resource manager base class
*********************************************************************/
#include "stdh.h"
#include "ResourceBase.h"

#include "FileSystem/IFileSystem.h"

#include "timer/timer.h"

#include "stringparser/stringparser.h"

#include "resdata/ResData.h"//for C_RES_SPLITTER

#include "Log/LogFile.h"

#ifndef MOBILE
#include "RenderSystem.h"
#endif

#include "resdata/TexData.h"


#include <assert.h>


#pragma warning(disable:4018)

LogFile g_logResMgr("ResManager");



//////////////////////////////////////////////////////////////////////////
//CResource
BOOL CResource::_bSyncTouch=TRUE;
BOOL CResource::_bDisableMT=TRUE;
CResource::CResource()
{
	_state=CResource::Empty;
	_nRef=0;
	_tUsed=0;

	_typeData=Res_None;

	_pMgr=NULL;

	_cGC=0;
	_ver=0;
}


BOOL CResource::LoadData(IFileSystem *pFS,BOOL bHeader)
{
	BOOL bLoaded=FALSE;

	std::string path;
	if (GetClass()->CheckName("CTexture"))
	{
		path=_pMgr->GetRS()->GetPath(Path_Res);
		path=path+"\\"+GetPath();
		IFile *fl=pFS->OpenFileAbs(path.c_str(),FileAccessMode_Read);
		if (fl)
		{
			TexData t;
			extern BOOL LoadTexData(IFile *fl,TexData *data);
			bLoaded=LoadTexData(fl,&t);
			fl->Close();
			if (bLoaded)
			{
				_typeData=Res_Texture;
				_data.swap(t.data);
				_textype=t.textype;
			}
		}
	}
	else
	if (GetClass()->CheckName("CSound"))
	{
		path=_pMgr->GetRS()->GetPath(Path_Res);
		path=path+"\\"+GetPath();
		DWORD sz=pFS->GetFileSizeAbs(path.c_str());
		if (sz!=0xffffffff)
		{
			if (sz<5120*1024)
			{
				IFile *fl=pFS->OpenFileAbs(path.c_str(),FileAccessMode_Read);
				if (fl)
				{
					int sz=fl->GetSize();
					_data.resize(sz);
					fl->Read(&_data[0],sz);
					fl->Close();
					_typeData=Res_Sound;
					bLoaded=TRUE;
				}
			}
			else
			{//文件很大,使用Stream方式载入
				_data.clear();
				_typeData=Res_Sound;
				bLoaded=TRUE;
			}
		}
	}
	else
	if (GetClass()->CheckName("CSheetR"))
	{
		path=_pMgr->GetRS()->GetPath(Path_Sheet);

		static std::string result;
		if(TRUE)
		{
			std::string pathDB;
			pathDB=path+"\\"+GetPath();
			MakeFileSuffix(pathDB,"xls");

			extern BOOL DumpDBTable(BOOL bMDBorXLS,const char *pathDB,const char *table,std::string &result);
			extern BOOL DumpDBTable2(const char *pathDB,const char *table,std::string &result);
			extern BOOL DumpDBTable3(const char *pathDB,const char *table,std::string &result);
//			bLoaded=DumpDBTable(FALSE,pathDB2.c_str(),buf[1].c_str(),result);
//			bLoaded=DumpDBTable2(pathDB2.c_str(),buf[1].c_str(),result);
			bLoaded=DumpDBTable3(pathDB.c_str(),"",result);
			if (bLoaded)
			{
				_data.resize(result.length()+1);
				memcpy(&_data[0],(BYTE*)result.c_str(),_data.size());
				_typeData=Res_Sheet;
			}

		}
	}
	else
	if (GetClass()->CheckName("CRagdoll"))
	{
		path=_pMgr->GetRS()->GetPath(Path_Res);
		path=path+"\\"+GetPath();
		IFile *fl=pFS->OpenFileAbs(path.c_str(),FileAccessMode_Read);
		if (fl)
		{
			int sz=fl->GetSize();
			_data.resize(sz);
			fl->Read(&_data[0],sz);
			fl->Close();
			_typeData=Res_Ragdoll;
			bLoaded=TRUE;
		}
	}
	else
	{
		if (GetClass()->CheckName("CBehaviorGraphR"))
			path=_pMgr->GetRS()->GetPath(Path_BehaviorGraph);
		else
		{
			if (GetClass()->CheckName("CRecordsR"))
				path=_pMgr->GetRS()->GetPath(Path_Records);
			else
				path=_pMgr->GetRS()->GetPath(Path_Res);
		}
			
		path=path+"\\"+GetPath();
		ResType LoadResRaw(IFileSystem *pFS,const char *path,std::vector<BYTE>&data,BOOL bHeader);
		_typeData=LoadResRaw(pFS,path.c_str(),_data,bHeader);

		if (_typeData!=Res_None)
			bLoaded=TRUE;
	}
	//XXXXX:more res type

	return bLoaded;
}



//////////////////////////////////////////////////////////////////////////
//CResourceMgr

BOOL CResourceMgr::Init(IRenderSystem *pRS,const char *nameMgr)
{
	_pRS=pRS;

	_name=nameMgr;

	_pathResRoot=pRS->GetPath(Path_Res);

#ifndef MOBILE
	_thrdResData=((CRenderSystem*)pRS)->GetResDataThread();
#endif

	_t=0;

	_itLast=_mapRes.end();
	_iLast=0;

	return TRUE;
}

void CResourceMgr::UnInit()
{
	_UnloadAll();

	_name="";
	_t=0;
	_pRS=NULL;
}


void CResourceMgr::_UnloadAll()
{
	std::hash_map<std::string,CResource*>::iterator it;
	for (it=_mapRes.begin();it!=_mapRes.end();it++)
	{
		((*it).second)->_OnUnload();
		Resource_Delete(((*it).second));
	}
	_mapRes.clear();

	for (int i=0;i<_vecRes.size();i++)
	{
		_vecRes[i]->_OnUnload();
		Resource_Delete(_vecRes[i]);
	}
	_vecRes.clear();
}

void CResourceMgr::Update()
{

}

void CResourceMgr::_ProcessOp(ResDataOp &op)
{
#ifndef MOBILE
	if (!CResource::_bDisableMT)
		_thrdResData->GetQueue()->PostOp(op);
	else
		_thrdResData->DoOp(op);//不使用多线程,直接处理
#else
	CResource *p=op.res;

	if (!p->LoadData(_pFS,FALSE))
		p->SetState(CResource::Failed);
	else
		p->SetState(CResource::Loaded);
#endif
}


//Check whether there is any resource left not released in the mgr
BOOL CResourceMgr::CheckResLeak()
{
	BOOL bLeak=FALSE;
	std::hash_map<std::string,CResource*>::iterator it;
	for (it=_mapRes.begin();it!=_mapRes.end();it++)
	{
		CResource *p=(*it).second;
		if (!p)			
			continue;
		if (p->GetRef()>0)
		{
			g_logResMgr.Dump("Res Leak found in [%s]:\"%s\"!",_name.c_str(),((*it).first).c_str());
			bLeak=TRUE;
		}
	}
	for (int i=0;i<_vecRes.size();i++)
	{
		if (!_vecRes[i])
			continue;
		if (_vecRes[i]->GetRef()>0)
		{
			g_logResMgr.Dump("Res Leak found in [%s]!",_name.c_str());
			bLeak=TRUE;
		}
	}

	return bLeak;
}

void CResourceMgr::GarbageCollect(DWORD step)
{
	if (_mapRes.size()>0)
	{
		while(step>0)
		{
			if (_itLast==_mapRes.end())
			{
				_itLast=_mapRes.begin();
				return;
			}

			std::hash_map<std::string,CResource*>::iterator it=_itLast;
			_itLast++;

			CResource *p=((*it).second);
			if (p->GetRef()<=0)
			{
				p->_cGC++;
				if ((p->_cGC>1000)&&(p->_state!=CResource::Loading))
				{//很长时间后仍然没有被引用,丢弃这个资源
					if (_CanGC(p))
					{
						p->_OnUnload();
						Resource_Delete((*it).second);
						_mapRes.erase(it);
					}
				}
			}
			step--;
		}
	}

	_gc_vec<CResource>(_vecRes,step,_iLast);

}
