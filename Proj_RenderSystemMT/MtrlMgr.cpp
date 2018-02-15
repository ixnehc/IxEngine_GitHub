/********************************************************************
	created:	2006/10/18   15:02
	filename: 	e:\IxEngine\Proj_RenderSystem\MtrlMgr.cpp
	author:		cxi
	
	purpose:	material resource manager
*********************************************************************/
#include "stdh.h"

#pragma warning(disable:4018)
#pragma warning(disable:4267)


#include "MtrlMgr.h"
#include "RenderSystem/ITexture.h"
#include "DeviceObject.h"
#include <assert.h>

#include "Log/LogFile.h"

#include "stringparser/stringparser.h"
#include "strlib/strlib.h"

#include "shaderlib/SLDefines.h"
#include "MtrlExtMgr.h"

#include "ShaderLibMgr.h"

//////////////////////////////////////////////////////////////////////////
//Repair Mtrl Data

void RepairMtrlLod(MtrlData::Lod &lod,IRenderSystem *pRS,MtrlExtData *dataMte,MteID idMte)
{
	IShaderLibMgr *slmgr=pRS->GetShaderLibMgr();

	CMtrlExt *mte=NULL;
	if (!lod.mte.empty())
	{
		mte=(CMtrlExt*)pRS->GetMtrlExtMgr()->ObtainRes(lod.mte.c_str());
		if (FALSE==mte->ForceTouch())
			SAFE_RELEASE(mte);
	}

	if (lod.unifeature!="")
	{
		lod.features="";//if using unifeature,no feature is needed

		//and check whether this unifeature exists in the lib
		if (!slmgr->ExistUniFeature(lod.slib.c_str(),lod.unifeature.c_str()))
			lod.unifeature="";
	}

	if (!dataMte)
	{
		if (mte)
			dataMte=mte->GetData();
	}
	if (idMte==MteID_Invalid)
	{
		if (mte)
			idMte=mte->GetID();
	}

	IShader *shader;
	if (lod.unifeature=="")
	{
		FeatureCode fcTotalMtrl=slmgr->EnumFeatureCode(lod.slib.c_str(),FF_MtrlEdit);
		FeatureCode fcTotal=slmgr->EnumFeatureCode(lod.slib.c_str());
		FeatureCode fcRepair=slmgr->EnumFeatureCode(lod.slib.c_str(),FF_MtrlRepair);
		FeatureCode fc,fcOld;
		fc.FromName(lod.features.c_str());
		fcOld=fc;
		fc.Filter(fcTotalMtrl);//mask by the available mtrl edit feature in this lib
		if (!(fcOld.Equals(fc)))
		{//有变化,更新名字
			std::vector<std::string>buf;
			SplitStringBy(",",lod.features,&buf);
			lod.features="";
			for (int i=0;i<buf.size();i++)
			{
				extern FeatureCode FeatureFromName(const char *name);
				FeatureCode fcT=FeatureFromName(buf[i].c_str());
				if (fcTotalMtrl.Test(fcT))
				{
					extern void AppendSepStr(std::string &s,const char *sep,const char *s2);
					AppendSepStr(lod.features,",",buf[i].c_str());
				}
			}
		}

		//add some additional feature to make all the necessary param available
		fc.Add(FC_dirlight_p);
		if (lod.demand.flags&(MtrlDemandF_Warp|MtrlDemandF_WarpMultiLayor))
			fc.Add(FC_warp);
		fc.Add(fcRepair);

		fc.Filter(fcTotal);//mask by the available feature in this lib

//		std::string name=fc.ToName();

		//Now see what the feature params fc could lead to
		shader=(IShader *)slmgr->ObtainShader(lod.slib.c_str(),fc,idMte);//make a shader using that fc
	}
	else
		shader=(IShader *)slmgr->ObtainShader(lod.slib.c_str(),lod.unifeature.c_str());
	std::vector<FeatureParamA>fps;
	if (shader&&(A_Ok==shader->Touch()))
	{
		for (int i=0;i<SizeOfEPInfo();i++)
		{
			if ((!(EPInfo_GetFlag(i)&EPF_MtrlEdit))&&(!(EPInfo_GetFlag(i)&EPF_MtrlView)))
				continue;
			if (shader->ExistEP(EPInfo_GetEP(i)))
			{
				FeatureParamA fp;
				fp.SetEP(EPInfo_GetName(i));
				fp.var=GVar(EPInfo_GetVarType(i),EPInfo_GetVarSem(i));
				fps.push_back(fp);
			}
		}
		if (dataMte)
		{
			for (int i=0;i<dataMte->epinfo.size();i++)
			{
				MtrlExtData::EPInfo *info=&dataMte->epinfo[i];
				if (shader->ExistEP(info->nmEP))
				{
					FeatureParamA fp;
					fp.SetEP(info->nmEP);
					fp.var=GVar(info->gvt,info->sem);
					fp.kt=info->kt;
					fp.code=info->sem.code;
					fps.push_back(fp);
				}
			}
		}
	}
	SAFE_RELEASE(shader);
	SAFE_RELEASE(mte);

	//overwrite with the existing params
	for (int i=0;i<fps.size();i++)
	for (int j=0;j<lod.fps.size();j++)
	{
		if (lod.fps[j].CheckSameEP(fps[i]))
			fps[i].ReplaceContent(lod.fps[j]);
	}
	lod.fps=fps;
}

//check whether the res data is in its proper format,if not ,try to repair it
BOOL RepairResData(MtrlData *data,IRenderSystem *pRS)
{
	for (int j=0;j<data->lods.size();j++)
		RepairMtrlLod(data->lods[j],pRS,NULL,MteID_Invalid);

	return TRUE;
}


const char *ResolveRefPath(const char *path,CResource *owner)
{
	static char buf[1024];
	if (path[0]=='.')
	{
		if (path[1]=='\\')
		{
			const char *pathOwner=owner->GetPath();
			assert(pathOwner[0]!='.');

			int idx=StringReverseFind(pathOwner,'\\');
			if (idx==-1)
				return path+2;
			memcpy(buf,pathOwner,idx+1);
			strcpy(buf+idx+1,path+2);
			return buf;
		}
	}
	return path;
}

//////////////////////////////////////////////////////////////////////////
//
IMPLEMENT_CLASS(MtrlLod);

inline void ChangeEpkTexRef(BYTE *data,DWORD nEP,BOOL bAddRef)
{
	if (nEP>0)
	{
		BYTE *p=data;
		for (int i=0;i<nEP;i++)
		{
			EpkHead *q=(EpkHead *)p;
			if (q->type==GVT_String)
			{//it's a texture handle
				ITexture**tex=(ITexture**)&q[1];
				if (!bAddRef)
				{
					SAFE_RELEASE(*tex);
				}
				else
				{
					SAFE_ADDREF(*tex);
				}
			}
			p+=sizeof(EpkHead)+q->sz;
		}
	}
}


void MtrlLod::ChangeTexRef(BOOL bAddRef)
{
	ChangeEpkTexRef(&epk[0],nEP,bAddRef);
}

void MtrlLod::Clean()
{
	//Release all the tex's reference
	ChangeTexRef(FALSE);

	anims.clear();

	Zero();
}

void MtrlLod::CommitAnim(AnimTick t0)
{
	if (t0==tAnim)
		return;//已经是最新的更新了

	tAnim=t0;

	BYTE buf[64];//足够大的buffer
	DWORD szBuf=0;

	i_math::vector3df v1,v2;
	AnimTick t;

	for (int i=0;i<anims.size();i++)
	{
		MtrlAnim *ma=&anims[i];
		if(ma->at==FeatureParamA::Anim_None)
			continue;

		EpkHead *head=(EpkHead *)&epk[ma->offEPK];
		BYTE *data=(BYTE *)&head[1];//紧跟着这个head后的位置

		ValueSet *vs=NULL;
		KeySet *ks=NULL;
		if (ma->at==FeatureParamA::Anim_ValueSet)
			vs=ma->vs;
		if (ma->at==FeatureParamA::Anim_Res)
		{
			if (ma->anim)
			{
				ma->anim->ForceTouch();
				ks=ma->anim->GetKeySet();
			}
		}

		if (vs)
		{
			switch(vs->GetKeyType())
			{
				case KT_Float:
				{
					*(float*)data=vs->GetFloat(t0);
					break;
				}
				case KT_Color:
				{
					((i_math::vector4df*)data)->fromDwordColor(vs->GetColor(t0));
					break;
				}
			}
		}

		if (ks)
		{
			AnimTick tRange=ks->GetRange();
			if (tRange>0)
				t=t0%tRange;
			else
				t=0;

			if (TRUE)
			{
				switch(ks->GetKeyType())
				{
					case KT_Float:
					{
						assert(head->sz==sizeof(float));
						if (ks->CalcKey(t,(Key_f*)buf))
							*(float*)data=((Key_f*)buf)->v;
						break;
					}
					case KT_Color:
					{
						assert(head->sz==sizeof(i_math::vector4df));
						if (ks->CalcKey(t,(Key_col*)buf))
							((i_math::vector4df*)data)->fromDwordColor(((Key_col*)buf)->color);
						break;
					}
					case KT_MapCoord:
					{
						assert(head->sz==sizeof(i_math::matrix43f));

						Key_mapcoord *key=(Key_mapcoord *)buf;
						if (ks->CalcKey(t,key))
						{
							i_math::matrix43f *mat=((i_math::matrix43f*)data);
							key->MakeMat(*mat);
						}
						break;
					}
				}
			}

		}

	}
}


//////////////////////////////////////////////////////////////////////////
//CMtrl
IMPLEMENT_CLASS(CMtrl);

CMtrl::CMtrl()
{
	Zero();
}

CMtrl::~CMtrl()
{
	Clean();
}



void CMtrl::Zero()
{
	memset(_lods,0,sizeof(_lods));
	_nLods=0;
}

void CMtrl::Clean()
{
	for (int i=0;i<_nLods;i++)
	{
		if (_lods[i])
		{
			_lods[i]->Clean();
			Class_Delete(_lods[i]);
		}
	}

	Zero();
}


BOOL FillMtrlLod(MtrlLod *lod,MtrlData::Lod *lod_d,IRenderSystem *pRS,std::string &err,CResource *owner)
{
	ShaderCode sc;

	//first resolve the ShaderCode
	if (lod_d->unifeature=="")
	{
		std::string errfeature;
		if (FALSE==FeatureCode::CheckValidName(lod_d->features.c_str(),&errfeature))
		{
			FormatString(err,"Not support feature found (%s)",errfeature.c_str());
			return FALSE;
		}

		FeatureCode fc;
		fc.FromName(lod_d->features.c_str());

		assert(pRS->GetShaderLibMgr());
		sc=pRS->GetShaderLibMgr()->MakeShaderCode(lod_d->slib.c_str(),fc,"");
		if (!sc.IsValid())
		{
			FormatString(err,"Unknown shader lib name(%s)",lod_d->slib.c_str());
			return FALSE;
		}
	}
	else
	{
		assert(pRS->GetShaderLibMgr());
		sc=pRS->GetShaderLibMgr()->MakeShaderCode(lod_d->slib.c_str(),
			FeatureCode(0),lod_d->unifeature.c_str());
		if (!sc.IsValid())
		{
			FormatString(err,"Unknown shader lib name(%s) or not supported unifeature name(%s)",
				lod_d->slib.c_str(),lod_d->unifeature.c_str());
			return FALSE;
		}
	}

	CMtrlExt *mte=NULL;
	if (!lod_d->mte.empty())
	{
		mte=(CMtrlExt*)pRS->GetMtrlExtMgr()->ObtainRes(ResolveRefPath(lod_d->mte.c_str(),owner));
		if (FALSE==mte->ForceTouch())
			SAFE_RELEASE(mte);
	}

	lod->sc=sc;
	lod->demand=lod_d->demand;
	lod->state=lod_d->state;
	if (mte)
		lod->sc.idMte=mte->GetID();

	lod->anims.reserve(4);

	//add FeatureParam
	if (TRUE)
	{
		EffectParamPacket<2048> epk;
		for (int i=0;i<lod_d->fps.size();i++)
		{
			int off=epk.n;
			FeatureParamA *fp=&lod_d->fps[i];
			ITexture *tex=NULL;
			BOOL bValid=FALSE;


			if (!fp->bMte)
			{
				EffectParam ep=EPfromName(fp->ep_name);

				if (ep!=EP_None)
				{
					extern int EPInfo_IndexFromEP(EffectParam);
					extern GVarType EPInfo_GetVarType(DWORD);
					extern GSem& EPInfo_GetVarSem(DWORD);

					DWORD idx=EPInfo_IndexFromEP(ep);
					if (fp->at==(BYTE)FeatureParamA::Anim_None)
					{
						if (EPInfo_GetVarType(idx)==fp->var.Type())
						{
							if ((fp->var.Type()==GVT_String)&&(EPInfo_GetVarSem(idx).code==GSem_TexturePath))
							{
								if (fp->var.Str()!="")
									tex=(ITexture *)pRS->GetTexMgr()->ObtainRes(ResolveRefPath(fp->var.Str().c_str(),owner));
								else
									tex=NULL;
								bValid=epk.AddEP(ep,tex);
							}
							else
							{
								DWORD szData;
								void *data=fp->var.GetData(szData);
								bValid=epk._AddEP(ep,fp->var.Type(),data,szData);
							}
						}
					}
					else
					{//使用动画的话,
						GVarType gvt=EPInfo_GetVarType(idx);
						int sz=SizeFromVarType(gvt);

						if (gvt==GVT_String)
							sz=sizeof(ITexture *);

						bValid=epk._AddEP(ep,gvt,NULL,sz);//预留位置
					}
				}
			}
			else
			{
				StringID nm=fp->GetEPNameID();
				if (mte)
				{
					MtrlExtData::EPInfo *info=mte->GetData()->FindEPInfo(nm);
					if (info)
					{
						if (fp->at==(BYTE)FeatureParamA::Anim_None)
						{
							if (info->gvt==fp->var.Type())
							{
								if ((fp->var.Type()==GVT_String)&&(info->sem.code==GSem_TexturePath))
								{
									if (fp->var.Str()!="")
										tex=(ITexture *)pRS->GetTexMgr()->ObtainRes(ResolveRefPath(fp->var.Str().c_str(),owner));
									else
										tex=NULL;
									bValid=epk.AddEP(nm,tex);
								}
								else
								{
									DWORD szData;
									void *data=fp->var.GetData(szData);
									bValid=epk._AddEP(nm,fp->var.Type(),data,szData);
								}
							}
						}
						else
						{//使用动画的话,
							int sz=SizeFromVarType(info->gvt);

							if (info->gvt==GVT_String)
								sz=sizeof(ITexture *);

							bValid=epk._AddEP(nm,info->gvt,NULL,sz);//预留位置
						}
					}
				}
			}
			if (!bValid)
			{
				FormatString(err,"effect param could not be resolved(%s)",fp->bMte?StrLib_GetStr(fp->GetEPNameID()):fp->GetEPName());
				SAFE_RELEASE(mte);
				ChangeEpkTexRef(epk.buf,epk.nEP,FALSE);//清除保存在epk中的贴图
				return FALSE;
			}

			//处理动画
			if (fp->at==FeatureParamA::Anim_ValueSet)
			{
				MtrlAnim ma;
				ma.at=(BYTE)FeatureParamA::Anim_ValueSet;
				ma.offEPK=off;
				ma.vs=Class_New2(ValueSet);
				ma.vs->Copy(&fp->vs);
				lod->anims.push_back(ma);
				ma.Zero();//避免ma在析构时,释放里面的内容
			}
			if (fp->at==FeatureParamA::Anim_Res)
			{
				MtrlAnim ma;
				ma.at=(BYTE)FeatureParamA::Anim_Res;
				ma.offEPK=off;
				ma.anim=(IAnim*)pRS->GetAnimMgr()->ObtainRes(ResolveRefPath(fp->pathAnim.c_str(),owner));
				lod->anims.push_back(ma);
				ma.Zero();//避免ma在析构时,释放里面的内容
			}
		}

		lod->epk.resize(epk.n);
		if (epk.n>0)
			memcpy(&lod->epk[0],epk.buf,epk.n);
		lod->nEP=epk.nEP;

	}


	SAFE_RELEASE(mte);

	return TRUE;
}


BOOL CMtrl::_OnTouch(IRenderSystem *pRS)
{
	VALIDATE_RES_TYPE(Res_Mtrl);

	MtrlData mtrldata;
	mtrldata.LoadData(_data);

	std::string err,errfinal;
	int j;


	_nLods=mtrldata.lods.size();
	for (j=0;j<_nLods;j++)
	{
		if (j>=ARRAY_SIZE(_lods))
		{
			LOG_DUMP_2P("MtrlMgr",Log_Warning,"When loading material \"%s\":Too many lods(should be less than %d) are found!",
																GetPath(),ARRAY_SIZE(_lods));
			break;
		}
		MtrlLod *lod=_lods[j]=Class_New(MtrlLod);
		MtrlData::Lod *lod_d=&mtrldata.lods[j];
		if (FALSE==FillMtrlLod(lod,lod_d,pRS,err,this))
		{
			//尝试修补一下
			RepairMtrlLod(*lod_d,pRS,NULL,MteID_Invalid);
			if (FALSE==FillMtrlLod(lod,lod_d,pRS,err,this))
			{
				FormatString(errfinal,"Error when loading material \"%s\" (lod%d ):%s!",
					GetPath(),j,err.c_str());
				goto _fail;
			}
			else
			{
				LOG_DUMP_3P("MtrlMgr",Log_Warning,"载入材质\"%s\"的Lod%d的时候,遇到了\"%s\"的错误,修复成功!",
					GetPath(),j,err.c_str());
			}
		}
	}
	_nLods=j;

	return TRUE;

_fail:
	LOG_DUMP("Error",Log_Error,errfinal.c_str());
	Clean();
	return FALSE;
}

void CMtrl::_OnUnload()
{
	Clean();
}

MtrlLod *CMtrl::GetLod(DWORD iLod)
{
	if (iLod>=_nLods)
		return NULL;
	return _lods[iLod];
}

MtrlDemand*CMtrl::GetLodDemand(DWORD iLod)
{
	if (iLod>=_nLods)
		return NULL;
	return &_lods[iLod]->demand;
}


BOOL CMtrl::GetShaderState(DWORD iLod,ShaderState &state)
{
	MtrlLod *lod=GetLod(iLod);
	if (!lod)
		return FALSE;
	state=lod->state;
	return TRUE;
}



BOOL CMtrl::IsOpaque(DWORD iLod)
{
	if (iLod<_nLods)
	{
		if (_lods[iLod]->state.modeBlend!=Blend_Opaque)
			return FALSE;
		if (_lods[iLod]->demand.flags&(MtrlDemandF_Warp|MtrlDemandF_WarpMultiLayor))
			return FALSE;//如果有warp效果,不算opaque
	}

	return TRUE;
}

BOOL CMtrl::IsAlphaTest(DWORD iLod)
{
	if (iLod<_nLods)
		if (_lods[iLod]->state.modeAlphaTest!=AlphaTest_Standard)
			return FALSE;

	return TRUE;
}

BOOL CMtrl::IsCover(DWORD iLod)
{
	if (iLod<_nLods)
	{
		if (_lods[iLod]->state.modeAlphaTest==AlphaTest_Standard)
			return FALSE;
		if (_lods[iLod]->state.modeBlend!=Blend_Opaque)
			return FALSE;
	}
	return TRUE;
}


BOOL CMtrl::Is2Sided(DWORD iLod)
{
	if (iLod<_nLods)
		return (_lods[iLod]->state.modeFacing==Facing_Both);
	return FALSE;
}

BOOL CMtrl::IsWarp(DWORD iLod)
{
	if (iLod<_nLods)
		if (_lods[iLod]->demand.flags&MtrlDemandF_Warp)
			return TRUE;

	return FALSE;
}

BOOL CMtrl::IsWarpML(DWORD iLod)
{
	if (iLod<_nLods)
		if (_lods[iLod]->demand.flags&MtrlDemandF_WarpMultiLayor)
			return TRUE;

	return FALSE;
}



void CMtrl::BindEP(IShader *shader,DWORD iLod,AnimTick t)
{
	MtrlLod *lod=GetLod(iLod);
	if (lod)
	{
		if (lod->anims.size()>0)
			lod->CommitAnim(t==ANIMTICK_INFINITE?_pMgr->GetRS()->GetPresentTick():t);
		if (lod->epk.size()>0)
			((CShader*)shader)->SetEPs(&lod->epk[0],lod->nEP);
		((CShader*)shader)->SetEP(EP_time,ANIMTICK_TO_SECOND(t==ANIMTICK_INFINITE?_pMgr->GetRS()->GetPresentTick():t));
	}
}

int CMtrl::ResolveLod(MtrlCap &cap)
{
	for (int i=0;i<_nLods;i++)
	{
		if ((_lods[i]->demand.flags&cap.flags)==_lods[i]->demand.flags)
			return i;
	}
	return -1;
}


ShaderState *CMtrl::GetState(DWORD iLod)
{
	MtrlLod *lod=GetLod(iLod);
	if (lod)
		return &lod->state;
	return NULL;
}


void CMtrl::BindState(IShader *shader,DWORD iLod)
{
	MtrlLod *lod=GetLod(iLod);
	if (lod)
		((CShader*)shader)->SetState(lod->state);
}

int CMtrl::GetShaderLib(DWORD iLod)
{
	MtrlLod *lod=GetLod(iLod);
	if (lod)
		return (int)lod->sc.GetLibIdx();
	return -1;
}

ShaderCode &CMtrl::GetShaderCode(DWORD iLod)
{
	MtrlLod *lod=GetLod(iLod);
	if (!lod)
	{
		static ShaderCode scEmpty;
		return scEmpty;
	}
	return lod->sc;
}
	

BOOL CMtrl::HasMte(StringID idMte)
{
	for (int i=0;i<_nLods;i++)
	{
		MtrlLod *lod=_lods[i];
		if (lod->sc.idMte==idMte)
			return TRUE;
	}
	return FALSE;
}



//////////////////////////////////////////////////////////////////////////
//CMtrlMgr
CMtrlMgr::CMtrlMgr()
{
	_tPaint=0;
}

BOOL CMtrlMgr::Init(IRenderSystem *pRS,const char *name)
{
	return CResourceMgr::Init(pRS,name);
}

IResource *CMtrlMgr::ObtainRes(const char *pathRes)
{
	return _ObtainResS<CMtrl>(pathRes);//同步载入资源
}

BOOL CMtrlMgr::ReloadRes(const char *pathRes)
{
	return CResourceMgr::_ReloadRes<CMtrl>(pathRes,'S');
}



//////////////////////////////////////////////////////////////////////////
//CDynMtrlMgr

IMtrl *CMtrlMgr::Create(MtrlData *data,const char *pathOverride)
{
	CMtrl *mtrl=_ObtainRes<CMtrl>();
	mtrl->_path=pathOverride;
	data->SaveData(mtrl->_data);
	mtrl->_typeData=Res_Mtrl;
	mtrl->SetState(CResource::Loaded);

	mtrl->ForceTouch();

	return mtrl;
}

void CMtrlMgr::ReloadMteMtrl(StringID idMte)
{
	std::hash_map<std::string,CResource*>::iterator it;
	static std::vector<std::string> pathes;
	pathes.clear();
	for (it=_mapRes.begin();it!=_mapRes.end();it++)
	{
		CMtrl *mtrl=(CMtrl *)((*it).second);
		if (mtrl)
		{
			if (mtrl->HasMte(idMte))
				pathes.push_back(std::string(mtrl->GetPath()));
		}
	}

	for (int i=0;i<pathes.size();i++)
		_ReloadRes<CMtrl>(pathes[i].c_str(),'S');
}
