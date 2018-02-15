#pragma once

#include "interface/interface.h"
#include "commondefines/general_stl.h"

#include "stringparser/stringparser.h"

#include "RenderSystem/IRenderSystem.h"

#include "FileSystem/IFileSystem.h"

#include "class/class.h"

#include "log/LogDump.h"
#include "Log/LogFile.h"

#ifndef MOBILE
#include "concurrent/mutexes.h"
#include "ResDataThread.h"
#endif

#include "resdata/ResData.h"


#include <vector>
#include <string>
#include <map>
#include <hash_map>


#define RESMGR_UNINIT(mgr,bLeak)\
(bLeak)|=(mgr).CheckResLeak();\
(mgr).UnInit()


#define RESOURCEMGR_CORE																								\
	virtual IRenderSystem *GetRS()	{		return CResourceMgr::GetRS();	}					\
	virtual void Update()	{		CResourceMgr::Update();	}												\
	virtual BOOL CheckResLeak()	{		return CResourceMgr::CheckResLeak();	}

#define Resource_Delete(pRes)											\
if (pRes)																					\
	(pRes->GetClass())->_del(pRes->_owner);

class CResource;
struct ResData;
class CDeviceQueue;
class CResDataQueue;
class CResourceMgr
{
public:
	CResourceMgr()
	{
		_t=0;
		_pRS=NULL;
	}

	virtual BOOL Init(IRenderSystem *pRS,const char *nameMgr);
	virtual void UnInit();
	virtual void GarbageCollect(DWORD step);

	const char *GetName()	{		return _name.c_str();	}

	//Interfaces
	IRenderSystem *GetRS()	{		return _pRS;	}
	void Update();
	virtual BOOL CheckResLeak();//Check whether there is any resource left not released in the mgr
	BOOL ReloadRes(const char *path)	{		return FALSE;	}



	template <typename T>
	T *NewRes(T *container=NULL)
	{
		T *ret;
		if (!container)
			ret=Class_New(T);
		else
		{
			ret=container;
			ret=new(ret)T;
		}
		ret->_pMgr=this;
		ret->_owner=ret;
		return ret;
	}

	void _ProcessOp(ResDataOp &op);

	//异步载入资源的所有数据
	template <typename T>
	T*_ObtainResA(const char *pathRes,T *container=NULL)
	{
		if (!pathRes[0])
			return NULL;
		std::string path=pathRes;
		StringLower(path);
		std::hash_map<std::string,CResource*>::iterator it=_mapRes.find(path);
		if (it!=_mapRes.end())
		{
			((*it).second)->AddRef();
			return static_cast<T*>((*it).second);
		}

		T *p=NewRes<T>(container);
		p->SetPath(pathRes);
		p->AddRef();

		ResDataOp op;
		op.res=static_cast<CResource*>(p);
		op.res->SetState(CResource::Loading);
		_ProcessOp(op);

		_mapRes[path]=static_cast<CResource*>(p);
		return p;
	}

	//同步的读取资源header数据,异步的读取主体数据(目前不支持贴图资源的载入)
	template <typename T>
	T*_ObtainResH(const char *pathRes,T*container=NULL)
	{
		if (!pathRes[0])
			return NULL;

		std::string path=pathRes;
		StringLower(path);
		std::hash_map<std::string,CResource*>::iterator it=_mapRes.find(path);
		if (it!=_mapRes.end())
		{
			((*it).second)->AddRef();
			return static_cast<T*>((*it).second);
		}

		T *p=NewRes<T>(container);
		p->SetPath(pathRes);
		p->AddRef();

		//同步的载入header数据
		if (TRUE)
		{
			BOOL bOk=FALSE;
			if (p->LoadData(_pRS->GetFS(),TRUE))
			{
				if (((CResource*)p)->_OnTouchHeader(_pRS))
					bOk=TRUE;
			}
			if (!bOk)
			{
				p->SetState(CResource::Failed);
				_mapRes[path]=static_cast<CResource*>(p);
				return p;
			}
		}

		ResDataOp op;
		op.res=static_cast<CResource*>(p);
		op.res->SetState(CResource::Loading);
		_ProcessOp(op);

		_mapRes[path]=static_cast<CResource*>(p);
		return p;
	}

	//同步的读取主体数据,(目前不支持贴图资源的载入)
	template <typename T>
	T*_ObtainResS(const char *pathRes,T*container=NULL)
	{
		if (!pathRes[0])
			return NULL;

		std::string path=pathRes;
		StringLower(path);
		std::hash_map<std::string,CResource*>::iterator it=_mapRes.find(path);
		if (it!=_mapRes.end())
		{
			((*it).second)->AddRef();
			return static_cast<T*>((*it).second);
		}

		T *p=NewRes<T>(container);
		p->SetPath(pathRes);
		p->AddRef();

		if (pathRes[0])
		{
			if(!p->LoadData(_pRS->GetFS(),FALSE))
				p->SetState(CResource::Failed);
			else
				p->SetState(CResource::Loaded);
		}

		p->Touch();

		_mapRes[path]=static_cast<CResource*>(p);
		return p;
	}



	template <typename T>
	T*_ObtainRes()
	{
		T *p=NewRes<T>();
		p->AddRef();

		_vecRes.push_back(static_cast<CResource*>(p));

		return p;
	}

	template<typename T>
	T *_UnloadRes(const char *pathRes,int &refcount,WORD &ver)
	{
		std::string path=pathRes;
		StringLower(path);
		std::hash_map<std::string,CResource*>::iterator it=_mapRes.find(path);

		if (it==_mapRes.end())
			return NULL;

		CResource*p=((*it).second);
		refcount=p->_nRef;
		ver=p->_ver;
		p->_OnUnload();
		(static_cast<T*>(p))->T::~T();

		_mapRes.erase(it);
		_itLast=_mapRes.end();
		return (static_cast<T*>(p));
	}



	template <typename T>
	BOOL _ReloadRes(const char *pathRes,char c)
	{
		int refcount;
		WORD ver;
		T*p=_UnloadRes<T>(pathRes,refcount,ver);
		if (!p)
			return FALSE;
		switch(c)
		{
			case 'A':
				_ObtainResA(pathRes,p);
				break;
			case 'S':
				_ObtainResS(pathRes,p);
				break;
			case 'H':
				_ObtainResH(pathRes,p);
				break;
		}
		p->ForceTouch();

		//恢复引用计数和版本号
		(static_cast<CResource*>(p))->_nRef=refcount;
		(static_cast<CResource*>(p))->_ver=ver+1;//每次重新载入后,版本号都+1
		return TRUE;
	}


protected:
	virtual void _UnloadAll();

	virtual BOOL _CanGC(CResource *res)	{		return TRUE;	}

	template<typename T>
	void _gc_vec(std::vector<T*>&buf,DWORD step,DWORD &idx)
	{
		DWORD c=buf.size();
		while(step>0)
		{
			step--;

			if (idx>=c)
			{
				idx=0;
				buf.resize(c);
				return;
			}

			CResource *p=static_cast<CResource*>(buf[idx]);
			if ((p->GetRef()<=0)&&(p->_state!=CResource::Loading))
			{
				p->_OnUnload();
				Resource_Delete(p);
				buf[idx]=buf[c-1];
				c--;
				continue;
			}
			idx++;
		}
		buf.resize(c);
	}

	IRenderSystem *_pRS;
	std::string _name;
	DWORD _t;//time

	//用于同步载入
	std::string _pathResRoot;
	std::string _pathWorking;

#ifndef MOBILE
	CResDataThread *_thrdResData;
#endif

	std::hash_map<std::string,CResource*> _mapRes;//有名字的资源
	std::vector<CResource*>_vecRes;//没有名字的资源

	//for garbage collect
	std::hash_map<std::string,CResource*>::iterator _itLast;
	DWORD _iLast;



friend class CResourceMgr;
friend class CRenderSystem;

};


#define RESOURCE_CORE()																					\
virtual IRenderSystem *GetRS(){	return CResource::GetRS();}							\
virtual int AddRef(){	return CResource::AddRef();}												\
virtual int Release(){	return CResource::Release();}											\
virtual const char *GetPath(){	return CResource::GetPath();}							\
virtual AResult Touch(){	return CResource::Touch();}											\
virtual BOOL ForceTouch(){	return CResource::ForceTouch();}							\
virtual DWORD GetVer(){	return CResource::GetVer();}							


class CResource
{
public:
	enum ResState
	{
		Empty=0,
		Loading,
		Loaded,
		Failed,
		Touched,
		Abandoned,
	};

	CResource();
	virtual CClass *GetClass()=0;

	//interfaces
	IRenderSystem *GetRS()	{		return _pMgr->GetRS();	}
	int AddRef()	{		_nRef++;		_cGC=0;return _nRef;	}
	int Release()	{		_nRef--;return _nRef;	}
	const char *GetPath()	{		return _path.c_str();	}
	inline AResult Touch()
	{
		AResult ret=A_Fail;
		switch(_state)
		{
			case Touched:
				return A_Ok;
			case Abandoned:
				return A_Fail;
			case Loading:
			{
				if (!_bSyncTouch)
					return A_Pending;
				while(_state==Loading)
					Sleep(0);
				if(_state==Failed)
					break;
				if (!_OnTouch(_pMgr->GetRS()))
					_state=Failed;
				else
				{
					_state=Touched;
					ret=A_Ok;
				}
				break;
			}
			case Failed:
				break;
			case Loaded:
				if (!_OnTouch(_pMgr->GetRS()))
					_state=Failed;
				else
				{
					_state=Touched;
					ret=A_Ok;
				}
				break;
			default:
				assert(FALSE);
				return A_Fail;
		}

		if (_state==Failed)
		{
			extern LogFile g_logResMgr;
			if (_path!="")
			{
				LOG_DUMP_1P(_pMgr->GetName(),Log_Error,"载入资源失败(\"%s\")!",_path.c_str());

				g_logResMgr.Dump("载入资源失败(\"%s\")!",_path.c_str());
			}
			else
			{
				LOG_DUMP(_pMgr->GetName(),Log_Error,"载入资源失败!");
				g_logResMgr.Dump("载入资源失败!");
			}
			_state=Abandoned;
		}

		VEC_EMPTY(BYTE,_data);
		_typeData=Res_None;

		return ret;

	}
	inline BOOL ForceTouch()
	{
		while(_state==CResource::Loading)
			Sleep(0);
		return Touch()==A_Ok;
	}

	int GetRef()	{		return _nRef;	}

	DWORD GetVer()	{		return _ver;	}

	void SetPath(const char *path)	{		_path=path;	}
	void Use(DWORD t)	{		_tUsed=t;	}
	ResState GetState();
	void SetState(ResState s)	{		_state=s;	}

	CResourceMgr *GetMgr()	{		return _pMgr;	}

	BOOL LoadData(IFileSystem *pFS,BOOL bHeader);



protected:
	virtual BOOL _OnTouch(IRenderSystem *pRS)	{		return FALSE;	}
	virtual BOOL _OnTouchHeader(IRenderSystem *pRS)	{		return FALSE;	}
	virtual void _OnUnload()	{	}

	CResourceMgr *_pMgr;
	void *_owner;
	int _nRef;
	std::string _path;
	tbb::atomic<int> _state;
	DWORD _tUsed;//最近一次使用这个资源的时间

	ResType _typeData;
	int _textype;//为TexData::TexDataType的类型,只在_typeData为Res_Texture时有效
	std::vector<BYTE>_data;

	WORD _cGC;//用于garbage collect的计数器
	WORD _ver;//这个资源每次重新载入,都会增加一个版本号

	static BOOL _bSyncTouch;
	static BOOL _bDisableMT;

	friend class CRenderSystem;
	friend class CRenderSystemGL;
	friend class CResourceMgr;
	friend class CTextureMgr;
	friend class CWTextureMgr;
	friend class CSurfaceMgr;
	friend class CRTextureMgr;
	friend class CDynSptMgr;
	friend class CDynMtrlMgr;
	friend class CResDataThread;
};

// #define SAFE_RELEASE_D3DRES(p)													\
// if (p)\
// {\
// 	if ((p)->Release()>0)\
// 	{\
// 		MessageBox(NULL,"D3D Resource still has ref count when released!","Error",MB_OK);\
// 	}\
// 	p=NULL;\
// }

#define SAFE_RELEASE_D3DRES(p)		SAFE_RELEASE(p)

#define VALIDATE_RES_TYPE(type)																\
if ((type)!=_typeData)																						\
{																						\
	ResData *data=ResData_New(type);																						\
	LOG_DUMP_2P("ResourceMgr",Log_Error,"资源类型不匹配(资源\"%s\"不是[%s]类型)!",_path.c_str(),data->GetTypeName());																						\
	ResData_Delete(data);																					\
	return FALSE;																						\
}
