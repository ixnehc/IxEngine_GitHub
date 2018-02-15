/********************************************************************
	created:	2012/11/20 
	author:		cxi
	
	purpose:	Session Mgr
*********************************************************************/
#include "stdh.h"
#include ".\BehaviorGraphmgr.h"
#include "assert.h"
#include "../common/Log/LogFile.h"
#include "resdata/BehaviorGraphData.h"
#include "commondefines/general.h"
#include "behaviorgraph/BehaviorGraphs.h"
#include "behaviorgraph/Behavior.h"
#include "behaviorgraph/BgnHelper.h"
#include "behaviorgraph/BgnInclude.h"


class CTempBGs:public CBehaviorGraphs
{
public:
	IMPLEMENT_REFCOUNT_C
	DEFINE_CLASS(CTempBGs);

	CTempBGs()
	{
		Zero();
	}

	~CTempBGs()
	{
		Clear();
	}

	void Zero()
	{
	}
	void Init(IFileSystem *pFS,IRenderSystem *pRS,const char *pathRoot);
	virtual void Clear();

protected:
	BOOL _LoadBGPads(IFile *fl,LinkPadClasses *clsses,CBehaviorGraphPads &pads);

	std::vector<BYTE>_bufTemp;
};

struct BgpClasses;
extern BgpClasses *BgpClasses_GetSingleton();

struct ResFileHeader2
{
	ResType type:8;
	DWORD off:24;//ResData完整数据的偏移(距离文件起始位置)
};


BOOL CTempBGs::_LoadBGPads(IFile *fl,LinkPadClasses *clsses,CBehaviorGraphPads &pads)
{

	ResFileHeader2 header;
	IFile_ReadVar(fl,header);

	if (header.type!=Res_BehaviorGraph)
		return FALSE;

	fl->Seek(header.off);

	DWORD sz;
	IFile_ReadVar(fl,sz);

	_bufTemp.resize(sz);
	fl->Read((char *)&_bufTemp[0],sz);

	return _LoadBGPadsFromData(&_bufTemp[0],pads,clsses);
}


void CTempBGs::Init(IFileSystem *pFS,IRenderSystem *pRS,const char *pathRoot)
{
	std::vector<std::string>pathes;
	pathes.reserve(512);
	IFileSystem_EnumFilesR(pFS,pathRoot,pathes);

	LinkPadClasses *clsses=pRS->GetBehaviorGraphMgr()->GetClasses();

	std::string path;
	for (int i=0;i<pathes.size();i++)
	{
		if (!CheckFileSuffix(pathes[i].c_str(),"bgr"))
			continue;
		path=pathRoot;
		path=path+"\\"+pathes[i];
		IFile *fl=pFS->OpenFileAbs(path.c_str(),FileAccessMode_Read);
		if (fl)
		{
			CBehaviorGraphPads *pads=Class_New2(CBehaviorGraphPads);
			if (!_LoadBGPads(fl,clsses,*pads))
			{
				fl->Close();
				Safe_Class_Delete(pads);
				continue;
			}
			fl->Close();
			_CompileBG(pads);
		}
	}
}

void CTempBGs::Clear()
{
	__super::Clear();
	_bufTemp.clear();
}

BOOL RepairResData(BehaviorGraphData*data ,IRenderSystem *pRS)
{
	DWORD c;
	CLinkPad **pads=data->pads.GetPads(c);
	BOOL bNeedRepair=FALSE;
	for (int i=0;i<c;i++)
	{
		CLinkPad *pad=pads[i];
		if (!pad)
			continue;

		if (!pad->GetClass()->CheckName("CBgp_Include"))
			continue;
		CBgp_Include *padInclude=(CBgp_Include *)pad;
		if (padInclude->_nm==StringID_Invalid)
		{
			padInclude->_stubin.clear();
			padInclude->_stubout.clear();
			continue;
		}
		bNeedRepair=TRUE;
		break;
	}

	if (!bNeedRepair)
		return TRUE;

	CTempBGs bgs;
	bgs.Init(pRS->GetFS(),pRS,pRS->GetPath(Path_BehaviorGraph));

	for (int i=0;i<c;i++)
	{
		CLinkPad *pad=pads[i];
		if (!pad)
			continue;

		if (!pad->GetClass()->CheckName("CBgp_Include"))
			continue;
		CBgp_Include *padInclude=(CBgp_Include *)pad;
		if (padInclude->_nm==StringID_Invalid)
			continue;

		padInclude->_stubin.clear();
		padInclude->_stubout.clear();

		CBehaviorGraph *bg=bgs.FindBG(padInclude->_nm);
		if (!bg)
			continue;

		DWORD c2;
		CLinkPad **pads2=bg->GetPads()?bg->GetPads()->GetPads(c2):NULL;
		if (!pads2)
			continue;

		for (int i=0;i<c2;i++)
		{
			CLinkPad *pad=pads2[i];
			if (!pad)
				continue;

			if (pad->GetClass()->CheckName("CBgp_StubIn"))
			{
				if (((CBgp_StubIn*)pad)->_nm!=StringID_Invalid)
				{
					padInclude->_stubin.push_back(((CBgp_StubIn*)pad)->_nm);
				}
			}
			if (pad->GetClass()->CheckName("CBgp_StubOut"))
			{
				if (((CBgp_StubOut*)pad)->_nm!=StringID_Invalid)
				{
					padInclude->_stubout.push_back(((CBgp_StubOut*)pad)->_nm);
				}
			}
		}
	}

	bgs.Clear();

	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//CBehaviorGraphR

IMPLEMENT_CLASS(CBehaviorGraphR);

CBehaviorGraphR::CBehaviorGraphR()
{	
}

CBehaviorGraphR::~CBehaviorGraphR(void)
{
}

void CBehaviorGraphR::_Clean()
{
	_dataBgp.Clean();
}
 

BOOL CBehaviorGraphR::_OnTouch(IRenderSystem *pRS)
{
	VALIDATE_RES_TYPE(Res_BehaviorGraph);

	_dataBgp.pads.SetClasses(((CBehaviorGraphMgr*)_pMgr)->GetClasses());

	_dataBgp.LoadData(_data);

	return TRUE;
}

void CBehaviorGraphR::_OnUnload()
{
	_Clean();
}


//////////////////////////////////////////////////////////////////////////
//CBehaviorGraphMgr

CBehaviorGraphMgr::CBehaviorGraphMgr(void)
{
	_clsses=NULL;
}

CBehaviorGraphMgr::~CBehaviorGraphMgr(void)
{

}
IResource * CBehaviorGraphMgr::ObtainRes(const char * path)
{	
    return _ObtainResS<CBehaviorGraphR>(path);	
}

BOOL CBehaviorGraphMgr::ReloadRes(const char *pathRes)
{
	return CResourceMgr::_ReloadRes<CBehaviorGraphR>(pathRes,'S');
}

void CBehaviorGraphMgr::_LoadAllRes()
{
	IRenderSystem *pRS=GetRS();
	const char *pathRoot=pRS->GetPath(Path_BehaviorGraph);

	IFileSystem *pFS=pRS->GetFS();
	std::vector<std::string>filelist;
	IFileSystem_EnumFilesR(pFS,pathRoot,filelist);

	for (int i=0;i<filelist.size();i++)
	{
		if (!CheckFileSuffix(filelist[i].c_str(),"bgr"))
			continue;

		IResource *res=ObtainRes(filelist[i].c_str());
		SAFE_RELEASE(res);
	}
}

IResource *CBehaviorGraphMgr::ObtainRes(StringID nmBG)
{
	_LoadAllRes();
	std::hash_map<std::string,CResource*>::iterator it;
	for (it=_mapRes.begin();it!=_mapRes.end();it++)
	{
		CBehaviorGraphR *p=(CBehaviorGraphR *)(*it).second;

		BehaviorGraphData *data=p->GetData();

		DWORD c=data->pads.GetPadCount();
		for (int i=0;i<c;i++)
		{
			CLinkPad *pad=data->pads.GetPad(i);
			if (pad)
			{
				if (pad->GetClass()->CheckName("CBgp_Graph"))
				{
					CBgp_Graph *bgp=(CBgp_Graph *)pad;
					if (bgp->_nm==nmBG)
						return static_cast<IResource*>(p);
				}
			}
		}
	}
	return NULL;
}




//////////////////////////////////////////////////////////////////////////
//CDynBehaviorGraphMgr

IBehaviorGraph *CDynBehaviorGraphMgr::Create(BehaviorGraphData *data)
{
	CBehaviorGraphR *ssn=_ObtainRes<CBehaviorGraphR>();
	data->SaveData(ssn->_data);
	ssn->_typeData=Res_BehaviorGraph;
	ssn->SetState(CResource::Loaded);

	ssn->ForceTouch();

	return ssn;
}

