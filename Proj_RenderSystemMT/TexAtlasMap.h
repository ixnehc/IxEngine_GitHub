#pragma once

#include "RenderSystem/ITexAtlas.h"
#include "RenderSystem/ITexture.h"


#include "class/class.h"

#define TEXATLASID_EMPTY (0xffffffffffffffff)
#define MAKE_TEXATLASID(pool,slot)  ((((unsigned __int64)(pool))<<32)|((unsigned __int64)(slot)))
#define TEXATLASID_POOL(id) ((DWORD)((id)>>32))
#define TEXATLASID_SLOT(id) ((DWORD)((id)&0xffffffff))


class CTexAtlasPool;
class CTexAtlas;
class IRenderSystem;
struct TexInfo;

class CTexAtlasPool:public ITexAtlasPool
{
public:
	IMPLEMENT_REFCOUNT_C;
	DEFINE_CLASS(CTexAtlasPool)

	CTexAtlasPool()
	{
		_pRS=NULL;
	}
	~CTexAtlasPool()
	{
		UnInit();
	}

	BOOL Init(IRenderSystem *pRS,TexInfo &ti);
	void UnInit();

//Interfaces
	virtual ITexAtlas *Alloc(ITexture *tex);//alloc an atlas with the same size of tex
	//alloc an atlas of size (w x h),scale if needed(use filter)
	virtual ITexAtlas *Alloc(DWORD w,DWORD h,ITexture *tex,int filter=D3DTEXF_LINEAR);
	virtual void FreeAll();//free all the atlas,NOTE: use ITexAtlas::Release() to free a single atlas
	virtual BOOL CheckLeak();//Check whether there is any atlas left not released in the mgr
protected:
	void _Free(ITexAtlas *atlas);//not opened to public

	BOOL _NewPool(DWORD dBase);
protected:
	struct _PoolInfo
	{
		_PoolInfo()
		{
			Zero();
		}
		void Zero()
		{
			nextempty=-1;
			tex=NULL;
			buffer=NULL;
			szBuffer=0;
			dBase=0;
		}
		void Clean();
		int FindEmptySlot(BOOL bRelocate=TRUE);
		i_math::recti GetSlotRect(int iSlot);
		BOOL CheckEmpty();


		ITexture *tex;
		DWORD dBase;//the size of each atlas
		CTexAtlas *buffer;
		DWORD szBuffer;//how many CTexAtlas in buffer
		int nextempty;
	};

	std::vector<_PoolInfo>_pools;
	TexInfo _ti;
	IRenderSystem* _pRS;

	friend class CTexAtlas;
};

class CTexAtlas:public ITexAtlas
{
public:
	IMPLEMENT_REFCOUNT_OVERRIDE;
	void OnRelease();

	CTexAtlas()
	{
		Zero();
	}

	void Zero()
	{
		_sz.set(0,0);
		_off.set(0,0);
		_tex=NULL;
		_id=TEXATLASID_EMPTY;
		_pool=NULL;
	}

	void Clean()
	{
		SAFE_RELEASE(_tex);
		Zero();
	}

	virtual BOOL CanBatchWith(ITexAtlas *atlas)//check whether this atlas could be used in a same batch with another one
	{
		return (_tex==atlas->GetTex());
	}

	virtual BOOL IsValid()	{		return _tex!=NULL;	}

	//Location on the owner texture
	virtual i_math::vector2df GetSize()	{		return _sz;	}
	virtual i_math::vector2df GetOff()	{		return _off;	}
	virtual i_math::rectf GetRect()	{		return i_math::rectf(_off.x,_off.y,_off.x+_sz.x,_off.y+_sz.y);	}

	//the owner texture (The texture this atlas belongs to)
	virtual ITexture *GetTex()	{		return _tex;	}

protected:
	i_math::vector2df _sz;
	i_math::vector2df _off;
	ITexture *_tex;

	unsigned __int64 _id;//the id used for finding in the pool
	CTexAtlasPool* _pool;

	friend class CTexAtlasPool;
	friend class CTexAtlasMap;
	friend struct CTexAtlasPool::_PoolInfo;
};

class CTexAtlasMap:public ITexAtlasMap
{
public:
	IMPLEMENT_REFCOUNT_C;
	DEFINE_CLASS(CTexAtlasMap)

	CTexAtlasMap()
	{
		_texMain=NULL;
		_bAllowResize=TRUE;
		_area=0;
	}
	~CTexAtlasMap()
	{
		UnInit();
	}

	BOOL Init(IRenderSystem *pRS,TexInfo &ti,BOOL bAllowResize);
	void UnInit();

	//Interfaces
	virtual void BeginStitch();
	//the ITexAtlas returned will contain right content after a successful EndStitch() calling
	virtual ITexAtlas *Add(ITexture *tex);
	virtual ITexAtlas *Add(ITexture *tex,TexAtlasArg &tag);
	virtual BOOL EndStitch();
	virtual ITexture *GetTex()	{		return _texMain;	}
protected:
	struct _AtlasInfo:public TexAtlasArg
	{
		_AtlasInfo()
		{
			tex=NULL;
			ta=NULL;
			wSlot=hSlot=0;
		}
		ITexture *tex;
		CTexAtlas *ta;
		DWORD wSlot;
		DWORD hSlot;
	};
	void _ClearAtlasInfo();
	BOOL _BuildAtlas(_AtlasInfo *info,i_math::recti &rc);
	ITexture *_texMain;
	BOOL _bAllowResize;
	std::vector<_AtlasInfo> _ai;
	DWORD _area;

	friend class CTexAtlas;

};



