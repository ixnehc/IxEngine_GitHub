
/************************************************************************/
/* 
e:\IxEngine\Proj_RenderSystem\MoppMgr.cpp
author: star
purpose: Mopp resource Manager
date: 2008-01-07
*/
/************************************************************************/

#include "stdh.h"
#include ".\moppmgr.h"
#include "assert.h"
#include "../common/Log/LogFile.h"
#include "resdata/MoppData.h"
#include "commondefines/general.h"
#include "spatialtester/spatialtester.h"

#include "Matrice43.h"
#include <stack>

//////////////////////////////////////////////////////////////////////////
//CMopp

IMPLEMENT_CLASS(CMopp);

CMopp::CMopp()
{	
}

CMopp::~CMopp(void)
{
}



void CMopp::_Clean()
{
	_moppdata.Clean();
}


BOOL CMopp::_OnTouch(IRenderSystem *pRS)
{
	VALIDATE_RES_TYPE(Res_Mopp);

	_moppdata.LoadData(_data);

	_moppdata.CalcAABB();

	return TRUE;
}

void CMopp::_OnUnload()
{
	_Clean();
}

BOOL CMopp::HitTest(const i_math::line3df & rayHit,DWORD &iFace,float &dist)
{
	DWORD nIBs = _moppdata.indices.size();
	i_math::vector3df * pv = &(_moppdata.vertices[0]);
	
	int idx = -1;
	float minDist = 99999999.0f;

	i_math::triangle3df tri;
	i_math::vector3df intersec;
	for(int i = 0;i<nIBs;i+= 3)
	{
		WORD i0 = _moppdata.indices[i+0];
		WORD i1 = _moppdata.indices[i+1];
		WORD i2 = _moppdata.indices[i+2];
		
		tri.set(pv[i0],pv[i1],pv[i2]);

		i_math::vector3df intersec,vec;
		i_math::plane3df p = tri.getPlane();

		float r0 = p.dotProduct(rayHit.start);
		float r1 = p.dotProduct(rayHit.end);

		//光源 与动态物体处于不同侧
		if(r0>0&&r1<0||r0<0&&r1>0)
		{
			float r = r0/(r0-r1);
			vec = r*(rayHit.end-rayHit.start);
			intersec = rayHit.start + vec;
			
			if(tri.isPointInsideFast(intersec))
			{
				float sQ = (float)vec.getLengthSQ();
				if(sQ<minDist)
				{
					idx = i/3;
					minDist = sQ;
				}
			}
		}
		else
		{
			continue; //同侧没有交点
		}

	}

	if(idx>=0)
	{
		iFace = DWORD(idx);
		dist = sqrtf(minDist);
		return TRUE;
	}

	return FALSE;
}
BOOL CMopp::GetFace(int idx,i_math::triangle3df & tri)
{
	assert(idx>=0);

	int t = 3*idx;

	WORD i0 = _moppdata.indices[t+0];
	WORD i1 = _moppdata.indices[t+1];
	WORD i2 = _moppdata.indices[t+2];

	DWORD nVtx = _moppdata.vertices.size();
	if(i0>=nVtx||i1>=nVtx||i2>=nVtx)
		return FALSE;

	i_math::vector3df * pv = &(_moppdata.vertices[0]);
	tri.set(pv[i0],pv[i1],pv[i2]);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//CMoppMgr

CMoppMgr::CMoppMgr(void)
{

}

CMoppMgr::~CMoppMgr(void)
{

}
IResource * CMoppMgr::ObtainRes(const char * path)
{	
    return _ObtainResS<CMopp>(path);	
}

BOOL CMoppMgr::ReloadRes(const char *pathRes)
{
	return CResourceMgr::_ReloadRes<CMopp>(pathRes,'S');
}
