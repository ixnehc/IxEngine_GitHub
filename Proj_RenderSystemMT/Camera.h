#pragma once

#include "RenderSystem/ITools.h"


//#include "base.h"

#include "class/class.h"

class CDeviceObject;
class CShaderMgr;
class CRenderSystem;

class CCamera:public ICamera
{
public:
	DEFINE_CLASS(CCamera)
	CCamera();
	BOOL Init();
	void UnInit();


	IMPLEMENT_REFCOUNT_C;
	virtual BOOL IsPerspective();
	virtual BOOL IsOrtho();
	virtual BOOL SetPosTarget(i_math::vector3df &eye,i_math::vector3df &at,i_math::vector3df &up);
	virtual BOOL SetPosTarget(i_math::vector3df &eye,i_math::vector3df &at);//use (0,1,0) as up
	virtual BOOL SetNearFar(i_math::f32 distNear,i_math::f32 distFar);
	virtual BOOL GetNearFar(i_math::f32 &distNear,i_math::f32 &distFar);
	virtual BOOL SetFov(i_math::f32 fov);//angle in rad
	virtual BOOL SetAspectRatio(i_math::f32 r);//ver/hor
	virtual BOOL SetOffCenterOrtho(f32 l,f32 t,f32 r,f32 b);
	virtual BOOL SetOffCenterOrtho(i_math::rectf &rc);
	virtual BOOL GetOffCenterOrtho(i_math::rectf &rc);
	virtual BOOL Bind(IShader *shader);//bind to shader
	virtual void TransPos(i_math::vector3df &pos);//transform a position from world space to projected space
	virtual void Clone(ICamera *camSrc);//copy content from camSrc
	virtual BOOL GetEyePos(i_math::vector3df &eye)	{		eye=_eye;return TRUE;	}
	virtual BOOL GetEyeLookAt(i_math::vector3df &at)	{at=_at;return TRUE;}
	virtual BOOL GetEyeDir(i_math::vector3df &dir)	{dir=(_at-_eye).normalize();return TRUE;}
	virtual BOOL GetEyeMat(i_math::matrix43f &mat);
	virtual BOOL GetFov(i_math::f32 &fov)	{		fov=_fov;		return TRUE;	}
	virtual BOOL GetAspectRatio(i_math::f32 &r)	{		r=_ratio;		return TRUE;	}
	virtual BOOL GetViewFrustum(i_math::volumeCvxf &vol);
	virtual BOOL GetView(i_math::matrix44f &mat);
	virtual BOOL GetProj(i_math::matrix44f &mat);
	virtual BOOL GetViewProj(i_math::matrix44f &mat);
	virtual BOOL GetViewProj_Raw(i_math::matrix44f &mat);
	virtual BOOL GetXAxis(i_math::vector3df &dir);
	virtual BOOL GetYAxis(i_math::vector3df &dir);
	virtual BOOL GetZAxis(i_math::vector3df &dir);
	virtual void TransPosInverse(i_math::vector3df &pos);//transform a position from projected space to world space
	virtual BOOL CalcHitProbe(i_math::line3df &probe,int x,int y,i_math::recti &rcViewport,float length);
	virtual BOOL EnableClipPlane(BOOL bEnable,i_math::plane3df *plane);
	virtual i_math::plane3df *GetClipPlane();//if clip is not enabled ,return NULL
	virtual BOOL GetProjScaleMask(i_math::recti &rcScrn,i_math::vector3df &pos,i_math::matrix43f &matScale);
	virtual FeatureCode *GetFC(DWORD &nFC);//the first is intended one,the others are fallbacks
	virtual void GetFrustumCorners(i_math::vector3df * corners);

protected:
	enum _Type
	{
		Perspective,
		OffCenterOrtho,
	};

	_Type _type;

	void _UpdateViewProj();
	BOOL _dirty;
	matrix44f _viewproj;
	matrix44f _view;
	matrix44f _proj;

	vector3df _eye,_at,_up;
	f32 _near,_far;
	f32 _fov,_ratio;
	f32 _l,_t,_r,_b;//offcenter ortho rect

	BOOL _clip;
	i_math::plane3df _clipplane;//note this plane is in world space,and is valid when _clip is TRUE
	i_math::vector4df _clipequation;
};
