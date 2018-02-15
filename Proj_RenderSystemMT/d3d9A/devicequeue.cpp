/********************************************************************
	created:	2008/08/08   16:57
	filename: 	d:\IxEngine\Proj_RenderSystemMT\d3d9A\devicequeue.cpp
	author:		cxi
	
	purpose:	device queue
*********************************************************************/


#include "stdh.h"

#include "devicequeue.h"

#include "Direct3DDeviceA.h"

#include "D3DXEffectA.h"
#include "D3DXBufferA.h"
#include "D3DXCompilerA.h"
#include "D3DXMeshA.h"

#include "timer/profiler.h"
#include "timer/timer.h"

#include "Log/LogFile.h"

CDeviceQueue::CDeviceQueue()
{
	_pIn0=NULL;
	_capacity=0;
	_sz=0;

	_bProcessing=FALSE;
	_qs=Running;

	_out.resize(2000000);
	_in.resize(2000000);

	_finish=0;

	Start();
	Sleep(50);

	_ttt=0;
}

CDeviceQueue::~CDeviceQueue()
{
	Finish();
}




extern LogFile g_logRS;
int CDeviceQueue::OnDo()
{
	_smphIn.Post();
	while(1)
	{
		_smphOut.Wait();

		if (_finish==1)
			break;

//		ProfilerStart(D3DProcess);
		_DoProcess();
//		ProfilerEnd();

		_smphIn.Post();
	}
	_finish=2;
	return TRUE;
}

void CDeviceQueue::Flip()
{
	DQ_PushFuncType(FuncType_Null);

	_smphIn.Wait();

	_out.swap(_in);
	if (_bLostIn)
		_bLostOut=TRUE;
	_bLostIn=FALSE;

	_in.resize(_capacity);
	_sz=0;
	_pIn0=&_in[0];

	_smphOut.Post();
}

void CDeviceQueue::Flush()
{
	Flip();
	Flip();
}

#define ParamDefine(type,var) type &var=*(type*)p;p+=sizeof(type);
#define ParamPtrDefine(type,var)																\
				type *var=NULL;																				\
				if (*(DWORD*)p==0)																		\
					p+=sizeof(DWORD);																	\
				else																									\
				{																										\
					var=(type *)(p+sizeof(DWORD));												\
					p+=sizeof(DWORD)+sizeof(type);											\
				}

#define ParamRelease(type)																			\
{																														\
	ParamDefine(type*,ptr);																				\
	SAFE_RELEASE(ptr->_core);																			\
	D3D9A_Delete(ptr);																						\
	break;																											\
}
	
int g_buf[100000];

BOOL CDeviceQueue::_DoProcess()
{
	if (TRUE)
	{
		tbb::spin_mutex::scoped_lock lock(_mutex);

		_bProcessing=TRUE;
	}

	if (FALSE)
	{
		int sum=0;
		for (int i=0;i<100000;i++)
			sum+=g_buf[i];
	}

	BYTE *p=&_out[0];

	HRESULT hr;
	while(1)
	{
		if (_qs==Questing)
			_qs=Freezing;
// 		_qs.compare_and_swap(Freezing,Questing);
		while(_qs==Freezing)
			Sleep(0);

		if(TRUE)
		{
			FuncType functype;
	//		FuncType functypeLast;
			functype=*(FuncType*)p;
	//		assert( (((DWORD)functype)&0xffff0000)==0xff7e0000);
	//		functype=(FuncType)(((DWORD)functype)&0xffff);
	//		functypeLast=functype;
			p+=sizeof(FuncType);
			switch(functype)
			{
				case FuncType_Null:
					goto _out;
				case Test_0:
				{
					ParamDefine(DWORD,t0);
					ParamDefine(DWORD,t1);
					for (int i=0;i<100000;i++);

					break;
				}
				case Test_1:
				{
					ParamDefine(DWORD,t0);
					ParamDefine(DWORD,t1);

					for (int i=0;i<1000;i++);
					break;
				}

				//----------------------------------------------------------------------------------------------------------
				//Device
				case Device_CreateVertexBuffer:
				{
					ParamDefine(IDirect3DDevice9 *,dev);
					ParamDefine(UINT,Length);
					ParamDefine(DWORD,Usage);
					ParamDefine(DWORD,FVF);
					ParamDefine(D3DPOOL,Pool);
					ParamDefine(CDirect3DVertexBuffer*,vb);

					dev->CreateVertexBuffer(Length,Usage,FVF,Pool,&vb->_core,NULL);
					break;
				}
				case Device_CreateIndexBuffer:
				{
					ParamDefine(IDirect3DDevice9 *,dev);
					ParamDefine(UINT,Length);
					ParamDefine(DWORD,Usage);
					ParamDefine(D3DFORMAT,Format);
					ParamDefine(D3DPOOL,Pool);
					ParamDefine(CDirect3DIndexBuffer*,ib);
					dev->CreateIndexBuffer(Length,Usage,Format,Pool,&ib->_core,NULL);
					break;
				}
				case Device_CreateRenderTarget:
				{
					ParamDefine(IDirect3DDevice9 *,dev);
					ParamDefine(UINT,Width);
					ParamDefine(UINT,Height);
					ParamDefine(D3DFORMAT,Format);
					ParamDefine(CDirect3DSurface*,surf);

					dev->CreateRenderTarget(Width,Height,Format,D3DMULTISAMPLE_NONE,0,FALSE,&surf->_core,NULL);

					break;
				}
				case Device_CreateDepthStencilSurface:
				{
					ParamDefine(IDirect3DDevice9 *,dev);
					ParamDefine(UINT,Width);
					ParamDefine(UINT,Height);
					ParamDefine(D3DFORMAT,Format);
					ParamDefine(CDirect3DSurface*,surf);

					dev->CreateDepthStencilSurface(Width,Height,Format,D3DMULTISAMPLE_NONE,0,TRUE,&surf->_core,NULL);

					break;
				}
				case Device_CreateOffscreenPlainSurface:
				{
					ParamDefine(IDirect3DDevice9 *,dev);
					ParamDefine(UINT,Width);
					ParamDefine(UINT,Height);
					ParamDefine(D3DFORMAT,Format);
					ParamDefine(D3DPOOL,pool);
					ParamDefine(CDirect3DSurface*,surf);

					dev->CreateOffscreenPlainSurface(Width,Height,Format,pool,&surf->_core,NULL);
					break;
				}
				case Device_BeginScene:
				{
					ParamDefine(IDirect3DDevice9 *,dev);
					dev->BeginScene();
					break;
				}
				case Device_EndScene:
				{
					ParamDefine(IDirect3DDevice9 *,dev);
					dev->EndScene();
					break;
				}
				case Device_SetTransform:
				{
					ParamDefine(IDirect3DDevice9 *,dev);
					ParamDefine(D3DTRANSFORMSTATETYPE,State);
					ParamDefine(D3DMATRIX,mat);
					dev->SetTransform(State,&mat);
					break;
				}
				case Device_Clear:
				{
					ParamDefine(IDirect3DDevice9 *,dev);
					ParamDefine(DWORD,count);
					D3DRECT *rects=NULL;
					if (count>0)
					{
						rects=(D3DRECT *)p;
						p+=sizeof(D3DRECT)*count;
					}
					ParamDefine(DWORD,flags);
					ParamDefine(D3DCOLOR,color);
					ParamDefine(float,Z);
					ParamDefine(DWORD,Stencil);

					hr=dev->Clear(count,rects,flags,color,Z,Stencil);
					break;
				}
				case Device_EvictManagedResources:
				{
					ParamDefine(IDirect3DDevice9 *,dev);
					dev->EvictManagedResources();
					break;
				}
				case Device_Present:
				{
					ParamDefine(IDirect3DDevice9 *,dev);

					ParamPtrDefine(RECT,pSourceRect);
					ParamPtrDefine(RECT,pDestRect);
					ParamDefine(HWND,hDestWindowOverride);

					if (D3DERR_DEVICELOST==dev->Present(pSourceRect,pDestRect,hDestWindowOverride,NULL))
						_bLostIn=TRUE;
					break;
				}
				case Device_CreateTexture:
				{
					ParamDefine(IDirect3DDevice9 *,dev);

					ParamDefine(UINT,Width);
					ParamDefine(UINT,Height);
					ParamDefine(UINT,Levels);
					ParamDefine(DWORD,Usage);
					ParamDefine(D3DFORMAT,Format);
					ParamDefine(D3DPOOL,pool);
					ParamDefine(CDirect3DTexture*,tex);

					dev->CreateTexture(Width,Height,Levels,Usage,Format,pool,
										(IDirect3DTexture9 **)&tex->_core,NULL);
					break;
				}
				case Device_ColorFill:
				{
					ParamDefine(IDirect3DDevice9 *,dev);

					ParamDefine(CDirect3DSurface*,surf);
					ParamPtrDefine(RECT,pRect);
					ParamDefine(D3DCOLOR,color);

					if (surf->_core)
						dev->ColorFill(surf->_core,pRect,color);
					break;
				}
				case Device_SetRenderTarget:
				{
					ParamDefine(IDirect3DDevice9 *,dev);

					ParamDefine(DWORD,RenderTargetIndex);
					ParamDefine(CDirect3DSurface*,pRenderTarget);

					if (pRenderTarget)
						hr=dev->SetRenderTarget(RenderTargetIndex,pRenderTarget->_core);
					else
						dev->SetRenderTarget(RenderTargetIndex,NULL);

					break;
				}
				case Device_SetDepthStencilSurface:
				{
					ParamDefine(IDirect3DDevice9 *,dev);

					ParamDefine(CDirect3DSurface*,pNewZStencil);
					if (pNewZStencil)
						dev->SetDepthStencilSurface(pNewZStencil->_core);
					else
						dev->SetDepthStencilSurface(NULL);

					break;
				}
				case Device_SetViewPort:
				{
					ParamDefine(IDirect3DDevice9 *,dev);
					ParamDefine(D3DVIEWPORT9,vp);

					dev->SetViewport(&vp);

					break;
				}
				case Device_SetRenderState:
				{
					ParamDefine(IDirect3DDevice9 *,dev);

					ParamDefine(D3DRENDERSTATETYPE,State);
					ParamDefine(DWORD,Value);

					dev->SetRenderState(State,Value);

					break;
				}

				case Device_DrawPrimitive:
				{
					ParamDefine(IDirect3DDevice9 *,dev);

					ParamDefine(D3DPRIMITIVETYPE,PrimitiveType);
					ParamDefine(UINT,StartVertex);
					ParamDefine(UINT,PrimitiveCount);

					dev->DrawPrimitive(PrimitiveType,StartVertex,PrimitiveCount);

					break;
				}
				case Device_DrawIndexedPrimitive:
				{
					ParamDefine(IDirect3DDevice9 *,dev);

					ParamDefine(D3DPRIMITIVETYPE,PrimitiveType);
					ParamDefine(INT,BaseVertexIndex);
					ParamDefine(UINT,MinVertexIndex);
					ParamDefine(UINT,NumVertices);
					ParamDefine(UINT,startIndex); 
					ParamDefine(UINT,primCount);

					hr=dev->DrawIndexedPrimitive(PrimitiveType,BaseVertexIndex,
											MinVertexIndex,NumVertices,startIndex,primCount);

					break;
				}
				case Device_CreateVertexDeclaration:
				{
					ParamDefine(IDirect3DDevice9 *,dev);

					ParamDefine(DWORD,count);

					D3DVERTEXELEMENT9 *decl=(D3DVERTEXELEMENT9 *)p;
					p+=count*sizeof(D3DVERTEXELEMENT9);

					ParamDefine(CDirect3DVertexDeclaration*,pDecl);

					dev->CreateVertexDeclaration(decl,&pDecl->_core);

					break;
				}
				case Device_SetVertexDeclaration:
				{
					ParamDefine(IDirect3DDevice9 *,dev);

					ParamDefine(CDirect3DVertexDeclaration*,pDecl);

					if (pDecl)
					if (pDecl->_core)
						hr=dev->SetVertexDeclaration(pDecl->_core);
					break;
				}
				case Device_SetFVF:
				{
					ParamDefine(IDirect3DDevice9 *,dev);

					ParamDefine(DWORD,FVF);

					dev->SetFVF(FVF);
					break;
				}
				case Device_SetStreamSource:
				{
					ParamDefine(IDirect3DDevice9 *,dev);

					ParamDefine(UINT,StreamNumber);
					ParamDefine(IDirect3DVertexBuffer9*,vb);
					ParamDefine(UINT,OffsetInBytes);
					ParamDefine(UINT,Stride);

					hr=dev->SetStreamSource(StreamNumber,vb,OffsetInBytes,Stride);


					break;
				}
				case Device_SetIndices:
				{
					ParamDefine(IDirect3DDevice9 *,dev);

					ParamDefine(IDirect3DIndexBuffer9*,ib);

					hr=dev->SetIndices(ib);
					break;
				}
				case Device_CreateQuery:
				{
					ParamDefine(IDirect3DDevice9 *,dev);
					ParamDefine(D3DQUERYTYPE,Type);
					ParamDefine(CDirect3DQuery*,pQuery);

					dev->CreateQuery(Type,&pQuery->_core);

					break;
				}

				//----------------------------------------------------------------------------------------------------------
				//Surface
				case Surface_Release:
				{
					ParamRelease(CDirect3DSurface);
					break;
				}

				//----------------------------------------------------------------------------------------------------------
				//VertexBuffer/IndexBuffer
				case VertexBuffer_Release:
				{
					ParamDefine(CDirect3DVertexBuffer*,ptr);
					if (ptr->_bStatic)
					{
						SAFE_RELEASE(ptr->_core);
					}
					else
						ptr->_dynpool->Free(ptr->_core);
					D3D9A_Delete(ptr);
					break;
				}
				case IndexBuffer_Release:
				{
					ParamDefine(CDirect3DIndexBuffer*,ptr);
					if (ptr->_bStatic)
					{
						SAFE_RELEASE(ptr->_core);
					}
					else
						ptr->_dynpool->Free(ptr->_core);
					D3D9A_Delete(ptr);
					break;
				}
				case VertexBuffer_ReleaseDyn:
				{
					ParamDefine(CDynVBPool*,pool);
					ParamDefine(IDirect3DVertexBuffer9*,ptr);
					pool->Free(ptr);
					break;
				}
				case IndexBuffer_ReleaseDyn:
				{
					ParamDefine(CDynVBPool*,pool);
					ParamDefine(IDirect3DIndexBuffer9*,ptr);
					pool->Free(ptr);
					break;
				}


				//----------------------------------------------------------------------------------------------------------
				//Vertex Declaration
				case VertexDeclaration_Release:
				{
					ParamRelease(CDirect3DVertexDeclaration);
					break;
				}


				//----------------------------------------------------------------------------------------------------------
				//Texture
				case Texture_Release:
				{
					ParamRelease(CDirect3DBaseTexture);
					break;
				}
				case Texture_SetAutoGenFilterType:
				{
					ParamDefine(CDirect3DBaseTexture*,tex);
					ParamDefine(D3DTEXTUREFILTERTYPE,FilterType);

					if (tex->_core)
						tex->_core->SetAutoGenFilterType(FilterType);
					break;
				}

				//----------------------------------------------------------------------------------------------------------
				//Query
				case Query_Release:
				{
					ParamRelease(CDirect3DQuery);
					break;
				}
				case Query_Flush:
				{
					ParamDefine(CDirect3DQuery*,query);
					if(query->_core)
					{
						query->_core->Issue(D3DISSUE_END);
						while(S_FALSE == query->_core->GetData( NULL, 0, D3DGETDATA_FLUSH ))
							Sleep(0);
					}
					break;
				}

				//----------------------------------------------------------------------------------------------------------
				//Effect
				case Effect_Release:
				{
					ParamRelease(CD3DXEffect);
					break;
				}
				case Effect_SetFloatArray:
				{
					ParamDefine(ID3DXEffect *,effect);
					ParamDefine(D3DXHANDLE,hParameter);
					ParamDefine(DWORD,count);
					FLOAT *pf=(FLOAT *)p;
					p+=count*sizeof(FLOAT);

					effect->SetFloatArray(hParameter,pf,count);
					break;
				}
				case Effect_SetTexture:
				{
					ParamDefine(ID3DXEffect *,effect);
					ParamDefine(D3DXHANDLE,hParameter);
					ParamDefine(CDirect3DBaseTexture*,tex);

					if (tex)
						hr=effect->SetTexture(hParameter,tex->_core);
					else
						hr=effect->SetTexture(hParameter,NULL);
					break;
				}
				case Effect_SetFloat:
				{
					ParamDefine(ID3DXEffect *,effect);
					ParamDefine(D3DXHANDLE,hParameter);
					ParamDefine(FLOAT,f);

					effect->SetFloat(hParameter,f);
					break;
				}
				case Effect_SetMatrix:
				{
					ParamDefine(ID3DXEffect *,effect);
					ParamDefine(D3DXHANDLE,hParameter);
					ParamDefine(D3DXMATRIX,mat);

					hr=effect->SetMatrix(hParameter,&mat);
					break;
				}
				case Effect_SetInt:
				{
					ParamDefine(ID3DXEffect *,effect);
					ParamDefine(D3DXHANDLE,hParameter);
					ParamDefine(INT,n);

					effect->SetInt(hParameter,n);
					break;
				}
				case Effect_SetIntArray:
				{
					ParamDefine(ID3DXEffect *,effect);
					ParamDefine(D3DXHANDLE,hParameter);
					ParamDefine(DWORD,count);
					INT *pn=(INT *)p;
					p+=count*sizeof(INT);

					effect->SetIntArray(hParameter,pn,count);
					break;
				}
				case Effect_SetVector:
				{
					ParamDefine(ID3DXEffect *,effect);
					ParamDefine(D3DXHANDLE,hParameter);
					ParamDefine(D3DXVECTOR4,vec);

					effect->SetVector(hParameter,&vec);
					break;
				}
				case Effect_Begin:
				{
					ParamDefine(ID3DXEffect *,effect);
					ParamDefine(DWORD,Flags);

					UINT nPass;
					effect->Begin(&nPass,Flags);

					break;
				}
				case Effect_BeginPass:
				{
					ParamDefine(ID3DXEffect *,effect);
					ParamDefine(UINT,Pass);

					hr=effect->BeginPass(Pass);
					break;
				}
				case Effect_CommitChanges:
				{
					ParamDefine(ID3DXEffect *,effect);
					hr=effect->CommitChanges();
					break;
				}
				case Effect_EndPass:
				{
					ParamDefine(ID3DXEffect *,effect);
					effect->EndPass();
					break;
				}
				case Effect_End:
				{
					ParamDefine(ID3DXEffect *,effect);
					effect->End();
					break;
				}

				//----------------------------------------------------------------------------------------------------------
				//Buffer
				case Buffer_Release:
				{
					ParamRelease(CD3DXBuffer);
					break;
				}

				//----------------------------------------------------------------------------------------------------------
				//Compiler
				case Compiler_Release:
				{
					ParamRelease(CD3DXEffectCompiler);
					break;
				}

				//----------------------------------------------------------------------------------------------------------
				//Mesh
				case Mesh_Release:
				{
					ParamRelease(CD3DXMesh);
					break;
				}


				//----------------------------------------------------------------------------------------------------------
				//Functions
				case Func_D3DXCreateCubeTexture:
				{
					ParamDefine(CDirect3DDevice*,pDevice);
					ParamDefine(UINT,Size);
					ParamDefine(UINT,MipLevels);
					ParamDefine(DWORD,Usage);
					ParamDefine(D3DFORMAT,Format);
					ParamDefine(D3DPOOL,Pool);
					ParamDefine(CDirect3DCubeTexture*,pCubeTexture);

					HRESULT hr=D3DXCreateCubeTexture(pDevice->_core,Size,MipLevels,
											Usage,Format,Pool,(IDirect3DCubeTexture9**)&pCubeTexture->_core);
					if (hr==D3D_OK)
						pCubeTexture->_FillSurfs(1);

					break;
				}
				case Func_D3DXCreateVolumeTexture:
				{
					ParamDefine(CDirect3DDevice*,pDevice);
					ParamDefine(UINT,Width);
					ParamDefine(UINT,Height);
					ParamDefine(UINT,Depth);
					ParamDefine(UINT,MipLevels);
					ParamDefine(DWORD,Usage);
					ParamDefine(D3DFORMAT,Format);
					ParamDefine(D3DPOOL,Pool);
					ParamDefine(CDirect3DVolumeTexture*,pVolumeTexture);

					D3DXCreateVolumeTexture(pDevice->_core,Width,Height,Depth,MipLevels,
											Usage,Format,Pool,(IDirect3DVolumeTexture9**)&pVolumeTexture->_core);

					break;
				}
				case Func_D3DXCreateTexture:
				{
					ParamDefine(CDirect3DDevice*,pDevice);
					ParamDefine(UINT,Width);
					ParamDefine(UINT,Height);
					ParamDefine(UINT,MipLevels);
					ParamDefine(DWORD,Usage);
					ParamDefine(D3DFORMAT,Format);
					ParamDefine(D3DPOOL,Pool);
					ParamDefine(CDirect3DTexture*,pTexture);

					HRESULT hr=D3DXCreateTexture(pDevice->_core,Width,Height,MipLevels,
											Usage,Format,Pool,(IDirect3DTexture9**)&pTexture->_core);

					if (hr==D3D_OK)
						pTexture->_FillSurfs(0);

					break;
				}
				case Func_D3DXLoadSurfaceFromSurface:
				{
					ParamDefine(CDirect3DSurface*,pDestSurface);
					ParamPtrDefine(RECT,pDestRect);
					ParamDefine(CDirect3DSurface*,pSrcSurface);
					ParamPtrDefine(RECT,pSrcRect);
					ParamDefine(DWORD,Filter);
					ParamDefine(D3DCOLOR,ColorKey);

					if  ((pDestSurface->_core)&&(pSrcSurface->_core))
						D3DXLoadSurfaceFromSurface(pDestSurface->_core,NULL,pDestRect,
														pSrcSurface->_core,NULL,pSrcRect,Filter,ColorKey);

					break;
				}
				case Func_D3DXFilterTexture:
				{
					ParamDefine(CDirect3DBaseTexture*,pBaseTexture);
					ParamDefine(UINT,SrcLevel);
					ParamDefine(DWORD,Filter);

					D3DXFilterTexture(pBaseTexture->_core,NULL,SrcLevel,Filter);
					break;
				}

				default:
					assert(FALSE);

			}
		}
	}




_out:

	if(TRUE)
	{
		tbb::spin_mutex::scoped_lock lock(_mutex);

		if (_qs==Questing)
			_qs=Freezing;
// 		_qs.compare_and_swap(Freezing,Questing);

		_bProcessing=FALSE;
	}

	return TRUE;

}
