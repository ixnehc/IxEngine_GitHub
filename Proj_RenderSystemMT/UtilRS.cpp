/********************************************************************
	created:	30:7:2006   10:42
	filename: 	d:\IxEngine\Proj_RenderSystem\UtilRS.cpp
	file path:	d:\IxEngine\Proj_RenderSystem
	file base:	UtilRS
	author:		ixnehc
	
	purpose:	IUtilRS implement,utility functions for RenderSystem
*********************************************************************/
#include "stdh.h"

#include "interface/interface.h"

#include "FileSystem/IFileSystem.h"
#include "RenderSystem/IRenderSystem.h"
#include "RenderSystem/ITexture.h"

#include "UtilRS.h"

#include "assert.h"

#include "Log/LastError.h"
#include "Log/LogFile.h"

#include "stringparser/stringparser.h"

#include <d3d9.h>
#include <d3dx9.h>

#include "ResFile.h"
#include "TexFile.h"

#include "resdata/ResData.h"
#include "resdata/BehaviorGraphData.h"

#include "shaderlib/ShaderLib.h"

#include "datapacket/DataPacket.h"
#include "fvfex/fvfex.h"


#pragma warning(disable:4018)


EXPOSE_SINGLE_INTERFACE(CUtilRS,IUtilRS,"UtilRS01")

#define CHECK_FS \
if (!_pFS)\
{\
	LogFile::Prompt("IUtilRS error: FileSystem not ready!");\
	return FALSE;\
}



extern BOOL CreateRefDevice(HWND hWnd,XDirect3D *&pD3D,XDirect3DDevice *&pDevice);
BOOL CUtilRS::_LockRefDevice()
{
	assert((_d3dRef&&_deviceRef)||((!_d3dRef)&&(!_deviceRef)));

	if (_deviceRef)
	{
		_deviceRef->AddRef();
		return TRUE;
	}

	D3D_FLUSH();

	if (FALSE==CreateRefDevice(GetDesktopWindow(),_d3dRef,_deviceRef))
		return FALSE;
	return TRUE;
}

void CUtilRS::_UnlockRefDevice()
{
	assert((_d3dRef&&_deviceRef)||((!_d3dRef)&&(!_deviceRef)));

	if(!_deviceRef)
		return;
	if (_deviceRef->Release()==0)
	{
		_deviceRef=0;
		D3D_FLUSH();
		SAFE_RELEASE(_d3dRef);
	}
}



BOOL CUtilRS::LoadTexData(const char *path,TexData *data)
{
	CHECK_FS;

	IFile *fl=_pFS->OpenFileAbs(path,FileAccessMode_Read);
	if (!fl)
		return FALSE;

	BOOL bRet=::LoadTexData(fl,data);
	fl->Close();

	return bRet;
}

BOOL CUtilRS::SaveTexData(const char *path,TexData *data)
{
	CHECK_FS;

	IFile *fl=_pFS->OpenFileAbs(path,FileAccessMode_Write);
	if (!fl)
		return FALSE;

	::SaveTexData(fl,data);
	fl->Close();

	return TRUE;
}


TexData *CUtilRS::LoadTexData(const char *path)
{
	TexData *data=(TexData *)ResData_New(Res_Texture);
	if (LoadTexData(path,data))
		return data;
	ResData_Delete((ResData*)data);
	return NULL;
}


BOOL CUtilRS::LoadTexInfo(TexInfo *ti,TexData *data)
{
	return ::LoadTexInfo(*ti,data);
}



BOOL CUtilRS::CullTexData(TexData *dest,TexData *src,i_math::recti &rc)
{
	if(!_LockRefDevice())
		return FALSE;

	BOOL bRet=FALSE;
	XDirect3DTexture* texSrc=NULL;
	XDirect3DTexture* texDest=NULL;
	TexInfo ti;
	if (::LoadTex(_deviceRef,src,ti,(XDirect3DBaseTexture*&)texSrc))
	{
		TexInfo tiDest;

		tiDest=ti;
		tiDest.width=rc.getWidth();
		tiDest.height=rc.getHeight();

		i_math::recti rcDest=rc;
		rcDest.zeroBase();

		if (TRUE)
		{
			//Create the target tex
			if (::CreateTex(_deviceRef,tiDest,(XDirect3DBaseTexture*&)texDest,FALSE))
			{
				XDirect3DSurface *surfSrc=NULL,*surfDest=NULL;
				if (D3D_OK==texSrc->GetSurfaceLevel(0,&surfSrc))
				if (D3D_OK==texDest->GetSurfaceLevel(0,&surfDest))
				if (D3D_OK==XD3DXLoadSurfaceFromSurface(surfDest,NULL,(RECT*)&rcDest,
															surfSrc,NULL,(RECT*)&rc,D3DX_FILTER_POINT,0))
				if (D3D_OK==XD3DXFilterTexture(texDest,NULL,0,D3DX_FILTER_TRIANGLE))
				{
					if (::SaveTex(dest,texDest))
						bRet=TRUE;
				}

				SAFE_RELEASE(surfSrc);
				SAFE_RELEASE(surfDest);
			}

		}
	}
	SAFE_RELEASE(texSrc);
	SAFE_RELEASE(texDest);

	_UnlockRefDevice();

	return bRet;
}




BOOL CUtilRS::RepairResData(ResData *data)
{
	if (!_pRS)
	{
		LogFile::Prompt("IUtilRS error(RepairResData()): RenderSystem not ready!");
		return FALSE;
	}

	extern BOOL RepairResData(ResData *resdata,IRenderSystem *pRS);
	BOOL bRepaired=FALSE;

	ResData *dataOld=data->Clone();

	RepairResData(data,_pRS);

	if (!dataOld->Equal(*data))
		bRepaired=TRUE;

	ResData_Delete(dataOld);

	return bRepaired;
}




void *CUtilRS::CompileEffect(const char *sFX,DWORD lenFX,std::string &error)
{
	error="";

	_LockRefDevice();

	if (!_deviceRef)
	{
		error="Device not ready!";
		return NULL;
	}


	XD3DXEffect *pEffect;
	XD3DXBuffer *pErrBuffer;

	DWORD flag=0;//D3DXSHADER_USE_LEGACY_D3DX9_31_DLL;

	if (D3D_OK!=XD3DXCreateEffect(_deviceRef,sFX,lenFX,NULL,NULL,flag,NULL,&pEffect,&pErrBuffer))
	{
		error=(char *)pErrBuffer->GetBufferPointer();
		pEffect=NULL;
	}

	SAFE_RELEASE(pErrBuffer);

	_UnlockRefDevice();
	return pEffect;
}

//check to see whether sFX could pass the compiler
//return FALSE if fail
BOOL CUtilRS::CheckCompileEffect(const char *sFX,DWORD lenFX,BOOL bAssemblyInfo,std::string &ErrorOrAssem)
{
	BOOL bRet=FALSE;
	ErrorOrAssem="";

	_LockRefDevice();

	if (!_deviceRef)
	{
		ErrorOrAssem="Device not ready!";
		return FALSE;
	}

	XD3DXEffectCompiler *pCompiler=NULL;
	XD3DXBuffer *pErrBuffer=NULL;
	XD3DXBuffer *effectcode=NULL;
 
	DWORD flag=D3DXSHADER_PREFER_FLOW_CONTROL;//D3DXSHADER_USE_LEGACY_D3DX9_31_DLL;
	if (D3D_OK!=XD3DXCreateEffectCompiler(sFX,lenFX,NULL,NULL,flag,&pCompiler,&pErrBuffer))
		goto _final;
	else
	{ 
		SAFE_RELEASE(pErrBuffer);
		if (D3D_OK!=pCompiler->CompileEffect(flag,&effectcode,&pErrBuffer))
			goto _final;

		SAFE_RELEASE(pErrBuffer);

		if (bAssemblyInfo)
		{
			XD3DXEffect *pEffect=NULL;
			XD3DXBuffer *assem=NULL;
			if (D3D_OK==XD3DXCreateEffect(_deviceRef,effectcode->GetBufferPointer(),effectcode->GetBufferSize(),NULL,
				NULL,flag,NULL,&pEffect,NULL))
			{
				if (D3D_OK==XD3DXDisassembleEffect(pEffect,FALSE,&assem))
					ErrorOrAssem=(char *)assem->GetBufferPointer();

				SAFE_RELEASE(pEffect);
			}
			SAFE_RELEASE(assem);
		}

		bRet=TRUE;
	}

_final:
	if(pErrBuffer)
		ErrorOrAssem=(char *)pErrBuffer->GetBufferPointer();

	SAFE_RELEASE(pCompiler);
	SAFE_RELEASE(pErrBuffer);
	SAFE_RELEASE(effectcode);

	_UnlockRefDevice();

	return bRet;

//	BOOL bRet;
//	XD3DXEffect *pEffect=CompileEffect(sFX,lenFX,error);
//	bRet=(pEffect!=NULL);
//	SAFE_RELEASE(pEffect);
//
//	return bRet;
}


extern BOOL SaveShaderLib(IFile *fl,CShaderLib &slib);
extern BOOL LoadShaderLib(IFile *fl,CShaderLib &slib);


BOOL CUtilRS::SaveShaderLib(CShaderLib &slib,const char *path)
{
	if (!_pFS)
		return FALSE;

	_pFS->RemoveFile(path);

	IFile *file;
	file=_pFS->OpenFile(path,FileAccessMode_Write);
	if (!file)
		return FALSE;

	BOOL bRet;
	bRet=::SaveShaderLib(file,slib);
	_pFS->CloseFile(file);

	return bRet;
}

BOOL CUtilRS::LoadShaderLib(CShaderLib &slib,const char *path)
{
	if (!_pFS)
		return FALSE;

	IFile *file;
	file=_pFS->OpenFile(path,FileAccessMode_Read);
	if (!file)
		return FALSE;

	BOOL bRet;
	bRet=::LoadShaderLib(file,slib);
	_pFS->CloseFile(file);

	return bRet;
}




ResData *CUtilRS::LoadRes(const char *path,BOOL bHeader)
{
	ResData *data;
	if (FALSE==LoadResData(_pFS,path,data,bHeader))
		return NULL;
	return data;
}

BOOL CUtilRS::SaveRes(const char *path,ResData *data)
{
	return SaveResData(_pFS,path,data);
}

RecordsData *CUtilRS::LoadRes_Records(const char *path,CClass *clss)
{
	RecordsData *data;
	extern BOOL LoadRecordsData(IFileSystem *pFS,const char *path,RecordsData *&dataRet,CClass *clss);
	if (FALSE==LoadRecordsData(_pFS,path,data,clss))
		return NULL;
	return data;
}

struct ResFileHeaderT
{
	ResType type:8;
	DWORD off:24;//ResData完整数据的偏移(距离文件起始位置)
};

BehaviorGraphData*CUtilRS::LoadRes_BehaviorGraph(const char *path,LinkPadClasses *clsses)
{
	IFile *fl=_pFS->OpenFileAbs(path,FileAccessMode_Read);
	if (!fl)
		return NULL;

	BehaviorGraphData*data=NULL;

	ResFileHeaderT header;
	fl->Read(&header,sizeof(header));

	CDataPacket dp;
	std::vector<BYTE>buf;

	if (header.type==Res_BehaviorGraph)
	{
		data=(BehaviorGraphData*)ResData_New(header.type);
		if (data)
		{
			data->pads.SetClasses(clsses);
			fl->Seek(header.off);
			IFile_ReadVector(fl,buf);
			dp.SetDataBufferPointer(&buf[0]);
			data->Load(dp);
		}
	}
	fl->Close();
	return data;
}
