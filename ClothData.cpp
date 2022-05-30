/********************************************************************
	created:	2019/02/01   11:21
	author:		cxi
d*********************************************************************/

#include "stdh.h"

#include "ClothData.h"


   
#include "datapacket/DataPacket.h"

#include <assert.h>


//////////////////////////////////////////////////////////////////////////
//ClothData
IMPLEMENT_CLASS(ClothData);


ClothData::ClothData()
{
	Zero();
}
ClothData::~ClothData()
{
	Clean();
}

void ClothData::Zero()
{
}

void ClothData::Clean()
{
}

ResType ClothData::GetType()
{
	return Res_Cloth;
}
const char *ClothData::GetTypeName()
{
	return "Cloth";
}

void ClothData::CalcContent(std::string &s)
{
	s="n/a";
}

