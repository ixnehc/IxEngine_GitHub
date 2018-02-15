#pragma once

#include "RenderSystem/IFont.h"

#pragma warning ( disable: 4786 )



#include <math.h>

#include "class/class.h"

#include "math/rect.h"
#include "math/size2d.h"

#include "FreeTypeFont.h"

#include "vertexfmt/vertexfmt.h"

#pragma comment(lib, "freetype219.lib")


class CFontMgr;

typedef unsigned __int64 CodeKey;


struct CodeKeyData
{
	DWORD code:16;//unicode
	DWORD idFont:3;
	DWORD FontSize:13;
	DWORD weight:6;
	DWORD blurness:6;
	FT_Fixed scale:20;//最多16倍

};


typedef DWORD TexSlotHandle;
#define TexSlotHandle_Invalid 0

struct TexSlotHandleData
{
	WORD iPool;
	WORD iSlot;
};

#define TexSlotHandle_SamePool(h1,h2) (FORCE_TYPE(TexSlotHandleData,h1).iPool==FORCE_TYPE(TexSlotHandleData,h2).iPool)


class IWTextureMgr;
class ITexture;


class CTexSlotPool
{
public:
	DEFINE_CLASS(CTexSlotPool)
	CTexSlotPool()
	{
		m_pTexMgr=NULL;
		m_pTextureData=NULL;
		m_allocated=0;
		m_allocatable=0;
		m_lenSlot=0;
	}
	BOOL Init(DWORD lenSlot,i_math::size2d_sh &szTotal,IWTextureMgr *pMgr);
	void Clear();
	WORD Alloc();//如果失败,返回0xffff
	void Free(WORD iSlot);

	BOOL GetSlot(WORD iSlot,i_math::rect_sh &rcSlot,i_math::rect_sh &rcInner);//rcInner以整张贴图的左上角为起始点
	BOOL FillSlot(WORD iSlot,BYTE *pData,i_math::size2d_sh &sz,DWORD pitch);//pData is 1 byte per pixel

	ITexture *GetTexture();

private:
	void _GetSlot(WORD iSlot,i_math::rect_sh &rcSlot);

	i_math::size2d_sh m_szTotal;
	int m_lenSlot;
	i_math::size2d_sh m_szInSlot;//以slot为大小的size

	WORD m_allocatable;
	WORD m_allocated;
	std::deque<WORD> m_frees;


	BYTE *LockDataBuffer(DWORD &pitch);
	void UnLockDataBuffer();

	std::vector<i_math::rect_sh> m_inners;
	ITexture *m_pTextureData;
	IWTextureMgr *m_pTexMgr;

	friend class CFontMgr;
	friend class CTexSlotMgr;
};

class CTexSlotMgr
{
public:
	CTexSlotMgr()
	{
		_texmgr=NULL;
	}
	struct PoolDesc
	{
		DWORD lenSlot;
		DWORD lenInSlot;
	};
	BOOL Init(IWTextureMgr *pMgr);
	void Clear();
	void ClearCache();

	TexSlotHandle Alloc(i_math::size2d_sh &sz);
	void Free(TexSlotHandle hTexSlot);

	BOOL GetSlot(TexSlotHandle hSlot,i_math::rect_sh &rcSlot,i_math::rect_sh &rcInner);//rcInner以整张贴图的左上角为起始点
	BOOL FillSlot(TexSlotHandle hSlot,BYTE *pData,i_math::size2d_sh &sz,DWORD pitch);//pData is 1 byte per pixel
	ITexture *GetTex(TexSlotHandle hSlot);

protected:
	void _AddDesc(DWORD lenSlot,DWORD lenInSlot);
	CTexSlotPool*_NewPool(PoolDesc &desc,IWTextureMgr *pMgr);
	std::vector<PoolDesc>_descs;
	std::vector<CTexSlotPool*>_pools;
	IWTextureMgr *_texmgr;

	friend class CFontMgr;
};


struct FontMetrics
{
	BYTE w,h;//in logic unit(不考虑pixel rate),排版用
	BYTE wBitmap,hBitmap;//in pixel(考虑pixel rate),绘制用
	short topBitmap;//in pixel(考虑pixel rate),绘制用
	char leftBitmap;//in pixel(考虑pixel rate),绘制用
	BYTE advance;//in logic unit(不考虑pixel rate),排版用
	BYTE ascender;//in logic unit(不考虑pixel rate),排版用
	BYTE descender;//in logic unit(不考虑pixel rate),排版用
};

struct CodeSlot
{
	CodeSlot()
	{
		memset(this,0,sizeof(*this));
	}
	int m_TouchCount;

	//if m_bTouched is TRUE,the code should definately NOT be 
	//discarded.if not TRUE,it maybe discarded on urgent 
	//situation(such as:the texture for font is nearly used up,some 
	//code has to be discarded,although their touch count is still 
	//above the discard threshold)
	BOOL m_bTouched;

	TexSlotHandle m_handle;//the slot
	FT_Fixed m_scale;

	//some code's additional information
	BYTE m_Type;
	FontMetrics m_metrics;
};

class CRichWordPiece;
class CRichWordInfo
{
public:
	CRichWordInfo()
	{
	}
	~CRichWordInfo()
	{
		Clear();
	}

	struct Format
	{
		void Zero()
		{
			memset(this,0,sizeof(*this));
			idFont=1;
			szFont=12;
			color=0xffff;
			alpha=63;
		}
		DWORD attr:5;
		DWORD idFont:3;
		DWORD szFont:7;
		int xShade:7;
		int yShade:7;
		DWORD weightShade:7;
		DWORD alpha:6;
		DWORD idLink:12;//Link id.If 0, this word info is not a link 
		DWORD blurnessShade:6;
		WORD color;
		WORD colorOutline;
		WORD colorShade;
	};
	BYTE m_Type;
	WORD m_Code;

	//排版相关
	short m_x,m_y;
	WORD m_iLine;
	FontMetrics m_metrics;
	Format m_fmt;

	//formats

	CodeKey MakeCodeKey(WORD wPasswordCode,FT_Fixed scale);

	void Clear();
	BOOL TestAttr(BYTE v)
	{
		return m_fmt.attr&v;
	}
	void SetAttr(BYTE v)
	{
		m_fmt.attr|=v;
	}
	void ClearAttr(BYTE v)
	{
		m_fmt.attr&=(~v);
	}
	
};

//Rich word piece parser
class CRwpParser
{
public:
	enum TagSymbol
	{
		None=0,
		FontID,
		FontSize,
		Color,
		Outline,
		Shade,
		OutlineColor,
		ShadeColor,
		Alpha,
		Link,
	};
	struct TagArg
	{
		DWORD v;
		const char *str;
	};
	struct Tag
	{
		TagSymbol symbol;
		TagArg args[4];//
	};

	void Parse(const char *buf,CRichWordPiece *rwp,CRichWordInfo::Format &fmt);
	void RegisterSymbol(const char *name,TagSymbol symbol);

protected:
	Tag *_ParseTag(char *&p);
	void _FlushAccum(CRichWordPiece *rwp,CRichWordInfo &rwiFmt);
	std::vector<char>_buf;
	std::vector<char>_buf2;
	std::vector<WORD>_buf3;
	Tag _result;

	std::hash_map<std::string,TagSymbol> _mapSymbols;

};

class IRenderSystem;
class CFontMgr:public IFontMgr
{
public:
	CFontMgr();
	virtual ~CFontMgr();

	BOOL Init(IRenderSystem *pRS);
	void UnInit();

	//interfaces
	virtual void Flush();//discard the unused code,and upload the used codes
	virtual ITextPiece *MakeText(const char *str);
	virtual void ClearCache();

	virtual ITexture *GetFontTexture(DWORD idx);

	CTexSlotMgr *GetSlotMgr()	{		return &m_slotmgr;	}

	CodeSlot *Touch(CodeKey ck);//Touch the code,means that the ck is in use/is going to be used,so do not discard it
	BOOL MakeCodeSlot(CodeSlot *pCodeSlot,CodeKey ck);

protected:
	void MakeBlankCodeSlot(CodeSlot *pCodeSlot,int idFont,int FontSize,FT_Fixed scale,int multiply);

	FT_Bitmap Process(FT_Bitmap&bm,DWORD blurness,DWORD weight);
	CTexSlotMgr m_slotmgr;

	std::hash_map<CodeKey,CodeSlot>m_mapCodeSlots;

	CRwpParser m_parser;


	//临时buffer
	std::vector<BYTE>m_temp;
	std::vector<CodeSlot *>m_temp2;

	//CRichWordPiece::GetText()/GetTextMB()使用的临时buffer
	std::vector<char>m_bufMB;
	std::vector<WORD>m_buf;

	friend class CRichWordPiece;

};



#define RWIT_NONE 0
#define RWIT_PICTURE 1
#define RWIT_FONT 2
#define RWIT_BLANK 3
#define RWIT_TAB 4
#define RWIT_RET 5


#define RWIA_SELECT 1
#define RWIA_SHADE 2
#define RWIA_OUTLINE 4
#define RWIA_LINK 8


class CRichWordPiece;
class CRichWordInfo;

typedef std::vector<CRichWordInfo> VectorRichWordPiece;





class CRichWordPieceLineInfo
{
public:
	int m_ascender;
	int m_descender;
	int m_yBase;
	int m_iStartChar;
};


class CRichWordPiece:public ITextPiece
{
public:
	IMPLEMENT_REFCOUNT_C;
	DEFINE_CLASS(CRichWordPiece);

	enum RWPState
	{
		State_Empty,
		State_Content,//word info has content
		State_Detailed,//word info has detail content(dimension,metrics)
		State_Arranged,//word info has been arranged
		State_Aligned,
	};
	CRichWordPiece();
	~CRichWordPiece()
	{
		Clear();
	}

	void SetMgr(CFontMgr *pmgr)	{		m_mgr=pmgr;	}

	void Zero();
	
	void Clear();

	BOOL EnsureAligned();//Ensure the state is arranged

	//Interfaces

	virtual TxtPatch*ObtainPatches(DWORD &c);
	virtual void ApplyArg(const DrawFontArg &arg);

	virtual void SetDefaultFormat(const char *strFmt);
	virtual void SetFormatText(const char* strText);
	virtual void SetWidthLimit(DWORD wLimit);
	virtual DWORD GetWidthLimit();
	virtual void SetLocation(const i_math::pos2d_sh &ptLoc);
	virtual void SetClip(const i_math::rect_sh &clip);
	virtual i_math::size2d_sh GetActualSize();
	virtual void SetAlign(DWORD dt);//DT_XXXX
	virtual DWORD GetAlign()	{		return m_dtAlign;	}

	virtual void SetLineSpan(WORD nSpan);
	virtual WORD GetLineSpan()	{		return m_LineGap;	}
	virtual void SetSingleLine(BOOL bSingleLine);
	virtual BOOL IsSingleLine();
	
	virtual int GetWhichLine(int nPos);
	virtual BOOL IsLineBegin(int nPos);
	
	virtual int GetLineHeight(int iLine=0);

	virtual int GetLineYBase(int iLine=0);
	virtual int GetLineAscender(int iLine=0);
	virtual int GetLineDescender(int iLine=0);
	
	virtual int GetLineNumber();
	virtual int GetLineEndPos(int nLine);
	virtual int GetLineBeginPos(int nLine);
	
	virtual BOOL GetCharCoordinate(int nIndex,i_math::pos2d_sh &pt);
	virtual BOOL GetLineEndCoordinate(int nLine,i_math::pos2d_sh&pt);	
	
	virtual int DeleteCharW(int nIndex,int nDeleteLen);//埃竚
	virtual int InsertChar(int nIndex,char* pChar, int nInsertLen);//础程竚+1
	virtual int InsertCharW(int nIndex,WORD* pCodes, int nInsertLen);//础程竚+1
	
	virtual void SetShowPassword(char cPassword= '\0');//if char=='\0', no password	
	virtual WORD GetPasswordCode();
	
	virtual void SetSelection( int nStart, int nEnd);//nStart < nEnd (nStart,nEnd设为0,-1时,会选中全部)
	virtual void ClearSelection();
	
	virtual TextHitResult LineHitTest(int nLine,int x);//return position in content
	virtual TextHitResult HitTest(i_math::pos2d_sh &pt);//return position in content
	
	virtual int GetTextLen()	{		return (int)m_aWordInfo.size();	}
	virtual int GetTextLenMB();
	virtual WORD *GetText();//return unicode bytes for text
	virtual char *GetTextMB();//return Multibytes bytes for text

	virtual int GetLineCount()	{		return m_aLineInfo.size();	}

	virtual void SetAlpha(DWORD vAlpha){		m_alpha=(BYTE)vAlpha;	}
	virtual void SetBgAlpha(DWORD vAlpha)	{		m_alphaBg=(BYTE)vAlpha;	}

	virtual void SetSelColor(DWORD colSel,DWORD colSelBg){		m_colSelBg=colSelBg;		m_colSel=colSel;		m_bSelColor=TRUE;	}
	virtual void ClearSelColor(){		m_bSelColor=FALSE;	}

	//For Link
	virtual void ShowLink(DWORD colHilightLink,BOOL bShowLink=TRUE){		m_bShowLink=bShowLink;		m_colLinkHilight=colHilightLink;	}
	virtual void SetHilightLinkID(WORD idHilightLink){		m_idHilightLink=idHilightLink;	}
	virtual WORD GetHilightLinkID()	{		return m_idHilightLink;	}
	virtual WORD LinkIDHitTest(i_math::pos2d_sh &pt);
	virtual BOOL GetLinkString(WORD idLink,std::string &strLink);
	//

	virtual void SetPixelRate(float rate);


	RWPState GetState()	{		return m_state;	}


protected:
	RWPState m_state;
	
	VectorRichWordPiece m_aWordInfo;
	std::vector<CRichWordPieceLineInfo>m_aLineInfo;

	WORD m_wLimit;
	WORD m_w,m_h;

	i_math::pos2d_sh m_ptLoc;
	i_math::rect_sh m_clip;

	DWORD m_dtAlign:4;
	DWORD m_bSingleLine:1;
	DWORD m_LineGap:10;
	DWORD m_bSelColor:1;
	DWORD m_bShowLink:1;

	CRichWordInfo::Format m_fmtDef;//缺省格式

	DWORD m_colSel;
	DWORD m_colSelBg;
	DWORD m_colLinkHilight;
	WORD m_idHilightLink;
	WORD m_wPassword;
	BYTE m_alpha;
	BYTE m_alphaBg;//选择背景的alpha值

	FT_Fixed m_pixelrate;//这个值表示一个逻辑单位对应多少像素

	CFontMgr *m_mgr;

	//temp buffer used for ObtainPatches(..)
	void ClearPatches();
	struct Patch
	{
		void Clear()
		{
			vertices.clear();
			indices.clear();
			tex=NULL;
		}
		ITexture *tex;
		std::vector<VtxQuad> vertices;
		std::vector<WORD>indices;
	};
	static Patch m_patches[256];//big enough
	static TxtPatch m_patches2[256];
	static DWORD m_nPatches;

	
	void AddWord(WORD *pCodes,int nCodes,CRichWordInfo &rwiFmt);//Directly set unicode string,0 (for idFont) is the default font

	
	void AddPicture(int idPicture);
	BOOL InsertPicture(int index,int idPicture);


	void ClearWordInfo();
	void Detail();//Try to calculate the dimension/metrics of each word info

	BOOL Arrange();//ReAdjust all the wordinfo's position.
	BOOL Align(DWORD dt,int nWord=-1);

	void MakeTxtPatches(int type,i_math::rect_sh &rcClip);


	BOOL CalcWordInfoMetrics(WORD code,int idFont,int FontSize,CRichWordInfo *pRWI);

	friend void ParseRichWordPiece(WORD *pWideCodes,int nWideCodes,CRichWordPiece *pWP);
	friend class CRenderPort;
	friend class CRwpParser;

};

