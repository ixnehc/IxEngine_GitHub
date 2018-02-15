#pragma once



struct TexInfo;

extern BOOL CreateTex(XDirect3DDevice *pDevice,TexInfo &ti,XDirect3DBaseTexture*&pTex,BOOL bRT);
extern void LoadTexInfo(TexInfo &ti,IFile * pFile);
extern BOOL LoadTexInfo(TexInfo &ti,TexData*data0);
extern BOOL LoadTexData(IFile *fl,TexData *data);
extern BOOL SaveTexData(IFile *fl,TexData *data);
extern BOOL LoadTex(XDirect3DDevice *pDevice,TexData *td,TexInfo &ti,XDirect3DBaseTexture*&pTex);
extern BOOL SaveTex(TexData *td,XDirect3DBaseTexture*pTex);//D3DXIFF_DDS


