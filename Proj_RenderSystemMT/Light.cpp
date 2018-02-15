/********************************************************************
	created:	2006/10/23   12:41
	filename: 	e:\IxEngine\Proj_RenderSystem\Light.cpp
	author:		CXI
	
	purpose:	light implement
*********************************************************************/
#include "stdh.h"
#include "Base.h"
#include "interface/interface.h"

#include "RenderSystem/ITexture.h"
#include "RenderSystem/IShader.h"

#include "shaderlib/SLFeature.h"

#include "Light.h"

#include "Log/LastError.h"
#include "Log\LogFile.h"

#include "assert.h"


CLight::CLight()
{
	_shamap=NULL;
	_flag=(LightFlag)0;
}

BOOL CLight::Init()
{
	SetDirLight(vector3df(0,0,1),0x00000000,0xffffffff,0xffffffff);
	return TRUE;
}
void CLight::UnInit()
{
	SAFE_RELEASE(_shamap);
}

void CLight::ModFlag(LightFlag flagToAdd,LightFlag flagToRemove)
{
	((DWORD &)_flag)|=(DWORD)flagToAdd;
	((DWORD &)_flag)&=~(DWORD)flagToRemove;
}


void CLight::SetDirLight(vector3df &dir,DWORD amb,DWORD diff,DWORD spec)
{
	_info.mode=LM_Dir;
	_info.dir=dir;
	_info.amb.fromDwordColor(amb);
	_info.diff.fromDwordColor(diff);
	_info.spec.fromDwordColor(spec);
}

void CLight::SetShadowMap(ITexture *shamap)
{
	SAFE_RELEASE(_shamap);
	_shamap=shamap;
	_shamap->AddRef();
}


BOOL CLight::Bind(IShader *shader,DWORD iSlot)
{
	BOOL bRet=TRUE;
	if (_info.mode==LM_Dir)
	{
		shader->SetEP(EP_dirDL,_info.dir,iSlot);
		shader->SetEP(EP_ambDL,_info.amb,iSlot);
		shader->SetEP(EP_difDL,_info.diff,iSlot);
		shader->SetEP(EP_specDL,_info.spec,iSlot);
	}

	return bRet;
}

#define RET_LIGHT_FC(fcArray)\
{\
	nFC=ARRAY_SIZE(fcArray);\
	return fcArray;\
}

//the first is intended one,the others are fallbacks
FeatureCode *CLight::GetFC(DWORD &nFC)
{
	static FeatureCode fcDL=FC_dirlight_p;
	if (_info.mode==LM_Dir)
	{
		nFC=1;
		return &fcDL;
	}

	return NULL;
}

void CLight::Clone(ILight *lgtSrc0)
{
	CLight *l=(CLight*)lgtSrc0;
	_info=l->_info;
	_flag=l->_flag;
	_shamap=l->_shamap;
	SAFE_ADDREF(_shamap);
}
