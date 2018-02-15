#pragma once



//D3DCOLOR to D3DCOLORVALUE
#define D3DColorToValue(c,v)\
{\
	(v).a=((float)(((c)&0xff000000)>>24))/255.0f;\
	(v).r=((float)(((c)&0x00ff0000)>>16))/255.0f;\
	(v).g=((float)(((c)&0x0000ff00)>>8))/255.0f;\
	(v).b=((float)(((c)&0x000000ff)))/255.0f;\
}

#ifndef ColorAlpha
#define ColorAlpha(c,a) (((c)&0x00ffffff)|(a)<<24)
#endif

#ifdef MULTITHREAD_D3D
#include "d3d9MTBase.h"
#else
#include "d3d9Base.h"
#endif




struct DeviceSettings
{
	UINT AdapterOrdinal;
	D3DDEVTYPE DeviceType;
	D3DFORMAT AdapterFormat;
	DWORD BehaviorFlags;
	D3DPRESENT_PARAMETERS pp;
};


struct ViewportInfo 
{
	DWORD  x;
	DWORD  y;            /* Viewport Top left */
	DWORD  w;
	DWORD  h;       /* Viewport Dimensions */
	float minz;         /* Min/max of clip Volume */
	float maxz;
};

