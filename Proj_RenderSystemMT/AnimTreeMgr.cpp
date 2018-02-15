/********************************************************************
	created:	12:4:2010   12:33
	file path:	d:\IxEngine\Proj_RenderSystemMT
	author:		chenxi
	
	purpose:	anim tree manager
*********************************************************************/

#include "stdh.h"
#include ".\animtreemgr.h"
#include "assert.h"
#include "../common/Log/LogFile.h"
#include "resdata/AnimTreeData.h"
#include "commondefines/general.h"

//////////////////////////////////////////////////////////////////////////
//CAnimTree

IMPLEMENT_CLASS(CAnimTree);

CAnimTree::CAnimTree()
{	
}

CAnimTree::~CAnimTree(void)
{
}

void CAnimTree::_Clean()
{
	_atdata.Clean();
}
 

BOOL CAnimTree::_OnTouch(IRenderSystem *pRS)
{
	VALIDATE_RES_TYPE(Res_AnimTree);

	_atdata.LoadData(_data);

	return TRUE;
}

void CAnimTree::_OnUnload()
{
	_Clean();
}


//////////////////////////////////////////////////////////////////////////
//CAnimTreeMgr

CAnimTreeMgr::CAnimTreeMgr(void)
{

}

CAnimTreeMgr::~CAnimTreeMgr(void)
{

}
IResource * CAnimTreeMgr::ObtainRes(const char * path)
{	
    return _ObtainResS<CAnimTree>(path);	
}

BOOL CAnimTreeMgr::ReloadRes(const char *pathRes)
{
	return CResourceMgr::_ReloadRes<CAnimTree>(pathRes,'S');
}


//////////////////////////////////////////////////////////////////////////
//CDynAnimTreeMgr

IAnimTree *CDynAnimTreeMgr::Create(AnimTreeData *data)
{
	CAnimTree *at=_ObtainRes<CAnimTree>();
	data->SaveData(at->_data);
	at->_typeData=Res_AnimTree;
	at->SetState(CResource::Loaded);

	at->ForceTouch();

	return at;
}

