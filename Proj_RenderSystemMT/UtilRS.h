#pragma once

#include "RenderSystem/IUtilRS.h"

#include "base.h"




class CUtilRS:public IUtilRS
{
public:
	CUtilRS()	{		_pRS=NULL;_pFS=NULL;	_d3dRef=NULL;_deviceRef=NULL;}
//interfaces
//working environment
	virtual void SetRS(IRenderSystem *pRS)	{		_pRS=pRS;	}
	virtual void SetFS(IFileSystem *pFS)	{		_pFS=pFS;	}
	virtual IRenderSystem *GetRS()	{		return _pRS;	}
	virtual IFileSystem *GetFS()		{		return _pFS;	}

//utility functions
	virtual BOOL LockRefDevice()	{		return _LockRefDevice();	}
	virtual void UnlockRefDevice()	{		return _UnlockRefDevice();	}

	//texture
	virtual BOOL LoadTexInfo(TexInfo *ti,TexData *data);
	virtual BOOL LoadTexData(const char *path,TexData *data);
	virtual BOOL SaveTexData(const char *path,TexData *data);
	virtual TexData *LoadTexData(const char *path);
	virtual BOOL CullTexData(TexData *dest,TexData *src,i_math::recti &rc);

	//Res File
	virtual BOOL RepairResData(ResData *resdata);//返回是否有修改
	virtual ResData *LoadRes(const char *path,BOOL bHeader);
	virtual RecordsData *LoadRes_Records(const char *path,CClass *clss);
	virtual BehaviorGraphData*LoadRes_BehaviorGraph(const char *path,LinkPadClasses *clsses);
	virtual BOOL SaveRes(const char *path,ResData *data);



	//Effect compile
	virtual void* CompileEffect(const char *sFX,DWORD lenFX,std::string &error);
	virtual BOOL CheckCompileEffect(const char *sFX,DWORD lenFX,BOOL bAssemblyInfo,std::string &ErrorOrAssem);//check to see whether sFX could pass the compiler

	//Shader Lib
	virtual BOOL SaveShaderLib(CShaderLib &slib,const char *path);
	virtual BOOL LoadShaderLib(CShaderLib &slib,const char *path);


protected:

	IRenderSystem *_pRS;
	IFileSystem *_pFS;

	BOOL _LockRefDevice();
	void _UnlockRefDevice();
	XDirect3D *_d3dRef;
	XDirect3DDevice *_deviceRef;



};




