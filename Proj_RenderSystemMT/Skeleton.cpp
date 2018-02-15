/********************************************************************
	created:	2006/8/4   16:58
	filename: 	e:\IxEngine\Proj_RenderSystem\SkeletonMgr.cpp
	author:		cxi
	
	purpose:	ISkeleton & ISkeletonMgr implement 
*********************************************************************/
#include "stdh.h"

#pragma warning(disable:4018)


#include "Skeleton.h"
#include <assert.h>

#include "stringparser/stringparser.h"

#include "Matrice43.h"


//////////////////////////////////////////////////////////////////////////
//CSkeleton
CSkeleton::CSkeleton()
{
	_pRS=NULL;
	_defmats=NULL;
}

DWORD CSkeleton::GetBoneCount()
{
	return (DWORD)_si.size();
}

//multiply each bone's local matrix through the skeleton hierachy,and make the final matrix for them
//xfm is the base(start) matrix
BOOL FinalizeBones(matrix43f *bones,SkeletonInfo &si,matrix43f &xfm)
{
	int i;
	for (i=0;i<si.size();i++)
	{
		if (si[i].iParent==-1)
			bones[i]*=xfm;
		else
		{
			assert(si[i].iParent<i);
			bones[i]*=bones[si[i].iParent];
		}
	}
	for (i=0;i<si.size();i++)
		bones[i]=si[i].matOff*bones[i];

	return TRUE;
}

// IMatrice43* CSkeleton::GetDefaultMatrice()
// {
// 	if (_defmats)
// 		return _defmats;
// 
// 	IMatrice43 *m=_pRS->CreateMatrice43();
// 	if (!m)
// 		return NULL;
// 
// 	matrix43f *final;
// 	m->SetCount((DWORD)_si.size());
// 	final=m->QueryPtr();
// 	for (int i=0;i<_si.size();i++)
// 		final[i]=_si[i].xformDef.getMatrix();
// 
// 	matrix43f xfm;//identity
// 	FinalizeBones(final,_si,xfm);
// 
// 	_defmats=m;
// 	return _defmats;
// }

void CSkeleton::_Clean()
{
	SAFE_RELEASE(_defmats);
	_si.clear();
}



BOOL CSkeleton::GetBoneDefXform(DWORD iBone,xformf &xformDef)
{
	if (iBone>=_si.size())
		return FALSE;
	xformDef=_si[iBone].xformDef;
	return TRUE;
}
BOOL CSkeleton::GetBoneOffMat(DWORD iBone,matrix43f &matOff)
{
	if (iBone>=_si.size())
		return FALSE;
	matOff=_si[iBone].matOff;
	return TRUE;
}

BOOL CSkeleton::GetBoneParent(DWORD iBone,int &iParent)
{
	if (iBone>=_si.size())
		return FALSE;
	iParent=_si[iBone].iParent;
	return TRUE;
}

BOOL CSkeleton::CalcSkeletonMatrice(IMatrice43*mats,BoneCtrls *bcs,i_math::matrix43f *matBase)
{
	assert(_si.size()==mats->GetCount());

	i_math::matrix43f *bones=((CMatrice43*)mats)->QueryPtr();
	if (!bcs)
	{
		for (int i=0;i<_si.size();i++)
		{
			if (_si[i].iParent==-1)
			{
				if (matBase)
					bones[i]*=*matBase;
			}
			else
			{
				assert(_si[i].iParent<i);
				bones[i]*=bones[_si[i].iParent];
			}
		}
	}
	else
	{
		i_math::matrix43f matOff,matScale;
		for (int i=0;i<_si.size();i++)
		{
			if (_si[i].iParent==-1)
			{
				if (matBase)
					bones[i]*=*matBase;
			}
			else
			{
				assert(_si[i].iParent<i);
				bones[i]*=bones[_si[i].iParent];
			}
			if (bcs->bonemask.test(i))
			{
				BoneCtrl *bc=&bcs->bcs[i];
				matOff.setTranslation(bc->off.x,bc->off.y,bc->off.z);
				bones[i]=matOff*bones[i];
			}
		}

		for (int i=0;i<_si.size();i++)
		{
			if (bcs->bonemask.test(i))
			{
				BoneCtrl *bc=&bcs->bcs[i];
				matScale.setScale(bc->scale.x,bc->scale.y,bc->scale.z);
				bones[i]=matScale*bones[i];
			}
		}
	}

	return TRUE;
}

BOOL CSkeleton::CalcSkeletonXforms(i_math::xformf *xfms,DWORD c,i_math::matrix43f *matBase)
{
	static i_math::matrix43f mats[256];
	for (int i=0;i<c;i++)
		xfms[i].getMatrix(mats[i]);

	for (int i=0;i<_si.size();i++)
	{
		if (_si[i].iParent==-1)
		{
			if (matBase)
				mats[i]*=*matBase;
		}
		else
		{
			assert(_si[i].iParent<i);
			mats[i]*=mats[_si[i].iParent];
		}
	}

	for (int i=0;i<c;i++)
		xfms[i].fromMatrix(mats[i]);

	return TRUE;
}


BOOL CSkeleton::CalcSkinMatrice(IMatrice43*mats)
{
	assert(_si.size()==mats->GetCount());

	i_math::matrix43f *bones=((CMatrice43*)mats)->QueryPtr();
	for (int i=0;i<_si.size();i++)
		bones[i]=_si[i].matOff*bones[i];

	return TRUE;
}



SkeletonMatchLevel CSkeleton::FindMatch(std::vector<DWORD>&result,
											ISkeleton *sklSub0,std::vector<DWORD>*validbones)
{
	result.clear();

	if (validbones)
	{
		if (validbones->size()<=0)
			validbones=NULL;
	}

	CSkeleton *sklSub;
	sklSub=(CSkeleton *)sklSub0;
	SkeletonInfo &siSub=sklSub->_si;

	//first do some fast check
	if (!validbones)
	{
		if (sklSub==this)
			return SklMatch_Full;
		if (_si.size()<siSub.size())
			return SklMatch_None;
	}
	else
	{
		if (sklSub==this)
		{
			result=*validbones;
			return SklMatch_Partial;
		}
		if (_si.size()<validbones->size())
			return SklMatch_None;
	}

	SkeletonMatchLevel ret=SklMatch_Full;

	int cBones;
	if (validbones)
		cBones=validbones->size();
	else
		cBones=siSub.size();

	//Check for full match
	if (_si.size()==cBones)
	{
		int i;
		for (i=0;i<cBones;i++)
		{
			BoneInfo *bi;
			if (!validbones)
				bi=&siSub[i];
			else
			{
				assert((*validbones)[i]<siSub.size());
				bi=&siSub[(*validbones)[i]];
			}
			if (!StringEqualNoCase(bi->name,_si[i].name))//name doesnot match,at most SklMatch_FullTopo
				ret=SklMatch_FullTopo;
			if (bi->iParent!=_si[i].iParent)
				break;
		}
		if (i>=cBones)
			return ret;
	}

	//now check for SklMatch_Partial
	ret=SklMatch_Partial;
	result.resize(cBones);
	int i;
	for (i=0;i<cBones;i++)
	{
		BoneInfo *biSub,*bi;
		if (!validbones)
			biSub=&siSub[i];
		else
		{
			assert((*validbones)[i]<siSub.size());
			biSub=&siSub[(*validbones)[i]];
		}

		int idx=_si.FindBone(biSub->name);
		if (idx==-1)
			break;//cannot find equal-name for this bone,not match
		result[i]=idx;

		int iSubParent=biSub->iParent;
		if (validbones)
		{
			int idx;
			VEC_FIND((*validbones),iSubParent,idx);
			if (idx==-1)//this bone's parent does not exist in validbones,so take this bone as no parent
				iSubParent=-1;
		}

		bi=&_si[idx];
		if (bi->iParent==-1)
		{
			if (iSubParent!=-1)
				break;//not match
			continue;//both have no parent
		}
		if (iSubParent==-1)//when (bi->iParent!=-1)
			continue;//ok

		//they both have parent,compare the parents' name
		BoneInfo *biLast=bi,*biSubLast=biSub;
		bi=&_si[bi->iParent];
		biSub=&siSub[biSub->iParent];
		if (!StringEqualNoCase(bi->name,biSub->name))
			break;//parent name doesn't match
	}

	if (i>=cBones)
		return ret;

	//not check SklMatch_PartialTopo,by now
	//...

	ret=SklMatch_None;
	return ret;
}

