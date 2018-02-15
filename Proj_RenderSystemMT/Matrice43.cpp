/********************************************************************
	created:	2006/8/7   10:55
	filename: 	d:\IxEngine\Proj_RenderSystem\Matrice43Mgr.cpp
	author:		ixnehc
	
	purpose:	IMatrice43 & IMatrice43Mgr implment,
*********************************************************************/
#include "stdh.h"

#pragma warning(disable:4018)


#include "Matrice43.h"
#include <assert.h>


//////////////////////////////////////////////////////////////////////////
//CMatrice43
CMatrice43::CMatrice43()
{
	_ver=0;
}

BOOL CMatrice43::Set(matrix43f &mat)
{
	return Set(&mat,1);
}
BOOL CMatrice43::Set(matrix43f *mats,DWORD count)
{
	_mats.resize(count);
	if (count>0)
		memcpy(&_mats[0],mats,sizeof(mats[0])*count);
	_IncVer();
	return TRUE;
}

BOOL CMatrice43::SetCount(DWORD count)
{
	_mats.resize(count);
	_IncVer();
	return TRUE;
}


