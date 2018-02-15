#pragma once

#include <string>
#include <vector>

class IFileSystem;
class IFile;

struct ResData;


BOOL LoadResData(IFileSystem *pFS,const char *path,ResData *&pResData,BOOL bHeader);
BOOL SaveResData(IFileSystem *pFS,const char *path,ResData *pResData);
