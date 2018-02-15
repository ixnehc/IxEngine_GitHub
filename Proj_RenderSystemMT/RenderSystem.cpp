/********************************************************************
	created:	2006/06/25
	created:	25:6:2006   20:18
	filename: 	d:\IxEngine\Proj_RenderSystem\RenderSystem.cpp
	author:		cxi
	
	purpose:	RenderSystem entry file,IRenderSystem implement
*********************************************************************/
#include "stdh.h"

#include "assert.h"

#include "interface/interface.h" 

#include "RenderSystem.h"

#include "ResourceBase.h"

#include "Camera.h"
#include "Light.h"
#include "RenderPort.h"
#include "Matrice43.h"
#include "Skeleton.h"


#include "TexAtlasMap.h"
#include "PatchGeom.h"
#include "PatchQueue.h"
#include "ConvexVolume.h"
#include "AnimPlayer.h"

#include "Log/LogFile.h"

#include "Log/LogDump.h"

#include "timer/profiler.h"

#include "strlib/strlib.h"




EXPOSE_SINGLE_INTERFACE(CRenderSystem,IRenderSystem,"RenderSystem01")


LogFile g_logRS("RenderSystem");


CRenderSystem::CRenderSystem()
{
	_pFS=NULL;
	_pFS2=NULL;
	_pFS3=NULL;

	_bMonitorResChange=FALSE;
	_bMonitoringResChange=FALSE;
	_bProcessResChange=FALSE;

}

extern IFolder *LocateFolder(RenderSystemPath path,CRenderSystem *pRS);
BOOL CRenderSystem::Init(const RenderSystemInit &param)
{
	//Device Queue测试代码
// 	while(0)
// 	{
// 		for (int i=0;i<100000;i++)
// 			g_buf2[i]=i;
// 		DQ_Flip();
// 	}
// 	while(0)
// 	{
// 		for (int i=0;i<100000;i++)
// 		{
// 			DQ_PushFuncType(Test_0);
// 			int v=100;
// 			DQ_Push4(v);
// 			DQ_Push4(v);
// 		}
// 		DQ_Flip();
// 	}


//	g_logRS.Dump("RS Init");

	_pFS=param.pFS;
	_pFS2=param.pFS2;
	_pFS3=param.pFS3;
	_curport=NULL;

	_path[Path_Effect]=param.pathEffect;
	_path[Path_Res]=param.pathRes;
	_path[Path_Font]=param.pathFont;
	_path[Path_ShaderCache]=param.pathShaderCache;
	_path[Path_FontCache]=param.pathFontCache;
	_path[Path_Sheet]=param.pathSheet;
	_path[Path_Records]=param.pathRecords;
	_path[Path_BehaviorGraph]=param.pathBehaviorGraph;

	_cfg=param.cfg;

	_bMonitorResChange=param.bMonitorResChange;
	_bMonitorResChange=TRUE;


	if (FALSE==_deviceobj.Init(this))
		return FALSE;

	_texmgr.Init(this,"Texture Manager");

	_vertexmgr.Init(this,"Vertex Manager");
	_wtexmgr.Init(this,"Writable Texture Manager2");
	_rtexmgr.Init(this,"Render Target Texture Manager");
	_surfmgr.Init(this,"Surface Manager");
	_boneanimmgr.Init(this,"BoneAnim2 manager");
	_dynboneanimmgr.Init(this,"BoneAnim2 dynamic manager");
	//
	_slmgr.Init(this,"ShaderLib Manager");
	_slmgr.AddShaderLib("main.slb");
	_slmgr.AddShaderLib("fixfunc.slb");
	_slmgr.AddShaderLib("draw2d.slb");
	_slmgr.AddShaderLib("terrain_ml.slb");
	_slmgr.AddShaderLib("terrain_nk.slb");
	_slmgr.AddShaderLib("postprocess.slb");
	_slmgr.AddShaderLib("bake.slb");
	_slmgr.AddShaderLib("liquid.slb");
	_slmgr.AddShaderLib("deferredshading.slb");
	_slmgr.AddShaderLib("speedtree.slb");
	_slmgr.AddShaderLib("sky.slb");
	_slmgr.AddShaderLib("vegetable.slb");
	_slmgr.AddShaderLib("particle.slb");
	_slmgr.AddShaderLib("shell.slb");
	_slmgr.AddShaderLib("road.slb");
	_slmgr.AddShaderLib("user.slb");

	_sheetmgr.Init(this,"Sheet Manager");


	_renderer.Init(this);

	if (FALSE==_fontmgr.Init(this))//init AFTER deviceobj is Init()
		return FALSE;

	if (FALSE==_meshmgr.Init(this,"Mesh Manager"))
		return FALSE;

	if (FALSE==_mtrlmgr.Init(this,"Material Manager"))
		return FALSE;

	if (FALSE==_animmgr.Init(this,"Anim Manager"))
		return FALSE;

	if (FALSE==_dynanimmgr.Init(this,"DynAnim Manager"))
		return FALSE;

	if(FALSE==_dummiesmgr.Init(this,"Dummies Manager"))
		return FALSE;

	if(FALSE==_dyndummiesmgr.Init(this,"Dynamic Dummies Manager"))
		return FALSE;

	if(FALSE==_sptmgr.Init(this,"SpeedTree manager"))
		return FALSE;
	if(FALSE==_dynsptmgr.Init(this,"Dynamic SpeedTree Manager"))
		return FALSE;

	if(FALSE==_moppmgr.Init(this,"Mopp Manager"))
		return FALSE;

	if(FALSE==_spgmgr.Init(this,"SpeedGrass Manager"))
		return FALSE;

	if(FALSE==_dynspgmgr.Init(this,"Dynamic SpeedGrass Manager"))
		return FALSE;

	if(FALSE==_animtreemgr.Init(this,"Anim Tree Manager"))
		return FALSE;

	if(FALSE==_dynanimtreemgr.Init(this,"Dynamic Anim Tree Manager"))
		return FALSE;

	if(FALSE==_mtrlextmgr.Init(this,"Material Feature Manager"))
		return FALSE;

	if(FALSE==_soundmgr.Init(this,_pFS3,"Sound Manager"))
		return FALSE;

	if(FALSE==_recordsmgr.Init(this,"Records Manager"))
		return FALSE;
	if(FALSE==_rgdmgr.Init(this,"Ragdoll Manager"))
		return FALSE;
	if(FALSE==_dtrmgr.Init(this,"Destructable Manager"))
		return FALSE;
	if(FALSE==_behaviorgraphmgr.Init(this,"BehaviorGraph Manager"))
		return FALSE;


	_patchBuilder.Create(this,FVFEX_XYZW0|FVFEX_NORMAL0,15000,60000);
	//XXXXX:more res type


	_tStart=GetTickCount();
	_tPresent=0;

	return TRUE;

}


//if any leak found,return FALSE,otherwise return TRUE
BOOL CRenderSystem::UnInit()
{
	if (_bMonitoringResChange)
	{
		_watcherRes.Stop();
		_watcherEffect.Stop();
		_watcherSheet.Stop();
		_watcherRecords.Stop();
		_watcherBehaviorGraph.Stop();
	}

	_curport=NULL;

	BOOL bLeak=FALSE;

	//XXXXX:more res type

	RESMGR_UNINIT(_behaviorgraphmgr,bLeak);
	RESMGR_UNINIT(_dtrmgr,bLeak);
	RESMGR_UNINIT(_recordsmgr,bLeak);
	RESMGR_UNINIT(_animtreemgr,bLeak);
	RESMGR_UNINIT(_dynanimtreemgr,bLeak);
	RESMGR_UNINIT(_mtrlmgr,bLeak);
	RESMGR_UNINIT(_animmgr,bLeak);
	RESMGR_UNINIT(_dynanimmgr,bLeak);
	RESMGR_UNINIT(_meshmgr,bLeak);
	RESMGR_UNINIT(_sptmgr,bLeak);
	RESMGR_UNINIT(_dynsptmgr,bLeak);
	RESMGR_UNINIT(_dummiesmgr,bLeak);
	RESMGR_UNINIT(_dyndummiesmgr,bLeak);
	RESMGR_UNINIT(_moppmgr,bLeak);
	RESMGR_UNINIT(_spgmgr,bLeak);
	RESMGR_UNINIT(_dynspgmgr,bLeak);
	RESMGR_UNINIT(_boneanimmgr,bLeak);
	RESMGR_UNINIT(_dynboneanimmgr,bLeak);
	RESMGR_UNINIT(_mtrlextmgr,bLeak);

	_fontmgr.UnInit();//UnInit BEFORE deviceobj is UnInit()

	_renderer.UnInit();
	RESMGR_UNINIT(_slmgr,bLeak);

	RESMGR_UNINIT(_sheetmgr,bLeak);
	RESMGR_UNINIT(_soundmgr,bLeak);
	RESMGR_UNINIT(_rgdmgr,bLeak);

	RESMGR_UNINIT(_texmgr,bLeak);
	RESMGR_UNINIT(_vertexmgr,bLeak);
	RESMGR_UNINIT(_wtexmgr,bLeak);
	RESMGR_UNINIT(_rtexmgr,bLeak);
	RESMGR_UNINIT(_surfmgr,bLeak);

	
	_patchBuilder.Destroy();

	bLeak|=(!_deviceobj.UnInit());

	_cfg=NULL;

//	D3D_FINISH();
//	g_logRS.Dump("RS UnInit");

	return !bLeak;
}

BOOL CRenderSystem::ResetDevice(DeviceConfig &cfg)
{

	if (FALSE==GetDeviceObj()->ResetConfig(cfg))
		return FALSE;

	_thrdResData.Init(_pFS2,this);

	_thrdResData.Start();

	return TRUE;
}

void CRenderSystem::CleanDevice()
{
	GetDeviceObj()->Clean();
}

BOOL CRenderSystem::IsDeviceReset()
{
	if (GetDeviceObj()->GetDevice())
		return TRUE;
	return FALSE;
}	


void CRenderSystem::OnIdle(__int64 &tIdle)
{
//	BOOL bMore;
//	bMore=TRUE;
//	while(bMore)
//	{
//		bMore=FALSE;
//		std::map<std::string,CResourceMgr *>::iterator it;
//		for (it=_allmgrs.begin();it!=_allmgrs.end();it++)
//		{
//			if (((*it).second)->_DoIdle(tIdle))
//				bMore=TRUE;
//			if (tIdle<=0)
//				return;
//		}
//	}
}



void CRenderSystem::Update(DWORD dwTick)
{
	GarbageCollect();
	UpdateResMonitor();
//	std::map<std::string,CResourceMgr *>::iterator it;
//	for (it=_allmgrs.begin();it!=_allmgrs.end();it++)
//		((*it).second)->Update()
	


}

//return whether there is any leak
BOOL CRenderSystem::CheckAllResLeak()
{
	BOOL bLeak=FALSE;
//	std::map<std::string,CResourceMgr *>::iterator it;
//	for (it=_allmgrs.begin();it!=_allmgrs.end();it++)
//	{
//		if (((*it).second)->CheckResLeak())
//		{
//			LogFile::Prompt("Error:Resource Leak found in \"%s\"!",((*it).second)->_name.c_str());
//			bLeak=TRUE;
//		}
//	}
	return bLeak;
}

void CRenderSystem::SetResSyncLoad(BOOL bSync)
{
	CResource::_bSyncTouch=bSync;
}



BOOL CRenderSystem::BeginFrame()
{
	if (FALSE==_renderer.Begin())
		return FALSE;

	LockPort(NULL);//Unlock all the port

	return TRUE;
}
BOOL CRenderSystem::EndFrame()
{
	_renderer.End();
	_fontmgr.Flush();

//	_thrdResData.GetQueue()->Flush();
	
	GarbageCollect();
 
	UpdateResMonitor();
	return TRUE;
}

void CRenderSystem::_UpdatePresentT()
{
	DWORD t=GetTickCount();
	t-=_tStart;

	_tPresent=ANIMTICK_FROM_SECOND(((float)t)/1000.0f);
}


BOOL CRenderSystem::Present(i_math::recti *rcDest,i_math::recti *rcSrc,HWND hwndOverride)
{
	BOOL b=_deviceobj.Present(rcDest,rcSrc,hwndOverride);
	D3D_FLUSH();

	_UpdatePresentT();

	return b;
}

BOOL CRenderSystem::PresentAsyn(i_math::recti *rcDest,i_math::recti *rcSrc,HWND hwndOverride)
{
	BOOL b=_deviceobj.Present(rcDest,rcSrc,hwndOverride);
	D3D_FLIP();

	_UpdatePresentT();

	return b;
}



CDeviceObject *CRenderSystem::GetDeviceObj()
{
	return &_deviceobj;
}



IRenderPort *CRenderSystem::CreateRenderPort()
{
	CRenderPort *p=Class_New2(CRenderPort);
	if (FALSE==p->Init(this))
		goto _fail;
	p->SetRect_Total();

	p->AddRef();

	return p;

_fail:
	Class_Delete(p);
	return NULL;
}

ILight *CRenderSystem::CreateLight()
{
	CLight *p=Class_New2(CLight);

	if (!p->Init())
	{
		Class_Delete(p);
		return NULL;
	}

	p->AddRef();
	return p;
}


ICamera *CRenderSystem::CreateCamera()
{
	CCamera *p=Class_New2(CCamera);

	if (FALSE==p->Init())
	{
		Class_Delete(p);
		return NULL;
	}
	p->AddRef();
	return p;
}

IMatrice43 *CRenderSystem::CreateMatrice43()
{
	CMatrice43 *p=Class_New2(CMatrice43);
	p->AddRef();
	return p;
}

ISkeleton *CRenderSystem::CreateSkeleton(SkeletonInfo &si)
{
	CSkeleton *p=Class_New2(CSkeleton);
	p->_si=si;
	p->_pRS=this;
	p->AddRef();

	return p;
}


IAnimPlayer*CRenderSystem::CreateAnimPlayer()
{
	IAnimPlayer*player=Class_New2(CAnimPlayer);
	player->AddRef();
	return player;
}

IMeshSnapshot *CRenderSystem::CreateMeshSnapshot()
{
	CMeshSnapshot *ss=Class_New2(CMeshSnapshot);
	ss->AddRef();
	return ss;
}

ITexAtlasPool *CRenderSystem::CreateTexAtlasPool(TexInfo &ti)
{
	CTexAtlasPool *pool=Class_New2(CTexAtlasPool);
	if (FALSE==pool->Init(this,ti))
	{
		Class_Delete(pool);
		return NULL;
	}
	pool->AddRef();
	return pool;
}

ITexAtlasMap *CRenderSystem::CreateTexAtlasMap(TexInfo &ti,BOOL bAllowResize)
{
	CTexAtlasMap *mp=Class_New2(CTexAtlasMap);
	if (FALSE==mp->Init(this,ti,bAllowResize))
	{
		Class_Delete(mp);
		return NULL;
	}
	mp->AddRef();
	return mp;
}

IPatchGeom *CRenderSystem::CreatePatchGeom()
{
	CPatchGeom *geo=Class_New2(CPatchGeom);
	geo->Init(this);
	geo->AddRef();
	return geo;
}
IPatchQueue *CRenderSystem::CreatePatchQueue()
{
	CPatchQueue *que=Class_New2(CPatchQueue);
	que->Init(this);
	que->AddRef();
	return que;
}


ICvxVolume*CRenderSystem::CreateConvexVolume()
{
	CCvxVolume *vol=Class_New2(CCvxVolume);
	vol->AddRef();
	return vol;
}

IPatchBuilder * CRenderSystem::GetPatchBuilder()
{
	return &_patchBuilder;
}

IFileSystem *CRenderSystem::GetFS()
{
	return _pFS;
}

const char *CRenderSystem::GetPath(RenderSystemPath rsp)
{
	if ((rsp<0)||(rsp>=Path_Max))
		return "";
	return _path[rsp].c_str();
}


ProfilerMgr *CRenderSystem::GetProfilerMgr()
{
	::GetProfilerMgr()->SetName("RenderSystem");
	return ::GetProfilerMgr();
}

void CRenderSystem::RegisterLogHandler(LogHandler &handler)
{
	extern void ::RegisterLogHandler(LogHandler &handler);
	::RegisterLogHandler(handler);
}


void CRenderSystem::SetStrLib(CStrLib *strlib)
{
	::StrLib_Set(strlib);
}


void CRenderSystem::OnDeviceLost()
{
	if (TRUE)
	{
		CClass *clss=Class_Ptr2(CPatchGeom);
		DWORD count;
		CPatchGeom **pgs=(CPatchGeom **)clss->GetInstances(count);
		for (int i=0;i<count;i++)
			pgs[i]->OnDeviceLost();
	}

	_renderer.OnDeviceLost();
	_slmgr.OnDeviceLost();
	_vertexmgr.OnDeviceLost();
	_rtexmgr.OnDeviceLost();
	_surfmgr.OnDeviceLost();
}

void CRenderSystem::OnDeviceReset()
{
	_vertexmgr.OnDeviceReset();
	_slmgr.OnDeviceReset();
	_surfmgr.OnDeviceReset();
	_rtexmgr.OnDeviceReset();
	//
	_renderer.OnDeviceReset();

	if (TRUE)
	{
		CClass *clss=Class_Ptr2(CPatchQueue);
		DWORD count;
		CPatchQueue**pqs=(CPatchQueue**)clss->GetInstances(count);
		for (int i=0;i<count;i++)
			pqs[i]->OnDeviceReset();
	}



}
void CRenderSystem::OnDeviceDestroy()
{
	_texmgr.OnDeviceDestroy();
	_wtexmgr.OnDeviceDestroy();
}
void CRenderSystem::OnDeviceCreate()
{
	_texmgr.OnDeviceCreate();
	_wtexmgr.OnDeviceCreate();
}


void CRenderSystem::GarbageCollect()
{
	DWORD step=100;
	_texmgr.GarbageCollect(step);
	_vertexmgr.GarbageCollect(step);
	_rtexmgr.GarbageCollect(step);
	_wtexmgr.GarbageCollect(step);
	_surfmgr.GarbageCollect(step);
	_slmgr.GarbageCollect(step);

	_sheetmgr.GarbageCollect(step);

	_meshmgr.GarbageCollect(step);
	_mtrlmgr.GarbageCollect(step);
	_animmgr.GarbageCollect(step);
	_dynanimmgr.GarbageCollect(step);
	_dummiesmgr.GarbageCollect(step);
	_dyndummiesmgr.GarbageCollect(step);
	_sptmgr.GarbageCollect(step);
	_dynsptmgr.GarbageCollect(step);
	_moppmgr.GarbageCollect(step);
	_spgmgr.GarbageCollect(step);
	_dynspgmgr.GarbageCollect(step);
	_animtreemgr.GarbageCollect(step);
	_dynanimtreemgr.GarbageCollect(step);
	_boneanimmgr.GarbageCollect(step);
	_mtrlextmgr.GarbageCollect(step);
	_soundmgr.GarbageCollect(step);
	_recordsmgr.GarbageCollect(step);
	_rgdmgr.GarbageCollect(step);
	_dtrmgr.GarbageCollect(step);
	_behaviorgraphmgr.GarbageCollect(step);
	//XXXXX:more res type


}

void CRenderSystem::FlushCommand()
{
	_deviceobj.FlushCommand();
}

void CRenderSystem::UpdateResMonitor()
{
	if (_bMonitorResChange)
	{
		if (!_bMonitoringResChange)
		{
			_watcherRes.Start(GetPath(Path_Res),WNF_CHANGE_FILE_NAME|WNF_CHANGE_LAST_WRITE|WNF_CHANGE_CREATION);
			_watcherEffect.Start(GetPath(Path_Effect),WNF_CHANGE_FILE_NAME|WNF_CHANGE_LAST_WRITE|WNF_CHANGE_CREATION);
			_watcherSheet.Start(GetPath(Path_Sheet),WNF_CHANGE_FILE_NAME|WNF_CHANGE_LAST_WRITE|WNF_CHANGE_CREATION);
			_watcherRecords.Start(GetPath(Path_Records),WNF_CHANGE_FILE_NAME|WNF_CHANGE_LAST_WRITE|WNF_CHANGE_CREATION);
			_watcherBehaviorGraph.Start(GetPath(Path_BehaviorGraph),WNF_CHANGE_FILE_NAME|WNF_CHANGE_LAST_WRITE|WNF_CHANGE_CREATION);

			_bMonitoringResChange=TRUE;

		}
	}

	if (_bMonitoringResChange)
	{
		if (!_bProcessResChange)//这个标志用来防止递归调用(LOG_DUMP(..)可能会导致这个函数被重入)
		{
			_bProcessResChange=TRUE;

			ChangedFileInformation *info=NULL;
			DWORD c=_watcherRes.FetchChangedFiles((const ChangedFileInformation *&)info);

			for (int i=0;i<c;i++)
			{
				ChangedFileAction action=info[i].action;
				switch(action)
				{
					case FA_REMOVED:
					case FA_MODIFIED:
					case FA_RENAMED_OLD_NAME:
					case FA_RENAMED_NEW_NAME:
						break;
					default:
						continue;
				}


				BOOL bReloaded=FALSE;

				bReloaded=_meshmgr.ReloadRes(info[i].name)||bReloaded;
				if (!bReloaded)
					bReloaded=_mtrlmgr.ReloadRes(info[i].name);
				if (!bReloaded)
					bReloaded=_animmgr.ReloadRes(info[i].name);
				if (!bReloaded)
					bReloaded=_dummiesmgr.ReloadRes(info[i].name);
				if (!bReloaded)
					bReloaded=_sptmgr.ReloadRes(info[i].name);
				if (!bReloaded)
					bReloaded=_spgmgr.ReloadRes(info[i].name);
				if (!bReloaded)
					bReloaded=_moppmgr.ReloadRes(info[i].name);
				if (!bReloaded)
					bReloaded=_texmgr.ReloadRes(info[i].name);
				if (!bReloaded)
					bReloaded=_sheetmgr.ReloadRes(info[i].name);
				if (!bReloaded)
					bReloaded=_animtreemgr.ReloadRes(info[i].name);
				if (!bReloaded)
					bReloaded=_boneanimmgr.ReloadRes(info[i].name);
				if (!bReloaded)
					bReloaded=_mtrlextmgr.ReloadRes(info[i].name);
				if (!bReloaded)
					bReloaded=_soundmgr.ReloadRes(info[i].name);
				if (!bReloaded)
					bReloaded=_recordsmgr.ReloadRes(info[i].name);
				if (!bReloaded)
					bReloaded=_rgdmgr.ReloadRes(info[i].name);
				if (!bReloaded)
					bReloaded=_dtrmgr.ReloadRes(info[i].name);
				if (!bReloaded)
					bReloaded=_behaviorgraphmgr.ReloadRes(info[i].name);
				//XXXXX:more res type

				if (bReloaded)
					LOG_DUMP_1P("ResourceMonitor",Log_Notify,"由于文件\"%s\"发生了改变,它对应的资源被重新载入了!",info[i].name);
			}

			c=_watcherEffect.FetchChangedFiles((const ChangedFileInformation *&)info);

			for (int i=0;i<c;i++)
			{
				ChangedFileAction action=info[i].action;
				switch(action)
				{
				case FA_REMOVED:
				case FA_MODIFIED:
				case FA_RENAMED_OLD_NAME:
				case FA_RENAMED_NEW_NAME:
					break;
				default:
					continue;
				}
				if (_slmgr.ReloadShaderLib(info[i].name))
					LOG_DUMP_1P("ResourceMonitor",Log_Notify,"由于文件\"%s\"发生了改变,它对应的shader被重新载入了!",info[i].name);
			}

			c=_watcherSheet.FetchChangedFiles((const ChangedFileInformation *&)info);

			for (int i=0;i<c;i++)
			{
				ChangedFileAction action=info[i].action;
				switch(action)
				{
				case FA_REMOVED:
				case FA_MODIFIED:
				case FA_RENAMED_OLD_NAME:
				case FA_RENAMED_NEW_NAME:
					break;
				default:
					continue;
				}
				if (_sheetmgr.ReloadSheets(info[i].name))
					LOG_DUMP_1P("ResourceMonitor",Log_Notify,"由于文件\"%s\"发生了改变,它对应的sheets被重新载入了!",info[i].name);
			}

			c=_watcherRecords.FetchChangedFiles((const ChangedFileInformation *&)info);

			for (int i=0;i<c;i++)
			{
				ChangedFileAction action=info[i].action;
				switch(action)
				{
				case FA_REMOVED:
				case FA_MODIFIED:
				case FA_RENAMED_OLD_NAME:
				case FA_RENAMED_NEW_NAME:
					break;
				default:
					continue;
				}
				if (_recordsmgr.ReloadRes(info[i].name))
					LOG_DUMP_1P("ResourceMonitor",Log_Notify,"由于文件\"%s\"发生了改变,它对应的records被重新载入了!",info[i].name);
			}

			c=_watcherBehaviorGraph.FetchChangedFiles((const ChangedFileInformation *&)info);

			for (int i=0;i<c;i++)
			{
				ChangedFileAction action=info[i].action;
				switch(action)
				{
				case FA_REMOVED:
				case FA_MODIFIED:
				case FA_RENAMED_OLD_NAME:
				case FA_RENAMED_NEW_NAME:
					break;
				default:
					continue;
				}
				if (_behaviorgraphmgr.ReloadRes(info[i].name))
					LOG_DUMP_1P("ResourceMonitor",Log_Notify,"由于文件\"%s\"发生了改变,它对应的BehaviorGraph被重新载入了!",info[i].name);
			}

			_bProcessResChange=FALSE;
		}

	}

}


