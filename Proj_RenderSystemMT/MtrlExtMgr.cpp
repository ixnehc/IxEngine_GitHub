/********************************************************************
	created:	2006/10/18   15:02
	filename: 	e:\IxEngine\Proj_RenderSystem\MtrlMgr.cpp
	author:		cxi
	
	purpose:	material resource manager
*********************************************************************/
#include "stdh.h"

#pragma warning(disable:4018)
#pragma warning(disable:4267)


#include "MtrlExtMgr.h"
#include "DeviceObject.h"
#include <assert.h>

#include "Log/LogFile.h"

#include "stringparser/stringparser.h"

#include "shaderlib/SLDefines.h"

#include "ShaderLibMgr.h"
#include "MtrlMgr.h"

#define ID_MTE_UNIQUE ((MteID)0xffff)
BOOL RepairResData(MtrlExtData *data,IRenderSystem *pRS)
{
	data->RefreshEPs();//更新eps

	CShaderLibMgr *mgr=(CShaderLibMgr *)pRS->GetShaderLibMgr();
	mgr->RegisterMte(data,ID_MTE_UNIQUE,"$");

	//修补sample
	if (TRUE)
	{
		MtrlData *dataMtrl=&data->sample;
		dataMtrl->lods.resize(1);

		extern void RepairMtrlLod(MtrlData::Lod&lod,IRenderSystem *pRS,MtrlExtData *dataMte,MteID idMte);
		RepairMtrlLod(dataMtrl->lods[0],pRS,data,ID_MTE_UNIQUE);
	}

	mgr->AbandonMte("$");

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CMtrlExt
IMPLEMENT_CLASS(CMtrlExt);

CMtrlExt::CMtrlExt()
{
	Zero();
}

CMtrlExt::~CMtrlExt()
{
	Clean();
}



void CMtrlExt::Zero()
{
	_dataMTE.Zero();
	_id=MteID_Invalid;
}

void CMtrlExt::Clean()
{
	_dataMTE.Clean();
}


BOOL CMtrlExt::_OnTouch(IRenderSystem *pRS)
{
	VALIDATE_RES_TYPE(Res_MtrlExt);

	if (_id==MteID_Invalid)
		_id=((CMtrlExtMgr*)_pMgr)->AllocID();

	_dataMTE.LoadData(_data);

	//注册到ShaderLib中去
	CShaderLibMgr *mgr=(CShaderLibMgr *)pRS->GetShaderLibMgr();
	mgr->RegisterMte(&_dataMTE,_id,GetPath());

	return TRUE;
}

void CMtrlExt::_OnUnload()
{
	Clean();
}


MteID CMtrlExt::GetID()
{
	Touch();
	return _id;
}



//////////////////////////////////////////////////////////////////////////
//CMtrlExtMgr
CMtrlExtMgr::CMtrlExtMgr()
{
	_idMteSeed=1;
}

BOOL CMtrlExtMgr::Init(IRenderSystem *pRS,const char *name)
{
	return CResourceMgr::Init(pRS,name);
}

IResource *CMtrlExtMgr::ObtainRes(const char *pathRes)
{
	return _ObtainResS<CMtrlExt>(pathRes);//同步载入资源
}

BOOL CMtrlExtMgr::ReloadRes(const char *pathRes)
{
	CShaderLibMgr *mgr=(CShaderLibMgr *)_pRS->GetShaderLibMgr();
	StringID idMte=mgr->AbandonMte(pathRes);//清除原先用到这个mte的shader,并返回这个路径原先对应的Mte的idMte

	BOOL bRet=CResourceMgr::_ReloadRes<CMtrlExt>(pathRes,'S');

	//重载所有以前用到idMte的材质
	CMtrlMgr *mgr2=(CMtrlMgr *)_pRS->GetMtrlMgr();
	mgr2->ReloadMteMtrl(idMte);

	return bRet;
}

IMtrlExt *CMtrlExtMgr::Create(MtrlExtData *data,const char *pathOverride)
{
	CMtrlExt *mte=_ObtainRes<CMtrlExt>();
	mte->_path=pathOverride;
	data->SaveData(mte->_data);
	mte->_typeData=Res_MtrlExt;
	mte->SetState(CResource::Loaded);

	mte->ForceTouch();

	return mte;
}

