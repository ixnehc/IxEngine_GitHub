#pragma once

#include "ft2build.h"
#include "freetype\freetype.h"

#include <vector>




#define CODEPAGE_BIG5 950
#define CODEPAGE_GB 936

#define CODEPAGE_CURRENT CODEPAGE_GB

#define MAX_ADDITIONAL_FONTS 8



class CWordPiece;
class CRichWordPiece;

class CRichFreeTypeFont2;

struct FaceData
{
	FaceData()
	{
		face=NULL;
		depend=-1;
		bHint=FALSE;
	}
	FT_Face face;
	int depend;//所谓Depend是指,这个字体的非中文字符要用哪个字体来画,如果为-1表示没有depend
	BOOL bHint;
};


class CRichFreeTypeFont2
{
public:
	CRichFreeTypeFont2()
	{
		m_InitRef=0;
	}

	FT_Library m_FontLibrary; 

	std::vector<FaceData>m_faces;


	DWORD m_InitRef;

	BOOL IsInit();
	BOOL Initialize();
	void InitAddRef();
	void UnInitialize();
	BOOL AddFont(const char *pathTTF,BOOL bHint,int depend=-1);


	FT_Face GetFTFace(int idFont);//the pointer returned is just for use,never manage(delete) it
	FT_Face GetFTFaceForCode(int idFont,WORD code);//the pointer returned is just for use,never manage(delete) it
	BOOL IsFontHint(int idFont);
	BOOL IsFontValid(int idFont)	{		return ((DWORD)idFont)<m_faces.size();	}

};


