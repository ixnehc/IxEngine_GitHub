#pragma once

#include "gds/GVar.h"

#include "../math/matrix43.h"

#include "../math/sphere.h"

#include <assert.h>

#include "gds/GObj.h"

//the existing defines should not be modified,otherwise the data in res file will be invalid
enum ResType
{
	Res_None=0,

	//Folder
	Res_Folder=10,

	//��νnative��Res��ָ������Դ���ļ���ʽ��һ�����е�,ͨ�õ�����
	Res_Native_Begin=50,
	Res_Texture=50,
	Res_Sheet,
	Res_Sound,
	Res_Ragdoll,
    Res_Cloth,

	//��νstandard��Res��ָ������Դ���ļ���ʽ���������Լ������
	
	//standard res type
	Res_Standard_Begin=100,
	Res_Mesh=100,
	ResA_XForm,
	ResA_Bones_obsolete,
	Res_Mtrl,
	ResA_MtrlColor,
	ResA_MapCoord,
	Res_Dummies,
	Res_Spt,
	Res_Mopp,
	Res_Spg,
	Res_AnimTree,
	ResA_Bones2,
	Res_MtrlExt,
	Res_Records,
	Res_Dtr,
	Res_BehaviorGraph,
	//XXXXX:more res type

	//IMPORTANT: add new res type here.
};

#define Res_IsFolder(type)  ((type)==Res_Folder)
#define Res_IsNative(type) (((type)>=Res_Native_Begin)&&((type)<Res_Standard_Begin))
#define Res_IsStandard(type) ((type)>=Res_Standard_Begin)


#define MD_MaxSurf 16
#define MD_MaxLod 8


enum 
{
	MtlData_MaxLayor=4,
};

enum LayorFlag
{
	LF_Invisible=1,
	LF_IgnoreLight=2,

};

enum ShaderBlendMode
{
	Blend_Opaque,
	Blend_AlphaBlend,
	Blend_Additive,
	Blend_Modulate,
	Blend_Enlighten,
	Blend_Inverse,

	Blend_Invalid,
	//XXXXX:more blend mode
};


enum StencilMode
{
	Sten_Disable=0,
	Sten_Write,//No test,directly overwrite the value in the stencil buffer with ref value
	Sten_WriteNoColor,//No test,directly overwrite the value in the stencil buffer with ref value,and do not write color
	Sten_Filter,//write pixel if the value in the stencil buffer equals the ref value
	Sten_InvFilter,//write pixel if the value in the stencil buffer does NOT equal the ref value
	Sten_Inc,//No test,add 1 to the stencil buffer
	Sten_IncNoColor,//No test,add 1 to the stencil buffer,do not write color
	Sten_FilterWrite,//write pixel if the value in the stencil buffer equals the ref value,and for
								//the passed pixel,write the ref value into the stencil buffer
	Sten_InvFilterWrite,//write pixel if the value in the stencil buffer does NOT equal the ref
									//value,and for the passed pixel,write the ref value into the stencil buffer
	Sten_FilterInc,//write pixel if the value in the stencil buffer equals the ref
									//value,and for the passed pixel,add 1 to the stencil buffer

	Sten_Invalid,
	//XXXXX:more stencil mode
};


enum DepthMode
{
	Depth_Default=0,//compare and write
	Depth_NoCmp,//just write depth to depth buffer
	Depth_NoWrite,//just compare and do not write to depth buffer
	Depth_Disable,//�Ȳ��Ƚ�Ҳ��д,
	Depth_FartherNoWrite,//���Ʊ���ס�Ĳ���,��дdepth buffer

	Depth_Invalid,
};


enum AlphaTestMode
{
	AlphaTest_Disable=0,
	AlphaTest_Standard=1,

	AlphaTest_Invalid,
};

//describe which side(front side/back side/or both side) should be drawn
enum FacingMode
{
	Facing_Front=0,
	Facing_Back=1,
	Facing_Both=2,

	Facing_Invalid,
};


struct ShaderState
{
	ShaderState()
	{
		Zero();
	}

	void Zero()
	{
		modeBlend=(BYTE)Blend_Opaque;
		modeStencil=(BYTE)Sten_Disable;
		modeDepth=(BYTE)Depth_Default;
		modeAlphaTest=(BYTE)AlphaTest_Disable;
		modeFacing=(BYTE)Facing_Front;

		refStencil=0;
		maskStencil=0xffff;
		refDepth=0.0f;
		refAlphaTest=0;

		reserved=0;
	}

	void SetInvalid()
	{
		modeBlend=(BYTE)Blend_Invalid;
		modeStencil=(BYTE)Sten_Invalid;
		modeDepth=(BYTE)Depth_Invalid;
		modeAlphaTest=(BYTE)AlphaTest_Invalid;
		modeFacing=(BYTE)Facing_Invalid;
	}


	BYTE modeBlend;//a ShaderBlendMode value
	BYTE modeStencil;//a StencilMode value
	BYTE modeDepth;//a DepthMode
	BYTE modeAlphaTest;//an AlphaTestMode value

	WORD refStencil;
	WORD maskStencil;
	float refDepth;
	WORD refAlphaTest;

	BYTE modeFacing;//a FacingMode value
	BYTE reserved;
};


struct ShaderBlend
{
	DWORD mode;//an LayorBlendMode value
	DWORD ref;//not used yet
};

struct StencilOp
{
	DWORD mode;//a StencilMode
	WORD ref;
	WORD mask;
};

struct DepthMethod
{
	DWORD mode;
	float ref;//not used yet
};




// struct FeatureParam
// {
// 	FeatureParam()
// 	{
// 		memset(ep_name,0,sizeof(ep_name));
// 	}
// 	void SetEP(const char *name)
// 	{
// 		strncpy(ep_name,name,sizeof(ep_name)-1);
// 		ep_name[sizeof(ep_name)-1]=0;
// 	}
// 	char ep_name[32];
// 	GVar var;
// 
// 	//comparing by ep_name
// 	bool operator>(const FeatureParam&src)
// 	{
// 		return (strcmp(ep_name,src.ep_name)>0);
// 	}
// 
// 	bool operator==(const FeatureParam&src)
// 	{
// 		if (strcmp(ep_name,src.ep_name)!=0)
// 			return false;
// 		if (!var.Equal((GVar&)src.var))
// 			return false;
// 		return true;
// 	}
// 
// 	FeatureParam& operator=(const FeatureParam&src)
// 	{
// 		memcpy(ep_name,src.ep_name,sizeof(ep_name));
// 		var=src.var;
// 		return *this;
// 	}
// };

struct DummyInfo
{
	enum
	{
		BoundType_Sphere,
		BoundType_AABB,
		BoundType_Point,
		BoundType_Capsule,
		BoundType_UNKNOWN=0xffffffff,
	};
	DummyInfo()
	{
		bt=BoundType_UNKNOWN;
		idxBone=0;
	}

	i_math::spheref * getSphere()
	{	
		if(bt!=BoundType_Sphere)
			return NULL;
		return (i_math::spheref*)(data);
	}
	i_math::aabbox3df * getAAbb()
	{
		if(bt!=BoundType_AABB)
			return NULL;
		return (i_math::aabbox3df*)(data);
	}
	i_math::vector3df * getPoint()
	{
		if(bt!=BoundType_Point)
			return NULL;
		return (i_math::vector3df *)(data);
	}
	i_math::capsulef * getCapsule()
	{
		if(bt!=BoundType_Capsule)
			return NULL;
		return(i_math::capsulef *)(data);
	}
	void setType(DWORD dummytype)
	{
		switch(dummytype)
		{
		case BoundType_AABB:
			{
				assert(sizeof(data)>=sizeof(i_math::aabbox3df));
				
				i_math::aabbox3df aabb;

				if(bt==BoundType_Point)
				{
					i_math::vector3df * p = getPoint();
					i_math::vector3df len(0.5f,0.5f,0.5f);
					
					aabb.MaxEdge = len + *p;
					aabb.MinEdge = -len + *p;
				}
				else if (bt == BoundType_Sphere)
				{
					i_math::spheref * sph = getSphere();

					i_math::vector3df c = sph->center;
					
					float r = sph->radius;
					i_math::vector3df len(r,r,r);

					aabb.MinEdge = c - len;
					aabb.MaxEdge = c + len;
				}
				
				memcpy(data,&aabb,sizeof(i_math::aabbox3df));
			
				break;
			}
		case BoundType_Sphere:
			{
				assert(sizeof(data)>=sizeof(i_math::spheref));

				i_math::spheref sph;

				if(bt==BoundType_AABB)
				{
					i_math::aabbox3df * abb = getAAbb();
					
					sph.center = abb->getCenter();
					sph.radius = float(abb->MinEdge.getDistanceFrom(abb->MaxEdge))/2.0f;
				}
				else if(bt==BoundType_Point)
				{
					i_math::vector3df * center = getPoint();
					sph.setCenter(*center);
					sph.setRadius(0.5f);
				}
				
				memcpy(data,&sph,sizeof(i_math::spheref));

				break;
			}
		case BoundType_Point:
			{
				assert(sizeof(data)>=sizeof(i_math::vector3df));
				
				i_math::vector3df center;

				if(bt==BoundType_AABB)
				{
					i_math::aabbox3df * abb = getAAbb();
					center = abb->getCenter();
				}
				else if(bt==BoundType_Sphere)
				{
					i_math::spheref * sph = getSphere();
					center = sph->center;
				}

				memcpy(data,&center,sizeof(i_math::vector3df));

				break;
			}
		case BoundType_Capsule:
			{
				assert(sizeof(data)>=sizeof(i_math::capsulef));
				break;
			}
		default:
			break;
		}

		bt = dummytype;
	}
	
	DWORD & getBoundType(){return bt;}

	DWORD GetVersion(){return 2;}

	DWORD idxBone;
	i_math::matrix43f matOff;
	i_math::matrix43f matOffInv;
protected:
	float data[7];	
	DWORD bt;
};

#define MAX_LOD_LEVEL 10
#define MAX_BB_IMAGES 10
#define MAX_NUM_WINDS 10
#define MAX_NUM_WINDANGLES 10
#define MAX_NUM_WINDMATRICES 10
//SpeedWind����������
struct SptWndCfg
{
	struct Range
	{
		void Set(float la,float ha,float ls,float hs)
		{
			fLowAngle=la;
			fHiAngle=ha;
			fLowSpeed=ls;
			fHiSpeed=hs;
		}
		float fLowAngle;
		float fHiAngle;
		float fLowSpeed;
		float fHiSpeed;

		BEGIN_GOBJ_PURE(Range,1);
			GELEM_VAR_INIT(float,fLowAngle,0.1f);
				GELEM_EDITVAR("��Сƫ��",GVT_F,GSem(GSem_Float,"0,360.0f,0.1f"),"��С��ƫת�Ƕ�");
			GELEM_VAR_INIT(float,fHiAngle,0.1f);
				GELEM_EDITVAR("���ƫ��",GVT_F,GSem(GSem_Float,"0,360.0f,0.1f"),"����ƫת�Ƕ�");
			GELEM_VAR_INIT(float,fLowSpeed,0.1f);
				GELEM_EDITVAR("��С�ٶ�",GVT_F,GSem(GSem_Float,"0.01f,1.0f,0.01f"),"��С�Ľ��ٶ�");
			GELEM_VAR_INIT(float,fHiSpeed,0.1f);
				GELEM_EDITVAR("����ٶ�",GVT_F,GSem(GSem_Float,"0.01f,1.0f,0.01f"),"���Ľ��ٶ�");
		END_GOBJ();
	};

	struct Gust
	{
		//GetGusting()
		float fGustStrengthMin;
		float fGustStrengthMax;
		float fGustDurationMin;
		float fGustDurationMax;
		float fGustFrequency;
		
		BEGIN_GOBJ_PURE(Gust,1);
			GELEM_VAR_INIT(float,fGustFrequency,4.0f);
				GELEM_EDITVAR("Ƶ��",GVT_F,GSem(GSem_Float,"0,100,1.0f"),"ǿ����ֵ�Ƶ��");
			GELEM_VAR_INIT(float,fGustStrengthMin,0.25f);
				GELEM_EDITVAR("��Сǿ��",GVT_F,GSem(GSem_Float,"0,1.0f,0.01f"),"ǿ����Сǿ��");
			GELEM_VAR_INIT(float,fGustStrengthMax,1.0f);
				GELEM_EDITVAR("���ǿ��",GVT_F,GSem(GSem_Float,"0,1.0f,0.01f"),"ǿ�����ǿ��");
			GELEM_VAR_INIT(float,fGustDurationMin,2.0f);
				GELEM_EDITVAR("��С����ʱ��",GVT_F,GSem(GSem_Float,"0,100.0f,0.1f"),"ǿ�����С����ʱ��");
			GELEM_VAR_INIT(float,fGustDurationMax,7.0f);
				GELEM_EDITVAR("������ʱ��",GVT_F,GSem(GSem_Float,"0,100.0f,0.1f"),"ǿ���������ʱ��");
		END_GOBJ()

	};
	
	BEGIN_GOBJ_PURE(SptWndCfg,1)
	
		GELEM_OBJ(Gust,gust);
			GELEM_EDITOBJ_EX("ǿ��","ǿ��Ĳ���",GSem_Unknown);
		
		GELEM_VAR_INIT(float,fBranchExponents,2.0f);
			GELEM_EDITVAR("����ָ��",GVT_F,GSem(GSem_Float,"0.01f,20.0f,0.1f"),"���ɷ��ǿ��ָ�����ߵ�ָ��");
		GELEM_OBJ(Range,branchHor);
			GELEM_EDITOBJ_EX("����ˮƽ�ڶ�","���ɵ�ˮƽ�ڶ�",GSem_Unknown);
		GELEM_OBJ(Range,branchVer);
			GELEM_EDITOBJ_EX("���ɴ�ֱ�ڶ�","���ɵĴ�ֱ�ڶ�",GSem_Unknown);
		
		GELEM_VAR_INIT(float,fLeafExponents,2.0f);
			GELEM_EDITVAR("Ҷ��ָ��",GVT_F,GSem(GSem_Float,"0.01f,20.0f,0.1f"),"Ҷ�ӷ��ǿ��ָ�����ߵ�ָ��");
		GELEM_OBJ(Range,leafRustling)
			GELEM_EDITOBJ_EX("��Ҷ����","��Ҷ������Ļ��������",GSem_Unknown);
		GELEM_OBJ(Range,leafRocking)
			GELEM_EDITOBJ_EX("��Ҷ�ڶ�","��Ҷ����Ļ����ڶ�",GSem_Unknown);

		GELEM_VAR_INIT(float,fMaxBend,35.0f);
			GELEM_EDITVAR("����",GVT_F,GSem(GSem_Float,"0,360.0f,1.0f"),"���������Ƕ�");
		GELEM_VAR_INIT(float,fResponse,0.5f);
			GELEM_EDITVAR("����",GVT_F,GSem(GSem_Float,"0,1.0f,0.01f"),"��ǿ�ȸı������");
		GELEM_VAR_INIT(float,fResponseLimit,0.001f);
			GELEM_EDITVAR("����",GVT_F,GSem(GSem_Float,"0,1.0f,0.001f"),"ÿ���ܸı�����ֵ��������ͻ�䡣");	
		
		GELEM_VAR_INIT(int,nMatrices,6);
		GELEM_VAR_INIT(int,nLeafAngles,8);
		GELEM_VAR_INIT(float,fStrength,1.0f);
		GELEM_VAR_INIT(i_math::vector3df,direction,i_math::vector3df(0,0,1.0f));

		END_GOBJ()

	Gust gust;

	Range branchHor;//GetBranchHorizontal()
	Range branchVer;//GetBranchVertical()
	Range leafRocking;//GetLeafRocking()
	Range leafRustling;//GetLeafRustling()

	//GetExponents()
	float fBranchExponents;
	float fLeafExponents;

	float fMaxBend;//GetMaxBendAngle()

	//GetWindResponse()
	float fResponse;
	float fResponseLimit;

	//GetQuntities()
	int nMatrices;
	int nLeafAngles;

	//strength and direction
	float fStrength;
	i_math::vector3df direction;

};
