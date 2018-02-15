#pragma once

#include "RenderSystem/ITools.h"

// #include "Base.h"
#include "ResourceBase.h"

#include "resdata/MeshData.h"


#include "class/class.h"



class CDeviceObject;


class CSkeleton:public ISkeleton
{
protected:

	SkeletonInfo _si;
	IMatrice43 *_defmats;
	IRenderSystem *_pRS;
public:
	DEFINE_CLASS(CSkeleton)
	IMPLEMENT_REFCOUNT_C;
	CSkeleton();

	//interfaces
	DWORD GetBoneCount();
	BOOL GetBoneDefXform(DWORD iBone,xformf &xformDef);
	BOOL GetBoneOffMat(DWORD iBone,matrix43f &matOff);
	BOOL GetBoneParent(DWORD iBone,int &iParent);
	SkeletonMatchLevel FindMatch(std::vector<DWORD>&resultindice,
		ISkeleton *sklSub,std::vector<DWORD>*validbones);//By now not support SklMatch_PartialTopo
	SkeletonInfo*GetSkeletonInfo()	{		return &_si;	}

	BOOL CalcSkeletonMatrice(IMatrice43*mats,BoneCtrls *bcs,i_math::matrix43f *matBase);
	BOOL CalcSkeletonXforms(i_math::xformf *xfms,DWORD c,i_math::matrix43f *matBase);
	BOOL CalcSkinMatrice(IMatrice43*mats);


protected:
	void _Clean();
	BOOL _FinalizeBones(matrix43f *bonesFinal,matrix43f &xfm,matrix43f *bones);

	friend class CRenderSystem;
	friend class CRenderSystemGL;
};

