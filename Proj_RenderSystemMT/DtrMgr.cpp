/********************************************************************
	created:	6:7:2012   15:21
	filename: 	e:\IxEngine\Proj_RenderSystem\DtrMgr.cpp
	author:		cxi
	
	purpose:	CDtr & CDtrMgr (implement of IDtr & IDtrMgr)
*********************************************************************/
#include "stdh.h"

#pragma warning(disable:4018)
#pragma warning(disable:4267)

#include "RenderSystem.h"


#include "DtrMgr.h"
#include "DeviceObject.h"

#include "VertexMgr.h"
#include <assert.h>

#include "Log/LogFile.h"

#include "fvfex/fvfex.h"


#include "stringparser/stringparser.h"

////////////////////////////////////////////////////////////////////////
//CDtrPieces
void CDtrPieces::Clear()
{
	SAFE_RELEASE(_vb);
	SAFE_RELEASE(_ib);
	_primsParts.clear();

	_verticesShp.clear();
	_indicesShp.clear();
	_codes.clear();
	_pieces.clear();


	Zero();
}


BOOL CDtrPieces::GetPieceData(DWORD idx,DtrPieceData &data)
{
	if (idx>=_pieces.size())
		return FALSE;

	DtrPieceInfo *info=&_pieces[idx];

	data.tpBuild=info->tpBuild;
	data.off=info->offset;

	data.codes=&_codes[info->startCode];
	data.szCodes=info->szCode;
	data.verticesShp=&_verticesShp[info->startVerticesShp];
	data.szVerticesShp=info->szVerticesShp;
	data.indicesShp=&_indicesShp[info->startIndicesShp];
	data.szIndicesShp=info->szIndicesShp;

	return TRUE;
}

void CDtrPieces::Touch(IVertexMgr *vtxmgr,DtrPiecesData &data)
{
	_aabb=data.aabb;

	_vb=vtxmgr->CreateVB(data.vertices.size(),FVFEx_DtrVtx,1,0);
	if (_vb)
	{
		BYTE *buf=(BYTE*)_vb->Lock(TRUE);
		if (buf)
		{
			memcpy(buf,&data.vertices[0],data.vertices.size()*sizeof(VtxDtr));
			_vb->Unlock();
		}
	}

	_ib=vtxmgr->CreateIB(data.indices.size(),0);
	if (_ib)
	{
		WORD*buf=(WORD*)_ib->Lock(TRUE);
		if (buf)
		{
			memcpy(buf,&data.indices[0],data.indices.size()*sizeof(WORD));
			_ib->Unlock();
		}
	}

	_primsParts.swap(data.primsParts);
	_nParts=data.nParts;

	_verticesShp.swap(data.verticesShp);
	_indicesShp.swap(data.indicesShp);
	_codes.swap(data.codes);
	_pieces.swap(data.pieces);

}

BOOL CDtrPieces::GetPieceBase(DWORD idx,i_math::vector3df &posBase)
{
	if (idx>=_pieces.size())
		return FALSE;

	DtrPieceInfo *info=&_pieces[idx];
	posBase=info->posBase;
	return TRUE;
}

IDtrPieces *CDtrPieces::GetSub(DWORD idx)
{
	if (idx>=_subs.size())
		return NULL;
	CDtrPieces *sub=_subs[idx];
	return (IDtrPieces*)sub;
}


IDtrPieces *CDtrPieces::ObtainSub(DWORD idx)
{
	if (idx>=_subs.size())
		return NULL;
	CDtrPieces *sub=_subs[idx];
	SAFE_ADDREF(sub);

	return (IDtrPieces*)sub;
}


BOOL CDtrPieces::Draw(IShader *shader0,i_math::matrix43f *mats,DWORD cMats,DWORD iPart,DWORD mask)
{
	if (iPart>=_nParts)
		return FALSE;

	CShader *shader=(CShader *)shader0;	

	assert(_vb);
	_vb->Touch();
	_ib->Touch();

	if (EPFail==shader->SetEP_World(mats,cMats))
		return FALSE;

	DWORD nPieces=_pieces.size();
	if (nPieces>30)
		nPieces=30;
	DtrPiecesData::PrimRange *prims=&_primsParts[iPart*nPieces];
	VBBindArg arg;
	for (int i=0;i<nPieces;i++)
	{
		if (prims->pc==0)
			continue;
		if (!((1<<i)&mask))
			continue;

		arg.SetPrimRange(prims[i].ps,prims[i].pc);
		if (shader->BindVB(_vb,_ib,&arg))
			shader->DoShadeRaw();
	}

	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//CDtr
IMPLEMENT_CLASS(CDtr);

CDtr::CDtr()
{
	_Zero();
}


void CDtr::_Zero()
{
}

void CDtr::_Clean()
{
	for (int i=0;i<_piecesAll.size();i++)
	{
		if (_piecesAll[i])
		{
			_piecesAll[i]->Clear();
			Safe_Class_Delete(_piecesAll[i]);
		}
	}
	_piecesAll.clear();

	_Zero();
}

BOOL CDtr::_OnTouchHeader(IRenderSystem *pRS)
{

	return TRUE;
}

BOOL CDtr::_OnTouch(IRenderSystem *pRS)
{
	VALIDATE_RES_TYPE(Res_Dtr);

	DtrData data;
	data.LoadData(_data);

	_piecesAll.resize(data.piecesAll.size());

	CVertexMgr *vtxmgr=(CVertexMgr *)((CRenderSystem*)pRS)->GetVertexMgr();
	assert(vtxmgr);

	for (int i=0;i<_piecesAll.size();i++)
	{
		CDtrPieces *pieces=Class_New2(CDtrPieces);
		pieces->_owner=(IDtr*)this;
		pieces->Touch(vtxmgr,data.piecesAll[i]);
		_piecesAll[i]=pieces;
	}

	//构建sub信息
	for (int i=0;i<data.piecesAll.size();i++)
	{
		DtrPiecesData *dataSub=&data.piecesAll[i];
		if (dataSub->iParent>=0)
		{
			if (dataSub->iParent<_piecesAll.size())
			{
				CDtrPieces *piecesParent=_piecesAll[dataSub->iParent];
				piecesParent->_subs.resize(piecesParent->GetPieceCount());

				if ((dataSub->iParentSub>=0)&&(dataSub->iParentSub<piecesParent->_subs.size()))
					piecesParent->_subs[dataSub->iParentSub]=_piecesAll[i];
			}
		}
	}

	return TRUE;
}

void CDtr::_OnUnload()
{
	_Clean();
}


//////////////////////////////////////////////////////////////////////////
//CDtrMgr
CDtrMgr::CDtrMgr()
{
}

IResource *CDtrMgr::ObtainRes(const char *path)
{
	return _ObtainResS<CDtr>(path);
}

BOOL CDtrMgr::ReloadRes(const char *pathRes)
{
	return CResourceMgr::_ReloadRes<CDtr>(pathRes,'S');
}
