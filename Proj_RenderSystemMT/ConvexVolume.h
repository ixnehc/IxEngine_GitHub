#pragma once

#include "RenderSystem/ITools.h"

#include "base.h"

#include "bitset/bitset.h"

#include "class/class.h"


class CDeviceObject;
class CShaderMgr;
class CRenderSystem;


class CCvxVolume:public ICvxVolume
{
public:
	CCvxVolume();

	IMPLEMENT_REFCOUNT_C;
	DEFINE_CLASS(CCvxVolume)

	virtual BOOL BuildFromCamera(ICamera *cam);

	virtual BOOL GetVolume(i_math::volumeCvxf &vol);
	virtual BOOL SetVolume(i_math::volumeCvxf &vol);

	virtual BOOL ExtrudeToPlane(i_math::plane3df &pl);
   
	//v should be outside the volume,and the current volume should not be empty
	virtual BOOL AddCorner(i_math::vector3df &v);

	virtual BOOL GetFrame(i_math::vector3df *&corners,DWORD &nCorner,
														DWORD *&edges,DWORD &nEdge);
	virtual BOOL GetCorners(i_math::vector3df *&corners,DWORD &nCorner);

protected:
	i_math::volumeCvxf _vol;

	BOOL _GetPlaneEdge(DWORD pl1,DWORD pl2,i_math::line3df &edge);

	void _CalcFrame();

	Bitset<1> _linkmap[MAX_VOLUME_PLANES];

	BOOL _bFrameDirty;

	std::vector<i_math::vector3df> _corners;

	struct _Edge
	{
		DWORD corner01:10;
		DWORD corner02:10;
		DWORD plane01:6;
		DWORD plane02:6;
	};
	std::vector<_Edge> _edges;


};
