/********************************************************************
	created:	2010/4/19   13:31
	file path:	d:\IxEngine\Interfaces\WorldSystem
	author:		chenxi
	
	purpose:	Anim Tree Ctrl interfaces
*********************************************************************/

#pragma once

#include "anim/animdefines.h"
#include "linkpad/LinkPad.h"

#include "ctrlqueue/CtrlQueue.h"
#include "bitset/bitset.h"

class CClass;
class IMatrice43;
class ISkeleton;
class IAnimNode;

struct ClothParam
{
    ClothParam()
    {
        mass = 0.2f;
    }
    StringID nmFabric;
    float mass;//in kg
};

class IClothCtrl
{
public:
	INTERFACE_REFCOUNT;
    virtual void Destroy() = 0;
    virtual ISkeleton *GetSkeleton() = 0;
    virtual BOOL IsValid() = 0;
    virtual void RegisterMesh(IMesh *mesh,ClothParam **params,DWORD nParams) = 0;
    virtual void UnRegisterMesh(IMesh *mesh) = 0;

    virtual BOOL FillMats(IMatrice43 *mats, AnimTick t, BYTE *affects, DWORD &nAffects) = 0;

    virtual void Tick(AnimTick t) = 0;

};
