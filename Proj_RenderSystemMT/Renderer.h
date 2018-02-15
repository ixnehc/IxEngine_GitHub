#pragma once

#include "RenderSystem/IRenderPort.h"
#include "RenderSystem/IMesh.h"


#include "Base.h"

#include "math/matrix44.h"

#include "resdata/MtrlData.h"

#include "shaderlib/SLDefines.h"

#include "MtrlMgr.h"

#include <vector>
#include <map>
#include <deque>

class CDeviceObject;
class IVertexBuffer;
class ITexture;
struct ViewportInfo;

#define MAX_FEATUREPARAM 64

class CMesh;
class CMtrl;
class VBInfo;
class IBInfo;
class CLight;
class CCamera;
class CMatrice43;
class CShader;

class CRenderer:public IRenderer
{
public:
	CRenderer();
	BOOL Init(IRenderSystem *pRS);
	void UnInit();

	void OnDeviceLost();
	void OnDeviceReset();

	//persistent binding modifying
	BOOL BindCamera(ICamera *cam);

	//Interfaces
	virtual BOOL Begin();
	virtual void End();

	virtual BOOL Render();

	//None-persistent binding modifying
	virtual BOOL ResetContent();//clear all the none-persistent binding
	virtual BOOL BindMesh(IMesh *mesh,DrawMeshArg &dmg);
	virtual BOOL BindVB(VBHandles &vbh,VBBindArg&dmg);
	virtual BOOL BindMtrl(IMtrl *mtrl,int iLayor);
	virtual BOOL BindMats(i_math::matrix43f *mat,DWORD c);
	virtual BOOL BindLight(ILight *l);

	virtual void ClearFeature();//clear all the additional features and feature params
	virtual BOOL AddFeature(FeatureCode &fc);//add additional feature(s)
	void AddEP(EffectParam ep,int v)	{		_epk.AddEP(ep,v);	}
	void AddEP(EffectParam ep,float v){		_epk.AddEP(ep,v);	}
	void AddEP(EffectParam ep,i_math::vector2df&v){		_epk.AddEP(ep,v);	}
	void AddEP(EffectParam ep,i_math::vector3df&v){		_epk.AddEP(ep,v);	}
	void AddEP(EffectParam ep,i_math::vector4df&v){		_epk.AddEP(ep,v);	}
	void AddEP(EffectParam ep,i_math::matrix43f&v){		_epk.AddEP(ep,v);	}
	void AddEP(EffectParam ep,i_math::matrix44f&v){		_epk.AddEP(ep,v);	}
	void AddEP(EffectParam ep,ITexture *v){		_epk.AddEP(ep,v);	}
	virtual BOOL RemoveFeature(FeatureCode &fc);


	virtual IShader *BeginRaw(IMtrl *mtrl,int iMtrlLayor=0,IMesh *mesh=NULL,ILight *lgt=NULL,FeatureCode *fc=NULL);
	virtual IShader *BeginRaw(ShaderCode&sc);
	virtual void EndRaw(IShader *shader);


	//Device State
	virtual BOOL SetRenderState(D3DRENDERSTATETYPE state, DWORD value);

	CDeviceObject *GetDevice()	{		return _devobj;	}


protected:
	CDeviceObject *_devobj;
	IShaderLibMgr *_slmgr;

	//Begin/End Scene state
	BOOL _BeginScene();
	void _EndScene();
	int _nBeginScene;


	//misc

	//render contents
	void _ZeroRenderContent(BOOL bKeepPersistent);
	void _CleanRenderContent(BOOL bKeepPersistent);

	CShader *_GenBatchShader(IMtrl *mtrl,int iMtrlLod,IMesh *mesh,ILight *lgt,FeatureCode *fcAdd);

	//None-Persistent Bindings----------------//
	//Note that the IRenderer only provide the interfaces to modify None-Persistent Bindings
	//mesh
	CMesh *_mesh;
	DrawMeshArg _dmg;//arg to draw the mesh
	//vb
	VBInfo *_vb;
	IBInfo *_ib;
	VBBindArg _argVB;
	//mtrl
	CMtrl *_mtrl;
	int _mtrllayor;
	//matrice
	i_math::matrix43f *_mats;
	DWORD _cMats;
	i_math::matrix43f _matIdentity;
	//lights
	CLight *_lgt;
	//additional features
	FeatureCode _fc;
	EffectParamPacket<2048> _epk;

	//Persistent Bindings----------------//
	//By now,persistent bindings could be modified only by the render port
	CCamera *_cam;//camera
	FeatureCode _fcCam;

};