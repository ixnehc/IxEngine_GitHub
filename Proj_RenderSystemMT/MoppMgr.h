#pragma once
#include "RenderSystem/IMopp.h"
#include "ResourceBase.h"
#include <map>
#include <vector>
#include "resdata/ResData.h"
#include "resdata/MoppData.h"
#include "class/class.h"


class CMopp:public CResource,public IMopp
{
	friend class CMoppMgr;

public:
	RESOURCE_CORE();

	DECLARE_CLASS(CMopp);


	CMopp();
	virtual  ~CMopp(void);
	i_math::aabbox3df &GetAabb()	{		return _moppdata.aabb;	}
	inline i_math::vector3df *GetVertices(DWORD &count)
	{
		count=_moppdata.vertices.size();
		return &_moppdata.vertices[0];
	}
 	inline WORD *GetIndices(DWORD &count)
	{
		count=_moppdata.indices.size();
		return &_moppdata.indices[0];
	}
	inline BYTE *GetCodes(DWORD &count)
	{
		count=_moppdata.codes.size();
		return &_moppdata.codes[0];
	}
	inline DWORD GetBuildType()
	{
		return _moppdata.buildtype;
	}
	i_math::vector4df &GetOffset()
	{
		return _moppdata.offset;
	}
	inline virtual DWORD GetNumberOfFaces()
	{
		return _moppdata.indices.size()/3;
	}
	virtual BOOL HitTest(const i_math::line3df & rayHit,DWORD &iFace,float &dist);
	virtual BOOL GetFace(int idx,i_math::triangle3df & tri);
protected:
	void _Clean();
	virtual BOOL _OnTouch(IRenderSystem *pRS);
	virtual void _OnUnload();


	MoppData _moppdata;
};




class CMoppMgr :public IMoppMgr,public CResourceMgr
{
public:
	RESOURCEMGR_CORE();
	CMoppMgr(void);
	~CMoppMgr(void);
	virtual IResource * ObtainRes(const char * path);
	virtual BOOL ReloadRes(const char *path);

protected:

};
