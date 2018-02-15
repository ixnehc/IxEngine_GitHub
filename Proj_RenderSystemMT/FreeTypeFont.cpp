/********************************************************************
	created:	2006/07/13
	created:	13:7:2006   21:44
	filename: 	d:\IxEngine\Proj_RenderSystem\FreeTypeFont.cpp
	author:		cxi
	
	purpose:	freetype font wrapper
*********************************************************************/
#include "stdh.h"
#pragma warning ( disable: 4786 )
#include "FreeTypeFont.h"

#include <ft2build.h>
#include <freetype\ftlist.h>
#include <freetype\internal\ftobjs.h>
#include <freetype\internal\ftdebug.h>
#include <freetype\internal\ftstream.h>
#include <freetype\tttables.h>
#include <freetype\ftsynth.h>



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
//CRichFreeTypeFont2
BOOL CRichFreeTypeFont2::IsInit()
{
	return m_InitRef>0;
}

void CRichFreeTypeFont2::InitAddRef()
{
	m_InitRef++;
}

BOOL CRichFreeTypeFont2::Initialize()
{
	FT_Init_FreeType(&m_FontLibrary);

	InitAddRef();
	return TRUE;
}


void CRichFreeTypeFont2::UnInitialize()
{
	m_InitRef--;
	if (m_InitRef>0)
		return;
	for (int i=0;i<m_faces.size();i++)
		FT_Done_Face(m_faces[i].face);

	FT_Done_FreeType(m_FontLibrary);
}

BOOL CRichFreeTypeFont2::AddFont(const char *pathTTF,BOOL bHint,int depend)
{
	FT_Error error;
	FT_Face face;
	error = FT_New_Face( m_FontLibrary, pathTTF, 0, &face);
	
	if (error==0)
	{
		FaceData data;
		data.face=face;
		data.bHint=bHint;
		data.depend=depend;
		m_faces.push_back(data);
		return TRUE;
	}

	
	return FALSE;
}



//the pointer returned is just for use,never manage(delete) it
FT_Face CRichFreeTypeFont2::GetFTFace(int idFont)
{
	if (idFont>=m_faces.size())
		return NULL;
	return m_faces[idFont].face;
}

FT_Face CRichFreeTypeFont2::GetFTFaceForCode(int idFont,WORD code)
{
	if (idFont>=m_faces.size())
		return NULL;
	if (code<128)
	{
		if (m_faces[idFont].depend!=-1)
			return m_faces[m_faces[idFont].depend].face;
	}
	return GetFTFace(idFont);
}

BOOL CRichFreeTypeFont2::IsFontHint(int idFont)
{
	if (idFont>=m_faces.size())
		return FALSE;
	return m_faces[idFont].bHint;
}
