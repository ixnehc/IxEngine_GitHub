#pragma once

#include "RenderSystem/IRenderSystem.h"
#include "RenderSystem/IVertexBuffer.h"

#include "ResData/ResDataDefines.h"

#include "GLbase.h"

#include "fvfex/fvfex_type.h"

#include <deque>
#include <map>


//--------------------------------------------------------------------------------------
// Structs
//--------------------------------------------------------------------------------------
class CRenderSystemGL;

#define MAX_RT_COUNT 1
#define MAX_BIND_VB 4

class VBInfoGL;
class IBInfoGL;
struct SurfHandle;

struct FVFExKey
{
	FVFEx fvfs[MAX_BIND_VB],fvfDraw;
	void Zero()		{			memset(this,0,sizeof(*this));		}
};


struct VertexDeclarationGL
{
	struct AttribInfo
	{
		BYTE id;//attrib id
		BYTE iBuf;//这个attrib在哪个buffer里
		BYTE count;//有几个元素
		bool bNormalized;
		DWORD fmt;//每个元素是什么类型
			//				GL_BYTE
			// 			GL_UNSIGNED_BYTE
			// 			GL_SHORT
			// 			GL_UNSIGNED_SHORT
			// 			GL_FLOAT
			// 			GL_FIXED
			// 			GL_HALF_FLOAT_OES*

		BYTE stride;
		BYTE off;
	};

	FVFExKey key;
	std::vector<AttribInfo> attribs;
	
};

class CDeviceObjectGL
{
	DeviceConfig _config;

	BOOL _bLost;

	IRenderSystem *_pRS;

	//RT/DS buffer stack
	struct _RT
	{
		_RT()
		{
			memset(this,0,sizeof(*this));
		}
		GLTexHandle rts[MAX_RT_COUNT];
	};
	struct _DS
	{
		_DS()
		{
			depth=GLHandle_Null;
			stencil=GLHandle_Null;
		}
		GLRenderBufHandle depth;
		GLRenderBufHandle stencil;
	};

	//RT/DS堆栈
	std::deque<_RT>_stackRT;
	std::deque<_DS>_stackDS;

	//当前使用的fbo
	BOOL _bFboDirty;
	GLFrameBufHandle _fboCur;

	DWORD _w,_h;

	//VB/IB bind/draw
	//For VB binding/drawing
	struct _DrawPrimArg
	{
		_DrawPrimArg()
		{
			_bValid=FALSE;
		}
		BOOL _bValid;
		BOOL _bIndexed;
		DWORD _nIndice;
		DWORD _nVertice;
		DWORD _iStreamOff;//In vertex
		int _primstart;
		int _primcount;
		int _ds;//a D3DFILLMODE value
		DWORD _dpt;//a D3DPRIMITIVETYPE value
	};

	VertexDeclarationGL *_declCur;
	std::map<FVFExKey,VertexDeclarationGL>_mapDecl;
	_DrawPrimArg _drawarg;
	VBBindArg _bindarg;


public:
	CDeviceObjectGL();
	~CDeviceObjectGL();
	void Zero();

	BOOL Init(CRenderSystemGL *pRS);
	BOOL UnInit();//if and leak found,return FALSE,otherwise return TRUE

	BOOL ResetConfig(DeviceConfig &config);
	BOOL GetConfig(DeviceConfig &config);
	void Clean();
	BOOL Present();
	BOOL IsLost();

	//State management
	BOOL SetViewport(ViewportInfo &v);

	BOOL PushRenderTarget(SurfHandle *rts,DWORD count=1);
	BOOL PushDSBuffer(SurfHandle &ds);
	void PopRenderTarget();
	void PopDSBuffer();

	BOOL BindVB(VBInfoGL** vbs,DWORD nVB,IBInfoGL*ib,VBBindArg *arg);//bind the vertex buffer to the device
	void ClearVBBind();
	BOOL SetVBInstanceCount(DWORD count);
	BOOL DrawPrim();


	void SetAlphaTest(AlphaTestMode mode,DWORD ref);
	void SetBlend(ShaderBlendMode mode);
	void SetStencilOp(StencilMode mode,WORD ref,WORD mask);
	void SetDepthMethod(DepthMode mode);
	void SetFacing(FacingMode mode);
	ShaderState &GetState()	{		return _state;	}

	//helpful functions
	DWORD Width();
	DWORD Height();
	BOOL ColorFill(DWORD color,DWORD iBackBuffer=0,RECT *pRect=NULL);
	BOOL ClearZBuffer(RECT *pRect=NULL);
	BOOL ClearBuffer(ClearBufferFlag flag,RECT *pRect=NULL,D3DCOLOR col=0,float z=1.0f,DWORD s=0);
	BOOL FlushCommand();


	//operations
	IRenderSystem *GetRS()const{return _pRS;}

protected:
	void _Error( HRESULT hr );
	virtual HRESULT _OnCreated();
	virtual HRESULT _OnReset();
	void _OnLost();
	virtual void _OnDestroyed();

public:

protected:

	void _BindVB(VBInfoGL** vbs,VertexDeclarationGL *decl,int vbase);
	void _ClearBindVB(VertexDeclarationGL *decl);
	void _BindIB(IBInfoGL*ib);


	//
	void _CleanStreamSource(DWORD iStart)
	{
// 		while(_vbCur[iStart])
// 		{
// 			_vbCur[iStart]=NULL;
// 			_device->SetStreamSource(iStart,NULL,0,0);
// 			iStart++;
// 		}
	}

	ShaderState _state;

	friend bool operator<(const FVFExKey&v1, const FVFExKey&v2);
	friend bool operator==(const FVFExKey&v1, const FVFExKey&v2);


};




