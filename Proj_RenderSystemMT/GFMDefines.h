#pragma once

#define KB 1024
#define GFM_WIDTH 32
#define GFM_SIZE  GFM_WIDTH*GFM_WIDTH 

#include "math/rect.h"

struct GFM
{
	i_math::recti rc;

	BYTE  map_GFM[GFM_SIZE];
};

struct GFM_FileHeader
{
	GFM_FileHeader()   {}
	virtual ~GFM_FileHeader()  {}

	enum {GFM_HeaderSize = 32*KB };
	
	struct GFMItem
	{
		WORD  code;		// unicode
		DWORD off_GFM;  // GFM offset
	};

	DWORD   count;		//the number of GFM
};


