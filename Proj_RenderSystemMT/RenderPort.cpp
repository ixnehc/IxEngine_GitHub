/********************************************************************
	created:	2006/8/8   9:49
	filename: 	e:\IxEngine\Proj_RenderSystem\RenderPort.cpp
	author:		cxi
	
	purpose:	render port,IRenderPort implement,used to render 3d stuff
*********************************************************************/
#include "stdh.h"
#include "Base.h"
#include "interface/interface.h"
#include "DeviceObject.h"

#include "FontMgr.h"
#include "RenderSystem/ITools.h"
#include "RenderSystem.h"

#include "RenderPort.h"

#include "ShaderLibMgr.h"

#include "Renderer.h"

#include "Log/LastError.h"
#include "Log\LogFile.h"

#include "timer/profiler.h"

#include "assert.h"

#include "vertexfmt/vertexfmt.h"

#pragma warning (disable: 4018)
#pragma warning (disable: 4244)

#define NEG_OFF 0
#define POS_OFF 0

#define IS_PORT_LOCKED(rp) (((CRenderSystem*)_pRS)->IsPortLocked(rp))
#define LOCK_PORT(rp) ((CRenderSystem*)_pRS)->LockPort(rp)
#define UNLOCK_PORT(rp) if (_pRS) ((CRenderSystem*)_pRS)->UnLockPort(rp)




//////////////////////////////////////////////////////////////////////////
//CRenderPort

CRenderPort::CRenderPort()
{
	Zero();
}

CRenderPort::~CRenderPort()
{
	UnInit();
}

CRenderPort::_State *CRenderPort::_TopState()
{
	assert(_states.size()>0);
	return &_states[_states.size()-1];
}



void CRenderPort::Zero()
{
	_pdevobj=NULL;
	_renderer=NULL;
	_pvm=NULL;
	_pRS=NULL;

	_quad=NULL;
	_line=NULL;
	_border=NULL;
	_geo=NULL;
	_surfTxt=NULL;
	_camOrtho=NULL;

	_states.clear();

	_draw2d=NULL;
	_draw3d=NULL;
	_fill=NULL;

	_blank=NULL;
}


BOOL CRenderPort::Init(IRenderSystem *pRS)
{
	_pRS=pRS;
	_pdevobj=((CRenderSystem*)pRS)->GetDeviceObj();
	_renderer=(CRenderer*)(((CRenderSystem*)pRS)->GetRenderer());
	_pvm=pRS->GetVertexMgr();

	_camOrtho=pRS->CreateCamera();

	PushState();

	_quad=_pvm->CreateVB(4,FVFEx_Quad,1,VBFlag_Dynamic);
	if (!_quad)
		goto fail;

	_line=_pvm->CreateVB(2,FVFEx_PosColor,1,VBFlag_Dynamic);
	if (!_line)
		goto fail;

	_border=_pvm->CreateVB(5,FVFEx_PosColor,1,VBFlag_Dynamic);
	if (!_border)
		goto fail;

	_geo=pRS->CreatePatchGeom();
	_surfTxt=_geo->AddSurf(FVFEx_Quad,4000,6000);

	_draw2d=(IShader*)pRS->GetShaderLibMgr()->ObtainShader("draw2d","draw2d");
	if (!_draw2d)
		goto fail;
	if (A_Ok!=_draw2d->Touch())
		goto fail;
	_draw3d=(IShader*)pRS->GetShaderLibMgr()->ObtainShader("draw2d","draw3d");
	if (!_draw3d)
		goto fail;
	if (A_Ok!=_draw3d->Touch())
		goto fail;
	_fill=(IShader*)pRS->GetShaderLibMgr()->ObtainShader("draw2d","fill");
	if (!_fill)
		goto fail;
	if (A_Ok!=_fill->Touch())
		goto fail;

	_blank=(ITexture*)pRS->GetTexMgr()->ObtainRes("_std\\blank.tga");
	_blank->ForceTouch();

	return TRUE;
fail:
	UnInit();

	return FALSE;
}

void CRenderPort::UnInit()
{
	UNLOCK_PORT(this);

	SAFE_RELEASE(_quad);
	SAFE_RELEASE(_line);
	SAFE_RELEASE(_border);
	SAFE_RELEASE(_geo);
	SAFE_RELEASE(_camOrtho);
	SAFE_RELEASE(_draw2d);
	SAFE_RELEASE(_draw3d);
	SAFE_RELEASE(_fill);
	SAFE_RELEASE(_blank);

	while(PopState());

	Zero();
}

BOOL CRenderPort::PushState()
{
	_states.resize(_states.size()+1);

	_TopState()->cam=_pRS->CreateCamera();

	if (_states.size()>1)//copy the camera and rect info
	{
		_states[_states.size()-1].cam->Clone(_states[_states.size()-2].cam);
		_states[_states.size()-1].rc=_states[_states.size()-2].rc;
	}

	return TRUE;
}

BOOL CRenderPort::PopState()
{
	if (_states.size()<=0)
		return FALSE;
	_State *s=_TopState();

	if (s->bRT)
		_pdevobj->PopRenderTarget();
	if (s->bDS)
		_pdevobj->PopDSBuffer();

	SAFE_RELEASE(s->cam);

	_states.resize(_states.size()-1);

	UNLOCK_PORT(this);

	return TRUE;
}


BOOL CRenderPort::SetRenderTarget(SurfHandle *rts,DWORD count)
{
	if (!rts)
		count=0;

	for (int i=0;i<count;i++)
	{
		if (rts[i].IsEmpty())
			return FALSE;
	}

	UNLOCK_PORT(this);
	_State *s=_TopState();

	if (s->bRT)
		_pdevobj->PopRenderTarget();
	s->bRT=FALSE;
	if (count>0)
	{
		if (_pdevobj->PushRenderTarget(rts,count))
			s->bRT=TRUE;
		else
			return FALSE;
	}

	return TRUE;
}


BOOL CRenderPort::SetDSBuffer(SurfHandle &ds)
{

	UNLOCK_PORT(this);
	_State *s=_TopState();

	if (s->bDS)
		_pdevobj->PopDSBuffer();
	s->bDS=FALSE;
	if (!ds.IsEmpty())
	{
		if (_pdevobj->PushDSBuffer(ds))
			s->bDS=TRUE;
		else
			return FALSE;
	}

	return TRUE;
}

void CRenderPort::SetRect(int left,int top,int right,int bottom)
{
	UNLOCK_PORT(this);

	_TopState()->rc.rcView.set(left,top,right,bottom);
	_TopState()->rc.bViewRect=TRUE;
}

void CRenderPort::SetRect(i_math::recti &rc)
{
	UNLOCK_PORT(this);

	_TopState()->rc.rcView=rc;
	_TopState()->rc.bViewRect=TRUE;
}

//Set to full size
void CRenderPort::SetRect_Total()
{
	UNLOCK_PORT(this);
	_TopState()->rc.bViewRect=FALSE;
}

void CRenderPort::GetRect(i_math::recti &rc)
{
	rc.set(0,0,0,0);
	if (_pdevobj)
		rc.set(0,0,_pdevobj->Width(),_pdevobj->Height());
	if(_TopState()->rc.bViewRect)
		rc.clipAgainst(_TopState()->rc.rcView);
}




//query to modify
ICamera* CRenderPort::QueryCamera()
{
	UNLOCK_PORT(this);
	return _TopState()->cam;
}

ICamera* CRenderPort::GetCamera()
{
	return _TopState()->cam;
}
BOOL CRenderPort::SetCamera(ICamera *cam)
{
	cam->AddRef();
	_State *s=_TopState();

	SAFE_RELEASE(s->cam);
	s->cam=cam;

	UNLOCK_PORT(this);
	return TRUE;
}

ICamera* CRenderPort::GetOrthoCamera()
{
	_PrepareDraw();
	return _camOrtho;
}



void CRenderPort::SetAutoRatio(BOOL bAutoRatio)
{
	_TopState()->bAutoRatio=bAutoRatio;
	UNLOCK_PORT(this);
}

BOOL CRenderPort::AdjustCameraRatio(ICamera *cam)
{
	recti rc;
	GetRect(rc);
	if (!rc.isValid())
		return FALSE;

	_State *s=_TopState();
	if (s->bAutoRatio)
		cam->SetAspectRatio(((f32)rc.getWidth())/((f32)rc.getHeight()));
	return TRUE;
}



IRenderer *CRenderPort::ObtainRenderer()
{
	if (!_PrepareDraw())
		return NULL;

	_renderer->ResetContent();

	assert(_TopState()->cam);
	((CRenderer *)_renderer)->BindCamera(_TopState()->cam);

	return _renderer;
}




BOOL CRenderPort::FillColor(DWORD col)
{
	if (!_PrepareDraw())
		return FALSE;

	if (!_pdevobj)
		return FALSE;

	recti rcView;
	GetRect(rcView);

	RECT rc;
	rectiToRECT(rcView,rc);

	return _pdevobj->ColorFill(col,0,&rc);
}

BOOL CRenderPort::FillColor(i_math::vector4df &col)
{
	PostProcessArg arg;
	arg.state.modeDepth=Depth_Disable;
	arg.epk.AddEP(EP_colDif,col);
	return _PostProcess((CShader *)_fill,arg);
}

BOOL CRenderPort::FillColor(i_math::recti &rc,i_math::vector4df &col)
{
	PostProcessArg arg;
	arg.state.modeDepth=Depth_Disable;
	arg.epk.AddEP(EP_colDif,col);
	arg.SetDest(rc.Left(),rc.Top(),rc.Right(),rc.Bottom());
	return _PostProcess((CShader *)_fill,arg);
}


BOOL CRenderPort::ClearBuffer(ClearBufferFlag flag,DWORD col,float z,DWORD s)
{
	if (!_PrepareDraw())
		return FALSE;

	if (!_pdevobj)
		return FALSE;

	recti rcView;
	GetRect(rcView);

	RECT rc;
	rectiToRECT(rcView,rc);

	return _pdevobj->ClearBuffer(flag,&rc,col,z,s);
}

BOOL CRenderPort::_PrepareDraw()
{
	if (IS_PORT_LOCKED(this))
		return TRUE;

	recti rc;
	GetRect(rc);
	if (!rc.isValid())
		return FALSE;

	_State *s=_TopState();

	ViewportInfo vi;
	vi.x=rc.Left();
	vi.y=rc.Top();
	vi.w=rc.getWidth();
	vi.h=rc.getHeight();
	vi.minz=0;
	vi.maxz=1;

	_pdevobj->SetViewport(vi);

	if (s->bAutoRatio)
		s->cam->SetAspectRatio(((f32)rc.getWidth())/((f32)rc.getHeight()));

	assert(_camOrtho);
	_camOrtho->SetOffCenterOrtho((f32)0,(f32)rc.getHeight(),(f32)rc.getWidth(),(f32)0);
//	_camOrtho->SetOffCenterOrtho((f32)0,(f32)0,(f32)rc.getWidth(),(f32)rc.getHeight());
	_camOrtho->SetNearFar(0,1);
	_camOrtho->SetPosTarget(vector3df(0,0,0),vector3df(0,0,1),vector3df(0,1,0));

	if (_draw2d)
		_draw2d->Touch();
	if (_draw3d)
		_draw3d->Touch();
	if (_fill)
		_fill->Touch();


	if (_fill)
		_fill->Touch();


	LOCK_PORT(this);

	return TRUE;
}


//convert a 3D pos to this port's coord
BOOL CRenderPort::TransPos(i_math::vector3df &v,int &x,int &y)
{
	assert(_TopState()->cam);

	i_math::vector3df pos;
	pos=v;
	_TopState()->cam->TransPos(pos);
	
	i_math::recti rc;
	GetRect(rc);
	x=(int)((pos.x-(-1.0f))*(i_math::f32)rc.getWidth()/2.0f);
	y=rc.getHeight()-(int)((pos.y-(-1.0f))*(i_math::f32)rc.getHeight()/2.0f);

	return TRUE;
}

BOOL CRenderPort::DrawTexture(ITexture *pTex,const DrawTextureArg &arg)
{
	CShader *shader=(CShader*)_draw2d;
	if (!shader)
		return FALSE;

	if (!_PrepareDraw())
		return FALSE;

	recti rc;
	GetRect(rc);

	if (!rc.isValid())
		return FALSE;

	if (A_Ok!=pTex->Touch())
		return FALSE;

	recti rcDest,rcDest0,rcSrc,rcSrc0;
	int w,h;
	w=pTex->GetWidth();
	h=pTex->GetHeight();

	rcSrc0.set(0,0,w,h);
	rcSrc=rcSrc0;
	if (arg.bSrcRect)
		rcSrc.clipAgainst(arg.rcSrc);
	rcSrc.Left()-=rcSrc0.Left();
	rcSrc.Right()-=rcSrc0.Left();
	rcSrc.Top()-=rcSrc0.Top();
	rcSrc.Bottom()-=rcSrc0.Top();

	rcDest=arg.rcDest;
	if (!arg.bDestRect)
	{
		rcDest.Right()=rcDest.Left()+rcSrc.getWidth();
		rcDest.Bottom()=rcDest.Top()+rcSrc.getHeight();
	}

	if ((!rcSrc.isValid())||(!rcDest.isValid()))
		return FALSE;

	rcDest0=rcDest;	
	rcDest+=rc.UpperLeftCorner;

	if (!rcDest.isRectCollided(rc))
		return TRUE;//totally clipped

	if (TRUE)
	{
		VtxQuad *p=(VtxQuad *)_quad->Lock(TRUE);
		if (!p)
			return FALSE;
		MakeQuad<VtxQuad>(p,NULL,0,rcDest0,1.0f,rcSrc,i_math::size2di(w,h),0xffffffff);

		_quad->Unlock();
	}

	if (TRUE)
	{
		VBBindArg arg;
		arg.dpt=5;
		shader->BindVB(_quad,NULL,&arg);
	}

	i_math::matrix44f proj2D;
	_camOrtho->GetProj(proj2D);
	shader->SetEP(EP_proj2D,proj2D);

	shader->SetEP(EP_diffusemap,pTex);
	ShaderState state;
	if (!arg.bForceOpaque)
		state.modeBlend=Blend_AlphaBlend;
	state.modeDepth=Depth_Disable;
	shader->SetState(state);

	shader->DoShade();

	return TRUE;
}

BOOL CRenderPort::_PostProcess(CShader *shader,PostProcessArg &arg)
{
	if (!_PrepareDraw())
		return FALSE;

	recti rc;
	GetRect(rc);

	if (!rc.isValid())
		return FALSE;

	recti rcDest,rcDest0;

	rcDest=arg.rcDest;
	if (!arg.bDestRect)
		rcDest.set(0,0,rc.getWidth(),rc.getHeight());

	if (!rcDest.isValid())
		return FALSE;

	rcDest0=rcDest;	
	rcDest+=rc.UpperLeftCorner;

	if (!rcDest.isRectCollided(rc))
		return TRUE;//totally clipped


	//bind the ppmap and the ppmap size
	for (int i=0;i<ARRAY_SIZE(arg.maps);i++)
	{
		if (arg.maps[i])
		{
			if (EPFail==shader->SetEP(EffectParam(EP_ppmap01+i),arg.maps[i]))
				return FALSE;
			i_math::vector2df sz;
			sz.set(arg.maps[i]->GetWidth(),arg.maps[i]->GetHeight());
			shader->SetEP(EffectParam(EP_ppmapsize01+i),sz);
		}
	}

	i_math::matrix44f proj2D;
	_camOrtho->GetProj(proj2D);
	shader->SetEP(EP_proj2D,proj2D);

	if (arg.cam)
		arg.cam->Bind(shader);
	else
		_TopState()->cam->Bind(shader);


	//bind the rt size
	shader->SetEP(EP_rtsize,i_math::vector2df(rcDest.getWidth(),rcDest.getHeight()));

	shader->SetEPs(arg.epk.buf,arg.epk.nEP);

	shader->SetState(arg.state);

	if (TRUE)
	{
		VtxQuad *p=(VtxQuad *)_quad->Lock(TRUE);
		if (!p)
			return FALSE;
		i_math::recti rcT;
		rcT.set(arg.rcSrc.Left()*8192.0f,arg.rcSrc.Top()*8192.0f,
			arg.rcSrc.Right()*8192.0f,arg.rcSrc.Bottom()*8192.0f);
		MakeQuad<VtxQuad>(p,NULL,0,rcDest0,1.0f,rcT,i_math::size2di(8192,8192),0xffffffff);
		_quad->Unlock();
	}


	if (TRUE)
	{
		VBBindArg arg;
		arg.dpt=5;
		shader->BindVB(_quad,NULL,&arg);
	}

	return shader->DoShade();
}


BOOL CRenderPort::PostProcess(const char *nameLib,const char *nameUF,const PostProcessArg &arg)
{
	CShader *shader=(CShader*)_pRS->GetShaderLibMgr()->ObtainShader(nameLib,nameUF);
	if (!shader)
	{
		LogFile::Prompt("Error:Failed to load shader of \"%s\" in shader lib \"%s\"!",
					nameUF,nameLib);
		return FALSE;
	}
	if (A_Ok!=shader->Touch())
		return FALSE;
	BOOL bRet=_PostProcess(shader,(PostProcessArg &)arg);
	SAFE_RELEASE(shader);
	return bRet;
}

BOOL CRenderPort::PostProcess(const char *nameLib,FeatureCode &fc,const PostProcessArg &arg)
{
	CShader *shader=(CShader*)_pRS->GetShaderLibMgr()->ObtainShader(nameLib,fc);
	if (!shader)
		return FALSE;
	if (A_Ok!=shader->Touch())
		return FALSE;
	BOOL bRet=_PostProcess(shader,(PostProcessArg &)arg);
	SAFE_RELEASE(shader);
	return bRet;
}


BOOL CRenderPort::PostProcess(IShader *shader,const PostProcessArg &arg)	
{
	if (A_Ok!=((CShader*)shader)->Touch())
		return FALSE;
	return _PostProcess((CShader*)shader,(PostProcessArg &)arg);	
}


BOOL CRenderPort::_Lines2D(IVertexBuffer *vb)
{
	CShader *shader=(CShader *)_draw2d;
	if (!shader)
		return FALSE;

	if (!_PrepareDraw())
		return FALSE;

	VBBindArg arg;
	arg.SetDPT((DWORD)D3DPT_LINELIST);
	shader->BindVB(vb,NULL,&arg);

	i_math::matrix44f proj2D;
	_camOrtho->GetProj(proj2D);
	shader->SetEP(EP_proj2D,proj2D);

	shader->SetEP(EP_diffusemap,_blank,0);

	ShaderState state;
	state.modeBlend=Blend_AlphaBlend;
	state.modeDepth=Depth_Disable;
	shader->SetState(state);

	return shader->DoShade();
}


BOOL CRenderPort::Line(int x1,int y1,int x2,int y2,DWORD col)
{

	if (TRUE)
	{
		VtxPosColor *p=(VtxPosColor *)_line->Lock(TRUE);
		if (!p)
			return FALSE;
		p[0].pos.set(x1,y1,0.0f);
		p[0].color=col;
		p[1].pos.set(x2,y2,0.0f);
		p[1].color=col;
		_line->Unlock();
	}
	return _Lines2D(_line);
}

BOOL CRenderPort::LinesStrip(i_math::vector3df *lines,DWORD count,DWORD col,ShaderState *state/*=NULL*/)
{
	std::vector<i_math::vector3df> temp(2*count-2);
	int k = 0;
	for(int i = 1;i<count;++i){
		temp[k++] = lines[i-1];
		temp[k++] = lines[i-0];
	}
	return Lines(&temp[0],count - 1,col,state);
}

//count为线段的个数
BOOL CRenderPort::Lines(i_math::pos2di *lines,DWORD count,DWORD col)
{
	IVertexBuffer *vb=_pvm->CreateVB(count*2,FVFEx_PosColor,1,VBFlag_Dynamic);
	if (!vb)
		return FALSE;

	BOOL bRet=FALSE;

	if (TRUE)
	{
		VtxPosColor *p=(VtxPosColor*)vb->Lock(TRUE);
		if (p)
		{
			for (int i=0;i<count*2;i++)
			{
				p[i].pos.set((float)lines[i].x,(float)lines[i].y,0.0f);
				p[i].color=col;
			}
			vb->Unlock();

			bRet=_Lines2D(vb);
		}
	}

	SAFE_RELEASE(vb);
	return bRet;
}


BOOL CRenderPort::FrameRect(i_math::recti &rc0,DWORD col)
{
	CShader *shader=(CShader*)_draw2d;
	if (!shader)
		return FALSE;

	if (!_PrepareDraw())
		return FALSE;
	recti rcView;
	GetRect(rcView);
	recti rc;
	rc=rc0;
	rc+=rcView.UpperLeftCorner;

	if (!rcView.isRectCollided(rc))
		return TRUE;

	if(TRUE)
	{
		VtxPosColor *p=(VtxPosColor *)_border->Lock(TRUE);
		if (!p)
			return FALSE;
		p[0].pos.set(rc0.Left(),rc0.Top(),0.0f);
		p[1].pos.set(rc0.Right(),rc0.Top(),0.0f);
		p[2].pos.set(rc0.Right(),rc0.Bottom(),0.0f);
		p[3].pos.set(rc0.Left(),rc0.Bottom(),0.0f);
		p[4].pos.set(rc0.Left(),rc0.Top(),0.0f);
		for (int i=0;i<5;i++)
			p[i].color=col;

		_border->Unlock();
	}


	VBBindArg arg;
	arg.SetDPT((DWORD)D3DPT_LINESTRIP);
	shader->BindVB(_border,NULL,&arg);

	i_math::matrix44f proj2D;
	_camOrtho->GetProj(proj2D);
	shader->SetEP(EP_proj2D,proj2D);

	shader->SetEP(EP_diffusemap,_blank,0);

	ShaderState state;
	state.modeBlend=Blend_AlphaBlend;
	state.modeDepth=Depth_Disable;
	shader->SetState(state);

	return shader->DoShade();
}

BOOL CRenderPort::FillRect(i_math::recti &rc0,DWORD col)
{
	CShader *shader=(CShader*)_draw2d;
	if (!shader)
		return FALSE;

	if (!_PrepareDraw())
		return FALSE;
	recti rcView;
	GetRect(rcView);

	i_math::recti rc;
	rc=rc0;
	rc+=rcView.UpperLeftCorner;
	rc.clipAgainst(rcView);
	if (!rc.isValid())
		return TRUE;

	if (TRUE)
	{
		VtxQuad *p=(VtxQuad *)_quad->Lock(TRUE);
		if (!p)
			return FALSE;
		MakeQuad<VtxQuad>(p,NULL,0,rc0,1.0f,i_math::recti(0,0,0,0),i_math::size2di(1,1),col);
		_quad->Unlock();
	}

	VBBindArg arg;
	arg.SetDPT(5);//D3DPT_TRIANGLESTRIP
	shader->BindVB(_quad,NULL,&arg);

	i_math::matrix44f proj2D;
	_camOrtho->GetProj(proj2D);
	shader->SetEP(EP_proj2D,proj2D);

	shader->SetEP(EP_diffusemap,_blank,0);

	ShaderState state;
	state.modeBlend=Blend_AlphaBlend;
	state.modeDepth=Depth_Disable;
	shader->SetState(state);

	return shader->DoShade();
}


BOOL CRenderPort::_DrawText0(CRichWordPiece* pRwp,i_math::size2di &szText,BOOL bCalc)
{
	CShader *shader=(CShader*)_draw2d;
	if (!shader)
		return FALSE;

	i_math::matrix44f proj2D;
	_camOrtho->GetProj(proj2D);
	shader->SetEP(EP_proj2D,proj2D);

	ShaderState state;
	state.modeBlend=Blend_AlphaBlend;
	state.modeDepth=Depth_Disable;
	shader->SetState(state);

	DWORD cPatches;
	TxtPatch *patches=pRwp->ObtainPatches(cPatches);

	if (!patches)
		return FALSE;

	shader->BeginRaw();

	VBBindArg arg2;
	for (int i=0;i<cPatches;i++)
	{
		TxtPatch *pa=&patches[i];

		shader->SetEP(EP_diffusemap,pa->tex,0);

		_geo->AddPatch(_surfTxt,pa->vtx,pa->nVtx,pa->idx,pa->nIdx,0);
		DWORD nVB=_geo->GetVBCount(_surfTxt);
		for (int j=0;j<nVB;j++)
		{
			VBHandles vbh=_geo->GetVB(_surfTxt,j,(DWORD&)arg2.primstart,(DWORD&)arg2.primcount,0);

			shader->BindVB(vbh.vb,vbh.ib,&arg2);
			shader->DoShadeRaw();
		}

		_geo->RemoveAllPatches(_surfTxt);
	}

	shader->EndRaw();

	return TRUE;
}


BOOL CRenderPort::_DrawText(const char *str,const DrawFontArg &arg,i_math::size2di &szText,BOOL bCalc)
{
	CRichWordPiece Rwp;
	Rwp.SetMgr((CFontMgr*)_pRS->GetFontMgr());
	
	Rwp.SetFormatText(str);
	Rwp.ApplyArg(arg);
	if (bCalc)
	{
		i_math::size2d_sh sz=Rwp.GetActualSize();
		szText.set(sz.w,sz.h);
		return TRUE;
	}

	return _DrawText0(&Rwp,szText,bCalc);
}

BOOL CRenderPort::_DrawText(ITextPiece *piece,const DrawFontArg &arg,i_math::size2di &szText,BOOL bCalc)
{
	CRichWordPiece *pRwp;
	pRwp=(CRichWordPiece *)piece;
	pRwp->ApplyArg(arg);
	if (bCalc)
	{
		i_math::size2d_sh sz=pRwp->GetActualSize();
		szText.set(sz.w,sz.h);
		return TRUE;
	}

	return _DrawText0(pRwp,szText,bCalc);
}



BOOL CRenderPort::DrawText(const char *str,const DrawFontArg &arg)
{
	if (!_PrepareDraw())
		return FALSE;
	i_math::size2di szText;
	return _DrawText(str,arg,szText,FALSE);
}

BOOL CRenderPort::CalcDrawText(const char *str,const DrawFontArg &arg,i_math::size2di &sz)
{
	return _DrawText(str,arg,sz,TRUE);
}

BOOL CRenderPort::DrawText(ITextPiece *piece,const DrawFontArg &arg)
{
	if (!_PrepareDraw())
		return FALSE;
	i_math::size2di szText;
	return _DrawText(piece,arg,szText,FALSE);

}

BOOL CRenderPort::CalcDrawText(ITextPiece *piece,const DrawFontArg &arg,i_math::size2di &sz)
{
	return _DrawText(piece,arg,sz,TRUE);
}

//cInPrim could be 1,2,3,representing point,line,triangle.
BOOL CRenderPort::_Prims(i_math::vector3df *vtxs,DWORD cInPrim,
											DWORD col,DWORD *cols,DWORD count,
											ShaderState *state0)
{
	CShader *shader=(CShader*)_draw3d;
	if (!shader)
		return FALSE;
	matrix43f mat;
	shader->SetEP_World(&mat,1);
	shader->SetEP(EP_diffusemap,(ITexture*)_blank,0);

	ShaderState state;
	state.modeBlend=Blend_AlphaBlend;
	state.modeFacing=Facing_Both;
	if (!state0)
		state0=&state;
	shader->SetState(*state0);

	_TopState()->cam->Bind(shader);

	VBBindArg arg;


	if (!_PrepareDraw())
		return FALSE;

	if (!vtxs)
		return FALSE;

	count*=cInPrim;

	IVertexBuffer *vb=_pvm->CreateVB(count,FVFEx_PosColor,1,VBFlag_Dynamic);
	if (!vb)
		return FALSE;

	BOOL bRet=FALSE;

	if (TRUE)
	{
		VtxPosColor *p=(VtxPosColor*)vb->Lock(TRUE);
		if (!p)
			goto _final;

		if (cols)
		{
			for (int i=0;i<count;i++)
			{
				p[i].pos=vtxs[i];
				p[i].color=cols[i];
			}
		}
		else
		{
			for (int i=0;i<count;i++)
			{
				p[i].pos=vtxs[i];
				p[i].color=col;
			}
		}
		vb->Unlock();
	}



	switch(cInPrim)
	{
		case 1:
			arg.SetDPT((DWORD)D3DPT_POINTLIST);
			break;
		case 2:
			arg.SetDPT((DWORD)D3DPT_LINELIST);
			break;
		case 3:
			arg.SetDPT((DWORD)D3DPT_TRIANGLELIST);
			break;
		default:
			goto _final;
	}

	if (FALSE==shader->BindVB(vb,NULL,&arg))
		goto _final;


	if (FALSE==shader->DoShade())
		goto _final;

	bRet=TRUE;

_final:
	SAFE_RELEASE(vb);
	return bRet;
}


BOOL CRenderPort::Points(i_math::vector3df *pPoints,DWORD count,DWORD col,ShaderState *state)
{
	return _Prims(pPoints,1,col,NULL,count,state);
}

BOOL CRenderPort::Points(i_math::vector3df *pPoints,DWORD count,DWORD *cols,ShaderState *state)
{
	return _Prims(pPoints,1,0,cols,count,state);
}


BOOL CRenderPort::Line(i_math::vector3df &v1,i_math::vector3df &v2,DWORD col,ShaderState *state)
{
	i_math::vector3df lines[2];
	lines[0]=v1;
	lines[1]=v2;

	return Lines(lines,1,col,state);
}


//2*count points stored in lines
BOOL CRenderPort::Lines(i_math::vector3df *lines,DWORD count,DWORD col,ShaderState *state)
{
	return _Prims(lines,2,col,NULL,count,state);
}

//2*count points stored in lines,cols contains 2*count color
BOOL CRenderPort::Lines(i_math::vector3df *lines,DWORD count,DWORD *cols,ShaderState *state)
{
	return _Prims(lines,2,0,cols,count,state);
}

BOOL CRenderPort::Triangles(i_math::vector3df *tris,DWORD count,DWORD col,ShaderState *state)
{
	return _Prims(tris,3,col,NULL,count,state);
}

BOOL CRenderPort::Triangles(i_math::vector3df *tris,DWORD count,DWORD *cols,ShaderState *state)
{
	return _Prims(tris,3,0,cols,count,state);
}



BOOL CRenderPort::SimpleDrawMesh(IMesh *mesh,i_math::matrix43f *mats,DWORD nMats,DWORD col,BOOL bWireframe,IMtrl *mtrl,ILight *lgt,ShaderState *state0)
{
	if (!mesh)
		return FALSE;
	IRenderer *rdr=ObtainRenderer();
	IShader *shader=NULL;

	i_math::vector4df v;
	v.fromDwordColor(col);

	if (mtrl)
	{
		FeatureCode fc;
		fc.Add(FC_col);
		if (nMats>1)
			shader=rdr->BeginRaw(mtrl,0,mesh,lgt,&fc);
		else
			shader=rdr->BeginRaw(mtrl,0,NULL,lgt,&fc);
		if (shader)
		{
			mtrl->BindEP(shader,0);
			mtrl->BindState(shader,0);

			shader->SetEP(EP_colDif,v);
			shader->SetEP(EP_colSpec,v);
			if(lgt)
				lgt->Bind(shader,0);
		}
	}
	else
	{

		ShaderCode sc=_pRS->GetShaderLibMgr()->MakeShaderCode("main",FC_none,"");
		sc.Add(FC_col);
		if (nMats>1)
			sc.Add(mesh->GetFeature());
		if(lgt)
		{
			DWORD nFC;
			FeatureCode *fc=lgt->GetFC(nFC);
			if (fc)
				sc.Add(*fc);
		}

		shader=rdr->BeginRaw(sc);

		if (shader)
		{

			ShaderState state;
			if (state0)
				state=*state0;
// 			if (bWireframe)
// 				state.modeFacing=Facing_Both;
			if (v.a<1.0f)
				state.modeBlend=Blend_AlphaBlend;

			shader->SetState(state);

// 			shader->SetEP(EP_amb,i_math::vector3df(0,0,0));
			shader->SetEP(EP_colDif,v);
			shader->SetEP(EP_colSpec,i_math::vector4df(0,0,0,0));
			if(lgt)
				lgt->Bind(shader,0);
		}
	}

	if (shader)
	{

		DrawMeshArg arg;
		if (bWireframe)
			arg.SetFillMode(2);



		mesh->Draw(shader,mats,nMats,arg);

		rdr->EndRaw(shader);
	}

	return TRUE;
}

BOOL CRenderPort::_DrawFace(i_math::vector3df *vertices,DWORD nVertices,WORD *indices,DWORD nIndices,i_math::matrix43f &mat,DWORD col,BOOL bWireframe)
{
	CShader *shader=(CShader*)_draw3d;
	if (!shader)
		return FALSE;
	shader->SetEP_World(&mat,1);
	shader->SetEP(EP_diffusemap,_blank,0);

	ShaderState state;
	state.modeBlend=Blend_AlphaBlend;
	state.modeFacing=Facing_Both;
	
	shader->SetState(state);

	if (bWireframe)
		shader->SetDepthBias(0,-0.0005f);

	_TopState()->cam->Bind(shader);

	VBBindArg arg;
	if (bWireframe)
		arg.fillmode=2;//wireframe

	if (!_PrepareDraw())
		return FALSE;
	if (!vertices)
		return FALSE;

	IVertexBuffer *vb=_pvm->CreateVB(nVertices,FVFEx_PosColor,1,VBFlag_Dynamic);
	IIndexBuffer *ib=NULL;
	if (indices)
		ib=_pvm->CreateIB(nIndices,VBFlag_Dynamic);

	BOOL bRet=TRUE;
	if (vb)
	{
		VtxPosColor *p=(VtxPosColor*)vb->Lock(TRUE);
		if (p)
		{
			for (int i=0;i<nVertices;i++)
			{
				p[i].pos=vertices[i];
				p[i].color=col;
			}
			vb->Unlock();
		}
		else
			bRet=FALSE;
		if (ib)
		{
			WORD *p2=(WORD *)ib->Lock(TRUE);
			if (p2)
			{
				memcpy(p2,indices,nIndices*sizeof(WORD));
				ib->Unlock();
			}
			else
				bRet=FALSE;
		}
	}

	if (bRet)
	{
		bRet=FALSE;
		if (shader->BindVB(vb,ib,&arg))
		{
			if (shader->DoShade())
				bRet=TRUE;
		}
	}

	if (bWireframe)
		shader->SetDepthBias(0,0);


	SAFE_RELEASE(vb);
	SAFE_RELEASE(ib);

	return bRet;
}


BOOL CRenderPort::DrawFrame(i_math::vector3df *vertices,DWORD nVertices,WORD *indices,DWORD nIndices,i_math::matrix43f &mat,DWORD col)
{
	return _DrawFace(vertices,nVertices,indices,nIndices,mat,col,TRUE);
}

BOOL CRenderPort::DrawFace(i_math::vector3df *vertices,DWORD nVertices,WORD *indices,DWORD nIndices,i_math::matrix43f &mat,DWORD col)
{
	return _DrawFace(vertices,nVertices,indices,nIndices,mat,col,FALSE);
}



BOOL CRenderPort::CalcHitProbe(int x,int y,HitProbe &probe,i_math::f32 length)
{
	recti rc;
	GetRect(rc);
	if (!rc.isValid())
		return FALSE;
	rc.zeroBase();
	ICamera *cam=_TopState()->cam;

	return cam->CalcHitProbe(probe,x,y,rc,length);
}

BOOL CRenderPort::CalcHitVolume(i_math::recti &rc,i_math::volumeCvxf &vol,i_math::f32 length)
{
	HitProbe LT,RT,LB,RB;
	if (rc.getArea()<=0)
		return FALSE;
	if (!CalcHitProbe(rc.Left(),rc.Top(),LT,100.0f))
		return FALSE;
	if (!CalcHitProbe(rc.Left(),rc.Bottom(),LB,100.0f))
		return FALSE;
	if (!CalcHitProbe(rc.Right(),rc.Top(),RT,100.0f))
		return FALSE;
	if (!CalcHitProbe(rc.Right(),rc.Bottom(),RB,100.0f))
		return FALSE;

	ICamera *cam=_TopState()->cam;
	i_math::vector3df vEye;
	if (FALSE==cam->GetEyePos(vEye))
		return FALSE;

	if (cam->IsOrtho())
		return FALSE;

	vol.clear();
	i_math::plane3df pl;
	pl.setPlane(vEye,LB.end,LT.end);
	vol.addPlane(pl);
	pl.setPlane(vEye,LT.end,RT.end);
	vol.addPlane(pl);
	pl.setPlane(vEye,RT.end,RB.end);
	vol.addPlane(pl);
	pl.setPlane(vEye,RB.end,LB.end);
	vol.addPlane(pl);

	return TRUE;
}


BOOL CRenderPort::TransAabb(i_math::aabbox3df&aabb,i_math::recti &rc)
{
	i_math::vector3df corners[8];
	aabb.getCorners(corners);

	i_math::plane3df plNear;//构建近截面的平面
	if (TRUE)
	{
		ICamera *cam=_TopState()->cam;
		i_math::vector3df pos;
		i_math::vector3df dir;
		cam->GetEyePos(pos);
		cam->GetEyeDir(dir);
		float n,f;
		cam->GetNearFar(n,f);
		pos+=dir*n;
		plNear.setPlane(pos,dir);
	}

	rc.set(0,0,0,0);
	int x,y;
	BOOL bSomeBehind=FALSE;
	for (int i=0;i<8;i++)
	{
		if (plNear.classifyPointRelation(corners[i])==ISREL3D_BACK)
		{
			bSomeBehind=TRUE;
			continue;//这个corner在近截面背面,不用考虑
		}
		TransPos(corners[i],x,y);
		rc.merge(x,y);
	}

	//如果有任何一个点在近截面背后,我们要检测所有的棱与近截面的交点
	if (bSomeBehind)
	{
		i_math::line3df edges[12];
		aabb.getEdges(edges);
		i_math::vector3df v;
		for (int i=0;i<ARRAY_SIZE(edges);i++)
		{
			if (plNear.getIntersectionWithLimitedLine(edges[i].start,edges[i].end,v))
			{
				TransPos(v,x,y);
				rc.merge(x,y);
			}
		}
	}

	return TRUE;
}


BOOL CRenderPort::TransSphere(i_math::vector3df &center,float radius,i_math::recti &rc)
{
	return FALSE;
}
