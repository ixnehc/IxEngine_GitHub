/********************************************************************
	created:	3:9:2008   13:50
	filename: 	d:\IxEngine\interfaces\PhysicsSystem\IPhysicsSystem.h
	author:		chenxi
	
	purpose:	exposed physics system interfaces for the physics system user
*********************************************************************/
#pragma once
#include "IPhysicsSystemDefines.h"

struct ProfilerMgr;
struct LogHandler;

struct MoppMesh;

class IPhysCloth
{
public:
    INTERFACE_REFCOUNT;

    BOOL IsSimulating() = 0;
    i_math::vector3df GetParticle(DWORD iParticle) = 0;

    void BeginUpdateParticles() = 0;
    void UpdateParticle_Pos(DWORD iParticle,i_math::vector3df &pos) = 0;
    void EndUpdateParticles() = 0;

    void BeginGetParticles() = 0;
    BOOL GetParticlePos(DWORD iParticle, i_math::vector3df &pos) = 0;
    void EndGetParticles() = 0;

};

class IPhysClothes
{
public:
    INTERFACE_REFCOUNT;

    void SetSpheres(i_math::spheref *sphs, DWORD nSpheres)=0;
    void SetCapsules(DWORD *indicesSphere, DWORD nCapsules) = 0;

    BOOL AddCloth(IPhysCloth *cloth) = 0;
    void RemoveCloth(IPhysCloth *cloth) = 0;

};

struct PhysClothParam
{
    PhysClothParam()
    {
        pos = NULL;
    }
    i_math::vector4df *pos;

};

class IPhysFabric
{
public:
    INTERFACE_REFCOUNT;

    IPhysCloth *CreateCloth(PhysClothParam &param) = 0;

};

class IPhysFabrics
{
public:
    INTERFACE_REFCOUNT;

    IPhysFabric *ObtainFabric(void *key, FabricData &data) = 0;

};



class IRenderSystem;
class IPhysWorld;
class IPhysRigidBody;
class IPhysRagdoll;
class IPhysicsSystem
{
public:
	virtual BOOL Init()=0;
	virtual BOOL UnInit()=0;
	virtual BOOL BeginThread()=0;
	virtual BOOL EndThread()=0;

	virtual ProfilerMgr *GetProfilerMgr()=0;
	virtual void RegisterLogHandler(LogHandler &handler)=0;

	virtual IPhysWorld*CreateWorld(PhysWorldConfig &cfg)=0;

	virtual BOOL BuildMoppCode(MoppMesh&data)=0;
	virtual BOOL BuildConvexHull(ConvexHullParam &param,ConvexHullData &result)=0;

    virtual BOOL BuildFabricData(PhysFabricMesh &mesh, PhysFabricData &data) = 0;

    virtual IPhysFabrics *GetFabrics() = 0;
    virtual IPhysClothes *CreateClothes() = 0;
};

class IPhysRagdolls;
class IPhysTerrain;
struct ProfilerMgr;
class IPhysWorld
{
public:
	INTERFACE_REFCOUNT();

	virtual IPhysTerrain*CreateTerrain(PhysTerrainParam &param)=0;
	virtual IPhysRigidBody *CreateRigidBody()=0;

	virtual IPhysRagdolls* GetRagdolls()=0;

	//����һ��aabb,������������Ѱ��һ��������ŵĵط�(ʹ���aabb�����κ������ص�)
	virtual BOOL FindFootPos(i_math::aabbox3df &aabb,i_math::vector3df &foot)=0;
	virtual void GroundHitTest(i_math::vector3df *pos,DWORD nPos,DWORD stride,i_math::rangef &rVer,BOOL *rets,i_math::vector3df *normal)=0;
	virtual BOOL RayHitTest(i_math::line3df &ray,PhysCollideLayor layor,i_math::vector3df &hitpos,i_math::vector3df *normal)=0;
	//Ŀǰ����layorֻ֧��CldLayor_WalkingBody_Zonable��CldLayor_FlyingBody_Zonable
	virtual BOOL SphereHitTest(i_math::spheref &sph,i_math::vector3df &vTarget,PhysCollideLayor layor,i_math::vector3df &hitpos,i_math::vector3df *normal)=0;

	virtual BaffleHandle AddBaffle(i_math::line2df &line,BOOL bDoubleSided)=0;
	virtual void RemoveBaffle(BaffleHandle bfl)=0;

	virtual void BeginDebug()=0;
	virtual void EndDebug()=0;

	virtual void Locate(i_math::vector3df &center)=0;
	virtual void SetStep(float step)=0;
	virtual float GetStep()=0;
	virtual void Step()=0;

    virtual void AddClothes(IPhysClothes *clothes) = 0;
    virtual void RemoveClothes(IPhysClothes *clothes) = 0;

};

class IPhysTerrain
{
public:
	INTERFACE_REFCOUNT();

	//ע��,ͬһ��ʱ��ֻ����һ��terrain attach��world����,����,AttachToWorld()���ԭ��attach��terrain
	//detach��.
	virtual BOOL AttachToWorld()=0;
	virtual void DetachFromWorld()=0;
};

class IPhysRigidBody
{
public:
	INTERFACE_REFCOUNT();

	virtual void BeginBuild()=0;
	virtual void EndBuild()=0;
	virtual void Build_SetType(PhysRigidBodyType type)=0;
	virtual void Build_SetCollideLayor(PhysCollideLayor layor)=0;
	virtual void Build_SetXform(i_math::xformf&xfm)=0;//ע��xfm�ﲻ�ܰ���scale�任
	virtual BOOL Build_AddSphere(i_math::spheref &sph)=0;
	virtual BOOL Build_AddCapsule(i_math::capsulef&cap)=0;
	virtual BOOL Build_AddAABB(i_math::vector3df&halfext,i_math::matrix43f *mat=NULL)=0;//ע��mat�ﲻ�ܰ���scale�任
	virtual BOOL Build_AddMesh(MoppMesh *mopp,float scale)=0;
	virtual BOOL Build_AddConvexMesh(ConvexHullData &data,float scale)=0;

	virtual BOOL EnableDetectImpact(float velImpact)=0;//�ٶȳ���velImpact��ײ���ᱻ��¼����
	virtual BOOL FetchImpact()=0;

	virtual BOOL AddToWorld(BOOL bAlwaysSimulate=FALSE)=0;
	virtual void RemoveFromWorld()=0;

	virtual BOOL CalcPointVelocity(i_math::vector3df &pos,i_math::vector3df &vel)=0;//posΪ��������ϵ��ֵ

	virtual void SetXform(i_math::xformf &xfm)=0; //�������ֻ��Keyframed��rigidbody��С,ע��xfm�ﲻ�ܰ���scale
	virtual void GetXform(i_math::xformf &xfm)=0; 

	virtual void GetPos(i_math::vector3df &pos)=0; 

	virtual void SetLinearVel(i_math::vector3df &vel)=0;
	virtual void SetAngularVel(i_math::vector3df &vel)=0;
	virtual void SetImpulse(i_math::vector3df &pos,i_math::vector3df &vel)=0;

};

class SkeletonInfo;
class IPhysRagdoll
{
public:
	INTERFACE_REFCOUNT();

	virtual BOOL Reset(i_math::xformf *xfms,i_math::xformf *xfms2,DWORD c,i_math::matrix43f &matBase,RagdollSwitchArg &arg,float dt)=0;
	virtual BOOL GetWorldXforms(i_math::xformf *xfms,DWORD c)=0;

	virtual void AddToWorld()=0;
	virtual void RemoveFromWorld()=0;

};

struct PhysRagdollData
{
	PhysRagdollData()
	{
		data=NULL;
		szData=0;
		infoSkl=NULL;
	}
	BYTE *data;
	DWORD szData;
	SkeletonInfo *infoSkl;
};


class IPhysRagdolls
{
public:
	virtual IPhysRagdoll *Obtain(const char *nm)=0;//���ص�ָ���һ�����ü���
	virtual BOOL Register(const char *nm,PhysRagdollData &data)=0;
};

struct PhysClothData
{
    PhysClothData()
    {
        data = NULL;
        szData = 0;
    }
    BYTE *data;
    DWORD szData;
};

class IPhysCloth
{
public:

    virtual void Update(IMatrice43 *mats,AnimTick t) = 0;
    virtual void Draw(AnimTick tFrame, IShader *shader) = 0;

};

class ISkeleton;
class IPhysClothes
{
public:
    virtual IPhysCloth *CreateCloth(const char *nm,i_math::matrix43f &mat) = 0;
    virtual BOOL RegisterClothData(const char *nm, PhysClothData &data, ISkeleton *skl) = 0;
};