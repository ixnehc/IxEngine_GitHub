#pragma once

#include "class/class.h"

#include "WorldSystem/IClothCtrl.h"

#include "WorldSystem/IAnimNodes.h"

#include <hash_set>

class CCloth
{
    DEFINE_CLASS(CCloth);
    CCloth()
    {
        Zero();
    }
    void Zero()
    {
        _mesh = NULL;
        _clothPhys = NULL;
        _dataFabric = NULL;

        memset(_t, 0, sizeof(_t));

        _flip = -1;
        _bReady = FALSE;
    }

    void Clear();

    void CollectXfms();//Collect xfms(of the current flip) of the affecting bones from the physics simulate
    BOOL FillMats(IMatrice43 *matrices,AnimTick t,i_math::matrix43f &matBase, std::hash_set<BYTE> &affects);

    void Step(AnimTick t);


protected:
    IMesh *_mesh;
    MeshData::FabricData *_dataFabric;
    IPhysCloth *_clothPhys;

    std::vector<i_math::xformf> _xfmBones[2];
    AnimTick _t[2];
    int _flip;
    BOOL _bReady;//is the xfms of current flip ready?

    friend class CClothCtrl;

};


class CClothCtrl :public IClothCtrl
{
public:
    IMPLEMENT_REFCOUNT_C
    DEFINE_CLASS(CClothCtrl);
    CClothCtrl()
    {
        Zero();
    }
    ~CClothCtrl()
    {
        Clear();
    }
    BOOL Init(AssetSystemState *ss,IAnimNodeSkin *anSkin);
    void Zero()
    {
        _ss = NULL;
        _ctrlAnimTree = NULL;
        _clothes = NULL;
        _t = 0;
    }
    void Clear();

    //Interfaces
    virtual void Destroy()    {        Clear();    }
    virtual ISkeleton *GetSkeleton();
    virtual BOOL IsValid() { return _ctrlAnimTree != NULL; }
    virtual void RegisterMesh(IMesh *mesh,ClothParam **params, DWORD nParams);
    virtual void UnRegisterMesh(IMesh *mesh);

    virtual void Tick(AnimTick t);

    virtual BOOL FillMats(IMatrice43 *mats,AnimTick t,BYTE *affects,DWORD &nAffects) override;

protected:
    BOOL _GetMats(i_math::matrix43f *&mats, i_math::matrix43f &matBaseInv,AnimTick t);

   AssetSystemState *_ss;
    IAnimNodeSkin *_anSkin;
    IAnimTreeCtrl *_ctrlAnimTree;

    std::deque<CCloth*> _bufClothes;
    IPhysClothes *_clothes;

    AnimTick _t;

};