/********************************************************************
	created:	2006/08/01
	created:	1:8:2006   15:05
	filename: 	e:\IxEngine\Proj_RenderSystem\ResFile.cpp
	file base:	ResFile
	file ext:	cpp
	author:		cxi
	
	purpose:	functions for reading/writing resource file
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include <map>

#include "ResFile.h"
#include "interface/interface.h"

#include "FileSystem/IFileSystem.h"

#include "RenderSystem/IRenderSystem.h"

#include "resdata/MtrlData.h"
#include "resdata/MtrlExtData.h"
#include "resdata/RecordsData.h"
#include "resdata/BehaviorGraphData.h"


#include "assert.h"

#include "Log/LastError.h"

#include "stringparser/stringparser.h"
#include "Log/LogFile.h"

#include "datapacket/DataPacket.h"

#include "resdata/ResData.h"

#pragma warning(disable:4018)

struct ResFileHeader
{
	ResType type:8;
	DWORD off:24;//ResData完整数据的偏移(距离文件起始位置)
};

//如果失败,返回Res_None
ResType LoadResRaw(IFileSystem *pFS,const char *path,std::vector<BYTE>&data,BOOL bHeader)
{
	ResType ret=Res_None;

	IFile *fl=pFS->OpenFileAbs(path,FileAccessMode_Read);
	if (!fl)
		return ret;

	ResFileHeader header;
	fl->Read(&header,sizeof(header));

	ret=header.type;

	CDataPacket dp;
	if (!bHeader)
		fl->Seek(header.off);

	IFile_ReadVector(fl,data);

	fl->Close();

	return ret;
}


//NOTE:generally the RepairResData() should be implement in the corresponding ResourceManager file
#define REPAIR_RD_ENTRY(type,classRD)\
	case type:\
	extern BOOL RepairResData(classRD* ,IRenderSystem *);\
	return RepairResData((classRD*)resdata,pRS)

//use this to ignore the repairing step for that resource type(some of res type need not repair)
#define REPAIR_RD_ENTRY_NULL(type)\
	case type:\
	return TRUE

BOOL RepairResData(ResData *resdata,IRenderSystem *pRS)
{
	switch (resdata->GetType())
	{
		REPAIR_RD_ENTRY(Res_Mtrl,MtrlData);
		REPAIR_RD_ENTRY_NULL(Res_Mesh);
		REPAIR_RD_ENTRY_NULL(ResA_XForm);
		REPAIR_RD_ENTRY_NULL(ResA_MtrlColor);
		REPAIR_RD_ENTRY_NULL(ResA_MapCoord);
		REPAIR_RD_ENTRY_NULL(Res_Dummies);
		REPAIR_RD_ENTRY_NULL(Res_Spt);
		REPAIR_RD_ENTRY_NULL(Res_Mopp);
		REPAIR_RD_ENTRY_NULL(Res_Spg);
		REPAIR_RD_ENTRY_NULL(Res_Texture);
		REPAIR_RD_ENTRY_NULL(Res_Sound);
		REPAIR_RD_ENTRY_NULL(Res_Sheet);
		REPAIR_RD_ENTRY_NULL(Res_Ragdoll);
		REPAIR_RD_ENTRY_NULL(Res_AnimTree);
		REPAIR_RD_ENTRY_NULL(ResA_Bones2);
		REPAIR_RD_ENTRY(Res_MtrlExt,MtrlExtData);
		REPAIR_RD_ENTRY_NULL(Res_Records);
		REPAIR_RD_ENTRY_NULL(Res_Dtr);
		REPAIR_RD_ENTRY(Res_BehaviorGraph,BehaviorGraphData);
		//XXXXX:more res type

	default:
		assert(FALSE);
	}
	return FALSE;
}


BOOL SaveResData(IFileSystem *pFS,const char *path_,ResData *pResData)
{
	std::string path=path_;
	if (TRUE)
	{
		const char *suffix=pResData->GetTypeSuffix();
		if (suffix[0])
		{
			RemoveFileSuffix(path);
			MakeFileSuffix(path,suffix);
		}
	}

	IFile *fl=pFS->OpenFileAbs(path.c_str(),FileAccessMode_Write);
	if (!fl)
		return FALSE;

	std::vector<BYTE>buf;

	DP_BeginSave(dp,buf);
		pResData->SaveHeader(dp);
	DP_EndSave();

	ResFileHeader head;
	head.type=pResData->GetType();
	head.off=sizeof(head)+sizeof(DWORD)+buf.size();

	//文件头
	fl->Write(&head,sizeof(head));

	//ResData的头数据
	IFile_WriteVector(fl,buf);

	//ResData的完整数据
	DP_BeginSave(dp,buf);
		pResData->Save(dp);
	DP_EndSave();

	IFile_WriteVector(fl,buf);

	fl->Close();

	return TRUE;
}


BOOL LoadResData(IFileSystem *pFS,const char *path,ResData *&data,BOOL bHeader)
{
	IFile *fl=pFS->OpenFileAbs(path,FileAccessMode_Read);
	if (!fl)
		return FALSE;

	BOOL bRet=FALSE;

	ResFileHeader header;
	fl->Read(&header,sizeof(header));

	CDataPacket dp;
	std::vector<BYTE>buf;

	data=ResData_New(header.type);
	if (data)
	{
		if (bHeader)
		{
			IFile_ReadVector(fl,buf);
			dp.SetDataBufferPointer(&buf[0]);
			data->LoadHeader(dp);
		}
		else
		{
			fl->Seek(header.off);
			IFile_ReadVector(fl,buf);
			dp.SetDataBufferPointer(&buf[0]);
			data->Load(dp);
		}
		bRet=TRUE;
	}

	fl->Close();

	return bRet;
}


BOOL LoadRecordsData(IFileSystem *pFS,const char *path,RecordsData *&dataRet,CClass *clss)
{
	dataRet=NULL;
	IFile *fl=pFS->OpenFileAbs(path,FileAccessMode_Read);
	if (!fl)
		return FALSE;

	BOOL bRet=FALSE;

	ResFileHeader header;
	fl->Read(&header,sizeof(header));

	CDataPacket dp;
	std::vector<BYTE>buf;

	ResData *data=ResData_New(header.type);
	if (data)
	{
		if (data->GetType()!=Res_Records)
		{
			ResData_Delete(data);
			data=NULL;
		}
	}
	if (data)
	{
		dataRet=(RecordsData*)data;
		dataRet->records.Init(clss);
		fl->Seek(header.off);
		IFile_ReadVector(fl,buf);
		dp.SetDataBufferPointer(&buf[0]);
		dataRet->Load(dp);

		bRet=TRUE;
	}

	fl->Close();

	return bRet;
}
