#pragma once

#include "interface/interface.h"
#include "RenderSystem/IMtrl.h"
#include "RenderSystem/IShader.h"
#include "DeviceObject.h"

#include "shaderlib/SLDefines.h"
#include "shaderlib/SLHolder.h"

#include "ResourceBase.h"


#include <vector>
#include <string>
#include <hash_map>

class CDeviceObject;
class CShader:public IShader,public CResource
{
public:
	DECLARE_CLASS(CShader);
	CShader()
	{
		_effect=NULL;
		_devobj=NULL;
		memset(_eph_buffer,0,sizeof(_eph_buffer));
		memset(_eph,0,sizeof(_eph));

	}

	//Caps
	RESOURCE_CORE()
	virtual int GetCap_maxbones();
	virtual int GetCap_weightcount();

	virtual ShaderCode &GetShaderCode()	{		return _sc;	}


	virtual BOOL ExistEP(EffectParam ep,DWORD idx=0);
	virtual BOOL ExistEP(StringID epname);
	virtual BOOL CheckValid();
	virtual const char *GetFX()	{		return _sFX.c_str();	}
	virtual const char *GetFXErr()	{		return _err.c_str();	}


	//general set ep functions
	virtual EPResult SetEP(EffectParam ep,color4df &c,DWORD idx=0);
	virtual EPResult SetEP(EffectParam ep,vector3df &v,DWORD idx=0);
	virtual EPResult SetEP(EffectParam ep,ITexture *tex,DWORD idx=0);
	virtual EPResult SetEP(EffectParam ep,f32 f,DWORD idx=0);
	virtual EPResult SetEP(EffectParam ep,vector2df &v,DWORD idx=0);
	virtual EPResult SetEP(EffectParam ep,i_math::s32 s,DWORD idx=0);
	virtual EPResult SetEP(EffectParam ep,i_math::vector4di &v,DWORD idx=0);
	virtual EPResult SetEP(EffectParam ep,i_math::matrix43f &mat,DWORD idx=0);
	virtual EPResult SetEP(EffectParam ep,i_math::matrix44f &mat,DWORD idx=0);
	virtual EPResult SetEP(EffectParam ep,float *fs,DWORD count);
	virtual EPResult SetEP(EffectParam ep,i_math::vector4df *data,DWORD count);
	virtual EPResult SetEP_World(i_math::matrix43f *mats,DWORD count);
	virtual EPResult SetEP_ViewProj(i_math::matrix44f &viewproj,i_math::matrix44f &view,i_math::matrix44f &proj);

	virtual EPResult SetEP(StringID nm,color4df &c);
	virtual EPResult SetEP(StringID nm,vector3df &v);
	virtual EPResult SetEP(StringID nm,ITexture *tex);
	virtual EPResult SetEP(StringID nm,f32 f);
	virtual EPResult SetEP(StringID nm,vector2df &v);
	virtual EPResult SetEP(StringID nm,i_math::s32 s);
	virtual EPResult SetEP(StringID nm,i_math::vector4di &v);
	virtual EPResult SetEP(StringID nm,i_math::matrix43f &mat);
	virtual EPResult SetEP(StringID nm,i_math::matrix44f &mat);

	virtual EPResult SetEPs(BYTE *data,DWORD nEP);

	//the blend mode
	virtual void SetState(ShaderState &state)
	{
		_devobj->SetAlphaTest((AlphaTestMode)state.modeAlphaTest,state.refAlphaTest);
		_SetAlphaTest((AlphaTestMode)state.modeAlphaTest,state.refAlphaTest);
		_devobj->SetBlend((ShaderBlendMode)state.modeBlend);
		_devobj->SetStencilOp((StencilMode)state.modeStencil,state.refStencil,state.maskStencil);
		_devobj->SetDepthMethod((DepthMode)state.modeDepth);
		_devobj->SetFacing((FacingMode)state.modeFacing);
	}
	virtual void SetState(ShaderState &state,BOOL bForceAlpha)
	{
		_devobj->SetAlphaTest((AlphaTestMode)state.modeAlphaTest,state.refAlphaTest);
		_SetAlphaTest((AlphaTestMode)state.modeAlphaTest,state.refAlphaTest);
		if (bForceAlpha&&(state.modeBlend==Blend_Opaque))
			_devobj->SetBlend(Blend_AlphaBlend);
		else
			_devobj->SetBlend((ShaderBlendMode)state.modeBlend);
		_devobj->SetStencilOp((StencilMode)state.modeStencil,state.refStencil,state.maskStencil);
		_devobj->SetDepthMethod((DepthMode)state.modeDepth);
		_devobj->SetFacing((FacingMode)state.modeFacing);
	}
		
	virtual ShaderState&GetState()	{		return _devobj->GetState();	}

	virtual void SetDepthBias(float biasSlope,float bias);

	virtual BOOL BindVB(IVertexBuffer *vb,IIndexBuffer *ib,VBBindArg *arg)
	{
		if (vb)
			return _devobj->BindVB((VBInfo **)&vb,1,(IBInfo*)ib,arg);
		else
			return _devobj->BindVB(NULL,0,NULL,NULL);
	}
	virtual BOOL BindVBs(IVertexBuffer **vbs,DWORD nVB,IIndexBuffer *ib,VBBindArg *arg)
	{
		return _devobj->BindVB((VBInfo **)vbs,nVB,(IBInfo*)ib,arg);
	}

	virtual BOOL SetVBInstanceCount(DWORD count)	{		return _devobj->SetVBInstanceCount(count);	}


	//raw drawing function
	virtual BOOL BeginRaw();
	virtual void EndRaw();
	virtual BOOL DoShadeRaw()	{		_effect->CommitChanges(); _devobj->DrawPrim();	return TRUE;}

	BOOL DoShade()
	{
		BeginRaw();
		DoShadeRaw();
		EndRaw();
		return TRUE;
	}

protected:

	virtual BOOL _OnTouch(IRenderSystem *pRS);
	virtual void _OnUnload();
	void _OnDeviceLost();
	void _OnDeviceReset();
	void _UnTouch();


	ShaderCode _sc;
	XD3DXEffect* _effect;
	CDeviceObject *_devobj;
	std::string _sFX;
	std::string _err;
	ShaderCap _caps[SCC_max];

	//EffectParam Handles
	void _LoadEPH();//call this after _effect is set
	void _UnloadEPH();
	struct EPH
	{
		D3DXHANDLE handle;//记录这个param的handle
		D3DXHANDLE *handles;//如果这个effect param是一个数组,则记录每一个数组元素的handle
												//如果不是数组,则handles[0]记录这个param的handle
		DWORD count;
	};
	EPH _eph[EP_Max];
	D3DXHANDLE _eph_buffer[512];//Big enough buffer
	std::hash_map<StringID,D3DXHANDLE>_eph2;

	void _SetAlphaTest(AlphaTestMode mode,DWORD ref);

private:

	friend class CShaderLibMgr;
};

//这个函数从stlport里的代码里拷过来
inline size_t hash_string(const char* __s) {
	unsigned long __h = 0;
	for ( ; *__s; ++__s)
		__h = 5*__h + *__s;
	return size_t(__h);
}


struct ShaderCacheHeader
{
	ShaderCacheHeader()
	{
		sz=0;
		nCaps=0;
		idxData=-1;
	}
	ShaderCode sc;
	std::string pathMte;
	DWORD start;//在data文件里的起始位置
	DWORD sz;//size of this shader's cache
	ShaderCap caps[SCC_max];
	DWORD nCaps;
	int idxData;//an index to a global buffer(_bufferCache),如果为-1,表示这个数据尚为载入
};


struct MteEntry
{
	DEFINE_CLASS(MteEntry);
	MteID id;
	std::string path;//Mte资源的路径
	BOOL bCompiled;
	SLFeature feature;
	std::vector<StringID> eps;
	std::vector<std::string>epnames;
};

struct ShaderCacheKey
{
	void FromHead(ShaderCacheHeader &head);
	FeatureCode fc;
	BYTE idxLib;
	BYTE idxUF;
	std::string pathMte;
};

_STLP_TEMPLATE_NULL struct std::hash<ShaderCacheKey>
{
	size_t operator()(ShaderCacheKey k) const 
	{
		int *p=(int*)&k.fc.c;
		return p[0]+p[1]+hash_string(k.pathMte.c_str());
	}
};

_STLP_TEMPLATE_NULL struct std::equal_to<ShaderCacheKey>
{
	bool operator()(const ShaderCacheKey& x, const ShaderCacheKey& y) const 
	{ 
		return (x.fc.c==y.fc.c)&&(x.idxLib==y.idxLib)&&(x.idxUF==y.idxUF)&&(x.pathMte==y.pathMte);
	}
};




class CDeviceObject;
class CShaderLib;
struct SLFeature;
class CShaderLibMgr:public IShaderLibMgr,public CResourceMgr
{
public:
	enum CacheState
	{
		Invalid,
		ReadOnly,
		ReadWrite,
	};

	CShaderLibMgr()
	{
		_texVoid=NULL;
		_stateCache=Invalid;
		_flHead=NULL;
		_flData=NULL;
	}
	BOOL Init(IRenderSystem *pRS,const char *name);
	virtual void UnInit();
	void OnDeviceLost();
	void OnDeviceReset();


	//Interfaces

	virtual IRenderSystem *GetRS()	{		return CResourceMgr::GetRS();	}
	virtual BOOL ReloadRes(const char *path){return CResourceMgr::ReloadRes(path);}	
	virtual void Update()	{		CResourceMgr::Update();	}
	virtual BOOL CheckResLeak();
	virtual IResource *ObtainRes(const char *pathRes)	{		return NULL;	}
	virtual BOOL RegisterMte(MtrlExtData *data,MteID id,const char *pathMte);
	virtual MteID AbandonMte(const char *pathMte);//返回这个pathMte对应的Mte名称
	virtual CShaderLib*GetShaderLib(const char *nameSL);
	virtual DWORD GetShaderCount()	{		return _mapShader.size();	}


	//ObtainShader(...): if the accuired shader lib could not support all of the required feature,
	//it will return NULL; and if the feature combo required could not pass compilation,
	//it will return a not-available IShader
	IShader *ObtainShader(const char *nameSL,FeatureCode &fc,MteID idMte=MteID_Invalid);
	IShader *ObtainShader(const char *nameSL,const char *nameUF,MteID idMte=MteID_Invalid);
	IShader *ObtainShader(ShaderCode &sc);
	ShaderCode MakeShaderCode(const char *nameSL,FeatureCode &fc,const char *nameUF,MteID idMte=MteID_Invalid);
	BOOL AddShaderLib(const char *path);
	BOOL ReloadShaderLib(const char *path);

	DWORD GetLibCount()	{		return _vecLibs.size();	}
	const char *GetLibName(DWORD iLib);
	int FindLibByName(const char *name);
	FeatureCode EnumFeatureCode(const char *nameLib,FeatureFlag flagMask);//Enumerate all the feature codes in the specified lib
	const char *EnumFeatureNames(const char *nameLib,FeatureFlag flagMask);//Enumerate all the feature names in the specified lib,用","分隔
	const char *EnumUniFeatures(const char *nameLib);
	BOOL ExistUniFeature(const char *nameLib,const char *nameUF);

	ITexture *GetVoidTex();


protected:
	virtual void _UnloadAll();

	//Overidables
	BOOL _TouchShader(CShader*p);

	MteEntry *_FindMteEntry(MteID idMte)
	{
		if (idMte==MteID_Invalid)
			return NULL;
		std::hash_map<MteID,MteEntry*>::iterator it=_mtes.find(idMte);
		if (it==_mtes.end())
			return NULL;
		return (*it).second;
	}


protected:

	void _CloseFiles();

	CShaderLib *_LoadShaderLib(const char *name);

	void _UnloadMteShader(MteID idMte);

	const char *_MakeShaderCodeName(ShaderCode &sc);
	std::map<ShaderCode,CShader*>_mapShader;

	std::hash_map<MteID,MteEntry*>_mtes;//Material Features

	CDeviceObject *_devobj;

	std::vector<CShaderLib *>_vecLibs;

	void _LoadCache(IRenderSystem *pRS);
	void _UnloadCache();
	void _AddCache(ShaderCacheHeader &head,BYTE *buffer);
	void _AppendCacheBuffer(ShaderCacheHeader &head,BYTE *buffer);
	std::hash_map<ShaderCacheKey,ShaderCacheHeader>_cache;
	std::vector<BYTE>_bufferCache;

	CacheState _stateCache;
	IFile *_flHead;
	IFile *_flData;


	ITexture *_texVoid;//当设一张NULL的texture时,使用的代用贴图


	friend class CShader;

};

