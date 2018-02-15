/********************************************************************
	created:	2006/06/17
	created:	17:6:2006   15:05
	filename: 	d:\IxEngine\Proj_RenderSystem\VertexMgr.cpp
	author:		cxi
	
	purpose:	IVertexBuffer,IVertexMgr implement
*********************************************************************/
#include "stdh.h"

#include "fvfex/fvfex.h"

//#include "GLRenderSystem.h"

#include "GLVertexMgr.h"


#include "GLDeviceObject.h"

#include "timer/profiler.h"

#include <assert.h>

#pragma warning (disable: 4018)

//////////////////////////////////////////////////////////////////////////
//VBBaseGL
VBBaseGL::VBBaseGL()
{
}


void VBBaseGL::_Zero()
{
	_bVB=TRUE;
	_flags=0;//VBFlag_XXXX

	_handle=NULL;
	_count=0;
	_memcount=0;

	_cLock=0;
}




//////////////////////////////////////////////////////////////////////////
//VBInfoGL
IMPLEMENT_CLASS(VBInfoGL);
void VBInfoGL::_Zero()
{
	VBBaseGL::_Zero();
	_fvf=0;
	_fvfsize=0;

	_state=CResource::Abandoned;
}


VBInfoGL::VBInfoGL()
{
	_Zero();
}

int VBInfoGL::Release()
{
	int c=CResource::Release();
	if (c<=0)
	{
		assert(_cLock==0);
		_OnUnload();
	}

	return c;
}


BOOL VBInfoGL::_OnTouch(IRenderSystem *pRS)
{
	if (_handle!=GLHandle_Null)
		return TRUE;

	glGenBuffers(1,&_handle);
	glBindBuffer(GL_ARRAY_BUFFER,_handle);
	glBufferData(GL_ARRAY_BUFFER,_memcount,NULL,(_flags&VBFlag_Dynamic)?GL_DYNAMIC_DRAW:GL_STATIC_DRAW);
	return TRUE;
}

void VBInfoGL::_OnUnload()
{
	if (_handle)
		glDeleteBuffers(1,&_handle);
	_handle=GLHandle_Null;
	_Zero();
}

void VBInfoGL::_OnDeviceLost()
{
// 	if (_flags&VBFlag_Dynamic)
// 	{
// 		SAFE_RELEASE_D3DRES((XD3DVertexBuffer*&)_handle);
// 		SetState(CResource::Abandoned);
// 	}
}

void VBInfoGL::_OnDeviceReset()
{
// 	SetState(CResource::Loaded);
}





void *VBInfoGL::Lock(BOOL bDiscard,FVFEx fvf,DWORD iFrame)
{
	assert(iFrame==0);

	int nOff=0;
	if (fvf!=0)
		nOff=fvfOffset(_fvf,fvf);
	if (nOff<0)
		return NULL;//fvf invalid

	if (A_Ok!=Touch())
		return NULL;
	if (_cLock<=0)
		_cLock=0;

	if (_buf.size()<=0)
		_buf.resize(_memcount);

	_dirties.clear();

	_cLock++;
	return (void *)(((unsigned char*)&_buf[0])+nOff);
}

void VBInfoGL::AddDirty(DWORD from,DWORD to)
{
	_AddDirty(from,to);
}



void VBInfoGL::Unlock()
{
	if (_cLock<=0)
		return;

	_cLock--;

	if (_cLock<=0)
	{
		if (_handle)
		{
			glBindBuffer(GL_ARRAY_BUFFER,_handle);
			if (!(_flags&VBFlag_Dynamic))
				glBufferSubData(GL_ARRAY_BUFFER,0,_memcount,&_buf[0]);
			else
			{
				for (int i=0;i<_dirties.size();i++)
				{
					glBufferSubData(GL_ARRAY_BUFFER,_dirties[i].low*(int)_fvfsize,(_dirties[i].hi-_dirties[i].low)*(int)_fvfsize,
						&_buf[0]+_dirties[i].low*(int)_fvfsize);
				}
			}
		}
	}

	_dirties.clear();

}

void VBInfoGL::UnlockAll()
{
	_cLock=0;
	Unlock();
}


BOOL VBInfoGL::SetFVF(FVFEx fvf)
{
	DWORD fvfsize=fvfSize(fvf);
	if (fvfsize<=0)
		return FALSE;
	if (_memcount==0)
	{
		//D3D资源尚未创建,不用更新count
		assert(!_handle);
		_fvf=fvf;
		_fvfsize=fvfsize;
	}
	else
	{
		_count=_memcount/fvfsize;
		_fvf=fvf;
		_fvfsize=fvfsize;
	}
	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//IBInfoGL
IMPLEMENT_CLASS(IBInfoGL);
void IBInfoGL::_Zero()
{
	VBBaseGL::_Zero();

	_bVB=FALSE;//mark myself as an IB
}


IBInfoGL::IBInfoGL()
{
	_Zero();
}

int IBInfoGL::Release()
{
	int c=CResource::Release();
	if (c<=0)
	{
		assert(_cLock==0);
		_OnUnload();
	}

	return c;
}


BOOL IBInfoGL::_OnTouch(IRenderSystem *pRS)
{
	if (_handle!=GLHandle_Null)
		return TRUE;

	glGenBuffers(1,&_handle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_handle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,_memcount,NULL,(_flags&VBFlag_Dynamic)?GL_DYNAMIC_DRAW:GL_STATIC_DRAW);
	return TRUE;

	return TRUE;
}

void IBInfoGL::_OnUnload()
{
	SAFE_RELEASE_D3DRES((XD3DIndexBuffer*&)_handle);
	_Zero();
}

void IBInfoGL::_OnDeviceLost()
{
// 	if (_flags&VBFlag_Dynamic)
// 	{
// 		SAFE_RELEASE_D3DRES((XD3DIndexBuffer*&)_handle);
// 		SetState(CResource::Loaded);
// 	}
}

void IBInfoGL::_OnDeviceReset()
{
// 	SetState(CResource::Loaded);
}

void *IBInfoGL::Lock(BOOL bDiscard)
{
	if (A_Ok!=Touch())
		return NULL;
	if (_cLock<=0)
		_cLock=0;

	if (_buf.size()<=0)
		_buf.resize(_memcount);

	_dirties.clear();

	_cLock++;
	return (void *)((unsigned char*)&_buf[0]);
}

void IBInfoGL::Unlock()
{
	if (_cLock<=0)
		return;

	_cLock--;

	if (_cLock<=0)
	{
		if (_handle)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_handle);
			if (!(_flags&VBFlag_Dynamic))
				glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,0,_memcount,&_buf[0]);
			else
			{
				for (int i=0;i<_dirties.size();i++)
				{
					glBufferSubData(GL_ARRAY_BUFFER,_dirties[i].low*sizeof(WORD),(_dirties[i].hi-_dirties[i].low)*sizeof(WORD),
						(&_buf[0])+_dirties[i].low*sizeof(WORD));
				}
			}
		}
	}

	_dirties.clear();

}

void IBInfoGL::UnlockAll()
{
	_cLock=0;
	Unlock();
}


//////////////////////////////////////////////////////////////////////////
//CVertexMgrGL
CVertexMgrGL::CVertexMgrGL()
{
}

BOOL CVertexMgrGL::Init(IRenderSystem *pRS,const char *name)
{
	if (FALSE==CResourceMgr::Init(pRS,name))
		return FALSE;

	_iLastVB=0;
	_iLastIB=0;

	return TRUE;
}

void CVertexMgrGL::UnInit()
{
	CResourceMgr::UnInit();//Will call UnloadAll() internally
}


//flags: VBFlag_XXXX
IVertexBuffer* CVertexMgrGL::CreateVB(DWORD nVertice,FVFEx fvf,
									DWORD nFrames,DWORD flags)
{
	if (nVertice<=0)
		return NULL;
	nFrames=1;//目前不支持多个frames
	if (!fvfCheck(fvf))
		return NULL;

	VBInfoGL *p=NewRes<VBInfoGL>();
	_vbs.push_back(p);

	p->AddRef();

	p->_fvf=fvf;
	p->_count=nVertice;
	p->_flags=flags;

	p->_fvfsize=fvfSize(fvf);

	p->SetState(CResource::Loaded);

	return p;
}
	
//flags: VBFlag_XXXX
IIndexBuffer* CVertexMgrGL::CreateIB(DWORD nIndice,DWORD flags)
{
	if (nIndice<=0)
		return NULL;

	IBInfoGL *p=NewRes<IBInfoGL>();
	_ibs.push_back(p);

	p->AddRef();

	p->_count=nIndice;
	p->_flags=flags;

	p->SetState(CResource::Loaded);

	return p;
}


void CVertexMgrGL::_UnloadAll()
{
	for (int i=0;i<_vbs.size();i++)
	{
		_vbs[i]->_OnUnload();
		Class_Delete(_vbs[i]);
	}
	_vbs.clear();

	for (int i=0;i<_ibs.size();i++)
	{
		_ibs[i]->_OnUnload();
		Class_Delete(_ibs[i]);
	}
	_ibs.clear();
}


void CVertexMgrGL::OnDeviceLost()
{
	for (int i=0;i<_vbs.size();i++)
		_vbs[i]->_OnDeviceLost();
	for (int i=0;i<_ibs.size();i++)
		_ibs[i]->_OnDeviceLost();
}

void CVertexMgrGL::OnDeviceReset()
{
	for (int i=0;i<_vbs.size();i++)
		_vbs[i]->_OnDeviceReset();
	for (int i=0;i<_ibs.size();i++)
		_ibs[i]->_OnDeviceReset();
}

void CVertexMgrGL::GarbageCollect(DWORD step)
{
	_gc_vec<VBInfoGL>(_vbs,step,_iLastVB);
	_gc_vec<IBInfoGL>(_ibs,step,_iLastIB);
}