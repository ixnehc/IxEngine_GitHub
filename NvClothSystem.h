#pragma once

#include <NvCloth/Callbacks.h>
#include <foundation/PxAllocatorCallback.h>
#include <foundation/PxErrorCallback.h>


class NvAllocator : public physx::PxAllocatorCallback
{
public:
    NvAllocator()
    {
        mEnableLeakDetection = false;
    }
    virtual void* allocate(size_t size, const char* typeName, const char* filename, int line)
    {
#ifdef _MSC_VER 
        void* ptr = _aligned_malloc(size, 16);
#else 
        void* ptr;
        if (posix_memalign(&ptr, 16, size)) ptr = 0;
#endif
        if (mEnableLeakDetection)
        {
            std::lock_guard<std::mutex> lock(mAllocationsMapLock);
            mAllocations[ptr] = Allocation(size, typeName, filename, line);
        }
        return ptr;
    }
    virtual void deallocate(void* ptr)
    {
        if (mEnableLeakDetection && ptr)
        {
            std::lock_guard<std::mutex> lock(mAllocationsMapLock);
            auto i = mAllocations.find(ptr);
            if (i == mAllocations.end())
            {
                printf("Tried to deallocate %p which was not allocated with this allocator callback.", ptr);
            }
            else
            {
                mAllocations.erase(i);
            }
        }
#ifdef _MSC_VER 
        _aligned_free(ptr);
#else 
        free(ptr);
#endif
    }

    void StartTrackingLeaks()
    {
        std::lock_guard<std::mutex> lock(mAllocationsMapLock);
        mAllocations.clear();
        mEnableLeakDetection = true;
    }

    void StopTrackingLeaksAndReport()
    {
        std::lock_guard<std::mutex> lock(mAllocationsMapLock);
        mEnableLeakDetection = false;

        size_t totalBytes = 0;
        std::stringstream message;
        message << "Memory leaks detected:\n";
        for (auto it = mAllocations.begin(); it != mAllocations.end(); ++it)
        {
            const Allocation& alloc = it->second;
            message << "* Allocated ptr " << it->first << " of " << alloc.mSize << "bytes (type=" << alloc.mTypeName << ") at " << alloc.mFileName << ":" << alloc.mLine << "\n";
            totalBytes += alloc.mSize;
        }
        if (mAllocations.size() > 0)
        {
            message << "=====Total of " << totalBytes << " bytes in " << mAllocations.size() << " allocations leaked=====";
            const std::string& tmp = message.str();
#ifdef _MSC_VER 
            //				OutputDebugString(tmp.c_str()); //Write to visual studio output so we can see it after the application closes
#endif
            printf("%s\n", tmp.c_str());
        }

        mAllocations.clear();
    }
private:
    bool mEnableLeakDetection;
    struct Allocation
    {
        Allocation() {}
        Allocation(size_t size, const char* typeName, const char* filename, int line)
            : mSize(size), mTypeName(typeName), mFileName(filename), mLine(line)
        {

        }
        size_t mSize;
        std::string mTypeName;
        std::string mFileName;
        int mLine;
    };
    std::map<void*, Allocation> mAllocations;
    std::mutex mAllocationsMapLock;
};

class NvErrorCallback : public physx::PxErrorCallback
{
public:
    NvErrorCallback() {}
    virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line);
};


class NvClothAssertHandler : public nv::cloth::PxAssertHandler
{
public:
    virtual void operator()(const char* exp, const char* file, int line, bool& ignore)
    {
        PX_UNUSED(ignore);
        printf("NV_CLOTH_ASSERT(%s) from file:%s:%d Failed\n", exp, file, line);
        assert(("Assertion failed, see log/console for more info.", 0));
    }
};



class CNvClothSystem
{
public:
    CNvClothSystem()
    {
        _allocator = NULL;
        _callbackError = NULL;
        _callbackAssert = NULL;

        _factory = NULL;
    }

    BOOL Init();
    void UnInit();

protected:
    NvAllocator * _allocator;
    NvErrorCallback* _callbackError;
    NvClothAssertHandler* _callbackAssert;

    nv::cloth::Factory* _factory;

};