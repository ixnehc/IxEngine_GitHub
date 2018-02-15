/********************************************************************
	created:	2006/10/17   17:58
	filename: 	e:\IxEngine\Proj_RenderSystem\ShaderLibFile.cpp
	author:		cxi
	
	purpose:	functions for read/write shaderlib file
*********************************************************************/
#include "stdh.h"

#include "interface/interface.h"

#include "FileSystem/IFileSystem.h"
#include "RenderSystem/IRenderSystemDefines.h"

#include "assert.h"

#include "Log/LastError.h"

#include "datapacket/DataPacket.h"

#include "shaderlib/ShaderLib.h"



BOOL SaveShaderLib(IFile *fl,CShaderLib &slib)
{
	CDataPacket dp;
	slib.Save(dp);
	std::vector<BYTE>buffer;
	buffer.resize(dp.GetDataSize());
	dp.SetDataBufferPointer(&buffer[0]);

	slib.Save(dp);

	IFile_WriteVector(fl,buffer);

	return TRUE;
}

BOOL LoadShaderLib(IFile *fl,CShaderLib &slib)
{
	std::vector<BYTE>buffer;
	IFile_ReadVector(fl,buffer);

	CDataPacket dp;
	dp.SetDataBufferPointer(&buffer[0]);
	slib.Load(dp);

	return TRUE;
}
