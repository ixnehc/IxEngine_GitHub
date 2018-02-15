/********************************************************************
	created:	2006/8/8   11:15
	filename: 	e:\IxEngine\Proj_RenderSystem\Camera.cpp
	author:		cxi
	
	purpose:	camera,ICamera implement
*********************************************************************/
#include "stdh.h"
//#include "Base.h"
#include "interface/interface.h"

#include "RenderSystem/ITexture.h"

#include "RenderSystem/IShader.h"
#include "shaderlib/SLFeature.h"

//#include "DeviceObject.h"

#include "Camera.h"

#include "Log/LastError.h"
#include "Log\LogFile.h"

#include "assert.h"


CCamera::CCamera()
{
	_type=Perspective;

	_near=0.1f;
	_far=100.0f;
	_fov=1.0;
	_ratio=1.0;

	_dirty=TRUE;

	_clip=FALSE;
}

BOOL CCamera::Init()
{
	SetPosTarget(vector3df(0,0,0),vector3df(0,0,1),vector3df(0,1,0));
	SetNearFar(0.1f,100.0f);
	SetFov(Pi/2.0f);
	SetAspectRatio(1.0f);

	return TRUE;
}
void CCamera::UnInit()
{
}

BOOL CCamera::IsPerspective()
{
	return _type==Perspective;
}
BOOL CCamera::IsOrtho()
{
	return _type==OffCenterOrtho;
}


BOOL CCamera::SetPosTarget(i_math::vector3df &eye,i_math::vector3df &at,i_math::vector3df &up)
{
	_eye=eye;
	_at=at;
	_up=up;
	


	_dirty=TRUE;

	return TRUE;
}

//use (0,1,0) as up
BOOL CCamera::SetPosTarget(i_math::vector3df &eye,i_math::vector3df &at)
{
	return SetPosTarget(eye,at,i_math::vector3df(0,1,0));
}


BOOL CCamera::SetNearFar(i_math::f32 distNear,i_math::f32 distFar)
{
	_near=distNear;
	_far=distFar;

	_dirty=TRUE;
	return TRUE;
}

BOOL CCamera::GetNearFar(i_math::f32 &distNear,i_math::f32 &distFar)
{
	distNear=_near;
	distFar=_far;
	return TRUE;
}


//angle in rad
BOOL CCamera::SetFov(i_math::f32 fov)
{
	_type=Perspective;
	_fov=fov;
	_dirty=TRUE;
	return TRUE;
}

//ver/hor
BOOL CCamera::SetAspectRatio(i_math::f32 r)
{
	_ratio=r;
	_dirty=TRUE;
	return TRUE;
}

BOOL CCamera::SetOffCenterOrtho(f32 l,f32 t,f32 r,f32 b)
{
	_type=OffCenterOrtho;
	_l=l;_t=t;_r=r;_b=b;
	_dirty=TRUE;
	return TRUE;
}

BOOL CCamera::SetOffCenterOrtho(i_math::rectf &rc)
{
	return SetOffCenterOrtho(rc.Left(),rc.Top(),rc.Right(),rc.Bottom());
}

BOOL CCamera::GetOffCenterOrtho(i_math::rectf &rc)
{
	if (_type!=OffCenterOrtho)
		return FALSE;
	rc.set(_l,_t,_r,_b);
	return TRUE;
}



void CCamera::_UpdateViewProj()
{
	if (!_dirty)
		return;

	_view.buildCameraLookAtMatrixLH(_eye,_at,_up);
	
	if (_type==Perspective)
		_proj.buildProjPerspectiveFovLH(_fov,_ratio,_near,_far);
	if (_type==OffCenterOrtho)
		_proj.buildProjOrthoOffCenterLH(_l,_t,_r,_b,_near,_far);


	_viewproj=_view*_proj;
	_dirty=FALSE;
}


//bind to device
BOOL CCamera::Bind(IShader *shader)
{
	_UpdateViewProj();
	shader->SetEP_ViewProj(_viewproj,_view,_proj);
	shader->SetEP(EP_camerapos,_eye);
	shader->SetEP(EP_nearfardist,i_math::vector4df(_near,_far,0,0));
	if (_clip)
		shader->SetEP(EP_clipplane,_clipequation);

	return TRUE;
}

//transform a position from world space to projected space
void CCamera::TransPos(i_math::vector3df &pos)
{
	_UpdateViewProj();
	i_math::vector3df pos2;
	_viewproj.transformVect(pos,pos2);
	pos=pos2;
}

//transform a position from projected space to world space
void CCamera::TransPosInverse(i_math::vector3df &pos)
{
	_UpdateViewProj();
	i_math::vector3df pos2;
	matrix44f viewprojInv;
	_viewproj.getInverse(viewprojInv);
	viewprojInv.transformVect(pos,pos2);
	pos=pos2;
}

BOOL CCamera::CalcHitProbe(i_math::line3df &line,int x,int y,i_math::recti &rc,float length)
{
	if (!rc.isValid())
		return FALSE;
	if (!rc.isPointInside(i_math::pos2di(x,y)))
		return FALSE;

	vector3df vOnNearPlane,vEye,vAt,vDir;
	vOnNearPlane.x=(float)(x-rc.Left())*2.0f/(float)rc.getWidth()-1.0f;
	vOnNearPlane.y=(float)(rc.Bottom()-y)*2.0f/(float)rc.getHeight()-1.0f;
	vOnNearPlane.z=0.0f;
	TransPosInverse(vOnNearPlane);
	if (IsPerspective())
	{
		GetEyePos(vEye);
		vDir=vOnNearPlane-vEye;
		vDir.setLength(length);
		line.setLine(vEye,vEye+vDir);
	}
	else
	{
		i_math::f32 distNear,distFar;
		GetNearFar(distNear,distFar);
		GetEyePos(vEye);
		GetEyeLookAt(vAt);
		vDir=vAt-vEye;
		vDir.setLength(distNear);
		vEye=vOnNearPlane-vDir;
		vDir.setLength(length);
		line.setLine(vEye,vEye+vDir);
	}

	return TRUE;
}



//copy content from camSrc
void CCamera::Clone(ICamera *camSrc)
{
	CCamera *cam=(CCamera *)camSrc;

	_type=cam->_type;

	_dirty=cam->_dirty;
	_viewproj=cam->_viewproj;
	_view=cam->_view;
	_proj=cam->_proj;

	_eye=cam->_eye;
	_at=cam->_at;
	_up=cam->_up;
	_near=cam->_near;
	_far=cam->_far;

	_fov=cam->_fov;
	_ratio=cam->_ratio;

	_l=cam->_l;
	_t=cam->_t;
	_r=cam->_r;
	_b=cam->_b;

	_clip=cam->_clip;
	_clipplane=cam->_clipplane;
	_clipequation=cam->_clipequation;
}


BOOL CCamera::GetViewFrustum(i_math::volumeCvxf &vol)
{
	if (_type==Perspective)
	{
		_UpdateViewProj();
		vol.fromViewProj(_viewproj);
		return TRUE;
	}

	if (_type==OffCenterOrtho)
	{
		_UpdateViewProj();

		//donot know how to get it from the _viewproj,we use the camera parameters to build one

		//6 planes in camera space,
		//IMPORTANT:should be in the order of n,f,l,r,t,b,same as in volume.h
		vol.nPlanes=6;
		i_math::plane3df *pl=vol.planes;
		pl[0]=i_math::plane3df(		0,0,_near,	0,0,-1);//near
		pl[1]=i_math::plane3df(		0,0,_far,		0,0,1);//far
		pl[2]=i_math::plane3df(		_l,0,0,			-1,0,0);//left
		pl[3]=i_math::plane3df(		_r,0,0,			1,0,0);//right
		//NOTE: switch the top/down here.We take _t as the small value,_b as the bigger value,
		//but in the 3D coordinate,top has bigger value than bottom in y-axis
		pl[4]=i_math::plane3df(		0,_b,0,			0,1,0);//top				
		pl[5]=i_math::plane3df(		0,_t,0,			0,-1,0);//bottom		

		//convert to world space
		i_math::matrix44f viewInv;
		_view.getInverse(viewInv);
		for (int i=0;i<6;i++)
			viewInv.transformPlane(pl[i]);

		return TRUE;
	}

	return FALSE;
}

BOOL CCamera::GetEyeMat(i_math::matrix43f &mat)
{
	i_math::matrix44f t;
	if (FALSE==GetView(t))
		return FALSE;

	mat43from44(mat,t);
	mat.makeInverse();

	return TRUE;
}


BOOL CCamera::GetView(i_math::matrix44f &mat)
{
	_UpdateViewProj();
	if ((_type==Perspective)||(_type==OffCenterOrtho))
	{
		mat=_view;
		return TRUE;
	}
	return FALSE;
}

BOOL CCamera::GetProj(i_math::matrix44f &mat)
{
	_UpdateViewProj();
	if ((_type==Perspective)||(_type==OffCenterOrtho))
	{
		mat=_proj;
		return TRUE;
	}
	return FALSE;
}

BOOL CCamera::GetXAxis(i_math::vector3df &dir)
{
	_UpdateViewProj();
	dir.x=_view(0,0);
	dir.y=_view(0,1);
	dir.z=_view(0,2);
	return TRUE;
}

BOOL CCamera::GetYAxis(i_math::vector3df &dir)
{
	_UpdateViewProj();
	dir.x=_view(1,0);
	dir.y=_view(1,1);
	dir.z=_view(1,2);
	return TRUE;
}

BOOL CCamera::GetZAxis(i_math::vector3df &dir)
{
	_UpdateViewProj();
	dir.x=_view(2,0);
	dir.y=_view(2,1);
	dir.z=_view(2,2);
	return TRUE;
}



BOOL CCamera::GetViewProj(i_math::matrix44f &mat)
{
	_UpdateViewProj();
	if ((_type==Perspective)||(_type==OffCenterOrtho))
	{
		mat=_viewproj;
		return TRUE;
	}
	return FALSE;
}

BOOL CCamera::GetViewProj_Raw(i_math::matrix44f &mat)
{
	mat=_viewproj;
	return TRUE;
}


BOOL CCamera::EnableClipPlane(BOOL bEnable,i_math::plane3df *plane)
{
	if ((_type!=Perspective)&&(bEnable))
		return FALSE;
	_clip=bEnable;
	if (_clip)
	{
		if (plane)
			_clipplane=*plane;
		else
			_clipplane=plane3df();

		_clipplane.getEquation(_clipequation);
	}



	return TRUE;
}

//if clip is not enabled ,return NULL
i_math::plane3df *CCamera::GetClipPlane()
{
	if (!_clip)
		return NULL;
	return &_clipplane;
}

BOOL CCamera::GetProjScaleMask(i_math::recti &rcScrn,i_math::vector3df &pos,i_math::matrix43f &matScale)
{
	i_math::matrix44f matProj, matView;	

	GetProj(matProj);
	GetView(matView);

	i_math::vector3df pos2(0,0,0);
	matView.transformVect(pos,pos2);
	i_math::vector3df posA(0,0,pos2.z),posB(1,1,pos2.z),posC(0,0,0),posD(0,0,0);

	matProj.transformVect(posA,posC);
	matProj.transformVect(posB,posD);

	i_math::recti rc=rcScrn;

	i_math::vector2df pos0,pos1;

	pos0.x = rc.getWidth()*(posC.x+1.0f)/2.0f;
	pos0.y = rc.getHeight()*(1.0f-(posD.y+1.0f)/2.0f);

	pos1.x = rc.getWidth()*(posD.x+1.0f)/2.0f;
	pos1.y = rc.getHeight()*(1.0f-(posD.y+1.0f)/2.0f);

	float fdist0 = (float)posB.getDistanceFrom(posA);
	float fdist1 = (float)pos0.getDistanceFrom(pos1);

	float ratio = fdist0/fdist1;

	matScale.setScale(ratio,ratio,ratio);

	return TRUE;
}


//the first is intended one,the others are fallbacks
FeatureCode *CCamera::GetFC(DWORD &nFC)
{
	static FeatureCode fc[]={FC_clipplane};
	static FeatureCode fcNone[]={FC_none};
	nFC=1;
	if (_clip)
		return fc;
	return fcNone;
}

void CCamera::GetFrustumCorners(i_math::vector3df * corners)
{	
	i_math::f32  fNear,fFar,radius,fRatio;

	fNear = _near;
	fFar = _far;
	fRatio = _ratio;
	radius = _fov/2.0f;
	/*
	1----0
	|	 |		x--->
	2----3		
	*/
	// the game camera may not roll, as say that it don't ratate around Z axis.
	float wn,hn,wf,hf ,w,h;

	h = tan(radius);
	w = fRatio*h;

	wn = fNear*w;
	hn = fNear*h;

	wf = fFar*w;
	hf = fFar*h;

	/*
	1----0			5----4
	|	 |			|	 |
	2----3			6----7
	// near			fear
	*/
#define CCamera_AssignCorner(i,x0,y0,z0)	\
	{	\
	corners[i].x = x0;		\
	corners[i].y = y0;		\
	corners[i].z = z0;		\
	}

	CCamera_AssignCorner(0,wn,hn,fNear);
	CCamera_AssignCorner(1,-wn,hn,fNear);
	CCamera_AssignCorner(2,-wn,-hn,fNear);
	CCamera_AssignCorner(3,wn,-hn,fNear);

	CCamera_AssignCorner(4,wf,hf,fFar);
	CCamera_AssignCorner(5,-wf,hf,fFar);
	CCamera_AssignCorner(6,-wf,-hf,fFar);
	CCamera_AssignCorner(7,wf,-hf,fFar);

	i_math::matrix43f mat;
	i_math::vector3df dz,dx,dy;
	dz = (_at-_eye).normalize();
	dx = _up.crossProduct(dz);
	dx.normalize();
	dy = dz.crossProduct(dx);
	dy.normalize();

	mat.set(dx.x,dx.y,dx.z,
		dy.x,dy.y,dy.z,
		dz.x,dz.y,dz.z,
		_eye.x,_eye.y,_eye.z);

	for(int i = 0; i< 8;i++)
		mat.transformVect(corners[i],corners[i]);
}
