#pragma once
#include "RenderSystem/ISpeedGrass.h"

#include "ResourceBase.h"

#include <map>

#include <vector>

#include "resdata/ResData.h"

#include "resdata/SpgData.h"

#include "class/class.h"

#include "RenderSystem/IRenderSystem.h"



class CSpg:public CResource,public ISpg
{
	friend class CSpgMgr;

public:
	RESOURCE_CORE();

	DECLARE_CLASS(CSpg);

	CSpg();
	virtual  ~CSpg(void);

protected:
	virtual ITexture * GetDiffuseMap();
	virtual i_math::aabbox3df GetAabb(){ return _spgdata.aabb;}
	virtual i_math::spheref GetDefaultSphere() const;
	virtual float GetHeight();

	virtual const WORD * GetIndexData(DWORD & c) {c = _spgdata.indices.size(); return &(_spgdata.indices[0]);}  // index buffer count

	virtual DWORD GetNumberOfVertexs();
	virtual const i_math::vector4df & GetVertex(int idx){assert(idx<_spgdata.blendPos.size()); return _spgdata.blendPos[idx];}
	virtual const i_math::vector3df & GetNormal(int idx){assert(idx<_spgdata.normals.size());  return _spgdata.normals[idx];}
	virtual const i_math::vector2df & GetTexVertex(int idx){assert(idx<_spgdata.uvs.size()); return _spgdata.uvs[idx];}

	void _Clean();
	virtual BOOL _OnTouch(IRenderSystem *pRS);
	virtual void _OnUnload();
	
	void Clean();
	
	friend BOOL FillResource(IRenderSystem *pRS,CSpg * pSpg,SpgData &data);
	friend class CDynSpgMgr;

protected:

	ITexture * _texDif;
	SpgData _spgdata;
};


class CSpgMgr :public ISpgMgr,public CResourceMgr
{
public:
	RESOURCEMGR_CORE();
	CSpgMgr(void);
	~CSpgMgr(void);
	virtual IResource * ObtainRes(const char * path);
	virtual BOOL ReloadRes(const char *path);	
protected:
};

class CDynSpgMgr:public IDynSpgMgr,public CResourceMgr
{
public:
	RESOURCEMGR_CORE
	virtual IResource *ObtainRes(const char *pathRes)	{		return NULL;	}
	virtual ISpg *Create(SpgData *data,const char * pathRes);
};









