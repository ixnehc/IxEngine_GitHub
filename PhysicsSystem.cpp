
/********************************************************************
	created:	3:9:2008   13:45
	filename: 	d:\IxEngine\Proj_PhysicsSystem\PhysicsSystem.cpp
	author:		chenxi
	
	purpose:	Physics System
*********************************************************************/
#include "stdh.h"
#include "interface/interface.h"

#include "PhysicsSystem.h"

#include "mathconvert.h"

#include <Common/Base/hkBase.h>
#include <Common/Base/System/hkBaseSystem.h>
#include <Common/Base/System/Error/hkDefaultError.h>
#include <Common/Base/Memory/MemoryClasses/hkMemoryClassDefinitions.h>

#include <Common/Base/Memory/System/hkMemorySystem.h>
#include <Common/Base/Memory/System/Util/hkMemoryInitUtil.h>

#include <Common/Base/Ext/hkBaseExt.h>


#include <Physics/Collide/Shape/Compound/Tree/Mopp/hkpMoppUtility.h>
#include <Physics/Internal/Collide/Mopp/Code/hkpMoppCode.h>
#include <Physics/Collide/Shape/Deprecated/Mesh/hkpMeshShape.h>

#include <Common/Base/KeyCode.h>
#define HK_FEATURE_REFLECTION_CLOTH_SETUP
#define HK_FEATURE_REFLECTION_CLOTH_SETUP_ANIMATION
#define HK_FEATURE_REFLECTION_COMMON_VISUALIZE
#define HK_CLASSES_FILE <Common/Serialize/Classlist/hkKeyCodeClasses.h>
// 
// #define HK_FEATURE_PRODUCT_PHYSICS
// #define HK_FEATURE_PRODUCT_ANIMATION
// #define HK_CLASSES_FILE <Common/Serialize/Classlist/hkClasses.h>  
// 
// #include <Common/Base/keycode.cxx>
#include <Common/Base/Config/hkProductFeatures.cxx>  


//   #define HK_COMPAT_FILE <Common/Compat/hkCompatVersions.h>
//   #include <Common/Compat/hkCompat.h>


#include <Common\Internal\ConvexHull\hkGeometryUtility.h>
#include <Common/Internal/GeometryProcessing/ConvexHull/hkgpConvexHull.h>

#include <NvCloth/Factory.h>

#include "World.h"

#include "timer/profiler.h"
#include "Log/LogDump.h"

#pragma warning(disable:4312)


EXPOSE_SINGLE_INTERFACE(CPhysicsSystem,IPhysicsSystem,"PhysicsSystem01")

CPhysicsSystem::CPhysicsSystem()
{
}

static void HK_CALL errorReportFunction(const char* str, void*)
{
//	printf("%s\n", str);
}

  
BOOL CPhysicsSystem::Init()
{


// 	hkMemoryRouter* memoryRouter = hkMemoryInitUtil::initDefault( hkMallocAllocator::m_defaultMallocAllocator, 
// 			hkMemorySystem::FrameInfo( 500* 1024 ) );
// 	hkResult result=hkBaseSystem::init( memoryRouter, errorReportFunction);
// 	hkpWorld *world;
// 	if (result==HK_SUCCESS)
// 	{
// 		hkpWorldCinfo info;
// 		info.setBroadPhaseWorldSize(100.0f); 
// 		info.setupSolverInfo( hkpWorldCinfo::SOLVER_TYPE_4ITERS_MEDIUM );
// 		world= new hkpWorld( info );
// 	}

	hkMemoryRouter* memoryRouter;
	hkMemorySystem::FrameInfo frameInfo(2048*1024);
	memoryRouter = hkMemoryInitUtil::initFreeListLargeBlock(&_mallocBase, frameInfo);

	extAllocator::initDefault();

	if (memoryRouter == HK_NULL)
	{
		return FALSE;
	}

	if ( hkBaseSystem::init( memoryRouter, errorReportFunction ) != HK_SUCCESS)
	{
		return FALSE;
	}
 
    nv::cloth::InitializeNvCloth();

	return TRUE;
}

BOOL CPhysicsSystem::UnInit()
{
	hkBaseSystem::quit();
	hkMemoryInitUtil::quit();
	extAllocator::quit();

	return TRUE;
}

ProfilerMgr *CPhysicsSystem::GetProfilerMgr()
{
	::GetProfilerMgr()->SetName("PhysicsSystem");
	return ::GetProfilerMgr();
}

void CPhysicsSystem::RegisterLogHandler(LogHandler &handler)
{
	extern void RegisterLogHandler(LogHandler& handler);
	::RegisterLogHandler(handler);
}



IPhysWorld*CPhysicsSystem::CreateWorld(PhysWorldConfig &cfg)
{
	CPhysWorld *p=Class_New(CPhysWorld);

	p->Init(cfg);
	return p;
}


BOOL CPhysicsSystem::BeginThread()
{ 
// 	hkThreadMemory *threadmem= new hkThreadMemory(&hkMemory::getInstance(),16);
// 	hkBaseSystem::initThread(threadmem);
// 
// 	if (TRUE)
// 	{
// 		int sz= 2*1024*1024; // 2MB stack
// 		char *stackbuf=hkAllocate<char>( sz, HK_MEMORY_CLASS_BASE);
// 		threadmem->setStackArea(stackbuf,sz);
// 		_threadstacks[threadmem]=stackbuf;
// 	}


	return TRUE;

}



BOOL CPhysicsSystem::EndThread()
{
// 	hkThreadMemory *threadmem=hkThreadMemory::getInstancePtr();
// 
// 	if (threadmem)
// 	{
// 		std::map<hkThreadMemory*,char *>::iterator it=_threadstacks.find(threadmem);
// 		if (it!=_threadstacks.end())
// 			hkDeallocate((*it).second);
// 		threadmem->removeReference();
// 	}
// 
// 	hkBaseSystem::clearThreadResources();

	return TRUE;
}

BOOL CPhysicsSystem::BuildMoppCode(MoppMesh&mm)
{
	hkpShape *shape=NULL;

	if (TRUE)
	{
		hkpMeshShape* mesh = new hkpMeshShape;
		mesh->setRadius( 0.05f );

		hkpMeshShape::Subpart part;

		part.m_vertexBase = (float*)mm.vertices;
		part.m_vertexStriding = sizeof(vector3df);
		part.m_numVertices = mm.nVertices;

		part.m_indexBase = mm.indices;
		part.m_indexStriding = sizeof( hkUint16)*3;
		part.m_numTriangles = mm.nIndice/3;
		part.m_stridingType = hkpMeshShape::INDICES_INT16;

		mesh->addSubpart( part );

		shape=mesh;
	}

	//build mopp
	if (TRUE)
	{
		hkpMeshShape* mesh=(hkpMeshShape*)shape;
		hkpMoppCompilerInput mci;
		hkpMoppCode* code = hkpMoppUtility::buildCode( mesh,mci);
		if (!code)
		{
			SAFE_HK_RELEASE(mesh);
			return FALSE;
		}

		mm.szCode=code->m_data.getSize();
		if (mm.code)
			memcpy(mm.code,&code->m_data[0],mm.szCode);

		mm.buildtype=code->m_buildType;
		memcpy(&mm.off,&code->m_info.m_offset,sizeof(mm.off));

		SAFE_HK_RELEASE(mesh);
		SAFE_HK_RELEASE(code);
	}

	return TRUE;
}

BOOL CPhysicsSystem::BuildConvexHull(ConvexHullParam &param,ConvexHull &result)
{
	hkStridedVertices input;
	hkGeometry output;

	input.m_numVertices=param.nVertices;
	input.m_vertices=(float*)param.vertices;
	input.m_striding=sizeof(i_math::vector3df);

	hkgpConvexHull hull;
	hull.build(input);
	hkgpConvexHull::SimplifyConfig cfg;
	cfg.m_maxVertices=16;
	hull.simplify(cfg);

	hull.generateGeometry(hkgpConvexHull::SOURCE_VERTICES,output);

// 	hkGeometryUtility::createConvexGeometry(input,output);

	static std::vector<i_math::vector3df> vertices;
	static std::vector<WORD> indices;

	vertices.resize(output.m_vertices.getSize());
	for (int i=0;i<output.m_vertices.getSize();i++)
	{
		vector3df_from_hkVector4(vertices[i],output.m_vertices[i]);
	}

	indices.reserve(output.m_triangles.getSize()*3);
	indices.clear();
	for (int i=0;i<output.m_triangles.getSize();i++)
	{
		indices.push_back((WORD)(output.m_triangles[i].m_a));
		indices.push_back((WORD)(output.m_triangles[i].m_b));
		indices.push_back((WORD)(output.m_triangles[i].m_c));
	}

	result.vertices=&vertices[0];
	result.nVertices=vertices.size();
	result.indices=&indices[0];
	result.nIndices=indices.size();

	return TRUE;
}
