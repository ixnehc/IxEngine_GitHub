// ***************************************************************
//  TexAtlas   version:  1.0   ? date: 11/25/2006
//  -------------------------------------------------------------
//  author:		cxi
//  -------------------------------------------------------------
//  Copyright (C) 2006 - All Rights Reserved
// ***************************************************************
//  Purpose: ITexAtlas ,ITexAtlasPool implement
// ***************************************************************
#include "stdh.h"

#include "stringparser/stringparser.h"

#include "Log/LogFile.h"

#include "TexAtlasMap.h"

#include "RenderSystem/IRenderSystem.h"

#include <assert.h>

#include <deque>

//////////////////////////////////////////////////////////////////////////
//CTexAtlas

void CTexAtlas::OnRelease()
{
	Clean();
	if(_pool)
		_pool->_Free(this);//this is an atlas from CTexAtlasPool
	else
		delete this;//this is a atlas from CTexAtlasMap
}





//////////////////////////////////////////////////////////////////////////
//CTexAtlasPool::_PoolInfo
int CTexAtlasPool::_PoolInfo::FindEmptySlot(BOOL bRelocate)
{
	if (nextempty==-1)
		return -1;
	int ret=nextempty;
	if (!bRelocate)
		return ret;

	//now re-locate the nextempty
	int i;
	for (i=0;i<szBuffer-1;i++)
	{
		if (buffer[(i+nextempty)%szBuffer]._id==TEXATLASID_EMPTY)
		{
			nextempty=(i+nextempty)%szBuffer;
			break;
		}
	}
	if (i>=szBuffer-1)//cannot find empty
		nextempty=-1;
	return ret;
}

i_math::recti CTexAtlasPool::_PoolInfo::GetSlotRect(int iSlot)
{
	DWORD c=tex->GetWidth()/dBase;

	DWORD x,y;
	x=iSlot/c;
	y=iSlot%c;

	recti rc;
	rc.set(x*dBase,y*dBase,(x+1)*dBase,(y+1)*dBase);

	return rc;
}

void CTexAtlasPool::_PoolInfo::Clean()
{
	if (buffer)
	{
		for (int i=0;i<szBuffer;i++)
			buffer[i].Clean();
		delete buffer;
	}
	SAFE_RELEASE(tex);
	Zero();
}

BOOL CTexAtlasPool::_PoolInfo::CheckEmpty()
{
	if (!buffer)
		return TRUE;
	for (int i=0;i<szBuffer;i++)
	{
		if (buffer[i]._id!=TEXATLASID_EMPTY)
			return FALSE;
	}
	return TRUE;
}




//////////////////////////////////////////////////////////////////////////
//CTexAtlasPool

BOOL CTexAtlasPool::Init(IRenderSystem *pRS,TexInfo &ti)
{
	_pRS=pRS;
	_ti=ti;

	//Check whether this texture format is supported
	ITexture *tex=_pRS->GetWTexMgr2()->Create(_ti);
	if (!tex)
		return FALSE;
	SAFE_RELEASE(tex);

	return TRUE;
}

void CTexAtlasPool::UnInit()
{
	FreeAll();

	_pRS=NULL;
	_ti.Zero();
}

//alloc an atlas
ITexAtlas *CTexAtlasPool::Alloc(ITexture *tex)
{
	if (!tex)
		return NULL;
	return Alloc(tex->GetWidth(),tex->GetHeight(),tex);
}

BOOL CTexAtlasPool::_NewPool(DWORD dBase)
{
	_PoolInfo pl;
	pl.tex=_pRS->GetWTexMgr2()->Create(_ti);
	if (!pl.tex)
		return FALSE;

	pl.dBase=dBase;
	pl.szBuffer=_ti.width/dBase*_ti.width/dBase;
	pl.buffer=new CTexAtlas[pl.szBuffer];
	pl.nextempty=0;

	_pools.push_back(pl);

	return TRUE;

}


//alloc an atlas of size (w x h),scale if needed
ITexAtlas *CTexAtlasPool::Alloc(DWORD w,DWORD h,ITexture *tex,int filter)
{
	if (!tex)
		return NULL;
	if (w!=h)
		return NULL;
	if (!i_math::ispower2(w))
		return NULL;
	if (w>_ti.width)
		return NULL;

	DWORD d=w;
	DWORD dSlot=d;
	if (dSlot<4)
		dSlot=4;

	int iPool;
	int iSlot=-1;
	for (iPool=0;iPool<_pools.size();iPool++)
	{
		if (_pools[iPool].dBase==dSlot)
		{
			iSlot=_pools[iPool].FindEmptySlot(FALSE);//No need to re-locate by now(maybe fail in the following steps)
			if (iSlot!=-1)
				break;
		}
	}

	if (iSlot==-1)
	{
		if (FALSE==_NewPool(dSlot)	)
			return NULL;
		iPool=_pools.size()-1;
		iSlot=_pools[iPool].FindEmptySlot(FALSE);//No need to re-locate by now(maybe fail in the following steps)
	}

	if (iSlot==-1)//in case ...
		return NULL;

	//calc the dest
	i_math::recti rcDest;
	rcDest=_pools[iPool].GetSlotRect(iSlot);
	rcDest.Right()=rcDest.Left()+d;//d could be less than dSlot
	rcDest.Bottom()=rcDest.Top()+d;//d could be less than dSlot

	if (TRUE)//try to do the copy
	{
		TexStretchArg arg;
		arg.rcDest=rcDest;
		arg.filter=filter;
		if (FALSE==_pools[iPool].tex->Stretch(tex,arg))
			return NULL;
	}

	//ensured ok
	if (iSlot!=_pools[iPool].FindEmptySlot())//this will re-locate
		assert(false);

	CTexAtlas *at=&_pools[iPool].buffer[iSlot];
	at->_id=MAKE_TEXATLASID(iPool,iSlot);
	at->_off.set((((f32)rcDest.Left())+0.5f)/((f32)_ti.width),(((f32)rcDest.Top())+0.5f)/((f32)_ti.height));
	at->_sz.set((((f32)rcDest.Right())-0.5f)/((f32)_ti.width),(((f32)rcDest.Bottom())-0.5f)/((f32)_ti.height));
	at->_sz-=at->_off;
	at->_tex=_pools[iPool].tex;
	at->_tex->AddRef();
	at->_pool=this;

	at->AddRef();

	return at;

}

void CTexAtlasPool::_Free(ITexAtlas *atlas0)
{
	CTexAtlas *atlas=(CTexAtlas *)atlas0;
	DWORD iPool,iSlot;
	iPool=TEXATLASID_POOL(atlas->_id);
	iSlot=TEXATLASID_SLOT(atlas->_id);

	if (iPool>=_pools.size())
		goto _fail;
	if (iSlot>=_pools[iPool].szBuffer)
		goto _fail;
	if (atlas!=&_pools[iPool].buffer[iSlot])
		goto _fail;

	atlas->Clean();
	_pools[iPool].nextempty=iSlot;

	return;

_fail:
	assert(FALSE);
	return;
}


//free all the atlas,NOTE: use ITexAtlas::Release() to free a single atlas
void CTexAtlasPool::FreeAll()
{
	for (int i=0;i<_pools.size();i++)
		_pools[i].Clean();
	_pools.clear();
}

//Check whether there is any atlas left not released in the mgr
BOOL CTexAtlasPool::CheckLeak()
{
	for (int i=0;i<_pools.size();i++)
	{
		if (!_pools[i].CheckEmpty())
			return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//CTexAtlasMap

BOOL CTexAtlasMap::Init(IRenderSystem *pRS,TexInfo &ti,BOOL bAllowResize)
{
	_texMain=pRS->GetWTexMgr2()->Create(ti);
	if (!_texMain)
		return FALSE;
	if (A_Ok!=_texMain->Touch())
		return FALSE;
	_bAllowResize=bAllowResize;
	return TRUE;
}

void CTexAtlasMap::UnInit()
{
	SAFE_RELEASE(_texMain);
	_ClearAtlasInfo();
}

void CTexAtlasMap::_ClearAtlasInfo()
{
	for (int i=0;i<_ai.size();i++)
	{
		SAFE_RELEASE(_ai[i].ta);
		SAFE_RELEASE(_ai[i].tex);
	}
	_ai.clear();
}

//Interfaces
void CTexAtlasMap::BeginStitch()
{
	_ClearAtlasInfo();
	_area=0;
}

ITexAtlas *CTexAtlasMap::Add(ITexture *tex)
{
	TexAtlasArg tag;
	return Add(tex,tag);
}

static void GetTagSize(DWORD &w,DWORD &h,ITexture *tex,TexAtlasArg &tag)
{
	if (tag.w!=0)
		w=tag.w;
	else
	{
		if (tag.rcSrc.getWidth()!=0)
			w=tag.rcSrc.getWidth();
		else
			w=tex->GetWidth();
	}
	if (tag.h!=0)
		h=tag.h;
	else
	{
		if (tag.rcSrc.getHeight()!=0)
			h=tag.rcSrc.getHeight();
		else
			h=tex->GetHeight();
	}
}

ITexAtlas *CTexAtlasMap::Add(ITexture *tex,TexAtlasArg &tag)
{
	if (!tex->ForceTouch())
		return NULL;
	tex->AddRef();
	DWORD w,h;
	GetTagSize(w,h,tex,tag);
	if ((w==0)||(h==0))
		return NULL;

	_AtlasInfo info;
	((TexAtlasArg &)info)=tag;
	info.tex=tex;
	info.w=w;
	info.h=h;
	info.wSlot=1<<fastmaxlog2(w);
	info.hSlot=1<<fastmaxlog2(h);
	if (!info.rcSrc.isValid())
		info.rcSrc.set(0,0,tex->GetWidth(),tex->GetHeight());
	if (info.wSlot!=info.hSlot)
		return NULL;
	if (!_bAllowResize)
	{
		if (_area+info.wSlot*info.hSlot>_texMain->GetWidth()*_texMain->GetHeight())
			return NULL;
	}
	_area+=info.wSlot*info.hSlot;
	info.ta=new CTexAtlas;
	info.ta->AddRef();//for myself

	_ai.push_back(info);

	info.ta->AddRef();//for the caller
	return info.ta;
}

BOOL CTexAtlasMap::_BuildAtlas(_AtlasInfo *p,i_math::recti &rc)
{
	//first copy the atlas texture to the _texMain
	if (TRUE)
	{
		TexStretchArg arg;
		arg.rcDest.set(0,0,p->w,p->h);
		arg.rcDest+=rc.UpperLeftCorner;
		arg.rcSrc=p->rcSrc;
		if (FALSE==_texMain->Stretch(p->tex,arg))
			return FALSE;
	}

	CTexAtlas *ta=p->ta;

	ta->_id=TEXATLASID_EMPTY;
	ta->_tex=_texMain;
	ta->_tex->AddRef();
	ta->_pool=NULL;

	f32 fw,fh;
	fw=(f32)_texMain->GetWidth();
	fh=(f32)_texMain->GetHeight();
	ta->_off.set((((f32)rc.Left())+0.5f)/fw,(((f32)rc.Top())+0.5f)/fh);
	ta->_sz.set((((f32)rc.Right())-0.5f)/fw,(((f32)rc.Bottom())-0.5f)/fh);
	ta->_sz-=ta->_off;

	return TRUE;
}

static void SplitRect(i_math::recti &rc,DWORD w,std::deque<i_math::recti>&pool)
{
	assert(rc.getWidth()==rc.getHeight());
	assert(rc.getWidth()%w==0);
	assert(rc.getWidth()>w);

	int c=rc.getWidth()/w;

	for (int i=0;i<c;i++)
	for (int j=0;j<c;j++)
	{
		i_math::recti rcNew;
		rcNew.set(i*w,j*w,i*w+w,j*w+w);
		rcNew+=rc.UpperLeftCorner;
		pool.push_back(rcNew);
	}

}


BOOL CTexAtlasMap::EndStitch()
{
	//first sort the _ai in decending order of the slot size
	for (int i=0;i<_ai.size();i++)
	for (int j=i+1;j<_ai.size();j++)
	{
		if (_ai[i].wSlot<_ai[j].wSlot)
		{
			_AtlasInfo t=_ai[j];
			_ai[j]=_ai[i];
			_ai[i]=t;
		}
	}

	//count the area 
	DWORD area=0;
	for (int i=0;i<_ai.size();i++)
		area+=_ai[i].wSlot*_ai[i].hSlot;

	DWORD sf=1;//scale factor
	while(area>_texMain->GetWidth()*_texMain->GetHeight())
	{
		sf*=2;
		area/=4;
	}

	if (sf>1)
	{
		for (int i=0;i<_ai.size();i++)
		{
			_ai[i].wSlot/=sf;
			_ai[i].hSlot/=sf;
			_ai[i].w/=sf;
			_ai[i].h/=sf;
			if ((_ai[i].w<=0)||(_ai[i].h<=0))
				return FALSE;//this atlas will be shinked to none,fail
#pragma message("------------------------------Maybe we could do not shrink this very small atlas,and continue shrink other big enough atlases?")
		}
	}

	std::deque<i_math::recti> rcpool;
	rcpool.push_back(i_math::recti(0,0,_texMain->GetWidth(),_texMain->GetHeight()));

	BOOL bRet=TRUE;
	for (int i=0;i<_ai.size();i++)
	{
		_AtlasInfo *p=&_ai[i];

		assert(rcpool.size()>0);

		if (rcpool[0].getWidth()!=p->wSlot)
		{
			DWORD c=rcpool.size();
			for (int j=0;j<c;j++)
			{
				i_math::recti rc=rcpool[0];
				rcpool.pop_front();
				SplitRect(rc,p->wSlot,rcpool);
			}
		}
		assert(rcpool[0].getWidth()==p->wSlot);

		bRet=bRet&&_BuildAtlas(p,rcpool[0]);
		rcpool.pop_front();
	}

	if (_texMain->GetMipLevel()!=0)
		bRet=bRet&&_texMain->Filter(5);//D3DX_FILTER_BOX,seems good effect

	_ClearAtlasInfo();

	return bRet;
}

