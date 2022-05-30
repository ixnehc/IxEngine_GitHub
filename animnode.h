#pragma once

#include "class/class.h"
#include "RenderSystem/IAnim.h"
#include "WorldSystem/IAssetSystemDefines.h"

#include "WorldSystem/IAnimNodes.h"

#include "timer/profiler.h"


#define MARK_INVALID_CACHE_T(__tCache,__t) (__tCache)=(__t);(__tCache)|=0x80000000;//把这个时候的cache标记为无效的
#define CHECK_INVALID_CACHE_T(__tCache,__t) ((__tCache&0x80000000)&&((__tCache|0x7fffffff)==__t))




class CAnimNodeMatFixed:public IAnimNodeMatFixed
{
public:
	IMPLEMENT_REFCOUNT_C
	DEFINE_CLASS(CAnimNodeMatFixed);

	virtual void Set(i_math::matrix43f &mat)	{		_matCache=mat;	}

	virtual i_math::matrix43f*GetMat(AnimTick t)	
	{		
		return &_matCache;	
	}
	virtual BOOL IsFixed()	{		return TRUE;	}

protected:
	i_math::matrix43f _matCache;
};


class CAnimNodeMatOffset:public IAnimNodeMatOffset
{
public:
	IMPLEMENT_REFCOUNT_C
	DEFINE_CLASS(CAnimNodeMatOffset);

	CAnimNodeMatOffset()
	{
		_tCache=ANIMTICK_INFINITE;
	}
	~CAnimNodeMatOffset()
	{
	}

	virtual void SetBase(IAnimNode *an)
	{
		if (!an)
			return;
		SAFE_REPLACE(_base,an);
	}
	virtual void SetOffset(i_math::matrix43f &mat)	
	{		
		_matOff=mat;	
	}

	virtual void SetFixed(i_math::matrix43f &mat) 
	{
		SAFE_RELEASE(_base);
		_cache=mat;
	}
	virtual i_math::matrix43f*GetMat(AnimTick t)	;

protected:

	AnimTick _tCache;

	i_math::matrix43f _matOff;
	i_math::matrix43f _cache;
};


template<typename T,typename TRef,typename T_Cache,typename T_Interface>
class CAnimNodeSimpleLerp:public T_Interface
{
public:
	IMPLEMENT_REFCOUNT_C
	CAnimNodeSimpleLerp()
	{
		_iCur=0;
		_t[0]=_t[1]=0;
		_tCache=ANIMTICK_INFINITE;
	}

	virtual void Reset(TRef v,AnimTick t)
	{
		_keys[0]=v;
		_keys[1]=v;
		_t[0]=t;
		_t[1]=t;
		_iCur=0;
		_tCache=ANIMTICK_INFINITE;
	}
	virtual void Add(TRef v,AnimTick t)
	{
		_iCur=1-_iCur;
		_keys[1-_iCur]=v;
		_t[1-_iCur]=t;
	}
	virtual AnimTick GetFirstT()	{		return _t[_iCur];	}
	virtual AnimTick GetSecondT()	{		return _t[1-_iCur];	}

	virtual TRef GetFirst()	{		return _keys[_iCur];	}
	virtual TRef GetSecond()	{		return _keys[1-_iCur];	}

	virtual BOOL IsFixed()	{		return FALSE;	}


protected:
	void _CalcLerpInfo(AnimTick t,int&iEdge,float &r)
	{
		iEdge=-1;
		if (t<=_t[_iCur])
			iEdge=_iCur;
		else
		{
			if (t>=_t[1-_iCur])
				iEdge=1-_iCur;
		}

		r=0.0f;
		if (iEdge==-1)
			r=((float)(t-_t[_iCur]))/(float)(_t[1-_iCur]-_t[_iCur]);
	}
	T _keys[2];
	AnimTick _t[2];
	DWORD _iCur;

	AnimTick _tCache;
	T_Cache _cache;
};



class CAnimNodePosEuler:public CAnimNodeSimpleLerp<PosEuler,PosEuler&,i_math::matrix43f,IAnimNodePosEuler>
{
public:
	DEFINE_CLASS(CAnimNodePosEuler);
	virtual i_math::matrix43f*GetMat(AnimTick t)
	{
		if (t==_tCache)
			return &_cache;

		int iEdge;
		float r;
		_CalcLerpInfo(t,iEdge,r);

		i_math::xformf xfm;

		i_math::vector3df elr;
		if (iEdge==-1)
		{
			elr.x=i_math::lerp_angle(_keys[_iCur].euler.x,_keys[1-_iCur].euler.x,r);
			elr.y=i_math::lerp_angle(_keys[_iCur].euler.y,_keys[1-_iCur].euler.y,r);
			elr.z=i_math::lerp_angle(_keys[_iCur].euler.z,_keys[1-_iCur].euler.z,r);

			xfm.pos=_keys[1-_iCur].pos.getInterpolated(_keys[_iCur].pos,r);
		}
		else
		{
			elr=_keys[iEdge].euler;
			xfm.pos=_keys[iEdge].pos;
		}

		//XXXXX:euler-model offset
// 		elr.x-=i_math::Pi;
// 		elr.y=-elr.y;

		xfm.rot.fromEuler(elr);
		xfm.getMatrix(_cache);
		_tCache=t;

		return &_cache;
	}

};

class CAnimNodeEulerOverride:public CAnimNodeSimpleLerp<i_math::vector3df,i_math::vector3df&,i_math::matrix43f,IAnimNodeEulerOverride>
{
public:
	DEFINE_CLASS(CAnimNodeEulerOverride);
	virtual i_math::vector3df*GetPos(AnimTick t)
	{
		return __super::GetPos(t);
	}

	virtual i_math::matrix43f*GetMat(AnimTick t)
	{
		if (t==_tCache)
			return &_cache;

		int iEdge;
		float r;
		_CalcLerpInfo(t,iEdge,r);

		i_math::xformf xfm;

		i_math::vector3df elr;
		if (iEdge==-1)
		{
			elr.x=i_math::lerp_angle(_keys[_iCur].x,_keys[1-_iCur].x,r);
			elr.y=i_math::lerp_angle(_keys[_iCur].y,_keys[1-_iCur].y,r);
			elr.z=i_math::lerp_angle(_keys[_iCur].z,_keys[1-_iCur].z,r);

		}
		else
			elr=_keys[iEdge];

		i_math::matrix43f *matBase=NULL;
		if (_base)
			matBase=_base->GetMat(t);
		if (!matBase)
			matBase=i_math::matrix43f::identity();

		xfm.rot.fromEuler(elr);
		xfm.pos=matBase->getTranslation();
		xfm.getMatrix(_cache);
		_tCache=t;

		return &_cache;
	}

};




class CAnimNodeMat:public CAnimNodeSimpleLerp<i_math::xformf,i_math::xformf&,i_math::matrix43f,IAnimNodeMat>
{
public:
	DEFINE_CLASS(CAnimNodeMat);
	virtual i_math::matrix43f *GetMat(AnimTick t)
	{
		if (t==_tCache)
			return &_cache;

		int iEdge;
		float r;
		_CalcLerpInfo(t,iEdge,r);

		if (iEdge==-1)
		{
			i_math::xformf xfm;
			xfm=_keys[1-_iCur].getInterpolated(_keys[_iCur],r);
			xfm.getMatrix(_cache);
		}
		else
			_keys[iEdge].getMatrix(_cache);
		_tCache=t;

		return &_cache;
	}
};

class CAnimNodeDtr:public IAnimNodeDtr
{
public:
	DEFINE_CLASS(CAnimNodeDtr);

	IMPLEMENT_REFCOUNT_C
	CAnimNodeDtr()
	{
		Zero();
	}
	~CAnimNodeDtr()
	{
		Clear();
	}

	void Zero()
	{
		_iCur=0;
		_t[0]=_t[1]=0;
		_tCache=ANIMTICK_INFINITE;
		_cache=NULL;
	}

	BOOL Init(AssetSystemState*ss);
	void Clear();

	virtual void Reset(i_math::xformf *xfms,DWORD count,i_math::matrix43f &matBase,AnimTick t);
	virtual i_math::xformf *Add(AnimTick t)
	{
		_iCur=1-_iCur;
		_t[1-_iCur]=t;
		return &_keys[1-_iCur][0];
	}

	virtual AnimTick GetFirstT()	{		return _t[_iCur];	}
	virtual AnimTick GetSecondT()	{		return _t[1-_iCur];	}

	virtual i_math::xformf *GetFirst()	{		return &_keys[_iCur][0];	}
	virtual i_math::xformf *GetSecond()	{		return &_keys[1-_iCur][0];	}

	virtual IMatrice43 *GetSkinMats(AnimTick t);
	virtual BOOL IsSkl()	{		return TRUE;	}

	virtual i_math::matrix43f *GetMat(AnimTick t)	{		return &_matBase;	}
	virtual BOOL IsFixed()	{		return FALSE;	}

protected:
	void _CalcLerpInfo(AnimTick t,int&iEdge,float &r)
	{
		iEdge=-1;
		if (t<=_t[_iCur])
			iEdge=_iCur;
		else
		{
			if (t>=_t[1-_iCur])
				iEdge=1-_iCur;
		}

		r=0.0f;
		if (iEdge==-1)
			r=((float)(t-_t[_iCur]))/(float)(_t[1-_iCur]-_t[_iCur]);
	}
	std::vector<i_math::xformf> _keys[2];
	AnimTick _t[2];
	DWORD _iCur;

	AnimTick _tCache;
	IMatrice43 *_cache;
	i_math::matrix43f _matBase;//开始destruct时的位置

};


class CAnimNodeProxy:public IAnimNodeProxy
{
public:
	IMPLEMENT_REFCOUNT_C
	DEFINE_CLASS(CAnimNodeProxy);

};

//Site的机制:
//Site由一个base和一个offset,一共两个offset矩阵组成(offset矩阵又称为local mat)
//Base可以是一个anim node或者是一个fixed的矩阵(如果是fixed矩阵,会在内部创建一个CAnimNodeMatFixed,但它不会被暴露出去)
//_bAttaching用来指示base到底是一个anim node还是fixed矩阵
class CAnimNodeSite:public IAnimNodeSite
{
public:
	IMPLEMENT_REFCOUNT_C
	DEFINE_CLASS(CAnimNodeSite);

	CAnimNodeSite()
	{
		_bAttaching=0;
		_bForceAttach=0;
		_matLocal=NULL;
	}

	virtual void SetFixed(i_math::matrix43f &mat)
	{
		SAFE_RELEASE(_base);
		_base=Class_New2(CAnimNodeMatFixed);
		_base->AddRef();
		((CAnimNodeMatFixed*)_base)->Set(mat);
		_bAttaching=0;
	}
	virtual void SetBase(IAnimNode *an,AnimTick t)
	{
		if (an)
		{
			SAFE_REPLACE(_base,an);
			_bAttaching=1;
		}
		else
		{
			if (_bAttaching)
			{
				i_math::matrix43f *mat=GetMat(t);
				if (!mat)
					mat=i_math::matrix43f::identity();
				SetFixed(*mat);
			}
		}
	}
	virtual IAnimNode *GetBase()
	{
		if (_bAttaching)
			return _base;
		return NULL;
	};
	virtual void SetForceAttach(BOOL bForceAttach)//所谓Force Attach是指:如果没有Attach某个anim node上,则这个anim node不工作(GetMat()返回NULL)
	{
		_bForceAttach=bForceAttach?1:0;
	}
	virtual BOOL GetForceAttach()
	{
		return _bForceAttach==1?TRUE:FALSE;
	}
	virtual BOOL IsAttaching()	{	return _bAttaching!=0;	}

	virtual i_math::matrix43f *GetLocalMat()	{		return _matLocal;	}
	virtual void SetLocalMat(i_math::matrix43f *matLocal)	{		_matLocal=matLocal;	}


	virtual i_math::matrix43f*GetMat(AnimTick t)	
	{
		if ((_bForceAttach)&&(!_bAttaching))
			return NULL;
		if(_base)
		{
			i_math::matrix43f *mat=_base->GetMat(t);
			if (_bAttaching)
			{
				if (_matLocal&&mat)
				{
					_cache=(*_matLocal)*(*mat);
					return &_cache;
				}
			}
			return mat;
		}
		return NULL;
	}
	virtual IMatrice43*GetSklMats(AnimTick t)
	{
		if ((_bForceAttach)&&(!_bAttaching))
			return NULL;
		if(_base)
			return _base->GetSklMats(t);
		return NULL;
	}
	virtual IMatrice43*GetSkinMats(AnimTick t)
	{
		if ((_bForceAttach)&&(!_bAttaching))
			return NULL;
		if(_base)
			return _base->GetSkinMats(t);
		return NULL;
	}


	virtual BOOL IsFixed()
	{
		if (_base)
			return _base->IsFixed();
		return FALSE;
	}

	virtual BOOL GetBaseMat(AnimTick t,i_math::matrix43f &mat0)
	{
		if (!_base)
			return FALSE;
		i_math::matrix43f *mat=_base->GetMat(t);
		if (!mat)
			return FALSE;
		if (_bAttaching)
			mat0=*mat;
		else
			mat0.makeIdentity();
		return TRUE;
	}



protected:
	DWORD _bAttaching:1;
	DWORD _bForceAttach:1;

	i_math::matrix43f _cache;

	i_math::matrix43f *_matLocal;
};

class CAnimNodeNoRoll:public IAnimNodeNoRoll
{
public:
	IMPLEMENT_REFCOUNT_C
	DEFINE_CLASS(CAnimNodeNoRoll);

	CAnimNodeNoRoll()
	{
		Zero();
	}
	~CAnimNodeNoRoll()
	{
		Zero();
	}

	void Zero()
	{
		_tCache=ANIMTICK_INFINITE;
	}


	virtual void SetBase(IAnimNode *an)
	{
		SAFE_REPLACE(_base,an);
		_tCache=ANIMTICK_INFINITE;
	}
	virtual i_math::matrix43f *GetMat(AnimTick t);
	virtual BOOL IsFixed()
	{
		if (_base)
			return _base->IsFixed();
		return FALSE;
	}

protected:
	i_math::matrix43f _cache;
	AnimTick _tCache;
};

class CAnimNodeSkeleton:public IAnimNodeSkeleton
{
public:
	IMPLEMENT_REFCOUNT_C
	DEFINE_CLASS(CAnimNodeSkeleton);
	CAnimNodeSkeleton()
	{
		Zero();
	}
	~CAnimNodeSkeleton()
	{
		Clear();
	}

	void Zero()
	{
		_ctrl=NULL;
		_matsCache=NULL;
		_tCache=ANIMTICK_INFINITE;
		_tCache2=ANIMTICK_INFINITE;

		_dummies=NULL;
		_iDummy=-1;

		_bLocalAabb=FALSE;
	}

	BOOL Init(AssetSystemState*ss);
	void Clear();

	virtual void SetAnimTreeCtrl(IAnimTreeCtrl *ctrl);

	virtual void SetBase(IAnimNode *an);
	virtual void SetBaseLink(IDummies *dummies,const char *name);//设定骨骼上的一个dummy 位点连接在base上
	virtual void SetBaseLink(IDummies *dummies,DWORD iDummy);
	virtual IMatrice43*GetSklMats(AnimTick t);
	virtual BOOL IsSkl()	{		return _ctrl?TRUE:FALSE;	}
	//计算一个矩阵,把局部空间的点转换到世界空间的点
	//举个例子子,比如说现在有一个骨骼系统,这个骨骼系统的原点在它的脚上,现在有一点A,在它的鞋子上,它的坐标为
	//(0.1,0,0),离原点很近吧,现在这个骨骼系统被绑在另一个anim node上（比如说一个巨人的手上)了,并且以腰上的一个dummy
	//作为base link(巨人抓着这个骨骼系统的腰),巨人将抓着的骨骼系统挥来挥去,鞋子上的点A的位置也在变来变去,现在假设某一个
	//时刻t,A点的世界空间位置为(10,120,10),那我们所求的矩阵就是把(0.1,0,0)这个点变换到(10,120,10)的一个转换矩阵
	virtual BOOL CalcBaseLinkMat(AnimTick t,i_math::matrix43f &mat);
	virtual BOOL CheckBaseLink(IDummies *dummies,const char *name);//检查当前是不是指定的base link

	virtual void SetLocalAabb(i_math::aabbox3df &aabb)
	{
		_bLocalAabb=!aabb.isInvalid();
		if (_bLocalAabb)
			_aabbLocal=aabb;
		_tCache2=ANIMTICK_INFINITE;
	}
	virtual i_math::aabbox3df *GetAabb(AnimTick t)
	{
		if (!_bLocalAabb)
			return _base?_base->GetAabb(t):NULL;
		if (_tCache2==t)
			return &_aabbCache;

		i_math::matrix43f *mat=NULL;
		if (_base)
			mat=_base->GetMat(t);

		_aabbCache=_aabbLocal;
		if (mat)
			mat->transformBoxEx(_aabbCache);
		_tCache2=t;
		return &_aabbCache;
	}
	virtual IAnimTreeCtrl*GetAnimTreeCtrl() override	{		return _ctrl;	}
	virtual BOOL IsFixed()	{		return FALSE;	}

protected:

	virtual i_math::matrix43f *_CalcBaseLinkMat(AnimTick t,i_math::matrix43f *mat);
	
	IAnimTreeCtrl *_ctrl;

	//base link信息
	IDummies *_dummies;
	int _iDummy;

	IMatrice43 *_matsCache;
	AnimTick _tCache;

	BOOL _bLocalAabb;
	i_math::aabbox3df _aabbLocal;
	i_math::aabbox3df _aabbCache;
	AnimTick _tCache2;

};

class CAnimNodeMatrice43:public IAnimNodeMatrice43
{
public:
	IMPLEMENT_REFCOUNT_C
	DEFINE_CLASS(CAnimNodeMatrice43);
	CAnimNodeMatrice43()
	{
		Zero();
	}
	~CAnimNodeMatrice43()
	{
		Clear();
	}

	void Zero()
	{
		_iCur=0;
		_t[0]=_t[1]=0;
		_tCache=ANIMTICK_INFINITE;
		_cache=NULL;
		_skl=NULL;
	}
	void Clear();

	BOOL Init(AssetSystemState*ss);

	virtual void SetSkeleton(ISkeleton *skl);

	virtual void Reset(i_math::xformf *xfms,DWORD count,AnimTick t);
	virtual i_math::xformf *Add(AnimTick t)
	{
		_iCur=1-_iCur;
		_t[1-_iCur]=t;
		return &_keys[1-_iCur][0];
	}

	virtual AnimTick GetFirstT()	{		return _t[_iCur];	}
	virtual AnimTick GetSecondT()	{		return _t[1-_iCur];	}

	virtual i_math::xformf *GetFirst()	{		return &_keys[_iCur][0];	}
	virtual i_math::xformf *GetSecond()	{		return &_keys[1-_iCur][0];	}

	virtual IMatrice43 *GetSklMats(AnimTick t);
	virtual BOOL IsSkl()	{		return TRUE;	}

	virtual BOOL IsFixed()	{		return FALSE;	}


protected:
	void _CalcLerpInfo(AnimTick t,int&iEdge,float &r)
	{
		iEdge=-1;
		if (t<=_t[_iCur])
			iEdge=_iCur;
		else
		{
			if (t>=_t[1-_iCur])
				iEdge=1-_iCur;
		}

		r=0.0f;
		if (iEdge==-1)
			r=((float)(t-_t[_iCur]))/(float)(_t[1-_iCur]-_t[_iCur]);
	}

	ISkeleton *_skl;
		 

	std::vector<i_math::xformf> _keys[2];
	AnimTick _t[2];
	DWORD _iCur;

	AnimTick _tCache;
	IMatrice43 *_cache;
};



class CAnimNodeRagdoll:public IAnimNodeRagdoll
{
public:
	IMPLEMENT_REFCOUNT_C
	DEFINE_CLASS(CAnimNodeRagdoll);
	CAnimNodeRagdoll()
	{
		Zero();
	}
	~CAnimNodeRagdoll()
	{
		Clear();
	}

	void Zero()
	{
		_ctrl=NULL;
		_matsCache=NULL;
		_tCache=ANIMTICK_INFINITE;
		_tCache2=ANIMTICK_INFINITE;
		_tCache3=ANIMTICK_INFINITE;

		_bLocalAabb=FALSE;
	}

	BOOL Init(AssetSystemState*ss);
	void Clear();

	virtual void SetRagdollCtrl(IRagdollCtrl*ctrl);

	virtual i_math::matrix43f *GetMat(AnimTick t);
	virtual IMatrice43*GetSklMats(AnimTick t);
	virtual BOOL IsSkl()	{		return _ctrl?TRUE:FALSE;	}

	virtual void SetLocalAabb(i_math::aabbox3df &aabb)
	{
		_bLocalAabb=!aabb.isInvalid();
		if (_bLocalAabb)
			_aabbLocal=aabb;
		_tCache2=ANIMTICK_INFINITE;
	}
	virtual i_math::aabbox3df *GetAabb(AnimTick t)
	{
		if (!_bLocalAabb)
			return NULL;
		if (_tCache2==t)
			return &_aabbCache;

		i_math::matrix43f *mat=GetMat(t);

		_aabbCache=_aabbLocal;
		if (mat)
			mat->transformBoxEx(_aabbCache);
		_tCache2=t;
		return &_aabbCache;
	}
	virtual IRagdollCtrl*GetRagdollCtrlCtrl()	{		return _ctrl;	}
	virtual BOOL IsFixed()	{		return FALSE;	}

protected:

	IRagdollCtrl*_ctrl;

	IMatrice43 *_matsCache;
	AnimTick _tCache;

	BOOL _bLocalAabb;
	i_math::aabbox3df _aabbLocal;
	i_math::aabbox3df _aabbCache;
	AnimTick _tCache2;

	i_math::matrix43f _matCache;
	AnimTick _tCache3;

};



class CAnimNodeSkin:public IAnimNodeSkin
{
public:
	IMPLEMENT_REFCOUNT_C
	DEFINE_CLASS(CAnimNodeSkin);
	CAnimNodeSkin()
	{
		Zero();
	}
	~CAnimNodeSkin()
	{
		Clear();
	}

	void Zero()
	{
		_skeleton=NULL;
		_matsCache=NULL;
        _ctrlCloth = NULL;
		_tCache=ANIMTICK_INFINITE;
        _tClothCache = ANIMTICK_INFINITE;
	}

	BOOL Init(AssetSystemState*ss);
	void Clear();

	virtual void SetSkeleton(ISkeleton *skl) override;
    virtual void SetClothCtrl(IClothCtrl *ctrlCloth) override;

	virtual void SetBase(IAnimNode *anSkeleton) override;
	virtual IMatrice43*GetSkinMats(AnimTick t) override;
	virtual BOOL IsFixed() override {		return FALSE;	}

protected:
	
	ISkeleton *_skeleton;

    IClothCtrl *_ctrlCloth;

	IMatrice43 *_matsCache;
	AnimTick _tCache;
    AnimTick _tClothCache;

};


class CSklLinks;
class CAnimNodePathNav:public IAnimNodePathNav
{
public:
	IMPLEMENT_REFCOUNT_C
	DEFINE_CLASS(CAnimNodePathNav);
	CAnimNodePathNav()
	{
		Zero();
	}
	~CAnimNodePathNav()
	{
		Clear();
	}

	void Zero()
	{
		_ctrl=NULL;
		_tCache=ANIMTICK_INFINITE;
		_owner=NULL;
		_skllinks=NULL;
	}

	BOOL Init(AssetSystemState*ss);

	void Clear();
	virtual void Destroy();//会释放引用计数 

	virtual void SetAnimTreeCtrl(IAnimTreeCtrl *ctrl);
	virtual IAnimTreeCtrl*GetAnimTreeCtrl()	{		return _ctrl;	}

	virtual ISklLinks*GetSklLinks()	{		return (ISklLinks*)_skllinks;	}
	virtual IMano*GetMano();
	virtual i_math::matrix43f *GetMat(AnimTick t);

	virtual void SetBase(IAnimNode *an);
	virtual BOOL IsSkl()	{		return FALSE;	}

	virtual IAsset*GetOwner()	{		return _owner;	}
	virtual void SetOwner(IAsset *ast);
	virtual BOOL IsFixed()	{		return FALSE;	}

protected:

	IAnimTreeCtrl *_ctrl;

	CSklLinks *_skllinks;

	i_math::matrix43f _matCache;
	AnimTick _tCache;

	IAsset *_owner;
};

class CAnimNodeAnimTreeCtrl :public IAnimNodeAnimTreeCtrl
{
public:
	IMPLEMENT_REFCOUNT_C
	DEFINE_CLASS(CAnimNodeAnimTreeCtrl);

	CAnimNodeAnimTreeCtrl(void);
	~CAnimNodeAnimTreeCtrl(void);
	virtual void SetAnimTreeCtrl(IAnimTreeCtrl * ctrl);
	virtual IAnimTreeCtrl * GetAnimTreeCtrl(){return _ctrl;}
protected:
	IAnimTreeCtrl * _ctrl;
};

template<typename T_cache,typename T_interface,KeyType T_KeyType>
class CSimpleAnim_AnimPlayer:public T_interface
{
public:
	IMPLEMENT_REFCOUNT_C

	CSimpleAnim_AnimPlayer()
	{
		_player=NULL;
		_tCache=ANIMTICK_INFINITE;
	}
	~CSimpleAnim_AnimPlayer()
	{
		SAFE_RELEASE(_player);
	}

	virtual void SetAnimPlayer(IAnimPlayer*player)
	{
		SAFE_REPLACE(_player,player);
	}
	virtual BOOL IsFixed()	{		return FALSE;	}


protected:
	IAnimPlayer *_player;
	T_cache _cache;
	AnimTick _tCache;

};

class CAnimNodeUV:public CSimpleAnim_AnimPlayer<i_math::matrix43f,IAnimNodeUV,KT_MapCoord>
{
public:
	DEFINE_CLASS(CAnimNodeUV);

	virtual i_math::matrix43f*GetMat(AnimTick t);

};


class CAnimNodePath:public CSimpleAnim_AnimPlayer<i_math::matrix43f,IAnimNodePath,KT_XForm>
{
public:
	DEFINE_CLASS(CAnimNodePath);

	CAnimNodePath()
	{
		_bLocalBase=0;
		_bBaseCalc=0;
	}

	virtual void SetLocalBase(BOOL bLocalBase)	{		_bLocalBase=bLocalBase?1:0;	}
	virtual BOOL GetLocalBase()	{		return (BOOL)(_bLocalBase);	}

	virtual i_math::matrix43f*GetMat(AnimTick t);

protected:
	DWORD _bLocalBase:1;
	DWORD _bBaseCalc:1;
	i_math::vector3df _posBase;

};

class CAnimNodeColorFixed:public IAnimNodeColorFixed
{
public:
	IMPLEMENT_REFCOUNT_C
	DEFINE_CLASS(CAnimNodeColorFixed);

	CAnimNodeColorFixed()
	{
		_colCache=0xffffffff;
	}


	virtual void SetColor(DWORD col)	{		_colCache=col;	}
	virtual DWORD *GetColor(AnimTick t)		{				return &_colCache;		}
	virtual BOOL IsFixed()	{		return TRUE;	}

protected:
	DWORD _colCache;
};

struct LocalTimeCache
{
	DEFINE_CLASS(LocalTimeCache);
	LocalTimeCache()
	{
		t=ANIMTICK_INFINITE;
	}
	CLocalTime localtime;
	AnimTick t;
};

class CSklLinks;
class CAnimNodeSklLinks:public IAnimNodeSklLinks
{
public:
	IMPLEMENT_REFCOUNT_C;
	DEFINE_CLASS(CAnimNodeSklLinks);


	CAnimNodeSklLinks()
	{
		_bv=NULL;
		_skllinks=NULL;
		_owner=NULL;
		_avs=NULL;
		_ltc=NULL;
		_ss=NULL;
	}
	~CAnimNodeSklLinks();

	BOOL Init(AssetSystemState*ss);

	virtual void Destroy();

	virtual void SetBase(IAnimNode *an)
	{
		SAFE_REPLACE(_base,an);
	}
	
	virtual void SetRatomsBv(IRatomsBv *bv);
	BOOL IsAlive()	{		return _skllinks!=NULL;	}
	virtual IRatomsBv *GetRatomsBv()	{		return _bv;	}
	virtual ISklLinks*GetSklLinks()	{		return (ISklLinks*)_skllinks;	}
	virtual IMano*GetMano();
	virtual IAsset*GetOwner()	{		return _owner;	}
	virtual void SetOwner(IAsset *ast);
	virtual CAvtrStates*GetAvs()	{		return _avs;	}
	virtual void SetAvs(CAvtrStates*avs);
	virtual void EnableLocalTime();
	virtual CLocalTime *GetLocalTime();
	virtual void SetLocalTimeRate(float rate)
	{
		if (_ltc)
		{
			_ltc->localtime.SetRate(rate);
		}
	}
	virtual void UpdateLocalTime()
	{
		GetLocalTime();
	}
	virtual BOOL IsFixed()	{		return FALSE;	}


protected:

	void _Clear();

	IRatomsBv *_bv;
	CSklLinks *_skllinks;

	LocalTimeCache *_ltc;

	AssetSystemState *_ss;

	IAsset *_owner;
	CAvtrStates *_avs;
};


class CAnimNodeCamera:public IAnimNodeCamera
{
public:
	IMPLEMENT_REFCOUNT_C
	DEFINE_CLASS(CAnimNodeCamera);

	CAnimNodeCamera()
	{
		_cam=NULL;
		_bLab=FALSE;
	}
	~CAnimNodeCamera();

	virtual void SetCamera(ICamera *cam,BOOL bLab);
	virtual ICamera *GetCamera(AnimTick t,BOOL &bLab)	{		bLab=_bLab;return _cam;	}

protected:
	ICamera *_cam;
	BOOL _bLab;


};



#define DEFINE_CREATE_ANIMNODE(clss) virtual IAnimNode##clss *Create##clss()							\
{																																									\
	CAnimNode##clss*p=Class_New2(CAnimNode##clss);																		\
	p->AddRef();																																			\
	return p;																																					\
}

#define DEFINE_CREATE_INITED_ANIMNODE(clss) virtual IAnimNode##clss *Create##clss()			\
{																																									\
	CAnimNode##clss*p=Class_New2(CAnimNode##clss);																		\
	p->AddRef();																																			\
	p->Init(_ss);																																				\
	return p;																																					\
}




struct AssetSystemState;
class CAnimNodes:public IAnimNodes
{
public:
	CAnimNodes(AssetSystemState *ss);
	DEFINE_CREATE_ANIMNODE(Mat);
	DEFINE_CREATE_ANIMNODE(MatFixed);
	DEFINE_CREATE_ANIMNODE(MatOffset);

	DEFINE_CREATE_ANIMNODE(PosEuler);
	DEFINE_CREATE_ANIMNODE(EulerOverride);

	DEFINE_CREATE_INITED_ANIMNODE(Dtr);

	DEFINE_CREATE_INITED_ANIMNODE(Skeleton);
	DEFINE_CREATE_INITED_ANIMNODE(Matrice43);
	DEFINE_CREATE_INITED_ANIMNODE(Skin);
	DEFINE_CREATE_INITED_ANIMNODE(Ragdoll);

	DEFINE_CREATE_INITED_ANIMNODE(PathNav);

	DEFINE_CREATE_ANIMNODE(UV);
	DEFINE_CREATE_ANIMNODE(Path);

	DEFINE_CREATE_ANIMNODE(Proxy);
	DEFINE_CREATE_ANIMNODE(Site);
	DEFINE_CREATE_ANIMNODE(NoRoll);

	DEFINE_CREATE_ANIMNODE(ColorFixed);

	DEFINE_CREATE_INITED_ANIMNODE(SklLinks);

	DEFINE_CREATE_ANIMNODE(Camera);

	DEFINE_CREATE_ANIMNODE(AnimTreeCtrl);

protected:
	AssetSystemState*_ss;
};
