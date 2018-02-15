#pragma once

#include "RenderSystem/ISpeedTree.h"

#include "Base.h"
#include "ResourceBase.h"


#include "resdata/SptData.h"

#include "shaderlib/SLDefines.h"

#include "SptTriSampAdapter.h"

#include <vector>
#include <string>
#include <map>

#include "class/class.h"

class CDeviceObject;
class CSptMgr;
class CDynSptMgr;

BOOL FillResource(IVertexMgr * pVertexMgr,ITextureMgr * pTexMgr,IDynDummiesMgr * pDummiesMgr, CResource * pRes,ResData * resdata);

struct SptVtxInfo
{
	DEFINE_CLASS(SptVtxInfo);
	
	std::vector<i_math::vector3df> brvtxs;
	std::vector<i_math::vector3df> brnors;
	std::vector<i_math::vector3df> brtans;
	std::vector<i_math::vector3df> brbinors;
	std::vector<i_math::vector2df> bruvs;
	std::vector<WORD> brIndices;

	std::vector<i_math::vector3df> frvtxs;
	std::vector<i_math::vector3df> frnors;
	std::vector<i_math::vector3df> frtans;
	std::vector<i_math::vector3df> frbinors;
	std::vector<i_math::vector2df> fruvs;
	std::vector<WORD> frIndices;

	//模型类树叶
	std::vector<i_math::vector3df> lmvtx;
	std::vector<WORD> lmindices;;
	std::vector<i_math::vector3df> lmhookpos;
	std::vector<i_math::matrix43f> lminstmat; 
	
	//树叶
	std::vector<i_math::vector3df> leavevtxs; //叶子的挂接点
	float radius;							  //叶子的半径

	void SetBrVtxCount(DWORD count){
		brvtxs.resize(count);
		brnors.resize(count);
		brtans.resize(count);
		brbinors.resize(count);
		bruvs.resize(count);
	}
	void SetFrVtxCount(DWORD count){
		frvtxs.resize(count);
		frnors.resize(count);
		frtans.resize(count);
		frbinors.resize(count);
		fruvs.resize(count);
	}
};

//注意:CSpt里不需要CSpeedTreeRT的类的支持
class CSpt:public ISpt,public CResource
{
public:
	DECLARE_CLASS(CSpt);

	CSpt();
	~CSpt();

	void Zero();
	void Clean();


	virtual BOOL _OnTouch(IRenderSystem *pRS)	;
	virtual void _OnUnload();


	RESOURCE_CORE()
	virtual SptMaps &GetMaps()
	{
		return _maps;
	}

	//注意:fDist单位为米
	virtual SptLod &GetLodByDistance(float fDist);
	
	virtual SptLod &GetLod(DWORD iLod);

	virtual DWORD GetNumberOfLod();

	virtual DWORD GetWindCount()
	{
		return 0;
	}
	virtual SptWndCfg &GetWind(DWORD idx)
	{
		return *(SptWndCfg*)NULL;
	}
	
	virtual i_math::aabbox3df & GetBoundingBox();
	
	virtual int  GetNumberOfCollisionObjects();
	
	virtual void * GetCollisionObject(int idx ,CollisionObjectType & coType);
	
	virtual BOOL TestCollision(const i_math::line3df & rayHit,i_math::vector3df &pos,float scale,float rotY,
							   i_math::vector3df &outIntersec,SptColType cTpye = SPTCT_ALL);
	
	virtual const i_math::spheref &GetLeafBoundSphere(){return _sphleaf;}

	virtual IDummies * GetCollisionDummies();
	
	virtual void GetMapSize(i_math::size2di &szBr,i_math::size2di &szFr,int lvl);
	
	virtual ISptTriSampleAdapter * GetTriSampleAdapter();
	
	virtual float GetMapScale(int lvl) const;
		
	//得到Branch的信息
	i_math::vector3df * GetBrVertexs(){ _BuildVtxInfo(); return &(_infoVtx->brvtxs[0]);}
	i_math::vector3df * GetBrNormals(){ _BuildVtxInfo(); return &(_infoVtx->brnors[0]);}
	i_math::vector3df * GetBrBiNormals(){ _BuildVtxInfo(); return &(_infoVtx->brbinors[0]);}
	i_math::vector3df * GetBrTangents(){ _BuildVtxInfo(); return &(_infoVtx->brtans[0]);}
	i_math::vector2df * GetBrLightMapUV(){ _BuildVtxInfo(); return &(_infoVtx->bruvs[0]);}
	WORD * GetBrIndices(){ _BuildVtxInfo(); return &(_infoVtx->brIndices[0]);}
	DWORD GetNumberOfVtxBr(){_BuildVtxInfo();return _infoVtx->brvtxs.size();}
	DWORD GetNumberOfIndicesBr(){_BuildVtxInfo();return _infoVtx->brIndices.size();}

	//得到Frond的信息
	i_math::vector3df * GetFrVertexs(){ _BuildVtxInfo(); return &(_infoVtx->frvtxs[0]);}
	i_math::vector3df * GetFrNormals(){ _BuildVtxInfo(); return &(_infoVtx->frnors[0]);}
	i_math::vector3df * GetFrBiNormals(){ _BuildVtxInfo(); return &(_infoVtx->frbinors[0]);}
	i_math::vector3df * GetFrTangents(){ _BuildVtxInfo(); return &(_infoVtx->frtans[0]);}
	i_math::vector2df * GetFrLightMapUV(){ _BuildVtxInfo(); return &(_infoVtx->fruvs[0]);}
	WORD * GetFrIndices(){ _BuildVtxInfo(); return &(_infoVtx->frIndices[0]);}
	DWORD GetNumberOfVtxFr(){_BuildVtxInfo();return _infoVtx->frvtxs.size();}
	DWORD GetNumberOfIndicesFr(){_BuildVtxInfo();return _infoVtx->frIndices.size();}

	//得到叶子的信息
	virtual void GetLeafHookPoint(i_math::vector3df *& pos,float& r,DWORD &count){
		_BuildVtxInfo(); pos = &(_infoVtx->leavevtxs[0]); 
		r = _infoVtx->radius;
		count = _infoVtx->leavevtxs.size();
	}

protected:
	void _CreateDummies(IDynDummiesMgr * mgr);
	void _BuildVtxInfo();
	void _GetVtxInfoFromVB(IVertexBuffer * vb,i_math::vector3df *pos,
						  i_math::vector3df * nor,i_math::vector3df * binor,
						  i_math::vector3df * tan,i_math::vector2df * uv);
	void _GetIndexFromIB(IIndexBuffer * ib,i_math::vector3df * pos,std::vector<WORD> &vecIB);
	void _GetLeafCenterVtxs();
	void _GetLeafMesh();

	friend class CSptMgr;

private:
	
	SptMaps _maps;
	SptLod  _sptLod;

	float fTransitionPrecent[MAX_LOD_LEVEL];
	float fTransitionDist[MAX_LOD_LEVEL];
	float fSlide;

	int   nLods;
	int   nLodBranch;
	int   nLodFrond;
	int   nLodLeafCard;
	int   nLodLeafMesh;

	std::vector<SptWndCfg> _winds;

    IVertexBuffer * _branchVB;
	IVertexBuffer * _frondVB;
	IVertexBuffer * _leafCardVBs;
	IIndexBuffer  * _leafIB;
	
	IVertexBuffer * _leafMeshVB;
	IIndexBuffer  * _leafMeshIB;
	int _leafMeshPrimStart[MAX_LOD_LEVEL];
	int _leafMeshNumPrim[MAX_LOD_LEVEL];

	int vBaseLeaf[MAX_LOD_LEVEL];
	int nPrimLeaf[MAX_LOD_LEVEL];

	int vbase[MAX_LOD_LEVEL]; 
	IVertexBuffer * _leafMeshVBs[MAX_LOD_LEVEL];

	IIndexBuffer  * _branchIBs[MAX_LOD_LEVEL];
	IIndexBuffer  * _frondIBs[MAX_LOD_LEVEL];
	
	IDummies * _dummies;

	i_math::aabbox3df _aabb;

	// collision objects.
	std::vector<i_math::capsulef> _caps;
	std::vector<i_math::aabbox3df> _obs;
	std::vector<i_math::spheref> _sphs;
	i_math::spheref _sphleaf;
	
	LMSize _mapSizeCalc;

	SptVtxInfo *_infoVtx;
	
	// tool
	static CSptTriSampAdapter _triSampleAdapter;
	friend BOOL FillResource(IVertexMgr * pVertexMgr,ITextureMgr * pTexMgr,IDynDummiesMgr * pDummiesMgr, CResource * pRes,ResData * resdata);
};

class CSptMgr:public ISptMgr,public CResourceMgr
{
public:
	CSptMgr();

	//interfaces
	RESOURCEMGR_CORE
	virtual IResource *ObtainRes(const char *pathRes);
	virtual BOOL ReloadRes(const char *path);
};

struct SptData;

class CDynSptMgr:public IDynSptMgr,public CResourceMgr
{
public:

	//interfaces
	RESOURCEMGR_CORE
	virtual IResource *ObtainRes(const char *pathRes)	{		return NULL;	}
	ISpt *Create(SptData *data,const char * pathRes);

};




