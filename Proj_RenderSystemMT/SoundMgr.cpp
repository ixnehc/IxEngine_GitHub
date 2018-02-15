/********************************************************************
	created:	2008/07/15   14:17
	filename: 	d:\IxEngine\Proj_RenderSystem\TextureMgr_a.cpp
	author:		cxi
	
	purpose:	异步的texture manager
*********************************************************************/

#include "stdh.h"

#include "RenderSystem.h"

#include "SoundMgr.h"

#include "irrKlang.h"
#include <assert.h>

#include <bitset>

using namespace irrklang;
#pragma comment(lib, "irrKlang.lib") // link with irrKlang.dll


//////////////////////////////////////////////////////////////////////////
//CSoundPlay
CSoundPlay::~CSoundPlay()
{
	irrklang::ISound *sound=(irrklang::ISound *)_sound;
	if (sound)
		sound->drop();
	sound=NULL;
}

//////////////////////////////////////////////////////////////////////////
//CMyFileFactory
// a class implementing the IFileFactory interface to override irrklang file access
class CMyFileFactory : public IFileFactory
{
public:
	CMyFileFactory(IRenderSystem *pRS,IFileSystem *pFS)
	{
		_pRS=pRS;
		_pFS=pFS;
	}

	//! Opens a file for read access. Simply return 0 if file not found.
	virtual IFileReader* createFileReader(const ik_c8* filename)
	{
		std::string path;
		path=_pRS->GetPath(Path_Res);
		path=path+"\\"+filename;
		IFile *fl=_pFS->OpenFileAbs(path.c_str(),FileAccessMode_Read);
		if (!fl)
			return 0;

		return new CMyReadFile(fl, filename);
	}

protected:

	IRenderSystem *_pRS;
	IFileSystem *_pFS;


	// an own implementation of IReadFile to overwrite read access to files 
	class CMyReadFile : public IFileReader
	{
	public:

		// constructor, store size of file and filename
		CMyReadFile(IFile *fl, const ik_c8* filename)
		{
			_fl= fl;
			_name=filename;
		}

		~CMyReadFile()
		{
			_fl->Close();
			_fl=NULL;
		}

		//! reads data, returns how much was read
		ik_s32 read(void* buffer, ik_u32 sizeToRead)
		{
			_fl->Read(buffer,sizeToRead);
			return sizeToRead;
		}

		//! changes position in file, returns true if successful
		bool seek(ik_s32 finalPos, bool relativeMovement)
		{
			if (!relativeMovement)
				_fl->Seek(finalPos);
			else
				_fl->Seek((DWORD)(((int)_fl->GetCurPos())+finalPos));
			return true;
		}

		//! returns size of file
		ik_s32 getSize()
		{
			return _fl->GetSize();
		}

		//! returns where in the file we are.
		ik_s32 getPos()
		{
			return _fl->GetCurPos();
		}

		//! returns name of file
		const ik_c8* getFileName()
		{
			return _name.c_str();
		}

		IFile *_fl;
		std::string _name;

	}; // end class CMyReadFile

}; // end class CMyFileFactory



//////////////////////////////////////////////////////////////////////////
//CSound
IMPLEMENT_CLASS(CSound);
CSound::CSound()
{
	_src=NULL;
}



BOOL CSound::_OnTouch(IRenderSystem *pRS)
{
	CSoundMgr *mgr=static_cast<CSoundMgr*>(GetMgr());
	ISoundEngine *engine=(ISoundEngine *)mgr->GetEngine();
	if (!engine)
		return FALSE;

	_buf.swap(_data);

	if (_buf.size()>0)
	{
		ISoundSource *src=engine->addSoundSourceFromMemory(&_buf[0],_buf.size(),GetPath(),false);
		if (!src)
			return FALSE;

		DWORD len=src->getPlayLength();

		_src=src;
	}
	else
	{//Stream 方式载入
		ISoundSource *src=engine->addSoundSourceFromFile(GetPath(),ESM_STREAMING,false);
		if (!src)
			return FALSE;
		_src=src;
	}

	return TRUE;
}

void CSound::_OnUnload()
{
	CSoundMgr *mgr=static_cast<CSoundMgr*>(GetMgr());
	ISoundEngine *engine=(ISoundEngine *)mgr->GetEngine();
	if (engine)
	{
		if (_src)
			engine->removeSoundSource((ISoundSource *)_src);
	}
	_src=NULL;
	_buf.clear();
}

ISoundPlay *CSound::Play2D(BOOL bLoop)
{
	if (Touch()!=A_Ok)
		return NULL;
	if (!_src)
		return NULL;

	CSoundMgr *mgr=static_cast<CSoundMgr*>(GetMgr());
	ISoundEngine *engine=(ISoundEngine *)mgr->GetEngine();
	if (!engine)
		return NULL;

	irrklang::ISound *sound=engine->play2D((ISoundSource *)_src,bLoop?true:false,false,true);
	if (!sound)
		return NULL;

	CSoundPlay *play=Class_New2(CSoundPlay);
	play->_sound=sound;
	play->AddRef();

	return play;
}

ISoundPlay *CSound::Play3D(i_math::vector3df &pos,BOOL bLoop)
{
	if (Touch()!=A_Ok)
		return NULL;
	if (!_src)
		return NULL;

	CSoundMgr *mgr=static_cast<CSoundMgr*>(GetMgr());
	ISoundEngine *engine=(ISoundEngine *)mgr->GetEngine();
	if (!engine)
		return NULL;

	irrklang::ISound *sound=engine->play3D((ISoundSource *)_src,*(vec3df*)&pos,bLoop?true:false,false,true);
	if (!sound)
		return NULL;

	CSoundPlay *play=Class_New2(CSoundPlay);
	play->_sound=sound;
	play->AddRef();

	return play;
}





//////////////////////////////////////////////////////////////////////////
//CSoundMgr

CSoundMgr::CSoundMgr()
{
	_pFS=NULL;
	_ikengine=NULL;
}


BOOL CSoundMgr::Init(IRenderSystem *pRS,IFileSystem *pFS,const char *name)
{
	CResourceMgr::Init(pRS,name);

	_ikengine=createIrrKlangDevice();

	CMyFileFactory* factory = new CMyFileFactory(pRS,pFS);
	((ISoundEngine *)_ikengine)->addFileFactory(factory);
	factory->drop(); // we don't need it anymore, delete it

	return TRUE;
}

void CSoundMgr::UnInit()
{
	if (_ikengine)
		((ISoundEngine*)_ikengine)->drop();
	_ikengine=NULL;

	CResourceMgr::UnInit();
}



IResource *CSoundMgr::ObtainRes(const char *pathRes)
{
	return CResourceMgr::_ObtainResA<CSound>(pathRes);
}

BOOL CSoundMgr::ReloadRes(const char *pathRes)
{
	return CResourceMgr::_ReloadRes<CSound>(pathRes,'A');
}


BOOL CSoundMgr::_CanGC(CResource *p)
{
	CSound*sound=static_cast<CSound*>(p);
	if (_ikengine)
	{
		if (sound->_src)
		{
			if (((ISoundEngine *)_ikengine)->isCurrentlyPlaying((ISoundSource *)sound->_src))
				return FALSE;//这个音效正在播放,暂时不能回收
		}
	}
	return TRUE;
		

}

