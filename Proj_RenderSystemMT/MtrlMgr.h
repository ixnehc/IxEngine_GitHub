#pragma once

#include "RenderSystem/IMtrl.h"
#include "RenderSystem/IAnim.h"


#include "Base.h"
#include "ResourceBase.h"


#include "resdata/MtrlData.h"

#include "shaderlib/SLDefines.h"


#include <vector>
#include <string>
#include <map>

class CDeviceObject;
class CMtrlMgr;

struct MtrlAnim
{
	MtrlAnim()
	{
		Zero();
	}
	~MtrlAnim()
	{
		Clear();
	}
	void Zero()
	{
		offEPK=0xffff;
		at=(BYTE)FeatureParamA::Anim_None;
		vs=NULL;
		anim=NULL;
		reserved=0;
	}
	void Clear()
	{
		if (at==(BYTE)FeatureParamA::Anim_ValueSet)
			Safe_Class_Delete(vs);
		if (at==(BYTE)FeatureParamA::Anim_Res)
			SAFE_RELEASE(anim);

		Zero();
	}
	WORD offEPK;//epk里面的偏移量,指向这个anim要更新的数据
	BYTE at;
	BYTE reserved;
	union
	{
		ValueSet *vs;
		IAnim *anim;
	};
};


struct MtrlLod
{
public:
	DECLARE_CLASS(MtrlLod);
	MtrlLod()		{			Zero();		}
	~MtrlLod()		{			Clean();		}
	void Zero()
	{
		demand.Zero();
		sc.Invalid();
		epk.clear();
		nEP=0;
		tAnim=ANIMTICK_INFINITE;
		state.Zero();
		anims.clear();
	}
	void Clean();
	void ChangeTexRef(BOOL bAddRef);

	void CommitAnim(AnimTick t);

	ShaderCode sc;
	std::vector<BYTE>epk;
	DWORD nEP;

	std::vector<MtrlAnim>anims;

	AnimTick tAnim;//上次更新anim的时间

	MtrlDemand demand;

	ShaderState state;
};

class IShader;
class CMtrl:public IMtrl,public CResource
{
public:
	DECLARE_CLASS(CMtrl);

	CMtrl();
	~CMtrl();

	void Zero();
	void Clean();

	//interfaces
	RESOURCE_CORE();

	virtual DWORD GetLodCount()	{		return _nLods;	}
	virtual MtrlLod *GetLod(DWORD iLod);
	virtual int ResolveLod(MtrlCap &cap);
	virtual MtrlDemand*GetLodDemand(DWORD iLod);
	virtual BOOL GetShaderState(DWORD iLod,ShaderState &state);

	virtual BOOL IsOpaque(DWORD iLod);
	virtual BOOL IsAlphaTest(DWORD iLod);
	virtual BOOL IsCover(DWORD iLod);
	virtual BOOL Is2Sided(DWORD iLod);
	virtual BOOL IsWarp(DWORD iLod);
	virtual BOOL IsWarpML(DWORD iLod);

	virtual int GetShaderLib(DWORD iLod);//如果失败,返回-1
	virtual ShaderCode &GetShaderCode(DWORD iLod);

	virtual ShaderState *GetState(DWORD iLod);

	virtual void BindEP(IShader *shader,DWORD iLod,AnimTick t);
	virtual void BindState(IShader *shader,DWORD iLod);

	BOOL HasMte(StringID idMte);//返回这个材质是否用到Mte

protected:
	virtual BOOL _OnTouch(IRenderSystem *pRS)	;
	virtual void _OnUnload();

	//layor's draw order : layor0,layor1,layor2...
	MtrlLod *_lods[MtlData_MaxLayor];
	DWORD _nLods;

private:

	friend class CMtrlMgr;

	friend BOOL FillMtrlLod(MtrlLod *lod,MtrlData::Lod*lod_d,IRenderSystem *pRS,std::string &err);

};

struct MtrlData;

class CMtrlMgr:public IMtrlMgr,public CResourceMgr
{
public:
	CMtrlMgr();
	BOOL Init(IRenderSystem *pRS,const char *name);

	RESOURCEMGR_CORE
	virtual IResource *ObtainRes(const char *pathRes);
	virtual BOOL ReloadRes(const char *path);

	virtual IMtrl *Create(MtrlData *data,const char *pathOverride);

	void ReloadMteMtrl(StringID idMte);

protected:

	AnimTick _tPaint;

};
