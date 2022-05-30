#pragma once

#include <string>
#include <vector>


#include "ResData.h"


class CDataPacket;
struct ClothData:public ResData
{
	DECLARE_CLASS(ClothData);

	ClothData();
	virtual ~ClothData();
	void Zero();
	void Clean();


	//Overriding
	virtual 	ResType GetType();
	virtual const char *GetTypeName();
	virtual const char *GetTypeSuffix()	{		return "cth";	}
	virtual void CalcContent(std::string &s);
	virtual void Save(CDataPacket &dp){}
	virtual void Load(CDataPacket &dp){}
	virtual void SaveHeader(CDataPacket &dp){}
	virtual void LoadHeader(CDataPacket &dp){}



};

