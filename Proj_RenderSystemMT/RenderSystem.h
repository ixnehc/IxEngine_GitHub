#pragma once

#include "RenderSystem/IRenderSystem.h"

#include "filewatcher/FileWatcher.h"

#include "Base.h"
#include "DeviceObject.h"

#include "TextureMgr.h"
#include "VertexMgr.h"
#include "SurfaceMgr.h"
#include "ShaderLibMgr.h"

#include "SheetMgr.h"

#include "Renderer.h"

#include "FontMgr.h"
#include "MeshMgr.h"
#include "MtrlMgr.h"
#include "AnimMgr.h"
#include "DummiesMgr.h"
#include "SptMgr.h"
#include "MoppMgr.h"
#include "SpgMgr.h"
#include "AnimTreeMgr.h"
#include "BoneAnimMgr.h"
#include "MtrlExtMgr.h"
#include "SoundMgr.h"
#include "RecordsMgr.h"
#include "RagdollMgr.h"
#include "DtrMgr.h"
#include "BehaviorGraphMgr.h"
//XXXXX:more res type


#include "ResDataThread.h"

#include "PatchBuilder.h"

struct ProfilerMgr;
class CRenderSystem:public IRenderSystem
{
public:
	CRenderSystem();
//interfaces
	virtual BOOL Init(const RenderSystemInit &param);
	virtual BOOL UnInit();//if and leak found,return FALSE,otherwise return TRUE
	virtual BOOL ResetDevice(DeviceConfig &cfg);
	virtual void CleanDevice();
	virtual BOOL IsDeviceReset();
	virtual DeviceCap&GetDeviceCap()	{		return _deviceobj.GetCap();	}
	virtual IFileSystem *GetFS();
	virtual const char *GetPath(RenderSystemPath rsp);
	virtual ProfilerMgr *GetProfilerMgr();
	virtual void RegisterLogHandler(LogHandler &handler);
	virtual void SetStrLib(CStrLib *strlib);
	virtual void OnIdle(__int64 &tIdle);
	virtual void Update(DWORD dwTick);
	virtual BOOL CheckAllResLeak();//return whether there is any leak
	virtual void SetResSyncLoad(BOOL bSync);//设定资源载入模式是异步还是同步,
	virtual BOOL BeginFrame();
	virtual BOOL EndFrame();
	virtual BOOL Present(i_math::recti *rcDest=NULL,i_math::recti *rcSrc=NULL,HWND hwndOverride=NULL);
	virtual BOOL PresentAsyn(i_math::recti *rcDest=NULL,i_math::recti *rcSrc=NULL,HWND hwndOverride=NULL);
	virtual void FlushCommand();
	virtual void UpdateResMonitor();

	virtual AnimTick GetPresentTick()	{		return _tPresent;	}

	virtual IVertexMgr *GetVertexMgr()	{		return &_vertexmgr;	}
	virtual ITextureMgr *GetTexMgr()	{		return &_texmgr;	}
	virtual IRTextureMgr *GetRTexMgr()	{		return &_rtexmgr;	}
	virtual IWTextureMgr *GetWTexMgr2()	{		return &_wtexmgr;	}
	virtual ISurfaceMgr *GetSurfMgr()	{		return &_surfmgr;	}
	virtual IShaderLibMgr *GetShaderLibMgr()	{		return &_slmgr;	}
	virtual IRenderer *GetRenderer()	{		return &_renderer;	}
	virtual IFontMgr *GetFontMgr()	{		return &_fontmgr;	}
	virtual IMeshMgr *GetMeshMgr()	{		return &_meshmgr;	}
	virtual IMtrlMgr *GetMtrlMgr()	{		return &_mtrlmgr;	}
	virtual IAnimMgr *GetAnimMgr()	{		return &_animmgr;	}
	virtual IDynAnimMgr *GetDynAnimMgr()	{		return &_dynanimmgr;	}
	virtual IDummiesMgr *GetDummiesMgr()	{		return (IDummiesMgr *)&_dummiesmgr;	}
	virtual IDynDummiesMgr * GetDynDummiesMgr()	{		return (IDynDummiesMgr *)&_dyndummiesmgr;	}
	virtual ISptMgr   * GetSptMgr()	{		return &_sptmgr;	}
	virtual IDynSptMgr * GetDynSptMgr()	{		return &_dynsptmgr;	}
	virtual IMoppMgr *GetMoppMgr()	{		return &_moppmgr;	}
	virtual ISpgMgr *GetSpgMgr()	{		return &_spgmgr;	}
	virtual ISheetMgr*GetSheetMgr()	{		return &_sheetmgr;	}
	virtual IDynSpgMgr *GetDynSpgMgr()	{		return &_dynspgmgr;	}
	virtual IAnimTreeMgr *GetAnimTreeMgr()	{		return &_animtreemgr;	}
	virtual IDynAnimTreeMgr *GetDynAnimTreeMgr()	{		return &_dynanimtreemgr;	}
	virtual IBoneAnimMgr *GetBoneAnimMgr()		{return &_boneanimmgr;}
	virtual IDynBoneAnimMgr *GetDynBoneAnimMgr() {return &_dynboneanimmgr;}
	virtual IMtrlExtMgr *GetMtrlExtMgr() {return &_mtrlextmgr;}
	virtual ISoundMgr*GetSoundMgr() {return &_soundmgr;}
	virtual IRecordsMgr*GetRecordsMgr() {return &_recordsmgr;}
	virtual IRagdollMgr*GetRagdollMgr() {return &_rgdmgr;}
	virtual IDtrMgr*GetDtrMgr() {return &_dtrmgr;}
	virtual IBehaviorGraphMgr*GetBehaviorGraphMgr() {return &_behaviorgraphmgr;}

	//XXXXX:more res type
	virtual IPatchBuilder * GetPatchBuilder();

	virtual IRenderPort *CreateRenderPort();
	virtual ICamera *CreateCamera();
	virtual ILight *CreateLight();
	virtual IAnimPlayer *CreateAnimPlayer();
	virtual IMatrice43 *CreateMatrice43();
	virtual ISkeleton *CreateSkeleton(SkeletonInfo &si);
	virtual IMeshSnapshot *CreateMeshSnapshot();
	virtual ITexAtlasPool *CreateTexAtlasPool(TexInfo &ti);
	virtual ITexAtlasMap *CreateTexAtlasMap(TexInfo &ti,BOOL bAllowResize);
	virtual IPatchGeom *CreatePatchGeom();
	virtual IPatchQueue *CreatePatchQueue();
	virtual ICvxVolume*CreateConvexVolume();

	CDeviceObject *GetDeviceObj();

	CResDataThread *GetResDataThread()	{		return &_thrdResData;	}

	BOOL IsPortLocked(IRenderPort *port)	{		return port==_curport;	}
	void LockPort(IRenderPort *port)	{		_curport=port;	}
	void UnLockPort(IRenderPort *port)		{	if (_curport==port)	_curport=NULL;		}

	void OnDeviceLost();
	void OnDeviceReset();
	void OnDeviceDestroy();
	void OnDeviceCreate();

	void GarbageCollect();


protected:

	CDeviceObject _deviceobj;

	CTextureMgr _texmgr;
	CVertexMgr _vertexmgr;
	CRTextureMgr _rtexmgr;
	CWTextureMgr _wtexmgr;
	CSurfaceMgr _surfmgr;
	CRenderer _renderer;
	CShaderLibMgr _slmgr;

	CFontMgr _fontmgr;
	CMeshMgr _meshmgr;
	CMtrlMgr _mtrlmgr;
	CAnimMgr _animmgr;
	CDynAnimMgr _dynanimmgr;
	CDummiesMgr   _dummiesmgr;
	CDynDummiesMgr _dyndummiesmgr;
	CSptMgr      _sptmgr;
	CDynSptMgr   _dynsptmgr;
	CMoppMgr _moppmgr;
	CSpgMgr _spgmgr;
	CDynSpgMgr _dynspgmgr; 
	CSheetMgr _sheetmgr;
	CAnimTreeMgr _animtreemgr;
	CDynAnimTreeMgr _dynanimtreemgr;
	CBoneAnimMgr _boneanimmgr;
	CDynBoneAnimMgr _dynboneanimmgr;
	CMtrlExtMgr _mtrlextmgr;
	CSoundMgr _soundmgr;
	CRecordsMgr _recordsmgr;
	CRagdollMgr _rgdmgr;
	CDtrMgr _dtrmgr;
	CBehaviorGraphMgr _behaviorgraphmgr;
	//XXXXX:more res type

	
	CPatchBuilder _patchBuilder;
	CResDataThread _thrdResData;

	CConfig *_cfg;

	IFileSystem *_pFS;
	IFileSystem *_pFS2;//为资源读取线程服务
	IFileSystem *_pFS3;//为音效线程服务

	IRenderPort *_curport;

	BOOL _bMonitorResChange;
	BOOL _bMonitoringResChange;
	BOOL _bProcessResChange;
	CFileWatcher _watcherRes;
	CFileWatcher _watcherEffect;
	CFileWatcher _watcherSheet;
	CFileWatcher _watcherRecords;
	CFileWatcher _watcherBehaviorGraph;

	//path configuration
	std::string _path[Path_Max];

	void _UpdatePresentT();
	AnimTick _tPresent;
	DWORD _tStart;//系统初始化的时间



};




