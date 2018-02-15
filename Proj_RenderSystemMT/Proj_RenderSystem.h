// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the PROJ_RENDERSYSTEM_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// PROJ_RENDERSYSTEM_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef PROJ_RENDERSYSTEM_EXPORTS
#define PROJ_RENDERSYSTEM_API __declspec(dllexport)
#else
#define PROJ_RENDERSYSTEM_API __declspec(dllimport)
#endif

// This class is exported from the Proj_RenderSystem.dll
class PROJ_RENDERSYSTEM_API CProj_RenderSystem {
public:
	CProj_RenderSystem(void)
	{

	}
	// TODO: add your methods here.
};

extern PROJ_RENDERSYSTEM_API int nProj_RenderSystem;

PROJ_RENDERSYSTEM_API int fnProj_RenderSystem(void);
