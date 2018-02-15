/********************************************************************
	created:	2007/3/30   15:55
	filename: 	e:\IxEngine\Proj_RenderSystem\ConvexVolume.cpp
	author:		cxi
	
	purpose:	a tool object that wrap a convex volume
*********************************************************************/
#include "stdh.h"
#include "Base.h"
#include "interface/interface.h"
    
#include "ConvexVolume.h"

#include "Log/LastError.h"
#include "Log\LogFile.h"

#include "assert.h"


CCvxVolume::CCvxVolume()
{
	_bFrameDirty=FALSE;
}


BOOL CCvxVolume::BuildFromCamera(ICamera *cam)
{
	if (FALSE==cam->GetViewFrustum(_vol))
		return FALSE;

	//We assumed the 6 planes are in the order : n,f,l,r,t,b

	//for near
	_linkmap[0].set(2);//l
	_linkmap[0].set(3);//r
	_linkmap[0].set(4);//t
	_linkmap[0].set(5);//b

	//for far
	_linkmap[1].set(2);//l
	_linkmap[1].set(3);//r
	_linkmap[1].set(4);//t
	_linkmap[1].set(5);//b

	//for left
	_linkmap[2].set(0);//n
	_linkmap[2].set(1);//f
	_linkmap[2].set(4);//t
	_linkmap[2].set(5);//b

	//for right
	_linkmap[3].set(0);//n
	_linkmap[3].set(1);//f
	_linkmap[3].set(4);//t
	_linkmap[3].set(5);//b


	//for top
	_linkmap[4].set(0);//n
	_linkmap[4].set(1);//f
	_linkmap[4].set(2);//l
	_linkmap[4].set(3);//r

	//for bottom
	_linkmap[5].set(0);//n
	_linkmap[5].set(1);//f
	_linkmap[5].set(2);//l
	_linkmap[5].set(3);//r

	_bFrameDirty=TRUE;

	return TRUE;
}

BOOL CCvxVolume::GetVolume(i_math::volumeCvxf &vol)
{
	vol=_vol;
	return TRUE;
}

BOOL CCvxVolume::SetVolume(i_math::volumeCvxf &vol)
{
	_vol=vol;

	//Rebuild the linkmap
	for (int i=0;i<ARRAY_SIZE(_linkmap);i++)
		_linkmap[i].resetAll();
	for (int i=0;i<_vol.nPlanes;i++)
	for (int j=0;j<_vol.nPlanes;j++)
	{
		if (i>=j)
			continue;

		i_math::line3df edge;
		if (_GetPlaneEdge(i,j,edge))
		{
			_linkmap[i].set(j);
			_linkmap[j].set(i);
		}
	}

	_bFrameDirty=TRUE;

	return TRUE;
}


BOOL CCvxVolume::GetCorners(i_math::vector3df *&corners,DWORD &nCorner)
{
	_CalcFrame();

	nCorner=_corners.size();
	corners=&_corners[0];
	return TRUE;
}


BOOL CCvxVolume::GetFrame(i_math::vector3df *&corners,DWORD &nCorners,
					  DWORD *&edges,DWORD &nEdges)
{
	_CalcFrame();

	nCorners=_corners.size();
	corners=&_corners[0];

	static std::vector<DWORD>temp;
	temp.resize(_edges.size()*2);

	for (int i=0;i<_edges.size();i++)
	{
		temp[i*2]=_edges[i].corner01;
		temp[i*2+1]=_edges[i].corner02;
	}
	edges=&temp[0];
	nEdges=_edges.size();

	return TRUE;
}


//Check whether the intersection line of the 2 planes is contained in the volume
BOOL CCvxVolume::_GetPlaneEdge(DWORD pl1,DWORD pl2,i_math::line3df &edge)
{
	i_math::vector3df v,n;
	if (false==_vol.planes[pl1].getIntersectionWithPlane(_vol.planes[pl2],v,n))
		return FALSE;

	BOOL bStart=FALSE,bEnd=FALSE;//the 2 ends are not limited by now
	float tStart,tEnd;
	//use other planes to trim this line
	for (int i=0;i<_vol.nPlanes;i++)
	{
		if ((i==pl1)||(i==pl2))
			continue;

		float t;
		bool bClipForward;
		if (false==_vol.planes[i].clipLine(v,n,t,bClipForward))
		{
			//The plane parellel with the line,check which side the line resides
			if (_vol.planes[i].classifyPointRelation(v)==ISREL3D_FRONT)
				return FALSE;
			continue;
		}

		if (bClipForward)
		{
			if (!bEnd)
			{
				tEnd=t;
				bEnd=TRUE;
			}
			else
				tEnd=min(tEnd,t);
		}
		else
		{
			if (!bStart)
			{
				tStart=t;
				bStart=TRUE;
			}
			else
				tStart=max(tStart,t);
		}


		if (bStart&&bEnd&&(tStart>=tEnd))
			return FALSE;
	}

	assert(bStart&&bEnd);

	edge.start=v+n*tStart;
	edge.end=v+n*tEnd;

	return TRUE;
}

void CCvxVolume::_CalcFrame()
{
	if (!_bFrameDirty)
		return;
	_corners.clear();
	_edges.clear();

	DWORD n=_vol.nPlanes;
	for (int i=0;i<n;i++)
	for (int j=0;j<n;j++)
	{
		if (i>=j)
			continue;

		if (_linkmap[i].test(j))
		{//the 2 planes share an edge
			i_math::line3df edge;
			if (_GetPlaneEdge(i,j,edge))
			{
				_Edge ei;
				ei.corner01=_corners.size();
				ei.corner02=_corners.size()+1;
				ei.plane01=i;
				ei.plane02=j;

				_corners.push_back(edge.start);
				_corners.push_back(edge.end);

				_edges.push_back(ei);
			}
		}
	}

	//TO DO:need to combine the corners
	//...

	_bFrameDirty=FALSE;
}



BOOL CCvxVolume::ExtrudeToPlane(i_math::plane3df &plTarget)
{
	//check whether any corner is beyond the plane
	if (TRUE)
	{
		for (int i=0;i<_corners.size();i++)
		{
			if (plTarget.classifyPointRelation(_corners[i])!=ISREL3D_BACK)
				return FALSE;
		}
	}

	std::map<__int64,DWORD>edgecount;

	std::map<__int64,DWORD>::iterator it;

	DWORD n=_vol.nPlanes;

	i_math::vector3df dir=plTarget.Normal;

	int state[MAX_VOLUME_PLANES];//0,not defined,1,keep,2,discard
	memset(state,0,sizeof(state));
	for (int i=0;i<n;i++)
	{
		float d=_vol.planes[i].Normal.dotProduct(dir);
		if (d<=0.000f)
		{//back plane,need keep it
			state[i]=1;
			continue;
		}

		for (int j=0;j<n;j++)
		{
			if (_linkmap[i].test(j))
			{//for all the edges
				i_math::pos2di pt;
				if (i<j)
					pt.set(i,j);
				else
					pt.set(j,i);

				it=edgecount.find(FORCE_TYPE(__int64,pt));
				if (it==edgecount.end())
					edgecount[FORCE_TYPE(__int64,pt)]=1;
				else
					(*it).second++;
			}
		}
	}

	volumeCvxf volNew;
	Bitset<1> linkmapNew[MAX_VOLUME_PLANES];

	//the target plane
	volNew.planes[volNew.nPlanes]=plTarget;
	volNew.nPlanes++;

	//Now record all the need-kept planes
	int remap[MAX_VOLUME_PLANES];
	for (int i=0;i<n;i++)
	{
		if (state[i]==1)
		{
			remap[i]=volNew.nPlanes;
			volNew.planes[volNew.nPlanes]=_vol.planes[i];
			volNew.nPlanes++;
		}
	}

	//link the kept planes to each other
	for (int i=0;i<n;i++)
	{
		if (state[i]==1)
		{
			for (int j=0;j<n;j++)
			{
				if ((state[j]==1)&&(_linkmap[i].test(j)))//if j is kept and originally linked to i
					linkmapNew[remap[i]].set(remap[j]);
			}
		}
	}

	std::vector<DWORD>newplanes;

	for (it=edgecount.begin();it!=edgecount.end();it++)
	{
		if ((*it).second==1)
		{//need add a new plane for this edge
			i_math::pos2di pt=FORCE_TYPE(i_math::pos2di,(*it).first);

			int iKept,iDiscard;
			iKept=pt.x;
			iDiscard=pt.y;
			if (state[iKept]!=1)
				Swap<int>(iKept,iDiscard);
			assert(state[iKept]==1);
			assert(state[iDiscard]!=1);


			i_math::plane3df plNew;
			if (TRUE)//Make a new plane with this edge and the extruding dir
			{
				i_math::vector3df n,v;
				if (_vol.planes[iKept].getIntersectionWithPlane(_vol.planes[iDiscard],v,n))
				{
					n=dir.crossProduct(n);
					plNew.setPlane(v,n);
				}
				else
					assert(FALSE);
			}


			newplanes.push_back(volNew.nPlanes);//record new added this plane
			volNew.planes[volNew.nPlanes]=plNew;

			//the new added plane MUST link to the corresponding back plane
			linkmapNew[remap[iKept]].set(volNew.nPlanes);
			linkmapNew[volNew.nPlanes].set(remap[iKept]);

			//and link to the target plane
			linkmapNew[volNew.nPlanes].set(0);
			linkmapNew[0].set(volNew.nPlanes);

			volNew.nPlanes++;
		}
	}

	_vol=volNew;
	memcpy(&_linkmap,&linkmapNew,sizeof(_linkmap));

	//now link the new added planes to each other
	for (int i=0;i<newplanes.size();i++)
	for (int j=0;j<newplanes.size();j++)
	{
		if (i>=j)
			continue;

		i_math::line3df edge;
		if (_GetPlaneEdge(newplanes[i],newplanes[j],edge))
		{
			_linkmap[newplanes[i]].set(newplanes[j]);
			_linkmap[newplanes[j]].set(newplanes[i]);
		}
	}

	_bFrameDirty=TRUE;

	return TRUE;
}

//v should be outside the volume,and the current volume should not be empty
BOOL CCvxVolume::AddCorner(i_math::vector3df &v)
{
	return FALSE;
}
