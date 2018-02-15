#pragma once

#include "GFMDefines.h"
#include "FileSystem/IFileSystem.h"
#include <map>

class GFMFile 
{
public:
	GFMFile(){_pFile = NULL; _count = 0;}
	~GFMFile(){Close();}
	BOOL Open(IFileSystem * pFS,const char * nameFont);
	BOOL GetGFM(unsigned short code,GFM * data_GFM);
	BOOL UpdateGFM(unsigned short code,GFM * data_GFM);
	BOOL Close();
	DWORD GetNumberOfCharacters(){return _count;}
	BOOL IsOpen(){ return (_pFile!=NULL); }

	typedef std::map<unsigned short,unsigned long> stl_fontTable;
private:
	std::string _nameFont;
	stl_fontTable _fontTable;
	DWORD _count;
	IFile * _pFile;
};

