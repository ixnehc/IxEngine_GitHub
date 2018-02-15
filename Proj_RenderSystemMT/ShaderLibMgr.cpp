/********************************************************************
	created:	2006/10/17   12:39
	filename: 	e:\IxEngine\Proj_RenderSystem\ShaderLibMgr.cpp
	author:		cxi
	
	purpose:	shader lib manager,a resource manager to manage IShader
*********************************************************************/
#include "stdh.h"
#include "RenderSystem.h"

#include "DeviceObject.h"

#include "ResourceBase.h"

#include "ShaderLibMgr.h"


#include "timer/timer.h"
#include "timer/profiler.h"

#include "stringparser/stringparser.h"
#include "Log/LogFile.h"

#include "shaderlib/ShaderLib.h"
#include "shaderlib/SLComposer.h"

#include "math/matrix44.h"
#include "math/matrix43.h"
#include <assert.h>


#pragma warning(disable:4018)

extern LogFile g_logResMgr;

LogFile g_logFX("fx");


//////////////////////////////////////////////////////////////////////////
//CShader
IMPLEMENT_CLASS(CShader);

BOOL CShader::_OnTouch(IRenderSystem *pRS)
{
	return ((CShaderLibMgr *)_pMgr)->_TouchShader(this);
}

void CShader::_OnUnload()
{
	_UnTouch();
}


void CShader::_UnTouch()
{
	_sFX="";
	SAFE_RELEASE_D3DRES(_effect);
	for (int i=0;i<SCC_max;i++)
		_caps[i].SetCode(SCC_none);
	_UnloadEPH();
}

void CShader::_OnDeviceLost()
{
	_UnTouch();

	CResource::SetState(CResource::Abandoned);
}

void CShader::_OnDeviceReset()
{
	CResource::SetState(CResource::Loaded);
}

BOOL CShader::CheckValid()
{
	Touch();
	if (!_effect)
		return FALSE;

#ifndef MULTITHREAD_D3D
	if (_effect)
	{
		D3DXHANDLE h;
		if (D3D_OK!=_effect->FindNextValidTechnique(NULL,&h))
			return FALSE;
	}
#endif

	return TRUE;
}




int CShader::GetCap_maxbones()
{
	if (_caps[SCC_maxbones].code==SCC_maxbones)
		return _caps[SCC_maxbones].value;
	return 0;
}

int CShader::GetCap_weightcount()
{
	if (_caps[SCC_weightcount].code==SCC_weightcount)
		return _caps[SCC_weightcount].value;
	return 0;
}

void CShader::_UnloadEPH()
{
	memset(_eph_buffer,0,sizeof(_eph_buffer));
	memset(_eph,0,sizeof(_eph));

	_eph2.clear();
}

//call this after _effect is set
void CShader::_LoadEPH()
{
	assert(_effect);
	if (!_effect)
		return;

	_UnloadEPH();
	extern DWORD SizeOfEPInfo();
	extern const char *EPInfo_GetEPName(DWORD idx);
	extern EffectParam EPInfo_GetEP(DWORD idx);
	extern int EPInfo_GetArraySize(DWORD idx);

	D3DXHANDLE hTec=_effect->GetTechnique(0);

	DWORD iBuffer=0;
	for (int i=0;i<SizeOfEPInfo();i++)
	{
		EPH &eph=_eph[EPInfo_GetEP(i)];

		const char *name=EPInfo_GetEPName(i);
		D3DXHANDLE t=_effect->GetParameterByName(NULL,name);
		if(!t)
			continue;
		if (!_effect->IsParameterUsed(t,hTec))
			continue;
		eph.handles=&_eph_buffer[iBuffer];
		eph.handle=t;

		DWORD arraysize;
		arraysize=EPInfo_GetArraySize(i);
		if (arraysize==1)
		{
			_eph_buffer[iBuffer]=t;
			iBuffer++;
			eph.count++;
			continue;
		}

		for (int j=0;j<arraysize;j++)
		{
			D3DXHANDLE t2=_effect->GetParameterElement(t,j);
			if (!t2)
			{
				if (j==0)
				{
					_eph_buffer[iBuffer]=t;
					iBuffer++;
					eph.count++;
				}
				break;
			}

			_eph_buffer[iBuffer]=t2;
			iBuffer++;
			eph.count++;
		}
	}

	assert(iBuffer<ARRAY_SIZE(_eph_buffer));

	//eph2
	if (TRUE)
	{
		CShaderLibMgr *mgr=(CShaderLibMgr *)GetMgr();
		MteEntry *entry=mgr->_FindMteEntry(_sc.idMte);
		if (entry)
		{
			for (int i=0;i<entry->eps.size();i++)
			{
				StringID nm=entry->eps[i];
				D3DXHANDLE t=_effect->GetParameterByName(NULL,entry->epnames[i].c_str());
				if(!t)
					continue;
				if (!_effect->IsParameterUsed(t,hTec))
					continue;
				_eph2[nm]=t;
			}
		}
	}
	
}


#define PARSE_EPH(handle)									\
{																				\
	if (!_effect)															\
		return EPMissing;											\
	assert(((DWORD)ep)<EP_Max);							\
	if (!_eph[ep].handles)											\
		return EPMissing;											\
	if (idx>=_eph[ep].count)									\
		return EPMissing;											\
	if (!((handle)=_eph[ep].handles[idx]))				\
		return EPMissing;											\
}

#define PARSE_EPH2(handle)																					\
{																																	\
	if (!_effect)																												\
		return EPMissing;																								\
	std::hash_map<StringID,D3DXHANDLE>::iterator it=_eph2.find(nm);				\
	if (it==_eph2.end())																								\
		return EPMissing;																								\
	if (!(handle=(*it).second))																						\
		return EPMissing;																								\
}

BOOL CShader::ExistEP(EffectParam ep,DWORD idx)
{
	D3DXHANDLE h;
	PARSE_EPH(h);

	return TRUE;
}

BOOL CShader::ExistEP(StringID epname)
{
	if (!_effect)
		return FALSE;
	std::hash_map<StringID,D3DXHANDLE>::iterator it=_eph2.find(epname);
	if (it==_eph2.end())
		return FALSE;
	return TRUE;
}


EPResult CShader::SetEP(EffectParam ep,color4df &c,DWORD idx)
{
	D3DXHANDLE h;
	PARSE_EPH(h);
	return (D3D_OK==_effect->SetFloatArray(h,(float*)&c,4))?EPOk:EPFail;
}
EPResult CShader::SetEP(EffectParam ep,vector3df &v,DWORD idx)
{
	D3DXHANDLE h;
	PARSE_EPH(h);
	return (D3D_OK==_effect->SetFloatArray(h,(float*)&v,3))?EPOk:EPFail;
}

EPResult CShader::SetEP(EffectParam ep,vector2df &v,DWORD idx)
{
	D3DXHANDLE h;
	PARSE_EPH(h);
	return (D3D_OK==_effect->SetFloatArray(h,(float*)&v,2))?EPOk:EPFail;
}


EPResult CShader::SetEP(EffectParam ep,ITexture *tex,DWORD idx)
{
	D3DXHANDLE h;
	PARSE_EPH(h);

	XDirect3DBaseTexture* p=NULL;
	if (tex)
	{
		if (A_Ok==tex->Touch())
			p=(XDirect3DBaseTexture*)tex->GetTex();
	}
	if (!p)
	{
		tex=((CShaderLibMgr*)GetMgr())->GetVoidTex();
		p=(XDirect3DBaseTexture*)tex->GetTex();
	}

	return (D3D_OK==_effect->SetTexture(h,p))?EPOk:EPFail;
}


EPResult CShader::SetEP(EffectParam ep,f32 f,DWORD idx)
{
	D3DXHANDLE h;
	PARSE_EPH(h);
	return (D3D_OK==_effect->SetFloat(h,f))?EPOk:EPFail;
}

EPResult CShader::SetEP(EffectParam ep,i_math::s32 s,DWORD idx)
{
	D3DXHANDLE h;
	PARSE_EPH(h);
	return (D3D_OK==_effect->SetInt(h,s))?EPOk:EPFail;
}

EPResult CShader::SetEP(EffectParam ep,i_math::vector4di &v,DWORD idx)
{
	D3DXHANDLE h;
	PARSE_EPH(h);
	return (D3D_OK==_effect->SetIntArray(h,(int*)&v,4))?EPOk:EPFail;
}


EPResult CShader::SetEP(EffectParam ep,i_math::matrix43f &mat,DWORD idx)
{
	D3DXHANDLE h;
	PARSE_EPH(h);
	return (D3D_OK==_effect->SetFloatArray(h,(float*)&mat,12))?EPOk:EPFail;
}

EPResult CShader::SetEP(EffectParam ep,i_math::matrix44f &mat,DWORD idx)
{
	D3DXHANDLE h;
	PARSE_EPH(h);
	return (D3D_OK==_effect->SetFloatArray(h,(float*)&mat,16))?EPOk:EPFail;
}

EPResult CShader::SetEP(EffectParam ep,float *fs,DWORD count)
{
	assert(ep<EP_Max);
	if (!_eph[ep].handle)
		return EPMissing;

	return D3D_OK==_effect->SetFloatArray(_eph[ep].handle,fs,count)?EPOk:EPFail;
}

EPResult CShader::SetEP(EffectParam ep,i_math::vector4df *data,DWORD count)
{
	assert(ep<EP_Max);
	if (!_eph[ep].handle)
		return EPMissing;

	for (int i=0;i<count;i++)
		_effect->SetVector(_eph[ep].handles[i],&((D3DXVECTOR4*)data)[i]);
	return EPOk;
//	return D3D_OK==_effect->SetVectorArray(_eph[ep].handle,(D3DXVECTOR4*)data,count)?EPOk:EPFail;
}




EPResult CShader::SetEP_World(matrix43f *mats,DWORD count)
{
	if (!_effect)
		return EPMissing;
	if (!_eph[EP_world].handles)
	{//for fixed function
		XDirect3DDevice *device;
		device=(XDirect3DDevice *)_devobj->GetDevice();
		for (int i=0;i<count;i++)
		{
			matrix44f mat;
			mat44from43(mat,mats[i]);
			device->SetTransform(D3DTS_WORLDMATRIX(i),(D3DMATRIX*)(&mat));
		}
		return EPOk;
	}
//	for (int i=0;i<count;i++)
//		_effect->SetFloatArray(_eph[EP_world].handles[i],)
	return (D3D_OK==_effect->SetFloatArray(_eph[EP_world].handle,(float*)mats,12*count))?EPOk:EPFail;
}


EPResult CShader::SetEP_ViewProj(i_math::matrix44f &viewproj,i_math::matrix44f &view,i_math::matrix44f &proj)
{
	if (!_effect)
		return EPMissing;

	if (_eph[EP_view].handles)
		_effect->SetMatrix(*_eph[EP_view].handles,(D3DXMATRIX *)&view);
	if (_eph[EP_proj].handles)
		_effect->SetMatrix(*_eph[EP_proj].handles,(D3DXMATRIX *)&proj);

	if (_eph[EP_viewproj].handles)
		return (D3D_OK==_effect->SetMatrix(*_eph[EP_viewproj].handles,(D3DXMATRIX *)&viewproj))?EPOk:EPFail;
	return EPMissing;
}

EPResult CShader::SetEP(StringID nm,color4df &c)
{
	D3DXHANDLE h;
	PARSE_EPH2(h);
	return (D3D_OK==_effect->SetFloatArray(h,(float*)&c,4))?EPOk:EPFail;
}
EPResult CShader::SetEP(StringID nm,vector3df &v)
{
	D3DXHANDLE h;
	PARSE_EPH2(h);
	return (D3D_OK==_effect->SetFloatArray(h,(float*)&v,3))?EPOk:EPFail;
}

EPResult CShader::SetEP(StringID nm,vector2df &v)
{
	D3DXHANDLE h;
	PARSE_EPH2(h);
	return (D3D_OK==_effect->SetFloatArray(h,(float*)&v,2))?EPOk:EPFail;
}


EPResult CShader::SetEP(StringID nm,ITexture *tex)
{
	D3DXHANDLE h;
	PARSE_EPH2(h);
	if (tex)
	{
		if (A_Ok!=tex->Touch())
		{
			_effect->SetTexture(h,NULL);
			return EPFail;
		}
		return (D3D_OK==_effect->SetTexture(h,(XDirect3DBaseTexture*)tex->GetTex()))?EPOk:EPFail;
	}
	else
		return (D3D_OK==_effect->SetTexture(h,(XDirect3DBaseTexture*)NULL))?EPOk:EPFail;
}


EPResult CShader::SetEP(StringID nm,f32 f)
{
	D3DXHANDLE h;
	PARSE_EPH2(h);
	return (D3D_OK==_effect->SetFloat(h,f))?EPOk:EPFail;
}

EPResult CShader::SetEP(StringID nm,i_math::s32 s)
{
	D3DXHANDLE h;
	PARSE_EPH2(h);
	return (D3D_OK==_effect->SetInt(h,s))?EPOk:EPFail;
}

EPResult CShader::SetEP(StringID nm,i_math::vector4di &v)
{
	D3DXHANDLE h;
	PARSE_EPH2(h);
	return (D3D_OK==_effect->SetIntArray(h,(int*)&v,4))?EPOk:EPFail;
}


EPResult CShader::SetEP(StringID nm,i_math::matrix43f &mat)
{
	D3DXHANDLE h;
	PARSE_EPH2(h);
	return (D3D_OK==_effect->SetFloatArray(h,(float*)&mat,12))?EPOk:EPFail;
}

EPResult CShader::SetEP(StringID nm,i_math::matrix44f &mat)
{
	D3DXHANDLE h;
	PARSE_EPH2(h);
	return (D3D_OK==_effect->SetFloatArray(h,(float*)&mat,16))?EPOk:EPFail;
}



EPResult CShader::SetEPs(BYTE *data,DWORD nEP)
{
	EPResult ret=EPOk;
	EPResult t;
	for (int i=0;i<nEP;i++)
	{
		EpkHead*p=(EpkHead*)data;
		data+=sizeof(EpkHead);

#define SetEP_Var(T)																									\
		if(p->ep<EP_Max)																								\
		{																															\
			t=SetEP((EffectParam)p->ep,*(T*)data,0);													\
			if ((int)t<(int)ret)																								\
				ret=t;																											\
			assert(sizeof(T)==p->sz);																				\
			data+=p->sz;																									\
		}																															\
		else																														\
		{																															\
			StringID *nm=(StringID*)(data+p->sz-sizeof(StringID));							\
			t=SetEP(*nm,*(T*)data);																					\
			if ((int)t<(int)ret)																								\
				ret=t;																											\
			assert(sizeof(T)==p->sz-sizeof(StringID));													\
			data+=p->sz;																									\
		}

		switch(p->type)
		{
		case GVT_S:
			SetEP_Var(int);
			break;
		case GVT_F:
		{
			if (p->ep<EP_Max)
			{
				if (p->sz==sizeof(float))
				{
					SetEP_Var(float);
				}
				else
				{
					t=SetEP((EffectParam)p->ep,(float*)data,p->sz/sizeof(float));
					if ((int)t<(int)ret)
						ret=t;
					data+=p->sz;
				}
			}
			else
			{
				if (p->sz==sizeof(float)+sizeof(StringID))
				{
					SetEP_Var(float);
				}
				else
				{
					assert(FALSE);//目前不支持设一个float数组到以StringID命名的effect param中
					data+=p->sz;
				}
			}
			break;
		}
		case GVT_Fx2:
			SetEP_Var(i_math::vector2df);
			break;
		case GVT_Fx3:
			SetEP_Var(i_math::vector3df);
			break;
		case GVT_Fx4:
			SetEP_Var(i_math::vector4df);
			break;
		case GVT_Fx12:
			SetEP_Var(i_math::matrix43f);
			break;
		case GVT_Sx4:
			SetEP_Var(i_math::vector4di);
			break;
		case GVT_Fx16:
			SetEP_Var(i_math::matrix44f);
			break;
		case GVT_String:
			SetEP_Var(ITexture*);
			break;
			//XXXXX more effect params
			//XXXXX: more GVarType

		default:
			assert(FALSE);
		}

	}

	return ret;
}


BOOL CShader::BeginRaw()
{
	assert(_devobj);
	if (!_effect)
		return FALSE;

	UINT nPass;
	if (D3D_OK!=_effect->Begin(&nPass, D3DXFX_DONOTSAVESTATE))
		return FALSE;
	if(nPass!=1)
		goto _fail;

	if (D3D_OK!=_effect->BeginPass(0))
		goto _fail;

	return TRUE;

_fail:
	_effect->End();
	return FALSE;

	
}

void CShader::EndRaw()
{
	_effect->EndPass();
	_effect->End();
}


void CShader::_SetAlphaTest(AlphaTestMode mode,DWORD ref)
{
	switch(mode)
	{
		case AlphaTest_Disable:
			SetEP(EP_alphatestref,0.0f);
			break;
		case AlphaTest_Standard:
		{
			if (ref<=0)
				SetEP(EP_alphatestref,0.0f);
			else
				SetEP(EP_alphatestref,(float)ref/255.0f);
			break;
		}
	}
}

void CShader::SetDepthBias(float biasSlope,float bias)
{
	_devobj->SetRenderState(D3DRS_DEPTHBIAS,*(DWORD*)&bias);
	_devobj->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS,*(DWORD*)&biasSlope);
}





//////////////////////////////////////////////////////////////////////////
//CShaderLibMgr

BOOL CShaderLibMgr::Init(IRenderSystem *pRS,const char *name)
{
	if (FALSE==CResourceMgr::Init(pRS,name))
		return FALSE;

	_LoadCache(pRS);

	_devobj=((CRenderSystem*)pRS)->GetDeviceObj();

	return TRUE;
}

void CShaderLibMgr::UnInit()
{
	CResourceMgr::UnInit();

	SAFE_RELEASE(_texVoid);

	for (int i=0;i<_vecLibs.size();i++)
	{
		_vecLibs[i]->Clean();
		delete _vecLibs[i];
	}
	_vecLibs.clear();

	_UnloadCache();

	_devobj=NULL;
}

void CShaderLibMgr::OnDeviceLost()
{
	std::map<ShaderCode,CShader*>::iterator it;
	for (it=_mapShader.begin();it!=_mapShader.end();it++)
	{
		CShader*p=(*it).second;
		p->_OnDeviceLost();
	}
}

void CShaderLibMgr::OnDeviceReset()
{
	std::map<ShaderCode,CShader*>::iterator it;
	for (it=_mapShader.begin();it!=_mapShader.end();it++)
	{
		CShader*p=(*it).second;
		p->_OnDeviceReset();
	}
}


//Clean all the resource
void CShaderLibMgr::_UnloadAll()
{
	std::map<ShaderCode,CShader*>::iterator it;
	for (it=_mapShader.begin();it!=_mapShader.end();it++)
	{
		CShader*p=(*it).second;
		p->_OnUnload();
		Class_Delete(p);
	}
	_mapShader.clear();

	//清除mtrl features
	if (TRUE)
	{
		std::hash_map<MteID,MteEntry*>::iterator it;
		for (it=_mtes.begin();it!=_mtes.end();it++)
		{
			MteEntry *entry=(*it).second;
			if (entry)
			{
				entry->feature.Clean();
				Class_Delete(entry);
			}
		}
		_mtes.clear();
	}
}



//Check whether there is any resource left not released in the mgr
BOOL CShaderLibMgr::CheckResLeak()
{
	BOOL bLeak=FALSE;
	std::map<ShaderCode,CShader*>::iterator it;
	for (it=_mapShader.begin();it!=_mapShader.end();it++)
	{
		CShader*p=(*it).second;
		if (p->GetRef()>=1)
		{
			bLeak=TRUE;
			g_logResMgr.Dump("Res Leak found in [%s]:%s!",_name.c_str(),((FeatureCode&)((*it).first)).ToName());
		}
	}
	return bLeak;
}


extern BOOL LoadShaderLib(IFile *fl,CShaderLib &slib);

CShaderLib *CShaderLibMgr::_LoadShaderLib(const char *path)
{
	if (!CheckFileSuffix(path,"slb"))
		return NULL;
	std::string pathAbs=_pRS->GetPath(Path_Effect);
	pathAbs=pathAbs+"\\"+path;
	IFile *fl=_pRS->GetFS()->OpenFileAbs(pathAbs.c_str(),FileAccessMode_Read);
	if (!fl)
		return NULL;

	CShaderLib *slib=new CShaderLib;
	BOOL bRet;
	bRet=LoadShaderLib(fl,*slib);

	fl->Close();

	if (!bRet)
	{
		delete slib;
		return NULL;
	}
	return slib;
}


BOOL CShaderLibMgr::AddShaderLib(const char *path)
{
	CShaderLib *slib=_LoadShaderLib(path);
	if (!slib)
	{
		LOG_DUMP_1P(_name.c_str(),Log_Error,"无法载入ShaderLib:\"%s\"",path);
		return FALSE;
	}

	_vecLibs.push_back(slib);

	return TRUE;
}

BOOL CShaderLibMgr::ReloadShaderLib(const char *path)
{
	CShaderLib *slib=_LoadShaderLib(path);
	if (!slib)
	{
		LOG_DUMP_1P(_name.c_str(),Log_Error,"重载入ShaderLib时,无法载入ShaderLib:\"%s\"",path);
		return FALSE;
	}

	std::string name=slib->GetName();
	for (int i=0;i<_vecLibs.size();i++)
	{
		if (name==_vecLibs[i]->GetName())
		{
			//先替换原来的lib
			_vecLibs[i]->Clean();
			delete _vecLibs[i];
			_vecLibs[i]=slib;

			//清除cache
			_UnloadCache();

			//Re-touch所有这个lib里的shader
			std::map<ShaderCode,CShader*>::iterator it;
			for (it=_mapShader.begin();it!=_mapShader.end();it++)
			{
				ShaderCode sc=(*it).first;
				if (sc.GetLibIdx()==i)
				{
					CShader *shader=(*it).second;
					shader->_UnTouch();
					shader->CResource::SetState(CResource::Loaded);
					shader->Touch();
				}
			}

			return TRUE;
		}
	}

	slib->Clean();
	delete slib;
	return FALSE;
}

BOOL CShaderLibMgr::RegisterMte(MtrlExtData *data,MteID id,const char *pathMte)
{
	MteEntry *entry=_FindMteEntry(id);
	if (entry)
	{
		//已经注册过了,我们就不注册了
		return TRUE;
	}

	entry=Class_New2(MteEntry);
	entry->id=id;
	entry->bCompiled=data->bCompiled;
	entry->feature.CopyFrom(data->feature);
	entry->path=pathMte;
	entry->eps.resize(data->epinfo.size());
	entry->epnames.resize(data->epinfo.size());
	for (int i=0;i<data->epinfo.size();i++)
	{
		entry->eps[i]=data->epinfo[i].nmEP;
		entry->epnames[i]=data->epinfo[i].name;
	}

	_mtes[id]=entry;

	return TRUE;
}

void CShaderLibMgr::_UnloadMteShader(MteID idMte)
{
	if (idMte==MteID_Invalid)
		return;
	std::map<ShaderCode,CShader*>::iterator it=_mapShader.begin();
	std::map<ShaderCode,CShader*>::iterator itCur;
	while(it!=_mapShader.end())
	{
		itCur=it;
		it++;

		if (idMte==((*itCur).first).idMte)
		{
			_UnloadCache();

			CShader *shader=(*itCur).second;
			assert(shader->GetRef()<=1);
			shader->_OnUnload();
			Resource_Delete(shader);
			_mapShader.erase(itCur);
		}
	}
}


MteID CShaderLibMgr::AbandonMte(const char *pathMte)
{
	std::hash_map<MteID,MteEntry*>::iterator it;
	for (it=_mtes.begin();it!=_mtes.end();it++)
	{
		MteEntry *entry=(*it).second;
		if (entry)
		{
			if (entry->path==pathMte)
			{
				MteID idMte=entry->id;
				Class_Delete(entry);
				_mtes.erase(it);

				//卸载所有用到这个mte的shader
				_UnloadMteShader(idMte);

				return idMte;
			}
		}
	}
	return MteID_Invalid;
}


ShaderCode CShaderLibMgr::MakeShaderCode(const char *nameSL,FeatureCode &fc,const char *nameUF,MteID idMte)
{
	ShaderCode sc;
	int i;
	for (i=0;i<_vecLibs.size();i++)
	{
		if (strcmp(_vecLibs[i]->GetName(),nameSL)==0)
			break;
	}

	if (i>=_vecLibs.size())
		return sc;//an invalid one

	if (nameUF&&(nameUF[0]!=0))
	{
		int idx=_vecLibs[i]->FindUniFeature(nameUF);
		if (idx==-1)
			return sc;//an invalid one
		sc.SetLibIdx(i);
		sc.SetIdxUF(idx);
		((FeatureCode&)sc)=FeatureCode(0);

		return sc;
	}

	((FeatureCode&)sc)=fc;
	sc.SetLibIdx(i);

	sc.idMte=idMte;

	return sc;
}

CShaderLib*CShaderLibMgr::GetShaderLib(const char *nameSL)
{
	int i;
	for (i=0;i<_vecLibs.size();i++)
	{
		if (strcmp(_vecLibs[i]->GetName(),nameSL)==0)
			break;
	}
	if (i>=_vecLibs.size())
		return NULL;//an invalid one

	return _vecLibs[i];
}


IShader*CShaderLibMgr::ObtainShader(const char *nameSL,const char *nameUF,MteID idMte)
{
	ShaderCode sc;
	sc=MakeShaderCode(nameSL,FeatureCode(0),nameUF,idMte);
	if (!sc.IsValid())
		return NULL;

	return ObtainShader(sc);
}

 
IShader *CShaderLibMgr::ObtainShader(const char *nameSL,FeatureCode &fc,MteID idMte)
{
	ShaderCode sc;
	sc=MakeShaderCode(nameSL,fc,"",idMte);
	if (!sc.IsValid())
		return NULL;

	return ObtainShader(sc);
}

IShader *CShaderLibMgr::ObtainShader(ShaderCode &sc0)
{
	std::map<ShaderCode,CShader*>::iterator it;
	it=_mapShader.find(sc0);
	if (it!=_mapShader.end())
	{
		((*it).second)->AddRef();
		return ((*it).second);
	}

	ShaderCode sc=sc0;
	if (!sc.UFExist())//first add the base feature to this sc if not using unifeature
	{
		BYTE idxLib;
		idxLib=sc.GetLibIdx();
		if (idxLib>=_vecLibs.size())
			return NULL;

		sc.Add(_vecLibs[idxLib]->GetBaseFeatureCode());
	}
	else
		sc.c=0;//for unifeature,clear the features

	CShaderLib *slib=_vecLibs[sc.GetLibIdx()];

	if (!sc.UFExist())
	{
		FeatureCode fc=(FeatureCode&)sc;

		fc.Remove(slib->GetFeatureCode());
		if (!fc.IsEmpty())//the ShaderLib could not support all the features
		{
			LOG_DUMP_2P(_name.c_str(),Log_Warning,"ObtainShader(..)时(%s)发现不存在的feature(%s),它们被忽略了.",
				_MakeShaderCodeName(sc),fc.ToName());
			sc.Remove(fc);
		}

		fc=(FeatureCode&)sc;
		if (slib->CheckFeatureConflict(fc))//check conflicting
		{
			LOG_DUMP_1P(_name.c_str(),Log_Warning,"ObtainShader(..)时(%s)发现存在Feature的冲突,ObtainShader(..)失败",
				_MakeShaderCodeName(sc));
			return NULL;
		}
	}

	CShader *p=NewRes<CShader>();
	p->_effect=NULL;
	p->_devobj=(CDeviceObject *)_devobj;

	for (int i=0;i<SCC_max;i++)
		p->_caps[i].SetCode(SCC_none);

	_mapShader[sc0]=p;//record it
	p->_nRef=1;
	p->_path="";
	p->_sc=sc;
	p->CResource::SetState(CResource::Loaded);

	return p;
}

BOOL CShaderLibMgr::_TouchShader(CShader *p)
{
	ShaderCacheHeader cache;

	ShaderCacheKey key;
	MteEntry *entryMte=NULL;
	if (p->_sc.idMte!=MteID_Invalid)
	{
		entryMte=_FindMteEntry(p->_sc.idMte);
		if (entryMte)
			key.pathMte=entryMte->path;
	}

	key.fc=p->_sc;
	key.idxLib=p->_sc.iLib;
	key.idxUF=p->_sc.idxUF;


	std::hash_map<ShaderCacheKey,ShaderCacheHeader>::iterator it=_cache.find(key);
	if (it==_cache.end())
	{//not in cache,we should build one

		XD3DXEffectCompiler *pCompiler=NULL;
		XD3DXBuffer *effectcode=NULL;

		CShaderLib *slib=_vecLibs[p->_sc.GetLibIdx()];

		std::string sFX;
		std::vector<ShaderCap>caps;
		if (!p->_sc.UFExist())
		{
			CSLComposer composer;
			if (slib->GetTemplate())
			{
				composer.SetTemplate(slib->GetTemplate()->Clone());
				FeatureCode fc=(FeatureCode&)p->_sc;
				SLFeature *f;
				for (int i=0;i<slib->GetFeatureCount();i++)
				{
					f=slib->GetFeature(i);
					if (fc.Test(f->fc))
						composer.AddFeature(f->Clone());
				}

				//加入mte
				BOOL bMteFail=FALSE;
				if(entryMte)
				{
					if (entryMte->bCompiled)
						composer.AddFeature(entryMte->feature.Clone());
					else
						bMteFail=TRUE;
				}
				
				if (!bMteFail)
					composer.ComposeFX(sFX,caps);
				else
					sFX="";
				composer.Clear();
			}
		}
		else
		{
			std::string name;
			slib->GetUniFeature(p->_sc.GetIdxUF(),name,sFX);
			caps.clear();//for UniFeature,caps is not supported
		}

		p->_sFX=sFX;
//		g_logFX.Dump(sFX.c_str());

		DWORD flag=0;//D3DXSHADER_USE_LEGACY_D3DX9_31_DLL;
		XD3DXBuffer *pErrBuffer=NULL;

		BOOL bOk=FALSE;

		if (D3D_OK==XD3DXCreateEffectCompiler(sFX.c_str(),sFX.length(),NULL,NULL,flag,&pCompiler,&pErrBuffer))
		{
			if (D3D_OK==pCompiler->CompileEffect(0,&effectcode,&pErrBuffer))
			{
				ShaderCacheHeader head;
				head.sc=p->_sc;
				if (entryMte)
					head.pathMte=entryMte->path;
				head.sz=effectcode->GetBufferSize();
				for (int i=0;i<caps.size();i++)
					head.caps[i]=caps[i];
				head.nCaps=caps.size();

				_AddCache(head,(BYTE*)effectcode->GetBufferPointer());

				bOk=TRUE;
			}
		}
		if((!bOk)&&pErrBuffer)
			p->_err=(char*)pErrBuffer->GetBufferPointer();

		SAFE_RELEASE(pErrBuffer);

		SAFE_RELEASE(pCompiler);
		SAFE_RELEASE(effectcode);
	}
	else
	{
		ShaderCacheHeader &cache=(*it).second;
		if (cache.idxData==-1)
		{//读入buffer中的数据
			_flData->Seek(cache.start);
			int sz=_bufferCache.size();
			_bufferCache.resize(sz+cache.sz);
			_flData->Read(&_bufferCache[0]+sz,cache.sz);
			cache.idxData=sz;
		}
	}

	it=_cache.find(key);
	if (it!=_cache.end())
	{
		ShaderCacheHeader &cache=(*it).second;

		XD3DXBuffer *pErrBuffer=NULL;

		XDirect3DDevice*device=(XDirect3DDevice*)_devobj->GetDevice();
		if (cache.idxData>=0)
		{
			if (D3D_OK!=XD3DXCreateEffect(device,&_bufferCache[cache.idxData],cache.sz,
									NULL,NULL,D3DXFX_NOT_CLONEABLE,NULL,&p->_effect,&pErrBuffer))
			{
				p->_effect=NULL;
				if (pErrBuffer)
					p->_err=(char*)pErrBuffer->GetBufferPointer();
			}
			else
			{
				for (int i=0;i<cache.nCaps;i++)
					p->_caps[cache.caps[i].code]=cache.caps[i];
			}
		}
		SAFE_RELEASE(pErrBuffer);
	}

	if (!p->_effect)
	{
		LOG_DUMP_1P(_name.c_str(),Log_Error,"Shader载入失败--%s !",_MakeShaderCodeName(p->_sc));
		if (!p->_err.empty())
		{
			LOG_DUMP_1P(_name.c_str(),Log_Error,"	编译错误:%s !",p->_err.c_str());
		}
		if (!p->_sFX.empty())
			g_logFX.Dump(p->_sFX.c_str());
		return FALSE;
	}

	p->_LoadEPH();

	return TRUE;
}

const char *CShaderLibMgr::GetLibName(DWORD iLib)
{
	if (iLib>=_vecLibs.size())
		return "";

	return _vecLibs[iLib]->GetName();
}

const char *CShaderLibMgr::_MakeShaderCodeName(ShaderCode &sc)
{
	if (!sc.IsValid())
		return "<unknown shader>";
	static std::string s;
	s=GetLibName(sc.iLib);
	if (sc.UFExist())
	{
		std::string ss;
		_vecLibs[sc.iLib]->GetUniFeatureName(sc.idxUF,ss);
		s=s+"("+ss+")";
	}
	else
	{
		s=s+"("+sc.ToName()+")";
		if (sc.idMte!=StringID_Invalid)
		{
			s+="+";
			MteEntry *entry=_FindMteEntry(sc.idMte);
			s+="\"";
			s+=entry->path;
			s+="\"";
		}
	}

	return s.c_str();
}


int CShaderLibMgr::FindLibByName(const char *name)
{
	for (int i=0;i<_vecLibs.size();i++)
	{
		if (strcmp(name,_vecLibs[i]->GetName())==0)
			return i;
	}
	return -1;
}


//Enumerate all the feature codes in the specified lib
FeatureCode CShaderLibMgr::EnumFeatureCode(const char *nameLib,FeatureFlag flagMask)
{
	CShaderLib *slib;
	for (int i=0;i<_vecLibs.size();i++)
	{
		slib=_vecLibs[i];
		if (strcmp(slib->GetName(),nameLib)==0)
		{
			if (flagMask==FF_All)
				return slib->GetFeatureCode();

			FeatureCode fc;
			for (int j=0;j<slib->GetFeatureCount();j++)
			{
				SLFeature *f=slib->GetFeature(j);
				if ((f->flag&flagMask)==flagMask)
					fc.Add(f->fc);
			}
			return fc;
		}
	}
	return FeatureCode(0);
}

const char *CShaderLibMgr::EnumFeatureNames(const char *nameLib,FeatureFlag flagMask)
{
	static std::string s;
	s="";

	CShaderLib *slib;
	for (int i=0;i<_vecLibs.size();i++)
	{
		slib=_vecLibs[i];
		if (strcmp(slib->GetName(),nameLib)==0)
		{
			for (int j=0;j<slib->GetFeatureCount();j++)
			{
				SLFeature *f=slib->GetFeature(j);
				if ((f->flag&flagMask)==flagMask)
				{
					if (s.empty())
						s=f->name;
					else
					{
						s+=",";
						s+=f->name;
					}
				}
			}
			return s.c_str();
		}
	}
	return "";
}


const char *CShaderLibMgr::EnumUniFeatures(const char *nameLib)
{
	static std::string s;
	s="";
	CShaderLib *slib;
	for (int i=0;i<_vecLibs.size();i++)
	{
		slib=_vecLibs[i];
		if (strcmp(slib->GetName(),nameLib)==0)
		{
			std::string ss;
			for (int j=0;j<slib->GetUniFeatureCount();j++)
			{
				if (j>0)
					s+=",";
				slib->GetUniFeatureName(j,ss);
				s+=ss;
			}
			return s.c_str();
		}
	}
	return "";
}

BOOL CShaderLibMgr::ExistUniFeature(const char *nameLib,const char *nameUF)
{
	CShaderLib *slib;
	std::string ss;
	for (int i=0;i<_vecLibs.size();i++)
	{
		slib=_vecLibs[i];
		if (strcmp(slib->GetName(),nameLib)==0)
		{
			for (int j=0;j<slib->GetUniFeatureCount();j++)
			{
				slib->GetUniFeatureName(j,ss);
				if (ss==nameUF)
					return TRUE;
			}
		}
	}
	return FALSE;
}



void CShaderLibMgr::_UnloadCache()
{
	_cache.clear();
	_bufferCache.clear();

	if (_flData)
		_flData->Close();
	_flData=NULL;

	if (_flHead)
		_flHead->Close();
	_flHead=NULL;

	_stateCache=Invalid;
}

//add the cache to the buffer in memory
void CShaderLibMgr::_AppendCacheBuffer(ShaderCacheHeader &head,BYTE *buffer)
{
	int idx=_bufferCache.size();
	_bufferCache.resize(_bufferCache.size()+head.sz);

	memcpy(&_bufferCache[idx],buffer,head.sz);

	ShaderCacheKey key;
	key.FromHead(head);

	head.idxData=idx;

	_cache[key]=head;
}

void LoadCacheHeader(IFile *fl,ShaderCacheHeader &header)
{
	IFile_ReadVar(fl,header.sc);
	IFile_ReadVar(fl,header.start);
	IFile_ReadVar(fl,header.sz);
	IFile_ReadArray(fl,header.caps);
	IFile_ReadVar(fl,header.nCaps);
	IFile_ReadString(fl,header.pathMte);
}


void SaveCacheHeader(IFile *fl,ShaderCacheHeader &header)
{
	IFile_WriteVar(fl,header.sc);
	IFile_WriteVar(fl,header.start);
	IFile_WriteVar(fl,header.sz);
	IFile_WriteArray(fl,header.caps);
	IFile_WriteVar(fl,header.nCaps);
	IFile_WriteString(fl,header.pathMte);
}

void ShaderCacheKey::FromHead(ShaderCacheHeader &head)
{
	fc=head.sc;
	idxLib=head.sc.iLib;
	idxUF=head.sc.idxUF;
	pathMte=head.pathMte;
}


#define ShaderCacheVer_Unique 0
//increase this value to discard the cache on disk and rebuild one
//set to ShaderCacheVer_Unique to ALWAYS discard the old cache and rebuild new one
//#define ShaderCacheVer_Cur 11 
#define ShaderCacheVer_Cur ShaderCacheVer_Unique

void CShaderLibMgr::_LoadCache(IRenderSystem *pRS)
{
	_UnloadCache();

	std::string pathHeader=pRS->GetPath(Path_ShaderCache);
	std::string pathData=pathHeader;
	RemoveFileSuffix(pathHeader);
	MakeFileSuffix(pathHeader,"hd");
	MakeFileSuffix(pathData,"dat");
	IFileSystem *pFS=pRS->GetFS();
	if (!pFS)
		return;

	ShaderCacheHeader head;
	ShaderCacheKey key;

	//如果cache的版本和当前版本不一致,先把cache删除
	if (pFS->ExistFileAbs(pathHeader.c_str()))
	{
		IFile *file=pFS->OpenFileAbs(pathHeader.c_str(),FileAccessMode_Read);
		if (file)
		{
			DWORD ver;
			(*file)>>ver;
			file->Close();
			if (ver!=ShaderCacheVer_Cur)
			{
				pFS->RemoveFileAbs(pathHeader.c_str());
				pFS->RemoveFileAbs(pathData.c_str());
			}
		}
	}

	//如果不存在则新建cache文件
	if(!pFS->ExistFileAbs(pathHeader.c_str()))
	{
		IFile *file=pFS->OpenFileAbs(pathHeader.c_str(),FileAccessMode_Write);
		if (file)
		{
			DWORD c=ShaderCacheVer_Cur;
			(*file)<<c;
			c=0;
			(*file)<<c;
			file->Close();
			pFS->RemoveFileAbs(pathData.c_str());
		}
	}
	if(!pFS->ExistFileAbs(pathData.c_str()))
	{
		IFile *file=pFS->OpenFileAbs(pathData.c_str(),FileAccessMode_Write);
		if (file)
			file->Close();
	}

	CacheState stateHead=Invalid;

	//我们先载入头文件
	if(pFS->ExistFileAbs(pathHeader.c_str()))
	{
		IFile *file=pFS->OpenFileAbs(pathHeader.c_str(),FileAccessMode_Modify);
		if (file)
			stateHead=ReadWrite;
		else
		{
			file=pFS->OpenFileAbs(pathHeader.c_str(),FileAccessMode_Read);
			if (file)
				stateHead=ReadOnly;
		}


		file->Seek(0);
		DWORD ver;
		(*file)>>ver;
		if ((ver==ShaderCacheVer_Cur)&&(ver!=ShaderCacheVer_Unique))
		{
			DWORD count;
			(*file)>>count;

			for (int i=0;i<count;i++)
			{
				LoadCacheHeader(file,head);
				key.FromHead(head);
				head.idxData=-1;

				_cache[key]=head;
			}
		}
		else
		{
			file->Close();
			file=NULL;
			stateHead=Invalid;
		}
		_flHead=file;
	}

	//打开cache数据文件
	if (stateHead==ReadOnly)
		_flData=pFS->OpenFileAbs(pathData.c_str(),FileAccessMode_Read);
	if (stateHead==ReadWrite)
		_flData=pFS->OpenFileAbs(pathData.c_str(),FileAccessMode_Modify);
	if (!_flData)
	{
		stateHead=ReadOnly;
		_flData=pFS->OpenFileAbs(pathData.c_str(),FileAccessMode_Read);
		if (!_flData)
			stateHead=Invalid;
	}

	if (stateHead==Invalid)
		_UnloadCache();
	else
		_stateCache=stateHead;
}

void CShaderLibMgr::_AddCache(ShaderCacheHeader &head,BYTE *buffer)
{
	//first add to the cache in memory
	_AppendCacheBuffer(head,buffer);

	if (_stateCache==ReadWrite)
	if (_flData&&_flHead)
	{
		//写到data文件的末尾
		DWORD sz=_flData->GetSize();
		_flData->Seek(sz);
		_flData->Write(buffer,head.sz);

		//将head数据写到head文件的末尾
		head.start=sz;
		sz=_flHead->GetSize();
		_flHead->Seek(sz);
		SaveCacheHeader(_flHead,head);

		//更新head的数量
		DWORD count=_cache.size();
		_flHead->Seek(sizeof(DWORD));
		(*_flHead)<<count;
	}
}

ITexture *CShaderLibMgr::GetVoidTex()
{
	if(_texVoid)
		return _texVoid;

	_texVoid=_pRS->GetWTexMgr2()->Create(1,1,D3DFMT_A8R8G8B8);
	_texVoid->Fill(0xff000000);
	return _texVoid;
}
