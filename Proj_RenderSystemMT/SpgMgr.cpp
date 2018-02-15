
/********************************************************************
	created:	27:4:2009   9:32
	filename: 	d:\IxEngine\Proj_RenderSystemMT\SpgMgr.cpp
	author:		chenxi
	
	purpose:	speed grass resource
*********************************************************************/

#include "stdh.h"

#include "RenderSystem/IRenderSystem.h"
#include "RenderSystem/ITexture.h"

#include ".\spgmgr.h"

#include "assert.h"

#include "resdata/SpgData.h"

#include "commondefines/general.h"

#include "fvfex/fvfex.h"

extern const char *ResolveRefPath(const char *path,CResource *owner);
//////////////////////////////////////////////////////////////////////////
//CSpg
BOOL FillResource(IRenderSystem *pRS,CSpg * pSpg,SpgData &data)
{
	assert(pRS);

	const char * path = ResolveRefPath(data.pathDifmap.c_str(),pSpg);
	pSpg->_texDif =(ITexture *)pRS->GetTexMgr()->ObtainRes(path);

	return TRUE;	
}

IMPLEMENT_CLASS(CSpg);

CSpg::CSpg()
{
	_texDif = NULL;
}

CSpg::~CSpg(void)
{
	Clean();
}

ITexture * CSpg::GetDiffuseMap()
{
	return _texDif;
}

BOOL CSpg::_OnTouch(IRenderSystem *pRS)
{
	VALIDATE_RES_TYPE(Res_Spg);

	_spgdata.LoadData(_data);
	
	FillResource(pRS,this,_spgdata);

	return TRUE;
}

i_math::spheref CSpg::GetDefaultSphere() const
{
	i_math::spheref sph;

	sph.fromAABB(_spgdata.aabb);

	return sph;
}

void CSpg::_OnUnload()
{
	Clean();
}

void CSpg::Clean()
{
	SAFE_RELEASE(_texDif);
}

float CSpg::GetHeight()
{
	return (_spgdata.aabb.MaxEdge.y - _spgdata.aabb.MinEdge.y);
}

DWORD CSpg::GetNumberOfVertexs()
{
	return _spgdata.blendPos.size();
}

//////////////////////////////////////////////////////////////////////////
//CSpgMgr

CSpgMgr::CSpgMgr(void)
{

}

CSpgMgr::~CSpgMgr(void)
{

}

IResource * CSpgMgr::ObtainRes(const char * path)
{	
    return _ObtainResS<CSpg>(path);	
}

BOOL CSpgMgr::ReloadRes(const char *pathRes)
{
	return CResourceMgr::_ReloadRes<CSpg>(pathRes,'S');
}

//////////////////////////////////////////////////////////////////////////
ISpg *CDynSpgMgr::Create(SpgData *data,const char * pathRes)
{
	CSpg *spg=_ObtainRes<CSpg>();
	
	data->SaveData(spg->_data);
	spg->_typeData=Res_Spg;
	spg->SetPath(pathRes);
	spg->SetState(CResource::Loaded);
	
	return spg;
}





