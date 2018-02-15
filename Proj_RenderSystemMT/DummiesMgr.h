#pragma once
#include "RenderSystem/IDummies.h"
#include "ResourceBase.h"
#include <hash_map>
#include <vector>
#include "resdata/ResData.h"
#include "resdata/MeshData.h"
#include "class/class.h"


class CDummies:public CResource,public IDummies
{
	friend class CDummiesMgr;
	friend class CDynDummiesMgr;
public:

	DECLARE_CLASS(CDummies);


	CDummies();
	virtual  ~CDummies(void);
	virtual DWORD  GetCount();
	const char * GetDummyName(DWORD idx); 
	DummyInfo * GetDummyInfo(DWORD &count);

	virtual int FindDummy(const char *name);

	virtual BOOL CalcMat(DWORD iDummy,i_math::matrix43f &matBase,i_math::matrix43f &mat);
	virtual BOOL CalcMat(const char *nameDummy,i_math::matrix43f &matBase,i_math::matrix43f &mat);
	virtual BOOL CalcMatAtSkeleton(DWORD iDummy,IMatrice43 *sklmats,i_math::matrix43f &mat);
	virtual BOOL CalcMatAtSkeleton(const char *nameDummy,IMatrice43 *sklmats,i_math::matrix43f &mat);
	virtual BOOL CalcMatAtBones(DWORD iDummy,IMatrice43 *bonemats,i_math::matrix43f &mat);
	virtual BOOL CalcMatAtBones(const char *nameDummy,IMatrice43 *bonemats,i_math::matrix43f &mat);

	virtual i_math::aabbox3df &GetDefAabb()	{		return _aabbDef;	}
	virtual BOOL CalcAabb(i_math::aabbox3df &aabb,IMatrice43 *sklmats,i_math::matrix43f *mat);
	virtual BOOL HitTest(i_math::vector3df &posRet,i_math::line3df &line,float radius,IMatrice43 *sklmats,i_math::matrix43f *matBase);

	RESOURCE_CORE();

protected:	
	virtual BOOL _AddDummy(DummyInfo &dummy,const char * name);
	virtual void _Clean();
	
	virtual BOOL _OnTouch(IRenderSystem *pRS)	;
	virtual void _OnUnload();

	BOOL _FillBody(CDummies * pDummies,ResData * pRes);
    
protected:
	std::hash_map<std::string,WORD>  _mpNameIdx; //name<->index   mapping
	std::vector<DummyInfo> _vecDummies;
	std::vector<short>_boneindices;//记录这个dummies依赖的skeleton中的每根bone的parent的索引
	i_math::aabbox3df _aabbDef;

	BOOL _bInit;
};




class CDummiesMgr :public IDummiesMgr,public CResourceMgr
{
public:
	RESOURCEMGR_CORE();
	virtual IResource * ObtainRes(const char * path);
	virtual BOOL ReloadRes(const char *path);

protected:

};

class CDynDummiesMgr:public IDynDummiesMgr,public CResourceMgr
{
public:
	RESOURCEMGR_CORE();
	virtual IResource * ObtainRes(const char * path){return NULL;}
	virtual IDummies * Create(const DummiesData * dummies);
};
