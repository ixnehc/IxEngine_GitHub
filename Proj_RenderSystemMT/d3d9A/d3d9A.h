#pragma once
#pragma warning(disable:4244)

#include "class/class.h"
#include "mempool/mempool.h"
#include "queuing_rw_mutex.h"



#define DEFINE_D3D9A_CLASS(clss)																	\
public:																														\
	typedef CMemPool<clss,tbb::queuing_rw_mutex,tbb::queuing_rw_mutex::scoped_lock> MemPoolClass;														\
	class CClass_##clss:public CClass																		\
	{																															\
	public:																													\
	MemPoolClass &_pool()	{		static MemPoolClass pool("MemPoolPool_"#clss);		return pool;	}									\
	virtual void *_new()		{			return _pool().Alloc();		}								\
	virtual void _del(void *p)		{	_pool().Free((clss*)p);		}								\
	virtual void _delall()		{			_pool().FreeAll();		}										\
	virtual void **_getinstances(DWORD &c)		{c=0;return NULL;	}				\
	};																															\
	static CClass_##clss *_instantiate()																	\
	{																															\
		static CClass_##clss instance;																		\
		instance.SetName(#clss);																				\
		return &instance;																							\
	}																															\
	static CClass *_class()																							\
	{																															\
		static CClass *clss=_instantiate();																	\
		return clss;																										\
	}																															\
	virtual CClass *GetClass()																					\
	{																															\
		return _class();																								\
	}



#define D3D9A_New(clss) Class_New2(clss)
#define D3D9A_Delete(p) Class_Delete(p)

#pragma warning(disable:4311)

#define D3D9A_ADDREF()												\
STDMETHOD_(ULONG,AddRef)(THIS)	{		return ++_nRef;	}

#define D3D9A_RELEASE(ft)											\
STDMETHOD_(ULONG,Release)(THIS)							\
{																						\
	ULONG ret=--_nRef;													\
	if (_nRef<=0)																\
	{																					\
		DQ_PushFuncType(ft);											\
		DQ_Push4C(this);													\
	}																					\
	return ret;																	\
}

