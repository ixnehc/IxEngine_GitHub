#pragma once

#include "RenderSystem/ITools.h"

//#include "base.h"

#include "class/class.h"

class CDeviceObject;
class CShaderMgr;
class CRenderSystem;

class CLight:public ILight
{
public:
	CLight();
	BOOL Init();
	void UnInit();


	IMPLEMENT_REFCOUNT_C;
	DEFINE_CLASS(CLight)
	virtual LightMode GetMode()	{		return _info.mode;	}
	virtual LightFlag GetFlag()	{		return _flag;	}
	virtual void SetFlag(LightFlag flag)	{		_flag=flag;	}
	virtual void ModFlag(LightFlag flagToAdd,LightFlag flagToRemove);

	virtual FeatureCode *GetFC(DWORD &nFC);//the first is intended one,the others are fallbacks

	virtual void SetDirLight(vector3df &dir,DWORD amb,DWORD diff,DWORD spec);
	virtual LightInfo *QueryInfo()	{		return &_info;	}
	virtual LightInfo *GetInfo()	{		return &_info;	}

	virtual void Clone(ILight *lgtSrc);

	virtual void SetShadowMap(ITexture *shamap);


	virtual BOOL Bind(IShader *shader,DWORD iSlot);
protected:
	//Note:when adding members,donot forget add corresponding code in Clone(..)
	LightInfo _info;
	LightFlag _flag;
	ITexture *_shamap;//shadow map
};
