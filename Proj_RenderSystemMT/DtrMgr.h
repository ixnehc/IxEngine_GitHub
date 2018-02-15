#pragma once

#include "RenderSystem/IRenderSystem.h"

//#include "Base.h"
#include "ResourceBase.h"

#include "math/matrix43.h"

#include "resdata/DtrData.h"

#include "RenderSystem/IDtr.h"



#include <vector>
#include <string>
#include <map>

class CDtrMgr;

class CDtrPieces:public IDtrPieces
{
public:
	DEFINE_CLASS(CDtrPieces);
	CDtrPieces()
	{
		Zero();
	}
	void Zero()
	{
		_owner=NULL;
		_aabb.reset(0,0,0);
		_vb=NULL;
		_ib=NULL;
		_nParts=0;
	}
	void Clear();
	void CDtrPieces::Touch(IVertexMgr *vtxmgr,DtrPiecesData &data);

	virtual int AddRef()
	{
		if (_owner)
			return _owner->AddRef();
		return 0;
	}
	virtual int Release()
	{
		if (_owner)
			return _owner->Release();
		return 0;
	}
	virtual void ReleaseAll()
	{
		//Do Nothing,不支持
	}

	virtual IDtr *GetOwner()	{		return _owner;	}
	virtual i_math::aabbox3df &GetAABB()	{		return _aabb;	}
	virtual DWORD GetPartCount()	{		return _nParts;	}
	virtual DWORD GetPieceCount()	{		return _pieces.size();	}
	virtual BOOL GetPieceData(DWORD idx,DtrPieceData &data);
	virtual BOOL GetPieceBase(DWORD idx,i_math::vector3df &posBase);
	virtual IDtrPieces *GetSub(DWORD idx);
	virtual IDtrPieces *ObtainSub(DWORD idx);

	virtual BOOL Draw(IShader *shader,i_math::matrix43f *mats,DWORD cMats,DWORD iPart,DWORD mask);

protected:
	IDtr *_owner;
	i_math::aabbox3df _aabb;

	IVertexBuffer *_vb;
	IIndexBuffer *_ib;
	DWORD _nParts;
	std::vector<DtrPiecesData::PrimRange> _primsParts;//这个数组记录了每个part的各个piece的prim range,
																							//所以这个数组的大小为nParts*pieces.size()

	std::vector<i_math::vector3df> _verticesShp;
	std::vector<WORD> _indicesShp;
	std::vector<BYTE> _codes;//所有mopp的code
	std::vector<DtrPieceInfo> _pieces;

	std::vector<CDtrPieces *> _subs;

	friend class CDtr;

};

class CDtr;
class CDtr:public IDtr,public CResource
{
public:
	DECLARE_CLASS(CDtr);
	CDtr();

	//Interfaces
	RESOURCE_CORE()
	IDtrPieces *ObtainRoot()
	{
		if (_piecesAll.size()>0)
		{
			_piecesAll[0]->AddRef();
			return _piecesAll[0];
		}
		return NULL;
	}


protected:
	virtual BOOL _OnTouch(IRenderSystem *pRS);
	virtual BOOL _OnTouchHeader(IRenderSystem *pRS);
	virtual void _OnUnload();

	void _Zero();
	void _Clean();
private:

	std::vector<CDtrPieces *>_piecesAll;



	friend class CDtrMgr;
};



class CDeviceObject;
class CDtrMgr:public IDtrMgr,public CResourceMgr
{
public:
	CDtrMgr();

	RESOURCEMGR_CORE
	IResource *ObtainRes(const char *path);
	virtual BOOL ReloadRes(const char *path);

protected:
	//Overidables
friend class CDtr;

};

