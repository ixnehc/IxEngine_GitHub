/********************************************************************
	created:	2008/08/12   16:25
	filename: 	d:\IxEngine\Proj_RenderSystemMT\d3d9A\Direct3DSurfaceA.cpp
	author:		cxi
	
	purpose:	Surface
*********************************************************************/


#include "stdh.h"
#include "Direct3DDeviceA.h"
#include "Direct3DSurfaceA.h"






HRESULT CDirect3DSurface::GetDevice(THIS_ CDirect3DDevice** ppDevice)
{
	*ppDevice=_dev;
	SAFE_ADDREF(_dev);
	return D3D_OK;
}


