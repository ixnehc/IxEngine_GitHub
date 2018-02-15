 #include "stdh.h"
#include "commondefines/general_stl.h"

#include "FontMgr.h"

#include "RenderSystem/ITexture.h"
#include "RenderSystem/IRenderSystem.h"
#include "RenderSystem/IVertexBuffer.h"

#include "Log/LogFile.h"
#include "imageprocess/gaussianblur.h"
#include "stringparser/stringparser.h"
#include "Log/LogDump.h"
#include <assert.h>

#pragma warning ( disable: 4786 )
#pragma warning ( disable: 4018 )
#pragma warning ( disable: 4244 )
#pragma warning ( disable: 4267 )




CRichFreeTypeFont2 g_ftf;

//////////////////////////////////////////////////////////////////////////
//CTexSlotPool

 
void CTexSlotPool::Clear()
{
	m_szTotal.set(0,0);
	m_szInSlot.set(0,0);
	m_lenSlot=0;

	m_frees.clear();

	m_allocatable=0;
	m_allocated=0;

	SAFE_RELEASE(m_pTextureData);
	m_pTexMgr=NULL;

}

BOOL CTexSlotPool::Init(DWORD lenSlot,i_math::size2d_sh &szTotal,IWTextureMgr *pMgr)
{
	Clear();

	m_szTotal=szTotal;
	m_lenSlot=lenSlot;

	m_szInSlot.w=m_szTotal.w/m_lenSlot;
	m_szInSlot.h=m_szTotal.h/m_lenSlot;

	m_inners.resize(m_szInSlot.getArea());

	m_allocatable=m_szInSlot.getArea();

	m_pTexMgr=pMgr;


	return TRUE;
}

WORD CTexSlotPool::Alloc()
{
	if (m_frees.size()>0)
	{
		WORD ret=m_frees[m_frees.size()-1];
		m_frees.pop_back();
		return ret;
	}

	if (m_allocated>=m_allocatable)
		return 0xffff;
	return m_allocated++;
}

void CTexSlotPool::Free(WORD iSlot)
{
	if (iSlot==0xffff)
		return;

	m_frees.push_back(iSlot);
}


void CTexSlotPool::_GetSlot(WORD iSlot,i_math::rect_sh &rcSlot)
{
	int x=iSlot%m_szInSlot.w;
	int y=iSlot/m_szInSlot.w;

	rcSlot.set(x*m_lenSlot,y*m_lenSlot,x*m_lenSlot+m_lenSlot,y*m_lenSlot+m_lenSlot);
}



BOOL CTexSlotPool::GetSlot(WORD iSlot,i_math::rect_sh &rcSlot,i_math::rect_sh &rcInner)
{
	if (iSlot==0xffff)
		return FALSE;

	_GetSlot(iSlot,rcSlot);

	rcInner=m_inners[iSlot];
	return TRUE;
}

BYTE *CTexSlotPool::LockDataBuffer(DWORD &pitch)
{
	if ((!m_pTexMgr)&&(!m_pTextureData))
		return NULL;

	if (!m_pTextureData)
		m_pTextureData=m_pTexMgr->Create(m_szTotal.w,m_szTotal.h,D3DFMT_A4R4G4B4);
	if (!m_pTextureData)
		return NULL;

	return (BYTE *)m_pTextureData->Lock(pitch,TexLock_WriteOnly);//TexLock_ReadWrite
}
void CTexSlotPool::UnLockDataBuffer()
{
	if (m_pTextureData)
		m_pTextureData->UnLock();
}

struct Pixel_A4R4G4B4
{
	WORD b:4;
	WORD g:4;
	WORD r:4;
	WORD a:4;
};

//pData is 1 byte per pixel
BOOL CTexSlotPool::FillSlot(WORD iSlot,BYTE *pData,i_math::size2d_sh &sz,DWORD pitchData)
{
	if (iSlot==0xffff)
		return FALSE;

	i_math::rect_sh rcSlot;
	_GetSlot(iSlot,rcSlot);
	i_math::rect_sh rcInner;
	rcInner=rcSlot.arrangeCenter(sz.w,sz.h);
	
	DWORD pitch;
	BYTE *pDataBuffer=LockDataBuffer(pitch);
	if  (!pDataBuffer)
		return FALSE;

	if (TRUE)
	{
		BYTE *q,*p; 
		q=pDataBuffer+(iSlot/m_szInSlot.w)*m_lenSlot*pitch+(iSlot%m_szInSlot.w)*m_lenSlot*2;

		for (int i=0;i<m_lenSlot;i++)
		{
			memset(q,0,m_lenSlot*2);
			q+=pitch;
		}

		q=pDataBuffer+rcInner.Top()*pitch+rcInner.Left()*2;
		p=pData;

		for (int i=0;i<sz.h;i++)
		{
			Pixel_A4R4G4B4 *pxls=(Pixel_A4R4G4B4 *)q;
			for (int j=0;j<sz.w;j++)
			{
				pxls[j].r=pxls[j].g=pxls[j].b=15;
				pxls[j].a=p[j]>>4;
			}
			q+=pitch;
			p+=pitchData;
		}
	}

	UnLockDataBuffer();

	m_inners[iSlot]=rcInner;

	return TRUE;
}

ITexture *CTexSlotPool::GetTexture()
{
	return m_pTextureData;
}

//////////////////////////////////////////////////////////////////////////
//CTexSlotMgr

CTexSlotPool*CTexSlotMgr::_NewPool(PoolDesc &desc,IWTextureMgr *pMgr)
{
	i_math::size2d_sh sz(desc.lenSlot*desc.lenInSlot,desc.lenSlot*desc.lenInSlot);
	CTexSlotPool *p=Class_New2(CTexSlotPool);
	p->Init(desc.lenSlot,sz,pMgr);
	_pools.push_back(p);
	return p;
}

void CTexSlotMgr::_AddDesc(DWORD lenSlot,DWORD lenInSlot)
{
	PoolDesc desc;
	desc.lenSlot=lenSlot;
	desc.lenInSlot=lenInSlot;

	_descs.push_back(desc);
}

BOOL CTexSlotMgr::Init(IWTextureMgr *pMgr)
{
	_texmgr=pMgr;

	_pools.push_back(NULL);

	//注意,必须按照lenSlot的增序加入
	_AddDesc(16,16);
	_AddDesc(24,16);
	_AddDesc(32,16);
	_AddDesc(48,16);
	_AddDesc(64,16);
	_AddDesc(96,8);
	_AddDesc(128,4);
	_AddDesc(256,4);

	return TRUE;
}

void CTexSlotMgr::ClearCache()
{
	for (int i=1;i<_pools.size();i++)
	{
		_pools[i]->Clear();
		Class_Delete(_pools[i]);
	}
	_pools.resize(1);
}

void CTexSlotMgr::Clear()
{
	ClearCache();

	_pools.clear();
	_descs.clear();
}

TexSlotHandle CTexSlotMgr::Alloc(i_math::size2d_sh &sz)
{
	DWORD len=sz.w>sz.h?sz.w:sz.h;

	PoolDesc *desc=NULL;
	for (int i=0;i<_descs.size();i++)
	{
		if (_descs[i].lenSlot>len)
		{
			desc=&_descs[i];
			break;
		}
	}
	if (!desc)
		return TexSlotHandle_Invalid;

	TexSlotHandleData ret;
	for (int i=_pools.size()-1;i>0;i--)
	{
		CTexSlotPool*pool=_pools[i];
		if (pool->m_lenSlot!=desc->lenSlot)
			continue;
		WORD v=pool->Alloc();
		if (v!=0xffff)
		{
			ret.iPool=i;
			ret.iSlot=v;
			return FORCE_TYPE(TexSlotHandle,ret);
		}
	}

	CTexSlotPool*pool=_NewPool(*desc,_texmgr);
	ret.iPool=_pools.size()-1;
	ret.iSlot=pool->Alloc();
	return FORCE_TYPE(TexSlotHandle,ret);
}

void CTexSlotMgr::Free(TexSlotHandle hTexSlot)
{
	if (hTexSlot==TexSlotHandle_Invalid)
		return;
	TexSlotHandleData &data=FORCE_TYPE(TexSlotHandleData,hTexSlot);
	_pools[data.iPool]->Free(data.iSlot);
}

BOOL CTexSlotMgr::GetSlot(TexSlotHandle hSlot,i_math::rect_sh &rcSlot,i_math::rect_sh &rcInner)
{
	if (hSlot==TexSlotHandle_Invalid)
		return FALSE;
	TexSlotHandleData &data=FORCE_TYPE(TexSlotHandleData,hSlot);
	return _pools[data.iPool]->GetSlot(data.iSlot,rcSlot,rcInner);
}

BOOL CTexSlotMgr::FillSlot(TexSlotHandle hSlot,BYTE *pData,i_math::size2d_sh &sz,DWORD pitch)
{
	if (hSlot==TexSlotHandle_Invalid)
		return FALSE;
	TexSlotHandleData &data=FORCE_TYPE(TexSlotHandleData,hSlot);
	return _pools[data.iPool]->FillSlot(data.iSlot,pData,sz,pitch);
}

ITexture *CTexSlotMgr::GetTex(TexSlotHandle hSlot)
{
	if (hSlot==TexSlotHandle_Invalid)
		return NULL;
	TexSlotHandleData &data=FORCE_TYPE(TexSlotHandleData,hSlot);
	return _pools[data.iPool]->GetTexture();
}






//////////////////////////////////////////////////////////////////////////
//CFontMgr

CFontMgr::CFontMgr()
{

}

CFontMgr::~CFontMgr()
{

}


BOOL CFontMgr::Init(IRenderSystem *pRS)
{
	if (!g_ftf.IsInit())
	{
		std::string path;
		std::string pathFont;
		pathFont=pRS->GetPath(Path_Font);
		if (FALSE==g_ftf.Initialize())
		{
			return FALSE;
		}

		path=pathFont+"\\arial.ttf";
		if (FALSE==g_ftf.AddFont(path.c_str(),TRUE))
			LogFile::Prompt("Failed to load additional font file:%s",path.c_str());

		path=pathFont+"\\msyh.ttf";
		if (FALSE==g_ftf.AddFont(path.c_str(),TRUE))
			LogFile::Prompt("Failed to load additional font file:%s",path.c_str());

		path=pathFont+"\\simli.ttf";
		if (FALSE==g_ftf.AddFont(path.c_str(),FALSE,0))
			LogFile::Prompt("Failed to load additional font file:%s",path.c_str());

		path=pathFont+"\\simkai.ttf";
		if (FALSE==g_ftf.AddFont(path.c_str(),FALSE,0))
			LogFile::Prompt("Failed to load additional font file:%s",path.c_str());

		path=pathFont+"\\simhei.ttf";
		if (FALSE==g_ftf.AddFont(path.c_str(),TRUE,0))
			LogFile::Prompt("Failed to load additional font file:%s",path.c_str());

	}
	else
		g_ftf.InitAddRef();//add reference

	//Init the texture slot pool
	m_slotmgr.Init(pRS->GetWTexMgr2());

	m_parser.RegisterSymbol("F",CRwpParser::FontID);
	m_parser.RegisterSymbol("Font",CRwpParser::FontID);
	m_parser.RegisterSymbol("S",CRwpParser::FontSize);
	m_parser.RegisterSymbol("Size",CRwpParser::FontSize);
	m_parser.RegisterSymbol("C",CRwpParser::Color);
	m_parser.RegisterSymbol("Color",CRwpParser::Color);
	m_parser.RegisterSymbol("O",CRwpParser::Outline);
	m_parser.RegisterSymbol("Outline",CRwpParser::Outline);
	m_parser.RegisterSymbol("OC",CRwpParser::OutlineColor);
	m_parser.RegisterSymbol("OutlineColor",CRwpParser::OutlineColor);
	m_parser.RegisterSymbol("Shade",CRwpParser::Shade);
	m_parser.RegisterSymbol("Shd",CRwpParser::Shade);
	m_parser.RegisterSymbol("ShadeColor",CRwpParser::ShadeColor);
	m_parser.RegisterSymbol("SC",CRwpParser::ShadeColor);
	m_parser.RegisterSymbol("A",CRwpParser::Alpha);
	m_parser.RegisterSymbol("Alpha",CRwpParser::Alpha);
	m_parser.RegisterSymbol("L",CRwpParser::Link);
	m_parser.RegisterSymbol("Link",CRwpParser::Link);

	return TRUE;
}

void CFontMgr::ClearCache()
{
	m_mapCodeSlots.clear();
	m_slotmgr.ClearCache();
}


void CFontMgr::UnInit()
{
	ClearCache();

	g_ftf.UnInitialize();
}



//Touch the code,means that the ck is in use,so do not discard it
//if the ck is not current in the texture,add it to the cache
CodeSlot *CFontMgr::Touch(CodeKey ck)
{
	std::hash_map<CodeKey,CodeSlot>::iterator it;
	it=m_mapCodeSlots.find(ck);
	if (it==m_mapCodeSlots.end())
	{
		//Make it
		CodeSlot *slot=&m_mapCodeSlots[ck];
		MakeCodeSlot(slot,ck);
		it=m_mapCodeSlots.find(ck);
	}


	if (!(*it).second.m_bTouched)
	{
		if ((*it).second.m_TouchCount<0)
			(*it).second.m_TouchCount=0;
		(*it).second.m_TouchCount++;
		(*it).second.m_bTouched=TRUE;
	}

	return &(*it).second;
}

void CFontMgr::MakeBlankCodeSlot(CodeSlot *slot,int idFont,int FontSize,FT_Fixed scale,int multiply=1)
{
	FT_Face face=g_ftf.GetFTFaceForCode(idFont,' ');
	assert(face);

	if (TRUE)
	{
		DWORD sz=(FontSize*scale)>>10;
		FT_Set_Char_Size( face, sz, sz, 0, 0);
	}

	int w,ascend,descend;
	FT_Load_Char(face,' ',0);//
	w=((face->glyph->metrics.horiAdvance+32)<<10)/scale;
	ascend=(face->size->metrics.ascender<<10)/scale;
	descend=(face->size->metrics.descender<<10)/scale;

	slot->m_metrics.ascender=ascend;
	slot->m_metrics.descender=-descend;
	slot->m_metrics.w=w*multiply;
	slot->m_metrics.h=0;
	slot->m_metrics.advance=w*multiply;

	slot->m_handle=TexSlotHandle_Invalid;
}


FT_Bitmap CFontMgr::Process(FT_Bitmap &bm,DWORD blurness,DWORD weight)
{
	i_math::size2d_sh sz,szOut;
	sz.w=bm.width;
	sz.h=bm.rows;

	szOut=sz;
	szOut.w+=blurness*2;
	szOut.h+=blurness*2;

	m_temp.resize(szOut.getArea()*sizeof(float));
	VEC_SET(m_temp,0);

	//拷到float的buffer中
	float *bufF=(float*)&m_temp[0];
	BYTE *p=bm.buffer;
	float *q=&bufF[blurness*szOut.w+blurness];
	for (int j=0;j<sz.h;j++)
	{
		for (int i=0;i<sz.w;i++)
		{
			float v=(float)p[i]/255.0f;
			q[i]=i_math::clamp_f(v,0.0f,1.0f);
		}
		q+=szOut.w;
		p+=bm.pitch;
	}

	//高斯模糊
	if (blurness>0)
	{
		CGaussianBlur<float,short> blur(0);
		blur.SetData(bufF,szOut.w,szOut.h);
		blur.Blur(i_math::rect_sh(),blurness*2);
		blur.Blur(i_math::rect_sh(),blurness*2);
		blur.GetData(bufF,szOut.w,szOut.h);
	}
	else
	{
		if (TRUE)
		{
			for (int i=0;i<szOut.getArea();i++)
				bufF[i]=(bufF[i]-0.5f)*1.3f+0.5f;
		}
		else
		{
			std::vector<float>bufT;
			bufT.resize(szOut.getArea());
			const float k=0.1f;
			float core[]={-k,-k,-k,
								-k,8.0f*k+1.0f,-k,
								-k,-k,-k};
			for (int i=0;i<szOut.w;i++)
			for (int j=0;j<szOut.h;j++)
			{
				float sum=0.0f;
				for (int ii=-1;ii<=1;ii++)
				for (int jj=-1;jj<=1;jj++)
				{
					int x=i+ii;
					int y=j+jj;
					if ((x<0)||(y<0)||(x>=szOut.w)||(y>=szOut.h))
						continue;

					sum+=bufF[y*szOut.w+x]*core[(jj+1)*3+ii+1];
				}
				bufT[j*szOut.w+i]=sum;
			}
			memcpy(bufF,&bufT[0],szOut.getArea()*sizeof(float));
	// 		for (int i=0;i<szOut.getArea();i++)
	// 			bufF[i]=(bufF[i]-0.5f)*1.2f+0.5f;
		}

	}

	//转成byte
	BYTE *bufB=&m_temp[0];
	for (int i=0;i<szOut.getArea();i++)
	{
		BYTE t=(BYTE)i_math::clamp_f(bufF[i]*255.0f*(1.0f+0.1f*(float)weight),0,255.0f);
		bufB[i]=t;
	}

	if (FALSE)
	{
		ITexture *tex=m_slotmgr._texmgr->Create(256,256,D3DFMT_A8);
		DWORD pitch;
		BYTE*q=(BYTE*)tex->Lock(pitch,TexLock_ReadOnly);
		BYTE *p=bufB;
		for (int j=0;j<szOut.h;j++)
		{
			memcpy(q,p,szOut.w);
			q+=pitch;
			p+=szOut.w;
		}
		tex->UnLock();
		tex->DumpTga("E:\\Temp\\ttt.tga");
	}

	FT_Bitmap ret=bm;
	ret.rows=szOut.h;
	ret.width=szOut.w;
	ret.pitch=szOut.w;
	ret.buffer=bufB;

	return ret;
}



BOOL CFontMgr::MakeCodeSlot(CodeSlot *slot,CodeKey ck0)
{
	slot->m_bTouched=TRUE;
	slot->m_TouchCount=1;

	CodeKeyData &ck=FORCE_TYPE(CodeKeyData,ck0);

	FT_Face face=g_ftf.GetFTFaceForCode(ck.idFont,ck.code);
	BOOL bHint=g_ftf.IsFontHint(ck.idFont);

	if (!face)
		face=g_ftf.GetFTFace(0);

	assert(face);

	if (ck.code==0x0020)//'blank'
	{
		MakeBlankCodeSlot(slot,ck.idFont,ck.FontSize,ck.scale);
		slot->m_Type=RWIT_BLANK;
		return TRUE;
	}
	if (ck.code==0x0009)//'tab'
	{
		MakeBlankCodeSlot(slot,ck.idFont,ck.FontSize,ck.scale,4);
		slot->m_Type=RWIT_TAB;
		return TRUE;
	}
	if ((ck.code==0x000a))//'\n'
	{
		MakeBlankCodeSlot(slot,ck.idFont,ck.FontSize,ck.scale,0);
		slot->m_Type=RWIT_RET;
		return TRUE;
	} 
	if ((ck.code==0x000d))//'\n'
	{
		MakeBlankCodeSlot(slot,ck.idFont,ck.FontSize,ck.scale,0);
		slot->m_Type=RWIT_BLANK;
		return TRUE;
	} 

	if(FALSE)
	{
		FT_Matrix t;
		t.xx=t.yy=ck.scale;
		t.xy=t.yx=0;
		FT_Set_Transform(face,&t,0);
		FT_Set_Char_Size( face, ck.FontSize<<6, ck.FontSize<< 6, 0, 0);
	}
	else
	{
		//我们采用调整FontSize的方式来匹配pixel rate,之所以不采用设Transform的方法,是因为使用Tranform会破坏hint的效果
		//导致字体边缘不清晰。
		//需要注意的是,FontSize和文字的缩放比例不是严格的线性变化,FontSize的变化只是大致的模拟了pixel rate的变化,并不十分准确
		//(就是说当FontSize乘2以后,画出来的字的大小是不能保证比原来的字在宽和高上都正好大一倍的,但肯定比原来大)
		//所以pixel rate修改后,需要重新排版
		DWORD sz=(ck.FontSize*ck.scale)>>10;
		FT_Set_Char_Size( face,sz ,sz, 0, 0);
	}

	int rv=FT_Load_Char(face,ck.code,bHint?
						(FT_LOAD_RENDER|FT_LOAD_NO_BITMAP):
						(FT_LOAD_RENDER|FT_LOAD_NO_BITMAP|FT_LOAD_NO_HINTING));


	int sz=face->glyph->bitmap.rows*face->glyph->bitmap.pitch;
	if (sz==0)
	{
		MakeBlankCodeSlot(slot,ck.idFont,ck.FontSize,ck.scale);
		slot->m_Type=RWIT_BLANK;
		return TRUE;
	}

	FT_Bitmap *bm=&face->glyph->bitmap;
	int bm_left=face->glyph->bitmap_left,bm_top=face->glyph->bitmap_top;

	FT_Bitmap bmT;
	if (TRUE)
	{
		bmT=Process(*bm,ck.blurness,ck.weight);
		bm=&bmT;
		bm_left-=ck.blurness;
		bm_top+=ck.blurness;
	}


	//the code has some content,we should make a texture slot for it
	i_math::size2d_sh szBitmap;
	szBitmap.w=bm->width;
	szBitmap.h=bm->rows;

	if (FALSE)
	{
		slot->m_metrics.ascender=face->size->metrics.ascender>>6;
		slot->m_metrics.descender=-(face->size->metrics.descender>>6);
		slot->m_metrics.w=(face->glyph->metrics.width+32)>>6;//szBitmap.w*65536/ck.scale;//
		slot->m_metrics.h=(face->glyph->metrics.height+32)>>6;//szBitmap.h*65536/ck.scale;//
		slot->m_metrics.advance=(face->glyph->metrics.horiAdvance+32)>>6;
	}
	else
	{
		//由于FT_Load_Char(..)得到的metrics都是以像素为单位的,但我们排版是通过逻辑单位来排的,所以要把这些
		//排版用的数据除以pixel rate,来得到逻辑单位的值
		if (face->size->metrics.ascender>0)
			slot->m_metrics.ascender=((face->size->metrics.ascender+32)<<10)/ck.scale;
		else
			slot->m_metrics.ascender=((face->size->metrics.ascender-32)<<10)/ck.scale;
		if (face->size->metrics.descender>0)
			slot->m_metrics.descender=-((face->size->metrics.descender+32)<<10)/ck.scale;
		else
			slot->m_metrics.descender=-((face->size->metrics.descender+32)<<10)/ck.scale;
		slot->m_metrics.w=((face->glyph->metrics.width+32)<<10)/ck.scale;
		slot->m_metrics.h=((face->glyph->metrics.height+32)<<10)/ck.scale;
		slot->m_metrics.advance=((face->glyph->metrics.horiAdvance+32)<<10)/ck.scale;
	}

	if (slot->m_metrics.w<=0)
		slot->m_metrics.w=1;
	if (slot->m_metrics.h<=0)
		slot->m_metrics.h=1;


	slot->m_metrics.wBitmap=szBitmap.w;
	slot->m_metrics.hBitmap=szBitmap.h;
	slot->m_metrics.topBitmap=bm_top;
	slot->m_metrics.leftBitmap=bm_left;


	slot->m_handle=m_slotmgr.Alloc(szBitmap);
	if (slot->m_handle==TexSlotHandle_Invalid)
	{
		MakeBlankCodeSlot(slot,ck.idFont,ck.FontSize,ck.scale);
		slot->m_Type=RWIT_BLANK;
		return TRUE;
	}

	slot->m_scale=ck.scale;

	//now copy the data to the slot
	m_slotmgr.FillSlot(slot->m_handle,bm->buffer,szBitmap,bm->pitch);
	slot->m_Type=RWIT_FONT;

	return TRUE;
}


#define DISCARD_THRESHOLD -200

//discard the unused code,and upload the used codes
void CFontMgr::Flush()
{
	//First try to discard the code that hasn't been used for a long time
	if (FALSE)
	{
		std::hash_map<CodeKey,CodeSlot>::iterator it;
		it=m_mapCodeSlots.begin();
		while(it!=m_mapCodeSlots.end())
		{
			CodeSlot *slot;
			slot=&((*it).second);
			slot->m_TouchCount--;
			if (slot->m_bTouched)
			{
				slot->m_bTouched=FALSE;//Reset the flag
				continue;//Do not discard a code with a touched flag
			}
			if (slot->m_TouchCount<DISCARD_THRESHOLD)
			{
				m_slotmgr.Free(slot->m_handle);

				std::hash_map<CodeKey,CodeSlot>::iterator it2;
				it2=it;
				it++;
				m_mapCodeSlots.erase(it2);
				continue;
			}

			it++;
		}
	}		

}


ITextPiece *CFontMgr::MakeText(const char *str)
{
	CRichWordPiece *pRWP=Class_New2(CRichWordPiece);
	pRWP->SetMgr(this);
	pRWP->AddRef();
	pRWP->SetFormatText(str);
	return pRWP;
}

ITexture *CFontMgr::GetFontTexture(DWORD idx)
{
	if (m_slotmgr._pools[1])
		return m_slotmgr._pools[1]->GetTexture();
	return NULL;
}



//////////////////////////////////////////////////////////////////////////
//CRichWordInfo
void CRichWordInfo::Clear()
{
	m_Type=RWIT_NONE;
	m_fmt.attr=0;
}

CodeKey CRichWordInfo::MakeCodeKey(WORD wPasswordCode,FT_Fixed scale)
{
	CodeKeyData ckd;
	memset(&ckd,0,sizeof(ckd));

	ckd.code=wPasswordCode!=0?wPasswordCode:m_Code;
	ckd.idFont=m_fmt.idFont;
	ckd.FontSize=m_fmt.szFont;
	ckd.scale=scale;

	return FORCE_TYPE(CodeKey,ckd);

}


////////////////////////////////////////////////////////////////////////
//CRichWordPiece

CRichWordPiece::Patch CRichWordPiece::m_patches[256];
TxtPatch CRichWordPiece::m_patches2[256];
DWORD CRichWordPiece::m_nPatches=0;



CRichWordPiece::CRichWordPiece()
{
	Zero();
}

void CRichWordPiece::Zero()
{
	m_state=State_Empty;

	m_LineGap=0;
	m_bSingleLine=FALSE;
	m_wLimit=TEXTWIDTHLIMIT_INFINITE;// very big 

	m_fmtDef.Zero();

	m_w=0;
	m_h=0;
	m_dtAlign=DT_LEFT;
	m_wPassword=0;

	m_ptLoc.set(0,0);
	m_clip.set(0,0,0,0);

	m_bSelColor=0;
	m_bShowLink=1;

	m_colSel=0xff000000;
	m_colSelBg=0xffffffff;
	m_colLinkHilight=0xffffffff;
	m_idHilightLink=0;
	m_alpha=0xff;
	m_alphaBg=0xff;

	m_pixelrate=1<<16;
}

void CRichWordPiece::ClearWordInfo()
{
	m_aWordInfo.clear();
	m_w=0;
	m_h=0;
	m_aLineInfo.clear();

	m_state=State_Empty;
}


void CRichWordPiece::Clear()
{
	ClearWordInfo();
	Zero();
}

//Ensure the state is arranged or empty
BOOL CRichWordPiece::EnsureAligned()
{
	if (m_state==State_Aligned)
		return TRUE;
	if (m_state==State_Empty)
		return FALSE;

	Detail();
	Arrange();
	Align(m_dtAlign);
	
	return TRUE;

}



//Try to calculate the dimension of each word info
//return whether all the code are committed to the texture
void CRichWordPiece::Detail()
{
//	assert(m_state>=State_Content);

	if (m_state!=State_Content)
		return;

	//Detail it
	for (int i=0;i<m_aWordInfo.size();i++)
	{
		CRichWordInfo *pWI=&m_aWordInfo[i];

		CodeKey ck=pWI->MakeCodeKey(m_wPassword,m_pixelrate);

		CodeSlot *slot=m_mgr->Touch(ck);
		assert(slot);

		pWI->m_Type=slot->m_Type;
		pWI->m_metrics=slot->m_metrics;
	}


	m_state=State_Detailed;
}


void CRichWordPiece::AddWord(WORD *pCodes,int nCodes,CRichWordInfo &rwiFmt)
{
	assert(m_state<=State_Content);
	DWORD sz=m_aWordInfo.size();
	m_aWordInfo.resize(sz+nCodes);

	rwiFmt.m_Type=RWIT_FONT;
	for (int i=0;i<nCodes;i++)
	{
		rwiFmt.m_Code=pCodes[i];
		m_aWordInfo[sz+i]=rwiFmt;
	}
}


//if nWord==-1,all the words are aligned
BOOL CRichWordPiece::Align(DWORD dt,int nWord)
{
	if (m_aWordInfo.size()<=0)
		return FALSE;
	if (nWord==0)
		return FALSE;

	assert(m_state==State_Arranged);
	if (m_state!=State_Arranged)
		return FALSE;
	if (nWord==-1)
		nWord=m_aWordInfo.size();

	int sz=m_aWordInfo.size();
	
	int iLine=m_aWordInfo[nWord-1].m_iLine;

	int c=0;
	for(int i=0;i<=iLine;i++)
	{
		if (c>=sz)
			break;
		if (m_aWordInfo[c].m_iLine>i)
			continue;
		int left,right;
		left=m_aWordInfo[c].m_x;
		for (;c<sz;c++)
		{
			if (m_aWordInfo[c].m_iLine>i)
				break;
		}
		c--;
		
		if (TRUE)
		{
			int v;
			if (c>nWord-1)
				v=nWord-1;
			else
				v=c;
			right=m_aWordInfo[v].m_x;
			if ((v+1>=sz)||(m_aWordInfo[v+1].m_iLine>i))
				right+=m_aWordInfo[v].m_metrics.advance;//.m_w
			else
				right=m_aWordInfo[v+1].m_x;
		}
		
		int off;
		if (dt&DT_CENTER)
			off=(-(right-left+1))/2-left;
		else
		{
			if (dt&DT_RIGHT)
				off=-right;
			else
				off=-left;
		}
		
		for (int j=c;j>=0;j--)
		{
			if (m_aWordInfo[j].m_iLine<i)
				break;
			m_aWordInfo[j].m_x+=off;
		}
		c++;
	}

	m_state=State_Aligned;
	
	return TRUE;
}


BOOL CRichWordPiece::Arrange()//ReAdjust all the wordinfo's position.
{
	if (m_state!=State_Detailed)
		return FALSE;

	//And then start to Arrange it...

	m_aLineInfo.clear();

	int cxBack=m_wLimit;
	if (m_bSingleLine)
		m_wLimit=0xffff;//Big enough
	
	int i;

	BOOL bNewLine=FALSE;
	int iLastLineStart=0;
	int xOff=0;

	i_math::pos2d_sh ptOrg;
	ptOrg=i_math::pos2d_sh(0,0);

	//ascender is the up distance of a char
	//descender is the down distance of a char
	int ascender,descender,descenderOld,ascenderOld;
	descenderOld=0;
	descender=0;
	ascender=0;
	ascenderOld=0;

	int wMax=-1;
	
	int iLine;
	iLine=0;

	int iLastLineStart2;
	iLastLineStart2=0;

	for (i=0;i<m_aWordInfo.size();i++) 
	{
		if (bNewLine)
		{
			int j;
			for (j=iLastLineStart;j<i-1;j++)
				m_aWordInfo[j].m_y+=ascender;

			iLastLineStart=iLastLineStart2;

			descenderOld=descender;
			ascenderOld=ascender;
			descender=0;
			ascender=0;
			bNewLine=FALSE;
		}

		if (m_aWordInfo[i].m_metrics.ascender>ascender)
			ascender=m_aWordInfo[i].m_metrics.ascender;
		if (m_aWordInfo[i].m_metrics.descender>descender)
			descender=m_aWordInfo[i].m_metrics.descender;

		if ((m_aWordInfo[i].m_Type==RWIT_BLANK)||(m_aWordInfo[i].m_Type==RWIT_TAB))
		{
			m_aWordInfo[i].m_x=ptOrg.X;
			m_aWordInfo[i].m_iLine=iLine;
			ptOrg.X+=m_aWordInfo[i].m_metrics.advance;
			if (m_aWordInfo[i].m_Type==RWIT_TAB)
				ptOrg.X=ptOrg.X/m_aWordInfo[i].m_metrics.w*m_aWordInfo[i].m_metrics.w;

			continue;
		}


		if (m_aWordInfo[i].m_Type==RWIT_RET)
		{
			m_aWordInfo[i].m_x=ptOrg.X;
			m_aWordInfo[i].m_iLine=iLine;

			CRichWordPieceLineInfo ll;
			ll.m_ascender=ascender;
			ll.m_descender=descender;
			ll.m_yBase=ptOrg.Y+ascender;
			ll.m_iStartChar=iLastLineStart;
			m_aLineInfo.push_back(ll);
			
			bNewLine=TRUE;
			iLastLineStart2=i+1;
			iLine++;
			
			ptOrg.X=0;
			ptOrg.Y+=ascender+descender+m_LineGap;
			continue;
		}
			
		if (m_aWordInfo[i].m_Type==RWIT_FONT)
		{
			if (ptOrg.X+m_aWordInfo[i].m_metrics.advance>m_wLimit)
			{
				CRichWordPieceLineInfo ll;
				ll.m_ascender=ascender;
				ll.m_descender=descender;
				ll.m_yBase=ptOrg.Y+ascender;
				ll.m_iStartChar=iLastLineStart;
				m_aLineInfo.push_back(ll);
				bNewLine=TRUE;
				iLastLineStart2=i;
				iLine++;
				ptOrg.X=0;
				ptOrg.Y+=ascender+descender+m_LineGap;
			}
			m_aWordInfo[i].m_x=ptOrg.X;
			m_aWordInfo[i].m_y=ptOrg.Y;
			m_aWordInfo[i].m_iLine=iLine;
			
			ptOrg.X+=m_aWordInfo[i].m_metrics.advance;

			if (ptOrg.X>=wMax)
				wMax=ptOrg.X;
		}
		else
		{
			assert(FALSE);
		}
	}
	if (i>0)
	{
		CRichWordPieceLineInfo ll;
		ll.m_ascender=ascender;
		ll.m_descender=descender;
		ll.m_yBase=ptOrg.Y+ascender;
		ll.m_iStartChar=bNewLine?iLastLineStart2:iLastLineStart;
		m_aLineInfo.push_back(ll);
	}
				
	if (TRUE)
	{
		int j;
		for (j=iLastLineStart;j<i;j++)
			m_aWordInfo[j].m_y+=ascender;
		iLastLineStart=iLastLineStart2;
	}

	if (wMax<0)
		m_w=0;//seems empty,we take the limit width as the max width.
	else
		m_w=wMax;
	
	if ((ptOrg.X>0)||bNewLine)
		m_h=ptOrg.Y+ascender+descender+m_LineGap;
	else
		m_h=ptOrg.Y;

	if (m_bSingleLine)
		m_wLimit=cxBack;//Big enough

	m_state=State_Arranged;

	return TRUE;
}

void ParseRichWordPiece(WORD *pWideCodes,int nWideCodes,CRichWordPiece *pWP);


void CRichWordPiece::SetFormatText(const char* pchNewText)
{
	ClearWordInfo();//Rolling back to State_Empty

	CRichWordInfo::Format fmt;
	//Parse the formatted string and put them into word info
	m_mgr->m_parser.Parse(pchNewText,this,fmt);

	if (m_aWordInfo.size()<=0)
		return;

	m_state=State_Content;

}



void CRichWordPiece::SetWidthLimit(DWORD wLimit)
{
	if (wLimit<=0)
		return;
	if (m_wLimit==wLimit)
		return;//same,need not update
	m_wLimit=wLimit;
	if (m_state>State_Detailed)
		m_state=State_Detailed;
}

DWORD CRichWordPiece::GetWidthLimit()
{
	return m_wLimit;
}


i_math::size2d_sh CRichWordPiece::GetActualSize()
{
	EnsureAligned();
	if (m_state==State_Aligned)
		return i_math::size2d_sh(m_w,m_h);
	return i_math::size2d_sh(0,0);
}

void CRichWordPiece::SetPixelRate(float rate)
{
	FT_Fixed t=(FT_Fixed)(rate*65536.0f);
	m_pixelrate=t;

	if (m_state>State_Content)
		m_state=State_Content;//注意:改变pixel rate会影响排版,所以我们要回滚到Content状态

}


void CRichWordPiece::SetAlign(DWORD dt)//DT_XXXX
{
	if (m_dtAlign==dt)
		return;//Same,need not update
	m_dtAlign=dt;
	if (m_state>State_Arranged)
		m_state=State_Arranged;
}


void CRichWordPiece::SetLineSpan(WORD nSpan)
{
	if (m_LineGap!=nSpan)
	{
		m_LineGap=nSpan;
		if (m_state>State_Detailed)
			m_state=State_Detailed;
	}
}

void CRichWordPiece::SetSingleLine(BOOL bSingleLine)
{
	if (m_bSingleLine==bSingleLine)
		return;//Same ,need not update
	m_bSingleLine=bSingleLine;
	if (m_state>State_Detailed)
		m_state=State_Detailed;
}

BOOL CRichWordPiece::IsSingleLine()
{
	return m_bSingleLine;
}

int CRichWordPiece::GetWhichLine(int nPos)
{
	EnsureAligned();
	if (m_state<State_Arranged)
		return 0;
	if (m_aWordInfo.size()<=0)
		return 0;
	BOOL bExceed=FALSE;
	if (nPos>=m_aWordInfo.size())
	{
		nPos=m_aWordInfo.size()-1;
		bExceed=TRUE;
	}
	if (nPos<0)
		return 0;

	if (bExceed)
	{
		assert(m_aWordInfo.size()>0);
		if (m_aWordInfo[m_aWordInfo.size()-1].m_Type==RWIT_RET)
			return m_aWordInfo[nPos].m_iLine+1;
	}
	return m_aWordInfo[nPos].m_iLine;
}

BOOL CRichWordPiece::IsLineBegin(int nPos)
{
	if (nPos<=0)
		return TRUE;
	return GetWhichLine(nPos)>GetWhichLine(nPos-1);
}




//note code is in WideChar(Unicode)
BOOL CRichWordPiece::CalcWordInfoMetrics(WORD code,int idFont,int FontSize,CRichWordInfo *pRWI)
{
	CodeKeyData ckd;
	memset(&ckd,0,sizeof(ckd));
	ckd.code=code;
	ckd.idFont=idFont;
	ckd.FontSize=FontSize;
	ckd.scale=m_pixelrate;

	CodeSlot *slot=m_mgr->Touch(FORCE_TYPE(CodeKey,ckd));
	assert(slot);

	pRWI->m_metrics=slot->m_metrics;

	return TRUE;
}

int CRichWordPiece::GetLineDescender(int iLine)
{
	EnsureAligned();
	if ((m_state<State_Arranged)||(iLine<0)||(iLine>=m_aLineInfo.size()))
	{
		CRichWordInfo rwi;
		if (FALSE==CalcWordInfoMetrics((WORD)'A',m_fmtDef.idFont,m_fmtDef.szFont,&rwi))
		{
			assert(FALSE);
			return 8;//Simply return an arbitary value
		}
		return rwi.m_metrics.descender;
	}
	
	return m_aLineInfo[iLine].m_descender;
}

int CRichWordPiece::GetLineYBase(int iLine)
{
	EnsureAligned();
	if ((m_state<State_Arranged)||(iLine<0)||(iLine>=m_aLineInfo.size()))
	{
		CRichWordInfo rwi;
		if (FALSE==CalcWordInfoMetrics((WORD)'A',m_fmtDef.idFont,m_fmtDef.szFont,&rwi))
		{
			assert(FALSE);
			return 8;//Simply return an arbitary value
		}
		return rwi.m_metrics.ascender;
	}
	
	return m_aLineInfo[iLine].m_yBase;
}


int CRichWordPiece::GetLineAscender(int iLine)
{
	EnsureAligned();
	if ((m_state<State_Arranged)||(iLine<0)||(iLine>=m_aLineInfo.size()))
	{
		CRichWordInfo rwi;
		if (FALSE==CalcWordInfoMetrics((WORD)'A',m_fmtDef.idFont,m_fmtDef.szFont,&rwi))
		{
			assert(FALSE);
			return 8;//Simply return an arbitary value
		}
		return rwi.m_metrics.ascender;
	}
	
	return m_aLineInfo[iLine].m_ascender;
}


int CRichWordPiece::GetLineHeight(int iLine)
{
	EnsureAligned();
	if ((m_state<State_Arranged)||(iLine<0)||(iLine>=m_aLineInfo.size()))
	{
		CRichWordInfo rwi;
		if (FALSE==CalcWordInfoMetrics((WORD)'A',m_fmtDef.idFont,m_fmtDef.szFont,&rwi))
		{
			assert(FALSE);
			return 8;//Simply return an arbitary value
		}
		return rwi.m_metrics.ascender+rwi.m_metrics.descender+m_LineGap;
	}

	return m_aLineInfo[iLine].m_ascender+m_aLineInfo[iLine].m_descender+m_LineGap;

}

int CRichWordPiece::GetLineNumber()
{
	EnsureAligned();
	if (m_state<State_Arranged)
		return 0;
	return m_aLineInfo.size();
}

int CRichWordPiece::GetLineEndPos(int nLine)
{
	EnsureAligned();
	if (m_state<State_Arranged)
		return -1;
	if ((nLine<0)||(nLine>=m_aLineInfo.size()))
		return -1;
	int iStart;
	iStart=m_aLineInfo[nLine].m_iStartChar;

	int i;
	for (i=iStart;i<m_aWordInfo.size();i++)
	{
		if ((m_aWordInfo[i].m_iLine!=nLine)||(m_aWordInfo[i].m_Type==RWIT_RET))
			return i;
	}

	return i;
}

int CRichWordPiece::GetLineBeginPos(int nLine)
{
	EnsureAligned();
	if (m_state<State_Arranged)
		return -1;
	if ((nLine<0)||(nLine>=m_aLineInfo.size()))
		return -1;
	return m_aLineInfo[nLine].m_iStartChar;
}


//注意:返回的pt是以m_ptLoc为基准点的
BOOL CRichWordPiece::GetCharCoordinate(int nIndex,i_math::pos2d_sh &pt)
{
	EnsureAligned();
	if (m_state<State_Arranged)
		nIndex=0;

	if (nIndex<0)
		nIndex=0;
	if (nIndex>m_aWordInfo.size())
		nIndex=m_aWordInfo.size();


	
	if ((nIndex==m_aWordInfo.size())||(m_state<State_Arranged))
	{
		if (nIndex==0)
		{
			CRichWordInfo rwi;
			if (FALSE==CalcWordInfoMetrics((WORD)'A',m_fmtDef.idFont,m_fmtDef.szFont,&rwi))
			{
				assert(FALSE);
				return FALSE;
			}

			pt.x=0;

			pt.y=rwi.m_metrics.ascender;

			return TRUE;
		}
		CRichWordInfo *pWordInfo;
		pWordInfo=&m_aWordInfo[nIndex-1];
		if (pWordInfo->m_Type==RWIT_RET)
		{
			pt.x=0;
			CRichWordPieceLineInfo *pLineInfo;
			pLineInfo=&m_aLineInfo[pWordInfo->m_iLine];
			pt.y=pLineInfo->m_yBase+pLineInfo->m_ascender+pLineInfo->m_descender+m_LineGap;
		}
		else
		{
			pt.y=m_aLineInfo[pWordInfo->m_iLine].m_yBase;
			if (pWordInfo->m_Type!=RWIT_PICTURE)
				pt.x=pWordInfo->m_x+pWordInfo->m_metrics.advance;
			else
			{
				assert(FALSE);
			}
		}

		return TRUE;
	}

	pt.x=m_aWordInfo[nIndex].m_x;
	pt.y=m_aLineInfo[m_aWordInfo[nIndex].m_iLine].m_yBase;
	return TRUE;
	
}

BOOL CRichWordPiece::GetLineEndCoordinate(int nLine,i_math::pos2d_sh&pt)
{
	EnsureAligned();
	if (m_state<State_Arranged)
		return FALSE;
	if ((nLine<0)||(nLine>=m_aLineInfo.size()))
		return FALSE;

	pt.y=m_aLineInfo[nLine].m_yBase;
	
	int iStart;
	iStart=m_aLineInfo[nLine].m_iStartChar;
	
	int i;
	for (i=iStart;i<m_aWordInfo.size();i++)
	{
		if ((m_aWordInfo[i].m_iLine!=nLine)||(m_aWordInfo[i].m_Type==RWIT_RET))
			break;
	}
	if (i==0)
	{
		pt.x=0;
		return TRUE;
	}
	else
	{
		CRichWordInfo *pWordInfo;
		pWordInfo=&(m_aWordInfo[i-1]);
		if (pWordInfo->m_Type!=RWIT_PICTURE)
		{
			pt.x=pWordInfo->m_x+pWordInfo->m_metrics.advance;
			return TRUE;
		}
	}
	assert(FALSE);
	return FALSE;
}

int CRichWordPiece::DeleteCharW(int nIndex,int nDeleteLen)
{
	m_aWordInfo.erase(m_aWordInfo.begin()+nIndex,m_aWordInfo.begin()+nIndex+nDeleteLen);

	if (m_state>State_Detailed)
		m_state=State_Detailed;

	return nIndex;
	
}

int CRichWordPiece::InsertChar(int nIndex,char* pChar, int nInsertLen)//础程竚+1
{
	if (m_state>State_Content)
		m_state=State_Content;//Roll back to State_Content
	
	WORD*pCodes=new WORD[nInsertLen*2];
	int nWideCodes=MultiByteToWideChar(CODEPAGE_GB,0,pChar,nInsertLen,(LPWSTR)pCodes,nInsertLen*2);

	int i;
	for (i=0;i<nWideCodes;i++)
	{
		CRichWordInfo rwi;
		rwi.m_Code=pCodes[i];
		rwi.m_Type=RWIT_FONT;
		rwi.m_fmt=m_fmtDef;
		if (nIndex>=0)
			m_aWordInfo.insert(m_aWordInfo.begin()+nIndex+i,rwi);
		else
			m_aWordInfo.insert(m_aWordInfo.begin()+i,rwi);
	}

	if (m_aWordInfo.size()>0)
		m_state=State_Content;
	
	delete pCodes;


	if (nIndex<0)
		return nWideCodes;
	return nIndex+nWideCodes;
}

int CRichWordPiece::InsertCharW(int nIndex,WORD* pCodes, int nInsertLen)//础程竚+1
{
	if (m_state>State_Content)
		m_state=State_Content;//Roll back to State_Content
	
	int nWideCodes=nInsertLen;
	
	int i;
	for (i=0;i<nWideCodes;i++)
	{
		CRichWordInfo rwi;
		rwi.m_Code=pCodes[i];
		rwi.m_Type=RWIT_FONT;
		rwi.m_fmt=m_fmtDef;
		m_aWordInfo.insert(m_aWordInfo.begin()+nIndex+i,rwi);
	}
	
	return nIndex+nWideCodes;
}

WORD CRichWordPiece::GetPasswordCode()
{
	return m_wPassword;
}


void CRichWordPiece::SetShowPassword(char cPassword)//if char=='\0', no password	
{
	WORD wPassword;
	MultiByteToWideChar(CODEPAGE_GB,0,&cPassword,1,(LPWSTR)&wPassword,2);

	if (wPassword==m_wPassword)
		return;//No change

	if (m_state>State_Content)
		m_state=State_Content;//Rolling back to State_Content

	m_wPassword = wPassword;
}


void CRichWordPiece::SetSelection( int nStart, int nEnd)//nStart < nEnd
{
	if (nEnd<nStart)
	{
		nStart=0;
		nEnd=m_aWordInfo.size();
	}
	for (int i=nStart;i<nEnd;i++)
		m_aWordInfo[i].SetAttr(RWIA_SELECT);
}

void CRichWordPiece::ClearSelection()
{
	int i;
	for (i=0;i<m_aWordInfo.size();i++)
		m_aWordInfo[i].ClearAttr(RWIA_SELECT);
}


WORD *CRichWordPiece::GetText()//return unicode bytes for text
{
	m_mgr->m_buf.resize(m_aWordInfo.size());

	DWORD sz=m_aWordInfo.size();
	for (int i=0;i<sz;i++)
		m_mgr->m_buf[i]=m_aWordInfo[i].m_Code;

	return (WORD*)&m_mgr->m_buf[0];
}


TextHitResult CRichWordPiece::LineHitTest(int nLine,int x)//return position in content
{
	EnsureAligned();
	TextHitResult ret;
	if (m_state<State_Arranged)
		return ret;
	if ((nLine<0)||(nLine>=m_aLineInfo.size()))
		return ret;

	int iStart;
	iStart=m_aLineInfo[nLine].m_iStartChar;
	int i;
	i=iStart;

	while(m_aWordInfo[i].m_iLine==nLine)
	{
		if (m_aWordInfo[i].m_x>x)
		{
			i--;
			if (i<iStart)
				i=iStart;
			ret.nPos=i;
			return ret;
		}
		i++;
		if (i>=m_aWordInfo.size())
			break;
	}

	if (i>m_aWordInfo.size())
		i=m_aWordInfo.size();
	if (x<m_aWordInfo[i-1].m_x+m_aWordInfo[i-1].m_metrics.advance)
		ret.nPos=i-1;
	else
	{
		if ((m_aWordInfo[i-1].m_Type==RWIT_RET)||(m_aWordInfo[i-1].m_Type==RWIT_BLANK))
			ret.nPos=i-1;
		else
		{
			ret.nPos=i;
			ret.bLineEnd=TRUE;
			ret.iLine=nLine;
		}
	}

	if (ret.nPos<iStart)
		ret.nPos=iStart;

	return ret;
}

TextHitResult CRichWordPiece::HitTest(i_math::pos2d_sh &pt)//return position in content
{
	EnsureAligned();
	if (m_state<State_Arranged)
		return TextHitResult();
	int i;
	int sz;
	sz=m_aLineInfo.size();
	for (i=0;i<sz;i++)
	{
		if (m_aLineInfo[i].m_yBase+m_aLineInfo[i].m_descender>=pt.Y)
		{
			return LineHitTest(i,pt.X);
		}
	}

	TextHitResult ret;
	ret.nPos=m_aWordInfo.size();
	return ret;
}


char *CRichWordPiece::GetTextMB()
{
	WORD *p=GetText();
	if (!p)
		return "";

	int nLen=GetTextLen();

	m_mgr->m_bufMB.resize(nLen*2+1);

	int nLenMB=WideCharToMultiByte(CODEPAGE_GB,0,(LPCWSTR)p,nLen,&m_mgr->m_bufMB[0],nLen*2+1,NULL,NULL);
	m_mgr->m_bufMB[nLenMB]=0;

	return &m_mgr->m_bufMB[0];
}

int CRichWordPiece::GetTextLenMB()
{
	char *p=GetTextMB();
	if (!p)
		return 0;
	return strlen(p);
}

WORD CRichWordPiece::LinkIDHitTest(i_math::pos2d_sh &pt)
{
	EnsureAligned();
	if (GetState()<State_Arranged)
		return 0;
	TextHitResult hit=HitTest(pt);
	int idx=hit.nPos;

	if ((idx<0)||(idx>=m_aWordInfo.size()))
		return 0;//No link id

	CRichWordInfo *pWI;
	pWI=&m_aWordInfo[idx];

	if (pWI->TestAttr(RWIA_LINK))
		return pWI->m_fmt.idLink;

	return 0;
}

BOOL CRichWordPiece::GetLinkString(WORD idLink,std::string &strLink)
{
	if (idLink==0)
		return FALSE;

	if (GetState()<State_Content)
		return FALSE;

	std::vector<WORD>vecWideCodes;
	int i;
	for (i=0;i<m_aWordInfo.size();i++)
	{
		CRichWordInfo *pWI;
		pWI=&m_aWordInfo[i];

		if (!pWI->TestAttr(RWIA_LINK))
			continue;
		
		if (pWI->m_fmt.idLink!=idLink)
		{
			if (vecWideCodes.size()>0)//the link words break
				break;
		}
		if (pWI->m_fmt.idLink==idLink)
			vecWideCodes.push_back(pWI->m_Code);
	}
	if (vecWideCodes.size()<=0)
		return FALSE;

	std::vector<char>vecTemp;
	vecTemp.resize(vecWideCodes.size()*2+1);
	int sz;
	sz=vecWideCodes.size();

	int nMB;
	nMB=WideCharToMultiByte(CODEPAGE_GB,0,(LPCWSTR)&vecWideCodes[0],
		vecWideCodes.size(),&vecTemp[0],vecTemp.size(),NULL,NULL);
	((char*)&vecTemp[0])[nMB]=0;

	strLink=(&vecTemp[0]);

	return TRUE;
}

void CRichWordPiece::ClearPatches()
{
	for(int i=0;i<m_nPatches;i++)
		m_patches[i].Clear();
	m_nPatches=0;
}

void CRichWordPiece::SetLocation(const i_math::pos2d_sh &ptLoc)
{
	m_ptLoc.set(ptLoc.x,ptLoc.y);
}




void CRichWordPiece::SetClip(const i_math::rect_sh &clip)
{
	m_clip=clip;
}


void CRichWordPiece::ApplyArg(const DrawFontArg &arg)
{
	SetLocation(arg.m_ptLoc);
	SetClip(arg.m_rcClip);
	SetWidthLimit(arg.m_wLimit);
	SetSingleLine(arg.m_bSingleLine);
	SetAlign(arg.m_dtAlign);
	SetAlpha(i_math::clamp_i((int)(arg.m_alpha*255.0f),0,255));
}

void CRichWordPiece::SetDefaultFormat(const char *strFmt)
{
	CRichWordPiece rwp;
	CRichWordInfo::Format fmt;
	fmt.Zero();
	m_mgr->m_parser.Parse(strFmt,&rwp,fmt);
	m_fmtDef=fmt;
}


#define PATCHTYPE_NORMAL 0
#define PATCHTYPE_OUTLINE 1
#define PATCHTYPE_SHADE 2
void CRichWordPiece::MakeTxtPatches(int type,i_math::rect_sh &rcClip)
{
	CTexSlotMgr *slotmgr=m_mgr->GetSlotMgr();

	std::vector<CodeSlot*>&vecCodeSlots=m_mgr->m_temp2;
	DWORD nWordInfo=m_aWordInfo.size();
	if (TRUE)//First record code slot for each word info
	{
		vecCodeSlots.resize(nWordInfo);
		VEC_SET(vecCodeSlots,0);
		switch(type)
		{
			case PATCHTYPE_NORMAL:
			{
				for (int i=0;i<nWordInfo;i++)
				{
					CRichWordInfo *pWI=&m_aWordInfo[i];
					if (pWI->m_Type!=RWIT_FONT)
						continue;
					CodeKey ck=pWI->MakeCodeKey(GetPasswordCode(),m_pixelrate);
					CodeKeyData ckd=FORCE_TYPE(CodeKeyData,ck);
//					ckd.weight=2;
// 					if (pWI->m_fmt.attr&RWIA_OUTLINE)
// 						ckd.weight=5;//因为有边框,我们必须把normal的字体加深到不与边框颜色混合得太厉害
					vecCodeSlots[i]=m_mgr->Touch(FORCE_TYPE(CodeKey,ckd));
					if (!vecCodeSlots[i]->m_handle)
						vecCodeSlots[i]=NULL;
				}
				break;
			}
			case PATCHTYPE_OUTLINE:
			{
				for (int i=0;i<nWordInfo;i++)
				{
					CRichWordInfo *pWI=&m_aWordInfo[i];
					if (pWI->m_Type!=RWIT_FONT)
						continue;
					if (pWI->m_fmt.attr&RWIA_SELECT)
						continue;
					if (pWI->m_fmt.attr&RWIA_OUTLINE)
					{
						CodeKey ck=pWI->MakeCodeKey(GetPasswordCode(),m_pixelrate);
						CodeKeyData ckd=FORCE_TYPE(CodeKeyData,ck);
						ckd.blurness=1;
						ckd.weight=60;
						vecCodeSlots[i]=m_mgr->Touch(FORCE_TYPE(CodeKey,ckd));
						if (!vecCodeSlots[i]->m_handle)
							vecCodeSlots[i]=NULL;
					}
					else
						continue;
				}
				break;
			}
			case PATCHTYPE_SHADE:
			{
				for (int i=0;i<nWordInfo;i++)
				{
					CRichWordInfo *pWI=&m_aWordInfo[i];
					if (pWI->m_Type!=RWIT_FONT)
						continue;
					if (pWI->m_fmt.attr&RWIA_SELECT)
						continue;
					if (pWI->m_fmt.attr&RWIA_SHADE)
					{
						CodeKey ck=pWI->MakeCodeKey(GetPasswordCode(),m_pixelrate);
						CodeKeyData ckd=FORCE_TYPE(CodeKeyData,ck);
						ckd.blurness=pWI->m_fmt.blurnessShade;
						ckd.weight=pWI->m_fmt.weightShade;
						vecCodeSlots[i]=m_mgr->Touch(FORCE_TYPE(CodeKey,ckd));
						if (!vecCodeSlots[i]->m_handle)
							vecCodeSlots[i]=NULL;
					}
					else
						continue;
				}
				break;
			}
		}
	}

	i_math::rect_sh rcClip2;
	rcClip2.Left()=((int)rcClip.Left())*m_pixelrate/65536;
	rcClip2.Top()=((int)rcClip.Top())*m_pixelrate/65536;
	rcClip2.Right()=((int)rcClip.Right())*m_pixelrate/65536;
	rcClip2.Bottom()=((int)rcClip.Bottom())*m_pixelrate/65536;

	BOOL bClip=(!(rcClip==TEXTCLIP_INFINITE));

	DWORD glcol = 0xffffffff;
	float scale=((float)m_pixelrate)/65536.0f;
	pos2di ptStart;
	ptStart.set(m_ptLoc.x,m_ptLoc.y);

	VtxQuad vertices[4];
	WORD indices[6];

	while(1)
	{
		int i;
		//find first not NULL code slot
		for (i=0;i<nWordInfo;i++)
		{
			if (vecCodeSlots[i])
				break;
		}

		if (i>=vecCodeSlots.size())//All NULL,break
			break;

		TexSlotHandle handle=vecCodeSlots[i]->m_handle;

		ITexture *pTex=slotmgr->GetTex(handle);
		assert(pTex);

		size2di szTex;
		szTex.w=pTex->GetWidth();
		szTex.h=pTex->GetHeight();

		Patch *pa=&m_patches[m_nPatches];
		m_nPatches++;
		pa->tex=pTex;

		if (TRUE)
		{
			for (int j=0;j<nWordInfo;j++)
			{
				if (!vecCodeSlots[j])
					continue;

				if (!TexSlotHandle_SamePool(vecCodeSlots[j]->m_handle,handle))
					continue;

				CodeSlot *slot=vecCodeSlots[j];
				vecCodeSlots[j]=NULL;//Mark it as processed

				CRichWordInfo *pWI=&m_aWordInfo[j];

				switch(type)
				{
					case PATCHTYPE_NORMAL:
					{
						if (pWI->TestAttr(RWIA_LINK)&&m_bShowLink&&pWI->m_fmt.idLink==m_idHilightLink)
							glcol=ColorAlpha(m_colLinkHilight,m_alpha*pWI->m_fmt.alpha/63);
						else
						{
							if (!pWI->TestAttr(RWIA_SELECT)||(!m_bSelColor))
								glcol=ColorAlpha(From565(pWI->m_fmt.color),m_alpha*pWI->m_fmt.alpha/63);
							else
								glcol=ColorAlpha(m_colSel,m_alpha*pWI->m_fmt.alpha/63);
						}
						break;
					}
					case PATCHTYPE_OUTLINE:
					{
						glcol=From565(pWI->m_fmt.colorOutline);
						break;
					}
					case PATCHTYPE_SHADE:
					{
						glcol=From565(pWI->m_fmt.colorShade);
						break;
					}
				}

				i_math::rect_sh rcWordPiece;//rect on screen
				if (TRUE)
				{
					int x,y;
					x=(scale*(float)(ptStart.x+pWI->m_x));
					y=(scale*(float)(ptStart.y+pWI->m_y));
					rcWordPiece.set(x+slot->m_metrics.leftBitmap,y-slot->m_metrics.topBitmap,
						x+slot->m_metrics.leftBitmap+slot->m_metrics.wBitmap,
						y-slot->m_metrics.topBitmap+slot->m_metrics.hBitmap);

					if (type==PATCHTYPE_SHADE)
					{
						rcWordPiece+=i_math::pos2d_sh((((int)pWI->m_fmt.xShade)*m_pixelrate+32768)/65536,
																			(((int)pWI->m_fmt.yShade)*m_pixelrate+32768)/65536);
					}
				}


				i_math::rect_sh rcSlot,rcInner;//rect on texture
				if (!slotmgr->GetSlot(slot->m_handle,rcSlot,rcInner))
				{
					assert(FALSE);
					continue;
				}

				if (bClip)//Clip both rcWordPiece and rcSlot
				{
					i_math::rect_sh rcClipped=rcWordPiece;
					rcClipped.clipAgainst(rcClip2);
					i_math::rect_sh rcClippedInner;
					rcClippedInner.Left()=rcInner.Left()+(rcClipped.Left()-rcWordPiece.Left())*rcInner.getWidth()/rcWordPiece.getWidth();
					rcClippedInner.Right()=rcInner.Left()+(rcClipped.Right()-rcWordPiece.Left())*rcInner.getWidth()/rcWordPiece.getWidth();
					rcClippedInner.Top()=rcInner.Top()+(rcClipped.Top()-rcWordPiece.Top())*rcInner.getHeight()/rcWordPiece.getHeight();
					rcClippedInner.Bottom()=rcInner.Top()+(rcClipped.Bottom()-rcWordPiece.Top())*rcInner.getHeight()/rcWordPiece.getHeight();

					rcWordPiece=rcClipped;
					rcInner=rcClippedInner;
				}

				if (!rcWordPiece.isValid())//This word piece is totally clipped
					continue;

				if (!rcInner.isValid())
					continue;

				DWORD vbase=pa->vertices.size();

				MakeQuad(vertices,indices,vbase,rcWordPiece,1.0f,rcInner,szTex,glcol);
				VEC_APPEND_BUFFER(pa->vertices,vertices,4);
				VEC_APPEND_BUFFER(pa->indices,indices,6);
			}
		}
	}
	
}

TxtPatch*CRichWordPiece::ObtainPatches(DWORD &cPatches)
{
	cPatches=0;

	EnsureAligned();

	if (m_state!=State_Aligned)
		return NULL;

	i_math::rect_sh rcClip=m_clip;
	if (!rcClip.isValid())
		return NULL;
	BOOL bClip=(!(rcClip==TEXTCLIP_INFINITE));

	VtxQuad vertices[4];
	WORD indices[6];

	CFontMgr *pfm=m_mgr;
	CTexSlotMgr *slotmgr=pfm->GetSlotMgr();
	pos2d_sh ptStart=m_ptLoc;
	float scale=((float)m_pixelrate)/65536.0f;

	ClearPatches();


	int nWordInfo=(int)m_aWordInfo.size();



	// determine text color
	DWORD glcol = 0xffffffff;

	if (m_bSelColor&&(m_alphaBg>0))//First draw the selection background
	{
		Patch *pa=&m_patches[m_nPatches];
		m_nPatches++;

		pa->tex=NULL;

		for (int j=0;j<nWordInfo;j++)
		{
			i_math::rect_sh rcWordPiece;//rect on screen
			if (TRUE)
			{
				CRichWordInfo *pWI=&m_aWordInfo[j];
				if (!pWI->TestAttr(RWIA_SELECT))
					continue;
				int iLine=GetWhichLine(j);
				int ascender,descender;
				ascender=GetLineAscender(iLine);
				descender=GetLineDescender(iLine);
				int yBase=GetLineYBase(iLine);
				rcWordPiece.set(pWI->m_x,yBase-ascender,
					pWI->m_x+pWI->m_metrics.advance,yBase+descender);
				rcWordPiece+=ptStart;
			}

			if (bClip)
				rcWordPiece.clipAgainst(rcClip);

			if (!rcWordPiece.isValid())//This word piece is totally clipped
				continue;

			DWORD vbase=pa->vertices.size();

			MakeQuad(vertices,indices,vbase,rcWordPiece,scale,
				i_math::rect_sh(0,0,1,1),i_math::size2d_sh(2,2),ColorAlpha(m_colSelBg,m_alphaBg));
			VEC_APPEND_BUFFER(pa->vertices,vertices,4);
			VEC_APPEND_BUFFER(pa->indices,indices,6);
		}

	}

	MakeTxtPatches(PATCHTYPE_SHADE,rcClip);
	MakeTxtPatches(PATCHTYPE_OUTLINE,rcClip);
	MakeTxtPatches(PATCHTYPE_NORMAL,rcClip);

	if (m_bShowLink)//now draw the underline
	{
		Patch *pa=&m_patches[m_nPatches];
		m_nPatches++;
		pa->tex=NULL;

		for (int j=0;j<nWordInfo;j++)
		{
			i_math::rect_sh rcWordPiece;//rect on screen
			CRichWordInfo *pWI;
			if (TRUE)
			{
				pWI=&m_aWordInfo[j];
				if (!pWI->TestAttr(RWIA_LINK))
					continue;
				int iLine=GetWhichLine(j);
				int yBase=GetLineYBase(iLine)+2;//

				rcWordPiece.set(pWI->m_x,yBase,
					pWI->m_x+pWI->m_metrics.advance,yBase+1);
				rcWordPiece+=ptStart;
			}


			//Clip rcWordPiece 
			if (bClip)
				rcWordPiece.clipAgainst(rcClip);

			if (!rcWordPiece.isValid())//This word piece is totally clipped
				continue;
			rcWordPiece.Bottom()=rcWordPiece.Top();

			DWORD col;
			if (pWI->m_fmt.idLink==m_idHilightLink)
				col=ColorAlpha(m_colLinkHilight,pWI->m_fmt.alpha*m_alpha/63);
			else
				col=ColorAlpha(From565(pWI->m_fmt.color),pWI->m_fmt.alpha*m_alpha/63);


			DWORD vbase=pa->vertices.size();

			MakeQuad(vertices,indices,vbase,rcWordPiece,scale,
									i_math::rect_sh(0,0,1,1),i_math::size2d_sh(2,2),col);
			VEC_APPEND_BUFFER(pa->vertices,vertices,4);
			VEC_APPEND_BUFFER(pa->indices,indices,6);
		}

	}

	//Now make the result for returning
	if (TRUE)
	{
		for (int i=0;i<m_nPatches;i++)
		{
			if (m_patches[i].vertices.size()<=0)
				continue;
			TxtPatch *pa=&m_patches2[cPatches];
			cPatches++;
			pa->fvf=FVFEx_Quad;
			pa->vtx=&m_patches[i].vertices[0];
			pa->nVtx=m_patches[i].vertices.size();
			pa->idx=&m_patches[i].indices[0];
			pa->nIdx=m_patches[i].indices.size();
			pa->tex=m_patches[i].tex;
		}
	}

	return m_patches2;
}


//////////////////////////////////////////////////////////////////////////
//CRwpParser

#define CODEPAGE_BIG5 950
#define CODEPAGE_GB 936

void CRwpParser::RegisterSymbol(const char *name,TagSymbol symbol)
{
	_mapSymbols[std::string(name)]=symbol;
}

CRwpParser::Tag *CRwpParser::_ParseTag(char *&p)
{
	if (*p=='\\')
	{
		p++;
		return NULL;
	}
	if ((*p)!='{')
		return NULL;

	char *pOrg=p;
	char *backs[16];//big enough
	char backcodes[16];
	DWORD nBacks=0;

	std::hash_map<std::string,TagSymbol>::iterator it;

	memset(&_result,0,sizeof(_result));
	for (int i=0;i<ARRAY_SIZE(_result.args);i++)
		_result.args[i].str="";

	backcodes[nBacks]=*p;
	backs[nBacks++]=p;
	*p++=0;

	char *name=p;

	BOOL bOver=FALSE;//是否结束标志

	while((*p)!=':')
	{
		if (!(*p))
			goto _fail;
		if (*p=='}')
		{
			bOver=TRUE;
			break;
		}
		p++;
	}

	backcodes[nBacks]=*p;
	backs[nBacks++]=p;
	*p++=0;

	it=_mapSymbols.find(std::string(name));
	if (it!=_mapSymbols.end())
		_result.symbol=(*it).second;
	else
		goto _fail;

	DWORD nArgs=0;
	while(!bOver)
	{
		char *arg=p;
		while((*p)!=',')
		{
			if (!(*p))
				goto _fail;
			if (*p=='}')
			{
				bOver=TRUE;
				break;
			}
			p++;
		}

		if (nArgs<ARRAY_SIZE(_result.args))
		{
			backcodes[nBacks]=*p;
			backs[nBacks++]=p;
			*p++=0;

			_result.args[nArgs].v=IntFromString(arg);
			_result.args[nArgs].str=arg;
			nArgs++;
		}
		else
			 goto _fail;
	}

	return &_result;

_fail:
	for (int i=0;i<nBacks;i++)
		*backs[i]=backcodes[i];
	p=pOrg;

	return NULL;
}


void CRwpParser::_FlushAccum(CRichWordPiece *rwp,CRichWordInfo &rwiFmt)
{
	_buf3.resize(_buf2.size());

	int nWideCodes=MultiByteToWideChar(CODEPAGE_GB,0,(LPCSTR)&_buf2[0],_buf2.size(),(LPWSTR)&_buf3[0],_buf3.size());

	rwp->AddWord(&_buf3[0],nWideCodes,rwiFmt);

	_buf2.clear();
}


void CRwpParser::Parse(const char *buf,CRichWordPiece *rwp,CRichWordInfo::Format &fmt)
{
	if (!buf[0])
		return;
	DWORD len=strlen(buf);
	_buf.resize(len+1);
	memcpy(&_buf[0],buf,len+1);

	char *p=&_buf[0];
	char *end=p+len;

	CRichWordInfo rwiFmt;
	rwiFmt.m_fmt=rwp->m_fmtDef;

	_buf2.clear();
	while(p<end)
	{
		Tag *tag=_ParseTag(p);
		if (tag)
		{
			_FlushAccum(rwp,rwiFmt);

			switch(tag->symbol)
			{
				case FontID:
				{
					rwiFmt.m_fmt.idFont=tag->args[0].v;
					if (!g_ftf.IsFontValid(tag->args[0].v))
					{
						rwiFmt.m_fmt.idFont=1;//1号字体应该肯定保证会有
						LOG_DUMP_2P("FontMgr",Log_Error,"在格式字符串\"%s\"中发现无效的字体id(%d)),调整为缺省字体(1)",buf,(int)tag->args[0].v);
					}
					break;
				}
				case FontSize:
				{
					rwiFmt.m_fmt.szFont=i_math::clamp_i(tag->args[0].v,8,120);
					break;
				}
				case Color:
				{
					rwiFmt.m_fmt.color=((tag->args[0].v>>3)<<11)|((tag->args[1].v>>2)<<5)|(tag->args[2].v>>3);
					break;
				}
				case OutlineColor:
				{
					rwiFmt.m_fmt.colorOutline=((tag->args[0].v>>3)<<11)|((tag->args[1].v>>2)<<5)|(tag->args[2].v>>3);
					break;
				}
				case ShadeColor:
				{
					rwiFmt.m_fmt.colorShade=((tag->args[0].v>>3)<<11)|((tag->args[1].v>>2)<<5)|(tag->args[2].v>>3);
					break;
				}
				case Outline:
				{
					if (tag->args[0].v)
						rwiFmt.m_fmt.attr|=RWIA_OUTLINE;
					else
						rwiFmt.m_fmt.attr&=~RWIA_OUTLINE;
					break;
				}
				case Shade:
				{
					if (tag->args[0].v)
						rwiFmt.m_fmt.attr|=RWIA_SHADE;
					else
						rwiFmt.m_fmt.attr&=~RWIA_SHADE;
					rwiFmt.m_fmt.blurnessShade=i_math::clamp_i(tag->args[0].v,0,15);//因为CGaussianBlur的算法限制,目前最大只能15,需要改进
					rwiFmt.m_fmt.xShade=i_math::clamp_i(tag->args[1].v,-60,60);
					rwiFmt.m_fmt.yShade=i_math::clamp_i(tag->args[2].v,-60,60);
					rwiFmt.m_fmt.weightShade=i_math::clamp_i(tag->args[3].v,0,100);
					break;
				}
				case Alpha:
				{
					rwiFmt.m_fmt.alpha=i_math::clamp_i(tag->args[0].v/4,0,63);
					break;
				}
				case Link:
				{
					rwiFmt.m_fmt.idLink=tag->args[0].v;
					if (rwiFmt.m_fmt.idLink!=0)
						rwiFmt.m_fmt.attr|=RWIA_LINK;
					else
						rwiFmt.m_fmt.attr&=~RWIA_LINK;
					break;
				}
				
			}
		}
		else
			_buf2.push_back(*p++);
	}

	_FlushAccum(rwp,rwiFmt);

	fmt=rwiFmt.m_fmt;
}


