/********************************************************************
	created:	2008/07/09
	created:	9:7:2008   17:41
	filename: 	d:\IxEngine\Proj_RenderSystem\ResDataThread.cpp
	file path:	d:\IxEngine\Proj_RenderSystem
	file base:	ResDataThread
	file ext:	cpp
	author:		cxi
	
	purpose:	读取资源文件的线程
*********************************************************************/

#include "stdh.h"
#include "ResDataThread.h"

#include "ResourceBase.h"

#include "TextureMgr.h"
#include "SheetMgr.h"

#include "resdata/ResData.h"


#include "timer/profiler.h"

//////////////////////////////////////////////////////////////////////////
//CResDataThread

BOOL CResDataThread::Init(IFileSystem *pFS,IRenderSystem *pRS)
{
	_pRS=pRS;
	_pFS=pFS;

	return TRUE;
}


void CResDataThread::DoOp(ResDataOp &op)
{
	CResource *p=op.res;

	if (!p->LoadData(_pFS,FALSE))
		p->SetState(CResource::Failed);
	else
		p->SetState(CResource::Loaded);
}


int CResDataThread::OnDo()
{
	ResDataOp op;
	while(1)
	{
		op=_queue.Pop();
		if (op.IsEmpty())
			break;

//		ProfilerStart(ResDataThread);
		DoOp(op);
//		ProfilerEnd();
	}
	return 0;
}
