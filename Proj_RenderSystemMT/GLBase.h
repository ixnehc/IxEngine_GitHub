#pragma once

#include "EGL/egl.h"
#include "GLES2/gl2.h"




struct ViewportInfoGL
{
	DWORD  x;
	DWORD  y;            /* Viewport Top left */
	DWORD  w;
	DWORD  h;       /* Viewport Dimensions */
	float minz;         /* Min/max of clip Volume */
	float maxz;
};

#define GLHandle_Null (0)

typedef GLuint GLTexHandle;
typedef GLuint GLFrameBufHandle;
typedef GLuint GLRenderBufHandle;
typedef GLuint GLVertexBufHandle;
