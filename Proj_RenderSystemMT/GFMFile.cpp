
#include "stdh.h"

#include "GFMFile.h"

BOOL GFMFile::Open(IFileSystem * pFS,const char * nameFont)
{
	Close();
	_nameFont = nameFont;
	
	if(FALSE==pFS->ExistFile(nameFont))
	{
		_pFile = pFS->OpenFile(nameFont,FileAccessMode_Write);
		_pFile->Write(&_count,sizeof(_count));
		_pFile->Close();
	}
	
	_pFile = pFS->OpenFile(nameFont,FileAccessMode_Modify);
	if(_pFile==NULL)
		return FALSE;

	DWORD sz = 0;
	_pFile->Read(&sz,sizeof(sz));
	
	if(sz)
	{
		GFM_FileHeader::GFMItem * item = new GFM_FileHeader::GFMItem[sz];
	
		_pFile->Read(item,sz*sizeof(GFM_FileHeader::GFMItem));

		for(int i = 0;i<sz;i++)
			_fontTable[item[i].code] = item[i].off_GFM;

		delete[] item;
	}

	_count=sz;

	return TRUE;
}
BOOL GFMFile::GetGFM(unsigned short code,GFM * data_GFM)
{
	if(NULL==_pFile)
		return FALSE;
	
	stl_fontTable::iterator it;
	it = _fontTable.find(code);
	if(it!=_fontTable.end())
	{
		DWORD off = (*it).second;

		_pFile->Seek(off);
		_pFile->Read(data_GFM,sizeof(GFM));
	
		return TRUE;
	}

	return FALSE;
}
BOOL GFMFile::UpdateGFM(unsigned short code,GFM * data_GFM)
{
	if(NULL==_pFile)
		return FALSE;

	stl_fontTable::iterator it;
	it = _fontTable.find(code);

	if(it!=_fontTable.end())
	{
		DWORD off = (*it).second;
		
		_pFile->Seek(off);
		
		_pFile->Write(data_GFM,sizeof(GFM));
	}
	else
	{
		DWORD off_data   = GFM_FileHeader::GFM_HeaderSize + _count*sizeof(GFM);
		DWORD off_header = sizeof(_count) + _count*sizeof(GFM_FileHeader::GFMItem);
	
		GFM_FileHeader::GFMItem item;
		item.code = code;
		item.off_GFM = off_data;
		
		// update to file
		_count++;
		_pFile->Reset();
		_pFile->Write(&_count,sizeof(_count));
		
		_pFile->Seek(off_header);
		_pFile->Write(&item,sizeof(GFM_FileHeader::GFMItem));

		_pFile->Seek(off_data);
		_pFile->Write(data_GFM,sizeof(GFM));

		// update memory data
		_fontTable[code] = off_data;
	}

	return TRUE;
}

BOOL GFMFile::Close()
{
	if(_pFile)
		_pFile->Close();
	
	_pFile = NULL;

	return TRUE;
}

	



