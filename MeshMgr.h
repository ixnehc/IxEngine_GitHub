#pragma once

#include "RenderSystem/IRenderSystem.h"

#include "Base.h"
#include "ResourceBase.h"

#include "math/matrix43.h"

#include "resdata/MeshData.h"



#include <vector>
#include <string>
#include <map>

class CDeviceObject;
class CMeshMgr;


class CMesh;

struct FramePtr
{
	int f1,f2;
	i_math::f32 r;//0: f1,1:f2,between 0~1,an interpolated frame between f1 and f2

	BOOL equals(FramePtr &src)		{			return (f1==src.f1)&&(f2==src.f2)&&(i_math::equals(r,src.r));		}
	BOOL isEmpty()		{			return (f1<0)||(f2<0);		}
	void setEmpty() {			f1=f2=-1;r=0;		}
	void setZero() {	f1=f2=0;r=0;	}
	void setFrame(DWORD frame)		{			f1=f2=frame;r=0.0f;		}
};



class CMesh:public IMesh,public CResource
{
public:
	DECLARE_CLASS(CMesh);
	CMesh();

	//Interfaces
	RESOURCE_CORE()
	virtual DWORD GetFrameCount();
	virtual i_math::aabbox3df &GetAABB()	{		return _aabb;	}

	virtual FeatureCode GetFeature();

	virtual IMeshSnapshot *ObtainSnapshot();

	virtual BOOL Draw(IShader *shader,i_math::matrix43f *mats,DWORD cMats,DrawMeshArg &dmg)	{		
		return _Draw(shader,mats,cMats,dmg);	//use identity matrix
	}
	virtual BOOL Draw(IShader *shader,i_math::matrix43f &mat,DrawMeshArg &dmg){
		return _Draw(shader,&mat,1,dmg);	
	}
	
	virtual DWORD GetNumberOfLod(){return _lods.size();}

	virtual float GetLodDist(DWORD idx) { if(idx>=_lods.size())return -1.0f; else return _lods[idx].dist;}

	virtual DWORD GetBoneCount()	{		return _nSkeletonBones;	}

	virtual BOOL GetBuffer(IVertexBuffer *&vb,IIndexBuffer *&ib,DWORD iLod=0)
	{
		if (!ForceTouch())
			return FALSE;
		if (iLod>=_lods.size())
			return FALSE;
		vb=_vb;
		ib=_lods[iLod].ib;
		return TRUE;
	}

    virtual DWORD GetFabricCount() override   {        return _fabrics.size();    }
    virtual MeshData::FabricData *GetFabric(DWORD idx)override;

protected:
	virtual BOOL _OnTouch(IRenderSystem *pRS);
	virtual BOOL _OnTouchHeader(IRenderSystem *pRS);
	virtual void _OnUnload();


//	struct SegInfo
//	{
//		WORD bs,bc;//bone start/count
//		WORD ps[MeshData::LodChannelMax],pc[MeshData::LodChannelMax];//primtive start/end
//	};

	//XXXXX:Mesh Head/Body
	//Header Data
	i_math::aabbox3df _aabb;
	DWORD _nWeight;//weight count for every vertex
	DWORD _flag;
	std::vector<WORD> _frames;//each frame record an index to vb's actual frame
	DWORD _nSkeletonBones;//The bone count of the skeleton this mesh is referring to,
												//if this value is 0,this mesh does not support a skeleton
	std::vector<WORD> _segbones;
	//Body Data
	IVertexBuffer *_vb;//real static vertex data 
//	IIndexBuffer *_ib;

	IVertexBuffer *_vbCache;//additional vb used when vb could not be directly used to render
	IIndexBuffer *_ibCache;
	FramePtr _fpCache;//frame ptr of the current cache,if no cache now,it's empty

	//bones that affecting this surface,should be 1 bone if vtxframes's size is more than 1
	//each DWORD records an index to the total bone array
	//this bone reference table are only used when the bones count is less than the hardware
	//capability.if too many bones for the hardware,we should use the segbones
	std::vector<WORD> _bones;

	//each segment refers to a primitive range in vb and a bone range in segbones.
	//each segment has a max limit of its bones count,such as 25. That will ensure the hw
	//shader could render a segment in a single DrP
//	std::vector<WORD> _segbones;
//	std::vector<SegInfo> _segs;	
	struct LodInfo
	{	
		std::vector<MeshData::SegInfo> segs;
		float dist;
		IIndexBuffer * ib;
		void clean(){segs.clear();dist = 50.0f;ib = NULL;}
	};
	std::vector<LodInfo> _lods;

    std::deque<MeshData::FabricData> _fabrics;

	//if a surf contains any atlases,this surf MUST have only 1 frame
	std::vector<UVAtlasInfo> _atlases;

	FramePtr _CalcFP(f32 fFrame);
	//find the actual frame in given surface,if need interpolation,make a cache internally
	BOOL _ResolveVB(f32 fFrame,int iLod,IVertexBuffer *&vb,IIndexBuffer *&ib,DWORD &iFrame);//return NULL if fail

	BOOL _Draw(IShader *shader,i_math::matrix43f *mats,DWORD cMats,DrawMeshArg &dmg);

	void _Zero();
	void _Clean();
private:

	friend class CMeshMgr;
	friend class CDynMeshMgr;
	friend class CMeshSnapshot;
};

class CMeshSnapshot:public IMeshSnapshot
{
public:
	IMPLEMENT_REFCOUNT_C;
	DEFINE_CLASS(CMeshSnapshot);
	CMeshSnapshot()
	{
		Zero();
	}
	~CMeshSnapshot()
	{
		Clean();
	}
	void Zero();
	void Clean();
	//interfaces
	virtual DWORD GetFrameCount();

	virtual BOOL TakeSnapshot(IMatrice43 *mats,MeshSnapshotArg &msa);
	virtual BOOL TakeSnapshot(i_math::matrix43f &mat,MeshSnapshotArg &msa);

	virtual DWORD GetVBCount();
	virtual DWORD GetIBCount(DWORD idx);
	virtual FVFEx GetFVF();
	virtual int FindUVAtlasChannel(DWORD start=0);
	virtual BOOL GetUVAtlasInfo(DWORD channel,UVAtlasInfo &ai);
	virtual WORD *GetIndices(DWORD idx);
	virtual void *GetVertices(FVFEx fvf,DWORD *stride);
	virtual i_math::vector3df *GetPos();
	virtual i_math::vector3df *GetNormal();
	virtual TriSample*GetMeshSamples(MeshSampleArg &arg,DWORD &nSamples);
	virtual BOOL HitTest(i_math::line3df &ray,float &dist);
	
	void Attach(CMesh *pMesh);


	struct Cache
	{
		Cache()
		{
		}
		f32 fFrame;
		IMatrice43 *matrice;
		DWORD verMatrice;

		FVFEx fvf;
		DWORD count;
		std::vector<BYTE>data;
		UVAtlasInfo ai[MAX_TEXTURE];
	};

protected:

	IVertexBuffer *_GetCurVB();

	BOOL _TakeSnapshot(matrix43f *mats,DWORD nMats,IMatrice43 *matrice,MeshSnapshotArg &msa);

	std::vector<Cache>_cache;

	int _iCurCache;

	CMesh *_pMesh;//if NULL, this snapshot is empty

	template<typename T_MeshSample,typename T_VtxType,int bTSI>
	T_MeshSample *_GetMeshSamples(MeshSampleArg &arg,DWORD &nSamples);


	friend class CMesh;
};


class CDeviceObject;
class CMeshMgr:public IMeshMgr,public CResourceMgr
{
public:
	CMeshMgr();

	RESOURCEMGR_CORE
	IResource *ObtainRes(const char *path);
	virtual BOOL ReloadRes(const char *path);

protected:
	//Overidables
friend class CMesh;

};

