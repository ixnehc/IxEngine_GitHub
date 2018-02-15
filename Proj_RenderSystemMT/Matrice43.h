#pragma once

#include "RenderSystem/ITools.h"

//#include "Base.h"
#include "ResourceBase.h"

#include "class/class.h"



class CDeviceObject;


class CMatrice43:public IMatrice43
{
public:
	IMPLEMENT_REFCOUNT_C;
	DEFINE_CLASS(CMatrice43)
	CMatrice43();

	//interfaces
	DWORD GetVer()	{		return _ver;	}
	BOOL Set(matrix43f &mat);
	BOOL Set(matrix43f *mats,DWORD count);
	DWORD GetCount()	{		return _mats.size();	}
	BOOL SetCount(DWORD count);
	matrix43f * GetPtr()//get the ptr for reading(will NOT increase version)
	{
		if (_mats.size()<=0)
			return NULL;
		return &_mats[0];
	}
	matrix43f *QueryPtr()//query the ptr to modify(will increase version)
	{
		if (_mats.size()<=0)
			return NULL;
		_IncVer();
		return &_mats[0];
	}

protected:
	void _IncVer()	{		_ver++;	}
	std::vector<matrix43f> _mats;
	DWORD _ver;
};

