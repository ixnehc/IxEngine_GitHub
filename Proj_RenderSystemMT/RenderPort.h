#pragma once

#include "RenderSystem/IRenderPort.h"
#include "RenderSystem/IPatchTools.h"


#include "base.h"


#include "class/class.h"

class CDeviceObject;
class CShaderMgr;
class CRenderSystem;
class CRenderer;

class CRichWordPiece;

//////////////////////////////////////////////////////////////////////////
//User-defined Clip Plane
#define MAX_CLIP_PLANE 6
struct ClipPlanes
{
	ClipPlanes()
	{
		Zero();
	}
	void Zero()
	{
		bEquation=FALSE;
		nPlanes=0;
	}
	void ToEquation()
	{
		if (!bEquation)
		{
			for (int i=0;i<nPlanes;i++)
				planes[i].getEquation(equation[i]);
			bEquation=TRUE;
		}
	}
	BOOL bEquation;//whether equation is valid 
	i_math::plane3df planes[MAX_CLIP_PLANE];
	i_math::vector4df equation[MAX_CLIP_PLANE];
	DWORD nPlanes;
};


class CShader;
class CRenderPort:public IRenderPort
{
public:
	DEFINE_CLASS(CRenderPort)
	CRenderPort();
	~CRenderPort();
	BOOL Init(IRenderSystem *pRS);
	void UnInit();
	void Zero();

	IMPLEMENT_REFCOUNT_C;

//Interfaces
	virtual BOOL PushState();
	virtual BOOL PopState();
	virtual BOOL SetRenderTarget(SurfHandle *rts,DWORD count=1);
	virtual BOOL SetDSBuffer(SurfHandle &ds);

	virtual void SetRect(int left,int top,int right,int bottom);
	virtual void SetRect(i_math::recti &rc);
	virtual void SetRect_Total();//Set to full size
	virtual void GetRect(i_math::recti &rc);

	virtual ICamera* QueryCamera();//will NOT add ref count internally
	virtual ICamera* GetCamera();//will NOT add ref count internally
	virtual BOOL SetCamera(ICamera *cam);//will add ref count internally
	virtual ICamera* GetOrthoCamera();//get the 2D drawing camera,will NOT add ref count internally
	virtual BOOL AdjustCameraRatio(ICamera *cam);//调整camera的aspect ratio以适应这个render port


	//indicate whether automatically adjust the camera's aspect ratio 
	//based on the viewport size
	virtual void SetAutoRatio(BOOL bAutoRatio);


	virtual IRenderSystem *GetRS()	{		return _pRS;	}

	virtual IRenderer *ObtainRenderer();

	//Operations
	virtual BOOL FillColor(DWORD col);
	virtual BOOL FillColor(i_math::vector4df &col);
	virtual BOOL FillColor(i_math::recti &rc,i_math::vector4df &col);
	virtual BOOL ClearBuffer(ClearBufferFlag flag,DWORD col=0,float z=1,DWORD s=0);

	virtual BOOL Line(int x1,int y1,int x2,int y2,DWORD col);
	virtual BOOL Lines(i_math::pos2di *lines,DWORD count,DWORD col);//count为线段的个数
	virtual BOOL FrameRect(i_math::recti &rc,DWORD col);
	virtual BOOL FillRect(i_math::recti &rc,DWORD col);
	virtual BOOL DrawTexture(ITexture *pTex,const DrawTextureArg &arg);
	virtual BOOL DrawText(const char *str,const DrawFontArg &arg);
	virtual BOOL CalcDrawText(const char *str,const DrawFontArg &arg,i_math::size2di &sz);
	virtual BOOL DrawText(ITextPiece *piece,const DrawFontArg &arg);
	virtual BOOL CalcDrawText(ITextPiece *piece,const DrawFontArg &arg,i_math::size2di &sz);

	virtual BOOL PostProcess(const char *nameLib,const char *nameUF,const PostProcessArg &arg);
	virtual BOOL PostProcess(const char *nameLib,FeatureCode &fc,const PostProcessArg &arg);
	virtual BOOL PostProcess(IShader *shader,const PostProcessArg &arg);

	virtual BOOL Points(i_math::vector3df *points,DWORD count,DWORD col,ShaderState *state=NULL);
	virtual BOOL Points(i_math::vector3df *points,DWORD count,DWORD *cols,ShaderState *state=NULL);
	virtual BOOL Line(i_math::vector3df &v1,i_math::vector3df &v2,DWORD col,ShaderState *state=NULL);
	virtual BOOL Lines(i_math::vector3df *lines,DWORD count,DWORD col,ShaderState *state=NULL);//count is in primitive,2*count points stored in lines
	virtual BOOL LinesStrip(i_math::vector3df *lines,DWORD count,DWORD col,ShaderState *state=NULL);//count: vtx count 
	virtual BOOL Lines(i_math::vector3df *lines,DWORD count,DWORD *cols,ShaderState *state=NULL);//count is in primitive,2*count points stored in lines,cols contains 2*count color
	virtual BOOL Triangles(i_math::vector3df *tris,DWORD count,DWORD col,ShaderState *state=NULL);//count is in primitive,3*count points stored in tris
	virtual BOOL Triangles(i_math::vector3df *tris,DWORD count,DWORD *cols,ShaderState *state=NULL);//count is in primitive,3*count points stored in tris,cols contains 3*count color
	virtual BOOL DrawFrame(i_math::vector3df *vertices,DWORD nVertices,WORD *indices,DWORD nIndices,i_math::matrix43f &mat,DWORD col=0xffffffff);//如果indices为NULL,不使用index
	virtual BOOL DrawFace(i_math::vector3df *vertices,DWORD nVertices,WORD *indices,DWORD nIndices,i_math::matrix43f &mat,DWORD col=0xffffffff);//如果indices为NULL,不使用index


	virtual BOOL TransPos(i_math::vector3df &v,int &x,int &y);//convert a 3D pos to 2D coordinate in this port
	virtual BOOL CalcHitProbe(int x1,int y1,HitProbe &probe,i_math::f32 length=HITPROBE_DefaultLength);
	virtual BOOL CalcHitVolume(i_math::recti &rc,i_math::volumeCvxf &vol,i_math::f32 length=HITPROBE_DefaultLength);
	virtual BOOL TransAabb(i_math::aabbox3df&aabb,i_math::recti &rc);
	virtual BOOL TransSphere(i_math::vector3df &center,float radius,i_math::recti &rc);

	virtual BOOL SimpleDrawMesh(IMesh *mesh,i_math::matrix43f &mat,DWORD col=0xffffffff,BOOL bWireframe=FALSE,IMtrl *mtrl=NULL,ILight *lgt=NULL,ShaderState *state=NULL)
	{
		return SimpleDrawMesh(mesh,&mat,1,col,bWireframe,mtrl,lgt,state);
	}
	virtual BOOL SimpleDrawMesh(IMesh *mesh,i_math::matrix43f *mats,DWORD nMats,DWORD col=0xffffffff,BOOL bWireframe=FALSE,IMtrl *mtrl=NULL,ILight *lgt=NULL,ShaderState *state=NULL);



private:

	struct _RectInfo
	{
		_RectInfo()
		{
			bViewRect=FALSE;
		}
		recti rcView;
		BOOL bViewRect;//if _bViewRect is FALSE,rcView is invalid,and the whole render target will be taken as the viewport
	};


	struct _State
	{
		_State()
		{
			cam=NULL;
			bAutoRatio=TRUE;
			bRT=FALSE;
			bDS=FALSE;
		}
		_RectInfo rc;
		ICamera *cam;

		BOOL bRT;//an override RT is set
		BOOL bDS;//an override DS is set
		BOOL bAutoRatio;//whether automatically adjust the camera's aspect ratio 
										//based on the viewport size
		ClipPlanes planes;
	};


	__inline _State *_TopState();

	BOOL _PrepareDraw();

	BOOL _DrawText(const char *str,const DrawFontArg &arg,i_math::size2di &szText,BOOL bCalc);
	BOOL _DrawText(ITextPiece *piece,const DrawFontArg &arg,i_math::size2di &szText,BOOL bCalc);
	BOOL _DrawText0(CRichWordPiece* pRwp,i_math::size2di &szText,BOOL bCalc);

	BOOL _Prims(i_math::vector3df *vtxs,DWORD cInPrim,DWORD col,DWORD *cols,DWORD count,ShaderState *state);
	BOOL _Lines2D(IVertexBuffer *vb);


	BOOL _DrawFace(i_math::vector3df *vertices,DWORD nVertices,WORD *indices,DWORD nIndices,i_math::matrix43f &mat,DWORD col,BOOL bWireframe);


	BOOL _PostProcess(CShader *shader,PostProcessArg &arg);


	IRenderSystem *_pRS;
	CDeviceObject *_pdevobj;
	CRenderer *_renderer;
	IVertexMgr *_pvm;

	std::vector<_State> _states;

	ICamera *_camOrtho;//camera used for 2D drawing

	IVertexBuffer *_line;//of D3DPT_LINELIST
	IVertexBuffer *_quad;//of D3DPT_TRIANGLESTRIP
	IVertexBuffer *_border;//of D3DPT_LINESTRIP

	IPatchGeom *_geo;//for drawing text
	PGSurfHandle _surfTxt;

	IShader *_draw2d;
	IShader *_draw3d;
	IShader *_fill;

	ITexture *_blank;


};



