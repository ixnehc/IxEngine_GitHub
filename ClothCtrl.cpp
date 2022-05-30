/********************************************************************
	created:	2019/12/17
	author:		chenxi
	
*********************************************************************/

#include "stdh.h"

#include "commondefines/general_stl.h"


#include "WorldSystem/IAssetSystemDefines.h"

#include "strlib/strlib.h"

#include "animnode.h"


#include "SklLinks.h"


#include "timer/profiler.h"

#include "Log/LogDump.h"

//////////////////////////////////////////////////////////////////////////
//CCloth
void CCloth::Clear()
{
    SAFE_RELEASE(_mesh);
    SAFE_RELEASE(_clothPhys);

    _xfmBones[0].clear();
    _xfmBones[1].clear();

    Zero();
}

void CCloth::Step(AnimTick t)
{
    if (_flip >= 0)
    {
        _flip = 1 - _flip;
        _t[_flip] = t;
    }
    else
        _t[0] = _t[1] = t;

    _bReady = FALSE;
}

void CCloth::CollectXfms()
{
    if (_bReady)
        return;

    int flip = _flip;
    if (flip < 0)
        flip = 0;

    AnimTick t = _t[flip];

    _xfmBones[flip].resize(_dataFabric->bones.size());

    _clothPhys->BeginGetParticles();
    for (int i = 0;i < _dataFabric->bones.size();i++)
    {
        MeshData::FabricData::BoneLink &bone = _dataFabric->bones[i];

        i_math::vector3df center, front, back, left, right;
        _clothPhys->GetParticlePos(bone.center, center);
        _clothPhys->GetParticlePos(bone.front, front);
        _clothPhys->GetParticlePos(bone.back, back);
        _clothPhys->GetParticlePos(bone.left, left);
        _clothPhys->GetParticlePos(bone.right, right);

        i_math::vector3df zAxis = front - back;
        i_math::vector3df xAxis = right - left;

        zAxis.normalize();
        xAxis.normalize();

        i_math::vector3df yAxis = zAxis.crossProduct(xAxis);
        yAxis.normalize();
        xAxis = yAxis.crossProduct(zAxis);
        xAxis.normalize();

        i_math::matrix43f mat;
        mat.buildMatrixLH(xAxis, yAxis, zAxis, center);

        _xfmBones[flip][i].fromMatrix(mat);
    }
    _clothPhys->EndGetParticles();

    if (_flip < 0)
        _xfmBones[1 - flip] = _xfmBones[flip];

    _flip = flip;

    _bReady = TRUE;
}

BOOL CCloth::FillMats(IMatrice43 *matrices, AnimTick t, i_math::matrix43f &matBase, std::hash_set<BYTE> &affects)
{
    if (!_bReady)
        return FALSE;

    DWORD nMats = matrices->GetCount();
    i_math::matrix43f *mats = matrices->QueryPtr();

    float r = 1.0f;
    if (TRUE)
    {
        AnimTick t1 = ANIMTICK_SAFE_MINUS(t, _t[1 - _flip]);
        AnimTick t2 = ANIMTICK_SAFE_MINUS(_t[_flip], _t[1 - _flip]);
        if (t2 > 0)
            r = i_math::clamp_f(((float)t1) / (float)t2);
    }

    i_math::xformf xfm;
    for (int i = 0;i < _dataFabric->bones.size();i++)
    {
        MeshData::FabricData::BoneLink &bone = _dataFabric->bones[i];

        if (bone.boneindex < nMats)
        {
            xfm=_xfmBones[_flip][i].getInterpolated(_xfmBones[1 - _flip][i], r);
            xfm.getMatrix(mats[bone.boneindex]);

            mats[bone.boneindex] = mats[bone.boneindex] * matBase;
            affects.insert(bone.boneindex);
        }
    }

    return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//CClothCtrl
BOOL CClothCtrl::Init(AssetSystemState *ss, IAnimNodeSkin *anSkin)
{
    _ss = ss;
    _ctrlAnimTree = anSkin->GetAnimTreeCtrl();
    SAFE_ADDREF(_ctrlAnimTree);

    _anSkin = anSkin;
    SAFE_ADDREF(_anSkin);

    _clothes = _ss->pPS->CreateClothes();
    if (_clothes)
        _ss->worldPhys->AddClothes(_clothes);

    return TRUE;
}

void CClothCtrl::Clear()
{
    for (int i = 0;i < _bufClothes.size();i++)
    {
        if (_bufClothes[i])
            _bufClothes[i]->Clear();
        Safe_Class_Delete(_bufClothes[i]);
    }
    _bufClothes.clear();

    if (_clothes)
    {
        _ss->worldPhys->RemoveClothes(_clothes);
        SAFE_RELEASE(_clothes);
    }

    SAFE_RELEASE(_ctrlAnimTree);
    SAFE_RELEASE(_anSkin);

    Zero();
}

ISkeleton *CClothCtrl::GetSkeleton()
{
    if (!_ctrlAnimTree)
        return NULL;

    return _ctrlAnimTree->GetSkeleton();
}

static Convert(MeshData::FabricData::Cooked &data,MeshData::FabricData::Cooked::Range &from, PhysFabricData::Range &to)
{
    to.begin = data.buf[from.begin];
    to.end = data.buf[from.end];
}

static Convert(MeshData::FabricData::Cooked &from, PhysFabricData &to)
{
    to.NumParticles = from.NumParticles;
    Convert(from.buf, from.PhaseIndices, to.PhaseIndices);
    Convert(from.buf, from.PhaseTypes, to.PhaseTypes);
    Convert(from.buf, from.Sets, to.Sets);
    Convert(from.buf, from.Restvalues, to.Restvalues);
    Convert(from.buf, from.StiffnessValues, to.StiffnessValues);
    Convert(from.buf, from.Indices, to.Indices);
    Convert(from.buf, from.Anchors, to.Anchors);
    Convert(from.buf, from.TetherLengths, to.TetherLengths);
    Convert(from.buf, from.Triangles, to.Triangles);
}

static i_math::vector3df CalcSkinPos(i_math::matrix43f *mats, i_math::matrix43f &matBaseInv, MeshData::FabricData::Vtx &vtx,int nWeights)
{
    i_math::vector3df posSum;
    i_math::vector3df pos;
    float wtAccum = 0.0f;

    if (nWeights > 1)
    {
        for (int j = 0;j < nWeights - 1;j++)
        {
            float wt = ((float*)&vtx.weight)[j];
            wtAccum += wt;

            DWORD idxBone = vtx.boneindex[j];

            if (wt > 0.0f)
            {
                mats[idxBone].transformVect(vtx.pos, pos);
                posSum += pos * wt;
            }
        }
    }
    //Last bone
    if (TRUE)
    {
        float wt = 1.0f - wtAccum;
        DWORD idxBone = vtx.boneindex[dataFabric->nWeights - 1];

        if (wt > 0.0f)
        {
            mats[idxBone].transformVect(vtx.pos, pos);
            posSum += pos * wt;
        }
    }

    matBaseInv.transformVect(posSum, posSum);

    return posSum;
}

BOOL CClothCtrl::_GetMats(i_math::matrix43f *&mats, i_math::matrix43f &matBaseInv,AnimTick t)
{
    if (!_anSkin)
        return FALSE;
    IMatrice43 *matrices = _anSkin->GetSkinMats(t);
    if (!matrices)
        return FALSE;
    mats = matrices->GetPtr();
    i_math::matrix43f *mat = _anSkin->GetMat(t);
    if (!mat)
        return FALSE;
    matBaseInv = *mat;
    matInv.makeInverse();

    return TRUE;
}


void CClothCtrl::RegisterMesh(IMesh *mesh, ClothParam **params, DWORD nParams)
{
    i_math::matrix43f *mats;
    i_math::matrix43f matInv;
    if (FALSE == _GetMats(mats, matInv,_ss->t))
        return;

    ClothParam *paramDef = NULL;
    for (int i = 0;i < nParams;i++)
    {
        if (params[i])
        {
            if (params[i]->nmFabric == StringID_Invalid)
            {
                paramDef = params[i];
                break;
            }
        }
    }

    DWORD nFabrics = mesh->GetFabricCount();
    for (int i = 0;i < nFabrics;i++)
    {
        MeshData::FabricData *dataFabric = mesh->GetFabric(i);
        if (dataFabric)
        {
            ClothParam *param = paramDef;
            if (!dataFabric->nm.empty())
            {
                for (int i = 0;i < nParams;i++)
                {
                    if (params[i])
                    {
                        if (params[i]->nmFabric != StringID_Invalid)
                        {
                            if (dataFabric->nm == StrLib_GetStr(params[i]->nmFabric))
                            {
                                param = params[i];
                                break;
                            }
                        }
                    }
                }
            }

            if (param)
            {
                PhysFabricData data;
                Convert(dataFabric->cooked, data);

                IPhysFabrics *fabrics = _ss->pPS->GetFabrics();
                if (fabrics)
                {
                    IPhysFabric *fabric = fabrics->ObtainFabric(dataFabric, data);
                    if (fabric)
                    {
                        PhysClothParam paramCloth;
                        static std::vector<i_math::vector4df> bufPos;
                        bufPos.resize(dataFabric->vertices.size());
                        paramCloth.pos = &bufPos[0];

                        //Calculate the cloth particles' position
                        if (TRUE)
                        {
                            i_math::vector3df pos;

                            for (int i = 0;i < dataFabric->vertices.size();i++)
                            {
                                MeshData::FabricData::Vtx &vtx = dataFabric->vertices[i];

                                pos = CalcSkinPos(mats, matInv, vtx, dataFabric->nWeights);
                                bufPos[i].setXYZ(pos);
                            }
                        }

                        //Calculate the mass of each particle
                        if (TRUE)
                        {
                            float massParticle = param->mass / (float)dataFabric->vertices.size();
                            float massInvParticle = 1.0f / massParticle;
                            for (int i = 0;i < dataFabric->vertices.size();i++)
                                bufPos[i].w = massInvParticle;

                            for (int i=0;dataFabric->borders.size();i++)
                                bufPos[dataFabric->borders[i].iVtx].w = 0.0f;
                        }

                        //
                        IPhysCloth *clothPhys = fabric->CreateCloth(paramCloth);
                        if (cloth)
                        {
                            _bufClothes.resize(_bufClothes.size() + 1);

                            CCloth *cloth = Class_New2(CCloth);
                            _bufClothes.push_back(cloth);

                            cloth->_clothPhys = clothPhys;
                            cloth->_mesh = mesh;
                            SAFE_ADDREF(cloth->_mesh);
                            cloth->_dataFabric = dataFabric;

                            _clothes->AddCloth(cloth->_clothPhys);
                        }
                    }

                    SAFE_RELEASE(fabric);
                }
            }
        }
    }
}

void CClothCtrl::UnRegisterMesh(IMesh *mesh)
{
    for (int i = 0;i < _bufClothes.size();i++)
    {
        CCloth *&cloth = _bufClothes[i];
        if (cloth)
        {
            if (cloth->_mesh == mesh)
            {
                cloth->Clear();
                Safe_Class_Delete(cloth);
            }
        }
    }

    DWORD c = 0;
    for (int i = 0;i < _bufClothes.size();i++)
    {
        if (_bufClothes[i])
            _bufClothes[c++] = _bufClothes[i];
    }
    _bufClothes.resize(c);
}

void CClothCtrl::Tick(AnimTick t)
{
    i_math::matrix43f *mats;
    i_math::matrix43f matInv;
    if (FALSE == _GetMats(mats, matInv,t))
        return;

    i_math::vector3df pos;
    for (int i = 0;i < _bufClothes.size();i++)
    {
        CCloth *cloth = _bufClothes[i];
        if (!cloth)
            continue;

        MeshData::FabricData *dataFabric = cloth->_dataFabric;
        if (dataFabric)
        {
            cloth->clothPhys->BeginUpdateParticles();
            for (int i = 0;dataFabric->borders.size();i++)
            {
                DWORD iVtx = dataFabric->borders[i].iVtx;
                MeshData::FabricData::Vtx &vtx = dataFabric->vertices[iVtx];
                pos = CalcSkinPos(mats, matInv, vtx, dataFabric->nWeights);

                cloth->clothPhys->UpdateParticle_Pos(iVtx, pos);
            }
            cloth->clothPhys->EndUpdateParticles();
        }

        cloth->Step(t);
    }

    _t = t;
}

BOOL CClothCtrl::FillMats(IMatrice43 *matrices, AnimTick t,i_math::matrix43f &matBase, BYTE *affects, DWORD &nAffects)
{
    if (_t < t)
        return FALSE;

    nAffects = 0;

    std::hash_set<BYTE> affects2;
    for (int i = 0;i < _bufClothes.size();i++)
    {
        CCloth *cloth = _bufClothes[i];
        if (!cloth)
            continue;

        cloth->FillMats(matrices, t, matBase,affects2);
    }

    if (affects)
    {
        nAffects = affects2.size();
        DWORD idx = 0;
        std::hash_set<BYTE>::iterator it;
        for (it = affects2.begin();it != affects2.end();it++)
        {
            affects[idx++] = (*it).first;
        }
    }

    return TRUE;

}
