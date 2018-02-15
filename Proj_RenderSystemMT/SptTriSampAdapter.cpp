
// generate bake sample that is used for trismapler from a Spt resource
//  star
#include "stdh.h"

#include "SptTriSampAdapter.h"

#include "RenderSystem/IRenderSystemDefines.h"

#include "log/LogDump.h"

#include <assert.h>

TriSample * CSptTriSampAdapter::BuildBranchTriSamples(DWORD &nSample,int lvl)
{
	nSample = 0;
	if(!_pSpt)
		return NULL;

	i_math::size2di szBr,szFr;
	_pSpt->GetMapSize(szBr,szFr,lvl);

	void * pVtx = NULL; WORD * pIB = NULL;
	DWORD nVtx = 0,nIBs = 0;
	TriSample * pSample = NULL;
	
	CTriSampler triSampler;
	if(BranchTriSampVtx(pVtx,nVtx,pIB,nIBs)){
		pSample = triSampler.Build(pVtx,nVtx,GetVtxFVF(),GetUvFVF(),pIB,nIBs,szBr.w,szBr.h,nSample,_pSpt->GetPath());
	}

	_triSamples.resize(nSample);
	memcpy(&_triSamples[0],pSample,nSample*sizeof(TriSample));

	return &_triSamples[0];
}
TriSample * CSptTriSampAdapter::BuildEnvSamples(DWORD &nSample,DWORD &w)
{
	w  = 32;
	DWORD nVtx = w*w;

	std::vector<i_math::vector3df> fPos(nVtx);
	std::vector<i_math::vector3df> fNormals(nVtx);
	std::vector<i_math::vector3df> fTans(nVtx);

	i_math::spheref sph = _pSpt->GetLeafBoundSphere();
	const int mw = w;
	const float uvl = 1.0f/float(mw);
	const float huvl = uvl/2.0f;

	for(int i = 0;i<mw;i++)	{		// u
		for(int j = 0;j<mw;j++)		// v
		{
			float  u = (huvl + float(i)*uvl);   // u:[0,1]
			float  v = (huvl + float(j)*uvl);	// v:[0,1]

			u = 2*u - 1.0f;	// u:[-1,1]
			v = 2*v - 1.0f;	// v:[-1,1]

			float x,y,z;
			// a:为与Y轴方向的夹角

			float r = sqrt(u*u + v*v); //到中心的距离
			float fcosa = 1.0f - 2*r;  //cosa 
			if(r>1.0f)
			{
				x =  0.0f;
				y =	 1.0f;
				z =  0.0f;
			}
			else
			{
				float fsinb = v/r;
				float fcosb = u/r;
				float fsina = sqrt(1.0f - fcosa*fcosa);

				x = fsina*fcosb;
				y = fcosa;
				z = fsina*fsinb;
			}

			fNormals[i*mw + j].x = x;
			fNormals[i*mw + j].y = y;
			fNormals[i*mw + j].z = z;
			
			i_math::vector3df pos,outIntersec,vec;
			i_math::line3df line;
			vec = fNormals[i*mw +j];
			pos = sph.center + fNormals[i*mw +j]*sph.radius; //球上的点
			line.start = pos;

			vec.setLength(sph.radius*0.5f);
			line.end = sph.center + vec;

			if(_pSpt->TestCollision(line,i_math::vector3df(0,0,0),1.0f,0,outIntersec,SPTCT_LEAF)){
				fPos[i*mw + j] = outIntersec;
			}
			else 
				fPos[i*mw + j] = line.end;
//			fPos[i*mw + j] = pos;
		}	
	}	
	
	_triSamples.resize(nVtx);
	for(int i = 0;i<nVtx;i++)
	{
		i_math::vector3df tan(1.0f,0,1.0f);
		if(fNormals[i].x<fNormals[i].y&&fNormals[i].x<fNormals[i].z)
			tan.set(0,fNormals[i].z,-fNormals[i].y);
		else if(fNormals[i].y<fNormals[i].z)
			tan.set(-fNormals[i].z,0,fNormals[i].x);
		else 
			tan.set(fNormals[i].y,-fNormals[i].x,0);
		
		tan.normalize();
		i_math::vector3df biNor = fNormals[i].crossProduct(tan);
		biNor.normalize();
		
		_triSamples[i].pos = fPos[i];
		_triSamples[i].normal = fNormals[i];
		_triSamples[i].binormal = biNor;
		_triSamples[i].tangent = tan;
		
		_triSamples[i].pt.x = i%w;
		_triSamples[i].pt.y = i/w;
	}	
	
	nSample = nVtx;
	return &(_triSamples[0]);
}

TriSample * CSptTriSampAdapter::BuildFrondTriSamples(DWORD &nSample,int lvl)
{
	nSample = 0;
	if(!_pSpt)
		return NULL;

	i_math::size2di szBr,szFr;
	_pSpt->GetMapSize(szBr,szFr,lvl);

	void * pVtx = NULL; WORD * pIB = NULL;
	DWORD nVtx = 0,nIBs = 0;
	TriSample * pSample = NULL;

	CTriSampler triSampler;
	if(FrondTriSampVtx(pVtx,nVtx,pIB,nIBs)){
		pSample = triSampler.Build(pVtx,nVtx,GetVtxFVF(),GetUvFVF(),pIB,nIBs,szFr.w,szFr.h,nSample,_pSpt->GetPath());
	}

	_triSamples.resize(nSample);
	memcpy(&_triSamples[0],pSample,nSample*sizeof(TriSample));

	return &_triSamples[0];
}
BOOL CSptTriSampAdapter::BranchTriSampVtx(void *& pVtx,DWORD & nVtx,WORD * &pIB,DWORD &nIBs)
{
	if(!_pSpt)
		return FALSE;

	pVtx = NULL;
	pIB = NULL;
	nVtx = 0;
	nIBs = 0;

	_InitTrans();

	SptLod & lod = _pSpt->GetLod(0);

	if(lod.branch.vb.vb){
	
		_TriSampVtxFromVB(lod.branch.vb.vb);
		_TriSampIndexFromIB(lod.branch.vb.ib);		

		pVtx = &_sampBufferVB[0];
		pIB  = &_sampBufferIB[0];
		
		nVtx = _nVtx;
		nIBs = _nIBs;

		return TRUE;
	}

	return FALSE;
}
void CSptTriSampAdapter::_InitTrans()
{
	_sampBufferVB.clear();
	_sampBufferIB.clear();
}
BOOL CSptTriSampAdapter::FrondTriSampVtx(void * &pVtx,DWORD & nVtx,WORD * &pIB,DWORD &nIBs)
{
	if(!_pSpt)
		return FALSE;
	
	pVtx = NULL;
	pIB = NULL;
	nVtx = 0;
	nIBs = 0;

	_InitTrans();

	SptLod & lod = _pSpt->GetLod(0);

	if(lod.frond.vb.vb&&lod.frond.vb.ib){

		_TriSampVtxFromVB(lod.frond.vb.vb);
		_TriSampIndexFromIB(lod.frond.vb.ib);		

		pVtx = &_sampBufferVB[0];
		pIB = &_sampBufferIB[0];

		nVtx = _nVtx;
		nIBs = _nIBs;

		return TRUE;
	}

	return FALSE;
}
i_math::vector3df * CSptTriSampAdapter::GetVtxPos(DWORD & nVtx)
{
	nVtx = _posVtxs.size();
	return &(_posVtxs[0]);
}
void CSptTriSampAdapter::_TriSampVtxFromVB(IVertexBuffer * vb)
{
	_nVtx = vb->GetCount();
	
	_posVtxs.resize(_nVtx);
	_uvVtxs.resize(_nVtx);

	std::vector<i_math::vector3df> vtxNor(_nVtx);
	std::vector<i_math::vector3df> vtxTan(_nVtx);
	std::vector<i_math::vector3df> vtxBi(_nVtx);

	FVFEx fvfVB = vb->GetFVF();
	BYTE * pVtx = (BYTE *)vb->Lock(FALSE);

	BYTE * pVer = pVtx + fvfOffset(fvfVB,FVFEX_XYZW0);//.xyz pos
	BYTE * pNor = pVtx + fvfOffset(fvfVB,FVFEX_NORMAL0);
	BYTE * pTan = pVtx + fvfOffset(fvfVB,FVFEX_TANGENT);
	BYTE * pUV  = pVer + fvfOffset(fvfVB,FVFEX_FLAG_QUX0) + 2*sizeof(float);

	DWORD stride = vb->GetStride();

	for(int i = 0;i<_nVtx;i++){
		_posVtxs[i] = *((i_math::vector3df*)pVer);
		_uvVtxs[i]  = *((i_math::vector2df*)pUV);
		vtxNor[i] = *((i_math::vector3df*)pNor);
		vtxTan[i] = *((i_math::vector3df*)pTan);
		vtxBi[i] = vtxNor[i].crossProduct(vtxTan[i]);
		vtxBi[i].normalize();

		pVer += stride;
		pUV  += stride;
		pNor += stride;
		pTan += stride;

	}
	vb->Unlock();

	FVFEx fvfSample = GetVtxFVF();
	stride = fvfSize(fvfSample);
	_sampBufferVB.resize(_nVtx*stride);
	BYTE * pBi = &(_sampBufferVB[0]) + fvfOffset(fvfSample,FVFEX_BINORMAL);
	pVer = &(_sampBufferVB[0]) + fvfOffset(fvfSample,FVFEX_XYZ0);
	pNor = &(_sampBufferVB[0]) + fvfOffset(fvfSample,FVFEX_NORMAL0);
	pTan = &(_sampBufferVB[0]) + fvfOffset(fvfSample,FVFEX_TANGENT);
	pUV  = &(_sampBufferVB[0]) + fvfOffset(fvfSample,FVFEX_FLAG_TEX0);

	BOOL bOverFlow = FALSE;
	for(int i = 0;i<_nVtx;i++){
		*((i_math::vector3df*)pVer) = _posVtxs[i];
		*((i_math::vector2df*)pUV)  = _uvVtxs[i];
		*((i_math::vector3df*)pNor) = vtxNor[i];
		*((i_math::vector3df*)pTan) = vtxTan[i];
		*((i_math::vector3df*)pBi)  = vtxBi[i];
		pVer += stride;
		pNor += stride;
		pBi  += stride;
		pUV  += stride;
		pTan += stride;
	
		if(_uvVtxs[i].u>1.0f||_uvVtxs[i].v>1.0f||
		   _uvVtxs[i].u<0||_uvVtxs[i].v<0){
			bOverFlow = TRUE;
		}
	}

	if(bOverFlow)
		LOG_DUMP("CSptTriSampAdapter",Log_Warning,"uv is overflow!");
}

BOOL CSptTriSampAdapter::_CheckUv(i_math::vector2df * uvs)
{
	assert(uvs);
	for(int i = 0;i<3;i++){
		if(uvs[i].u<0||uvs[i].v<0)
			return FALSE;
	}
	return TRUE;
}

void CSptTriSampAdapter::_TriSampIndexFromIB(IIndexBuffer * ib)
{
	//展开索引
	DWORD nIB = ib->GetCount();
	_nIBs = 0;
	_sampBufferIB.resize(3*(nIB-2));

	WORD * pSampIB = &_sampBufferIB[0];

	WORD * pIB = (WORD *)ib->Lock(FALSE);
	for(int i = 0;i<nIB-2;i++){
		
		// 三角形的三点
		i_math::vector3df pos[3];
		pos[0] = _posVtxs[pIB[0]];
		pos[1] = _posVtxs[pIB[1]];
		pos[2] = _posVtxs[pIB[2]];
		
		//uv 
		i_math::vector2df uvs[3];
		uvs[0] = _uvVtxs[pIB[0]];
		uvs[1] = _uvVtxs[pIB[1]];
		uvs[2] = _uvVtxs[pIB[2]];
		
		//三角形的面积
		float a = (float)pos[0].getDistanceFrom(pos[1]);
		float b = (float)pos[0].getDistanceFrom(pos[2]);
		float c = (float)pos[1].getDistanceFrom(pos[2]);
		float p = (a+b+c)/2;
		float area = sqrt(p*(p-a)*(p-b)*(p-c));
		
		if(_CheckUv(uvs)&&area>0.001f&&!(pIB[0]==pIB[1]||pIB[0]==pIB[2]||pIB[1]==pIB[2])){
			*(pSampIB++) = pIB[0];
			*(pSampIB++) = pIB[1];
			*(pSampIB++) = pIB[2];		
			_nIBs += 3;
		}

		pIB++;
	}
	ib->Unlock();	
}
//释放占用的内存
void CSptTriSampAdapter::Release()
{
	std::vector<BYTE>().swap( _sampBufferVB);
	std::vector<WORD>().swap(_sampBufferIB);
	std::vector<i_math::vector3df>().swap(_posVtxs);
	std::vector<i_math::vector2df>().swap(_uvVtxs);
	_nVtx = 0;
	_nIBs = 0;
	_pSpt = NULL;
}


