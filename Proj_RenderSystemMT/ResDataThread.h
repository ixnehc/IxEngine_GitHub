
#pragma once

#include "concurrent/thread.h"
#include "concurrent/blockqueue.h"


class IRenderSystem;
class IUtilRS;
class IFileSystem;

class CResource;
struct ResData;
class CResDataQueue;


class CResource;
struct ResData;

struct ResDataOp
{
	ResDataOp()
	{
		res=NULL;
	}
	BOOL IsEmpty()	{		return (res==NULL);	}
	CResource *res;
};



class CResDataQueue:public CBlockQueue<ResDataOp>
{
public:
	void PostOp(ResDataOp &op)	{		PushBack(op);	}
};

class CDeviceQueue;
class CResDataThread:public CThread
{
public:
	BOOL Init(IFileSystem *pFS,IRenderSystem *pRS);
	virtual int OnDo();

	void DoOp(ResDataOp &op);

	CResDataQueue *GetQueue()	{		return &_queue;	}

protected:
	BOOL _LoadRes(const char *path,ResData *&data);
	CResDataQueue _queue;

	IRenderSystem *_pRS;
	IFileSystem *_pFS;

};
