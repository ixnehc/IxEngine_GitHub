
#include "stdh.h"


#include <NvCloth/Factory.h>

#include "timer/profiler.h"
#include "Log/LogDump.h"

#pragma warning(disable:4312)



BOOL CNvClothSystem::Init()
{
    _allocator = new NvAllocator;
    _allocator->StartTrackingLeaks();
    _callbackError = new NvErrorCallback;
    _callbackAssert = new NvClothAssertHandler;

    nv::cloth::InitializeNvCloth(_allocator, _callbackError, _callbackAssert, nullptr);

    _factory = NvClothCreateFactoryCPU();


    return TRUE;
}

void CNvClothSystem::UnInit()
{
    NvClothDestroyFactory(_factory);


    _allocator->StopTrackingLeaksAndReport();
    delete _callbackError;
    delete _allocator;
    delete _callbackAssert;

}

