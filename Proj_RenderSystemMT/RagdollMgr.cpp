/********************************************************************
	created:	16:5:2012   15:41
	author:		cxi
	
	purpose:	Ragdoll ืสิด
*********************************************************************/

#include "stdh.h"

// #include "RenderSystem.h"

#include "RagdollMgr.h"



//////////////////////////////////////////////////////////////////////////
//CRagdoll
IMPLEMENT_CLASS(CRagdoll);
CRagdoll::CRagdoll()
{
}



BOOL CRagdoll::_OnTouch(IRenderSystem *pRS)
{
	_buf.swap(_data);

	return TRUE;
}

void CRagdoll::_OnUnload()
{
	_buf.clear();
}


//////////////////////////////////////////////////////////////////////////
//CRagdollMgr

CRagdollMgr::CRagdollMgr()
{
}



IResource *CRagdollMgr::ObtainRes(const char *pathRes)
{
	return CResourceMgr::_ObtainResS<CRagdoll>(pathRes);
}

BOOL CRagdollMgr::ReloadRes(const char *pathRes)
{
	return CResourceMgr::_ReloadRes<CRagdoll>(pathRes,'S');
}

