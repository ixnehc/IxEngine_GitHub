
#include "stdh.h"

//#include "omp.h"

#include "RenderSystem/ITools.h"

#pragma warning(disable:4018)
#pragma warning(disable:4267)

#include "BoneAnimMgr.h"

#include <assert.h>

extern void decodeQuat(const i_math::vector4ds & qSrc,i_math::quatf & qDst);

BOOL AnimFromBoneData2(CBoneAnim *anim,IRenderSystem *pRS)
{	
	anim->_skeleton = pRS->CreateSkeleton(anim->_boneData2.skeleton);
	if(!anim->_skeleton)
		return FALSE;
	
	anim->_nBones = anim->_skeleton->GetBoneCount();

	for(DWORD i = 0;i<anim->_boneData2.animpieces.size();i++){
		StringID strID = anim->_boneData2.animpieces[i].name;
		anim->_mapStr2Idx[strID] = i;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

//CAnim2
IMPLEMENT_CLASS(CBoneAnim)
CBoneAnim::CBoneAnim()
{
	_nBones = 0;
	_skeleton = NULL;
}

void CBoneAnim::Zero()
{
	_skeleton = NULL;
	_mapStr2Idx.clear();
}

void CBoneAnim::Clean()
{
	SAFE_RELEASE(_skeleton);
	Zero();
}

BOOL CBoneAnim::_OnTouch(IRenderSystem *pRS)
{
	_boneData2.LoadData(_data);
	return AnimFromBoneData2(this,pRS);
}

void CBoneAnim::_OnUnload()
{
	Clean();
}

DWORD CBoneAnim::GetAnimPieceCount()
{
	return _boneData2.animpieces.size();
}

StringID CBoneAnim::GetAnimPieceName(DWORD idx)
{
	if (idx>=_boneData2.animpieces.size())
		return StringID_Invalid;
	return (StringID)(_boneData2.animpieces[idx].name);
}

int CBoneAnim::FindAnimPiece(StringID name)
{
	int idx = 0;
	itMapStr2Idx it = _mapStr2Idx.find(name);
	if(it!=_mapStr2Idx.end())
		idx = int(it->second);
	return idx;
}

BOOL CBoneAnim::GetAPRange(DWORD idxAP,AnimTick &tStart,AnimTick &tEnd)
{
	if (idxAP>=_boneData2.animpieces.size())
		return FALSE;

	tStart = _boneData2.animpieces[idxAP].tStart;
	tEnd = _boneData2.animpieces[idxAP].tEnd;

	return TRUE;
}


AnimEvent *CBoneAnim::GetEvents(DWORD idxAP,DWORD &count)
{
	count=0;
	if (idxAP>=_boneData2.animpieces.size())
		return NULL;
	count=_boneData2.animpieces[idxAP].events.size();
	return &(_boneData2.animpieces[idxAP].events[0]);
}

BOOL CBoneAnim::GetAPParam(DWORD idxAP,DWORD iParam,float &v)
{
	if (idxAP>=_boneData2.animpieces.size())
		return FALSE;

	if (iParam>=sizeof(_boneData2.animpieces[idxAP].params))
		return FALSE;
	v=_boneData2.animpieces[idxAP].params[iParam];
	return TRUE;
	
}

int depthPos=0;
int depthScale=0;
int depthRot=0;


template<class T>
inline BOOL FindKey(T * s,int t,DWORD n,T * &k1,T * &k2,int *depth)
{
	int s0 = 0,s1 = n,mid = 0;

	T *keyCur = NULL,* keyCurNext = NULL;
	//
	while(s1-s0>0){
		mid = s0 + (s1-s0)/2;
		keyCur = s + mid;

		if (depth)
			(*depth)++;

		if(keyCur->t==t){
			keyCurNext = NULL;
			break;
		}
		else if(keyCur->t<t){
			// 下一个Key
			keyCurNext = (mid+1<n)?(keyCur +1):NULL;
			if(keyCurNext&&(keyCurNext->t>t))
				break;
			else
				s0 = mid+1;
		}
		else{
			s1 = mid;
		}
		keyCur = NULL;
	}

	k1 = keyCur;
	k2 = keyCurNext;

	if(!k1)
		return FALSE;
	
	return TRUE;
}

// mats:temp pointer
BOOL CBoneAnim::CalcKey(AnimTick t,i_math::xformf *xfms,DWORD nXfm,DWORD idxAP,Bitset<8>*mask)
{
	if(_nBones!=nXfm)
		return FALSE;
//	ProfilerStart(CalcKey);

	depthPos=depthRot=depthScale=0;
	
	BonesData2::BoneAnimPiece * ap = NULL;
	
	assert(idxAP<_boneData2.animpieces.size());
	ap = &(_boneData2.animpieces[idxAP]);

	if (t>ap->tEnd)
		t=ap->tEnd;
	if (t<ap->tStart)
		t=ap->tStart;

	BOOL bMask=FALSE;
	if (mask)	{
		if (!mask->all())
			bMask=TRUE;
	}

	

	BonesData2::BonePos * seq = &ap->bones[0];
	if (!bMask)
	{
		if(_boneData2.dataType==1){
			for(int i = 0;i<_nBones;i++){
				_GetBonePos1(seq[i].sPos,seq[i+1].sPos,t,xfms[i].pos);
				_GetBoneScale1(seq[i].sScale,seq[i+1].sScale,t,xfms[i].scale_);
				_GetBoneRot1(seq[i].sRot,seq[i+1].sRot,t,xfms[i].rot);
			}
		}
		else{
			for(int i = 0;i<_nBones;i++){
				_GetBonePos2(seq[i].sPos,seq[i+1].sPos,t,xfms[i].pos);
				_GetBoneScale2(seq[i].sScale,seq[i+1].sScale,t,xfms[i].scale_);
				_GetBoneRot2(seq[i].sRot,seq[i+1].sRot,t,xfms[i].rot);
			}
		}
	}
	else
	{
		if(_boneData2.dataType==1){
			for(DWORD i = 0;i<_nBones;i++){
				if (mask->test(i)){
					_GetBonePos1(seq[i].sPos,seq[i+1].sPos,t,xfms[i].pos);
					_GetBoneScale1(seq[i].sScale,seq[i+1].sScale,t,xfms[i].scale_);
					_GetBoneRot1(seq[i].sRot,seq[i+1].sRot,t,xfms[i].rot);
				}
			}
		}
		else{
			for(DWORD i = 0;i<_nBones;i++){
				if (mask->test(i)){
					_GetBonePos2(seq[i].sPos,seq[i+1].sPos,t,xfms[i].pos);
					_GetBoneScale2(seq[i].sScale,seq[i+1].sScale,t,xfms[i].scale_);
					_GetBoneRot2(seq[i].sRot,seq[i+1].sRot,t,xfms[i].rot);
				}
			}
		}
	}

//	ProfilerEnd();

	return TRUE;
}

inline BOOL CBoneAnim::_GetBonePos1(DWORD s,DWORD e,AnimTick t,i_math::vector3df &pos)
{
	Key_ref * key = &(_boneData2.data1.ksPos[s]);

	Key_ref * k1 = NULL,*k2 = NULL;
	if(!FindKey<Key_ref>(key,t,e-s,k1,k2,NULL))
		return FALSE;

	if(k2){
		float r = float(t-k1->t)/float(k2->t - k1->t);
		i_math::vector3df & p0 = _boneData2.data1.bufPos[k1->idx];
		i_math::vector3df & p1 = _boneData2.data1.bufPos[k2->idx];
		pos = (1.0f - r)*p0 + r*p1;
	}
	else
		pos = _boneData2.data1.bufPos[k1->idx];	

	return TRUE;
}

inline BOOL CBoneAnim::_GetBonePos2(DWORD s,DWORD e,AnimTick t,i_math::vector3df &pos)
{
	Key_pos * key = &(_boneData2.data2.keysPos[s]);

//	ProfilerStart(FindKey_Pos);

	Key_pos * k1 = NULL,*k2 = NULL;
	if(!FindKey<Key_pos>(key,t,e-s,k1,k2,NULL))
		return FALSE;

//	ProfilerEnd();


	if(k2){
		float r = float(t - k1->t)/float(k2->t - k1->t);
		i_math::vector3df & p0 = k1->v;
		i_math::vector3df & p1 = k2->v;
		pos = (1.0f - r)*p0 + r*p1;
	}
	else
		pos = k1->v;	

	return TRUE;
}

inline BOOL CBoneAnim::_GetBoneScale1(DWORD s,DWORD e,AnimTick t,float&scale)
{
	Key_ref * k1 = NULL,*k2 = NULL;

	Key_ref * key = &(_boneData2.data1.ksScale[s]);
	if(!FindKey<Key_ref>(key,t,e-s,k1,k2,NULL))
		return FALSE;

	if(k2){
		float p0 = _boneData2.data1.bufScale[k1->idx];
		float p1 = _boneData2.data1.bufScale[k2->idx];
		float r = float(t-k1->t)/float(k2->t - k1->t);
		scale = (1.0f - r)*p0 + r*p1;
	}
	else
		scale = _boneData2.data1.bufScale[k1->idx];	

	return TRUE;
}

inline BOOL CBoneAnim::_GetBoneScale2(DWORD s,DWORD e,AnimTick t,float&scale)
{
	Key_f* k1 = NULL,*k2 = NULL;

	Key_f* key = &(_boneData2.data2.keysScale[s]);
	if(!FindKey<Key_f>(key,t,e-s,k1,k2,&depthScale))
		return FALSE;

	if(k2){
		float p0 = k1->v;
		float p1 = k2->v;

		float r = float(t-k1->t)/float(k2->t - k1->t);
		scale = (1.0f-r)*p0 + r*p1;
	}
	else
		scale = k1->v;

	return TRUE;
}

inline float ReciprocalSqrt( float x ) 
{
	long i;
	float y, r;
	y = x * 0.5f;
	i = *(long *)( &x );
	i = 0x5f3759df - ( i >> 1 );
	r = *(float *)( &i );
	r = r * ( 1.5f - r * r * y );
	return r;
}

inline float SinZeroHalfPI( float a ) 
{
	float s, t;
	s = a * a;
	t = -2.39e-08f;
	t *= s;
	t += 2.7526e-06f;
	t *= s;
	t += -1.98409e-04f;
	t *= s;
	t += 8.3333315e-03f;
	t *= s;
	t += -1.666666664e-01f;
	t *= s;
	t += 1.0f;
	t *= a;
	return t;
}

inline float ATanPositive( float y, float x ) 
{
	float a, d, s, t;
	if ( y > x ) 
	{
		a = -x / y;
		d = i_math::Pi / 2;
	}
	else 
	{
		a = y / x;
		d = 0.0f;
	}
	s = a * a;
	t = 0.0028662257f;
	t *= s;
	t += -0.0161657367f;
	t *= s;
	t += 0.0429096138f;
	t *= s;
	t += -0.0752896400f;
	t *= s;
	t += 0.1065626393f;
	t *= s;
	t += -0.1420889944f;
	t *= s;
	t += 0.1999355085f;
	t *= s;
	t += -0.3333314528f;
	t *= s;
	t += 1.0f;
	t *= a;
	t += d;
	return t;
}

inline void SlerpOptimized( const i_math::quatf &from, const i_math::quatf &to, float t, i_math::quatf &result ) 
{
	float cosom, absCosom, sinom, sinSqr, omega, scale0, scale1;
	cosom = from.X * to.X + from.Y * to.Y + from.Z * to.Z + from.W * to.W;
	absCosom = fabs( cosom );
	if ( ( 1.0f - absCosom ) > 1e-6f ) 
	{
		sinSqr = 1.0f - absCosom * absCosom;
		sinom = ReciprocalSqrt( sinSqr );
		omega = ATanPositive( sinSqr * sinom, absCosom );
		scale0 = SinZeroHalfPI( ( 1.0f - t ) * omega ) * sinom;
		scale1 = SinZeroHalfPI( t * omega ) * sinom;
	} 
	else 
	{
		scale0 = 1.0f - t;
		scale1 = t;
	}
	scale1 = ( cosom >= 0.0f ) ? scale1 : -scale1;
	result.X = scale0 * from.X+ scale1 * to.X;
	result.Y = scale0 * from.Y+ scale1 * to.Y;
	result.Z= scale0 * from.Z+ scale1 * to.Z;
	result.W= scale0 * from.W+ scale1 * to.W;
}

#include "ssequat.h"

inline void SlerpSSE(i_math::quatf &q0,i_math::quatf &q1,float r,i_math::quatf &ret)
{
	
	ret=*(i_math::quatf*)&XMQuaternionSlerp(XMVectorSet(q0.X,q0.Y,q0.Z,q0.W),XMVectorSet(q1.X,q1.Y,q1.Z,q1.W),r);
}


inline BOOL CBoneAnim::_GetBoneRot1(DWORD s,DWORD e,AnimTick t,i_math::quatf &rot)
{
	Key_ref * k1 = NULL,*k2 = NULL;

	Key_ref * key = &(_boneData2.data1.ksRot[s]);
	if(!FindKey<Key_ref>(key,t,e-s,k1,k2,NULL))
		return FALSE;

	i_math::vector4ds p0,p1;
	p0 = _boneData2.data1.bufRot[k1->idx];
	if(k2){
		p1 = _boneData2.data1.bufRot[k2->idx];
		float r = float(t-k1->t)/float(k2->t - k1->t);
		i_math::quatf q0,q1;
		decodeQuat(p0,q0);
		decodeQuat(p1,q1);
//		SlerpOptimized(q0,q1,r,rot);
		rot.slerp(q0,q1,r);//四元数线性插值
		rot.normalize();
	}
	else{
		decodeQuat(p0,rot);	
	}

	return TRUE;
}

inline BOOL CBoneAnim::_GetBoneRot2(DWORD s,DWORD e,AnimTick t,i_math::quatf &rot)
{
	Key_s4 * k1 = NULL,*k2 = NULL;

	Key_s4 *key = &(_boneData2.data2.keysRot[s]);
	if(!FindKey<Key_s4>(key,t,e-s,k1,k2,&depthRot))
		return FALSE;

	i_math::vector4ds p0,p1;
	p0 = k1->v;
	if(k2){
		p1 = k2->v;
		float r = float(t-k1->t)/float(k2->t - k1->t);
		i_math::quatf q0,q1;
		decodeQuat(p0,q0);
		decodeQuat(p1,q1);
//		SlerpOptimized(q0,q1,r,rot);
		rot.slerp(q0,q1,r);//四元数线性插值
		rot.normalize();
	}
	else{
		decodeQuat(p0,rot);	
	}

	return TRUE;
}

BOOL CBoneAnim::GetAnimRange(AnimTick &tStart,AnimTick &tEnd)
{
	_boneData2.GetTickRange(tStart,tEnd);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//CAnimMgr
CBoneAnimMgr::CBoneAnimMgr()
{
}

IResource *CBoneAnimMgr::ObtainRes(const char *path)
{
	return _ObtainResH<CBoneAnim>(path);
}

BOOL CBoneAnimMgr::ReloadRes(const char *pathRes)
{
	return CResourceMgr::_ReloadRes<CBoneAnim>(pathRes,'H');
}



//////////////////////////////////////////////////////////////////////////
//CDynAnimMgr

CDynBoneAnimMgr::CDynBoneAnimMgr()
{
}

IBoneAnim *CDynBoneAnimMgr::Create(ResData *data)
{	
	assert(data->GetType()==ResA_Bones2);
	CBoneAnim *anim=_ObtainRes<CBoneAnim>();

	std::vector<BYTE> dataContent;
	data->SaveData(dataContent);
	anim->_boneData2.LoadData(dataContent);
	
	if (FALSE==AnimFromBoneData2(anim,_pRS))
	{
		anim->SetState(CResource::Abandoned);
		return anim;
	}
	anim->SetState(CResource::Touched);
	return (IBoneAnim*)anim;
}

