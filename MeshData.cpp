/********************************************************************
	created:	2006/8/3   16:15
	filename: 	e:\IxEngine\Common\resdata\MeshData.cpp
	author:		cxi
	
	purpose:	mesh resource data
*********************************************************************/
#include "stdh.h"


#include "datapacket/DataPacket.h"

#include "stringparser/stringparser.h"

#include "MeshData.h"

#include <map>
#include <set>

#include <hash_set>

#include <assert.h>

#pragma warning(disable:4018)


#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#endif    



//////////////////////////////////////////////////////////////////////////
//MeshData::VtxFrames
MeshData::VtxFrames::VtxFrames()
{
	m_nVtx=0;
}
MeshData::VtxFrames::~VtxFrames()
{
	Clean();
}

void MeshData::VtxFrames::Zero()
{
	m_nVtx=0;
	if (size()>0)
		memset(&(*this)[0],0,sizeof(MeshData::VtxData)*size());
}

void MeshData::VtxFrames::CopyFrom(MeshData::VtxFrames &src)
{
	m_nVtx=src.m_nVtx;
	resize(src.size());
	if (src.size()>0)
		memcpy(&(*this)[0],&src[0],size()*sizeof((*this)[0]));
}


void MeshData::VtxFrames::Build(DWORD nVtx,DWORD nFrames)
{
	Clean();
	resize(nFrames);
	Zero();
	m_nVtx=nVtx;
}

void MeshData::VtxFrames::Build(DWORD nVtx,VtxFrames&pattern)
{
	Clean();
	if (pattern.size()<=0)
		return;

	resize(pattern.size());
	Zero();

	//alloc data if it doest not exist
	int i,j;
	for (i=0;i<pattern.size();i++)
	{
		MeshData::VtxData &vdSrc=pattern[i];
		MeshData::VtxData &vdDest=(*this)[i];

		if (vdSrc.pos)
			vdDest.pos=new i_math::vector3df[nVtx];
		if (vdSrc.normal)
			vdDest.normal=new i_math::vector3df[nVtx];
		if (vdSrc.binormal)
			vdDest.binormal=new i_math::vector3df[nVtx];
		if (vdSrc.tangent)
			vdDest.tangent=new i_math::vector3df[nVtx];
		if (vdSrc.color)
			vdDest.color=new DWORD[nVtx];
		if (vdSrc.weight)
			vdDest.weight=new i_math::weight3f[nVtx];
		if (vdSrc.boneindex0)
			vdDest.boneindex0=new DWORD[nVtx];
		if (vdSrc.boneindex1)
			vdDest.boneindex1=new DWORD[nVtx];

		for (j=0;j<sizeof(vdSrc.tex)/sizeof(vdSrc.tex[0]);j++)
		{
			if (vdSrc.tex[j])
				vdDest.tex[j]=new i_math::texcoordf[nVtx];
		}

		if (vdSrc.colorF)
			vdDest.colorF=new i_math::vector4df[nVtx];

		//XXXXX:More VtxData Element
	}

	m_nVtx=nVtx;
}

void MeshData::VtxFrames::CleanFrame(DWORD iFrame)
{
	assert(iFrame<size());

	int j;
	MeshData::VtxData &vd=(*this)[iFrame];

	SAFE_DELETE(vd.pos);
	SAFE_DELETE(vd.normal);
	SAFE_DELETE(vd.binormal);
	SAFE_DELETE(vd.tangent);
	SAFE_DELETE(vd.color);
	SAFE_DELETE(vd.weight);
	SAFE_DELETE(vd.boneindex0);
	SAFE_DELETE(vd.boneindex1);

	for (j=0;j<ARRAY_SIZE(vd.tex);j++)
		SAFE_DELETE(vd.tex[j]);

	for (j=0;j<ARRAY_SIZE(vd.reserve);j++)
		SAFE_DELETE(vd.reserve[j]);
	SAFE_DELETE(vd.colorF);

	//XXXXX:More VtxData Element

}

void MeshData::VtxFrames::Clean()
{
	int i;
	for (i=0;i<size();i++)
		CleanFrame(i);

	clear();

	Zero();
}

void MeshData::VtxFrames::VertexCopyFrom(VtxFrames &other,DWORD iDest,DWORD iSrc)
{
	assert(other.size()==size());
//	assert(iDest<m_nVtx);
	assert(iSrc<other.m_nVtx);

	int i,j;
	for (i=0;i<other.size();i++)
	{
		MeshData::VtxData &vdSrc=other[i];
		MeshData::VtxData &vdDest=(*this)[i];

		if (vdSrc.pos)
			vdDest.pos[iDest]=vdSrc.pos[iSrc];
		if (vdSrc.normal)
			vdDest.normal[iDest]=vdSrc.normal[iSrc];
		if (vdSrc.binormal)
			vdDest.binormal[iDest]=vdSrc.binormal[iSrc];
		if (vdSrc.tangent)
			vdDest.tangent[iDest]=vdSrc.tangent[iSrc];

		for (j=0;j<sizeof(vdSrc.tex)/sizeof(vdSrc.tex[0]);j++)
		{
			if (vdSrc.tex[j])
				vdDest.tex[j][iDest]=vdSrc.tex[j][iSrc];
		}

		if (vdSrc.color)
			vdDest.color[iDest]=vdSrc.color[iSrc];

		if (vdSrc.weight)
			vdDest.weight[iDest]=vdSrc.weight[iSrc];

		if (vdSrc.boneindex0)
			vdDest.boneindex0[iDest]=vdSrc.boneindex0[iSrc];
		if (vdSrc.boneindex1)
			vdDest.boneindex1[iDest]=vdSrc.boneindex1[iSrc];

		if (vdSrc.colorF)
			vdDest.colorF[iDest]=vdSrc.colorF[iSrc];

		//XXXXX:More VtxData Element
	}

}
BOOL MeshData::VtxFrames::VertexSame(DWORD i1,DWORD i2)
{
	int i,j;
	for (i=0;i<size();i++)
	{
		MeshData::VtxData &vd=(*this)[i];
		if (vd.pos)
			if (!vd.pos[i1].equals(vd.pos[i2]))	return FALSE;
		if (vd.normal)
			if (!vd.normal[i1].equals(vd.normal[i2]))	return FALSE;
		if (vd.binormal)
			if (!vd.binormal[i1].equals(vd.binormal[i2]))	return FALSE;
		if (vd.tangent)
			if (!vd.tangent[i1].equals(vd.tangent[i2]))	return FALSE;

		for (j=0;j<sizeof(vd.tex)/sizeof(vd.tex[0]);j++)
		{
			if (vd.tex[j])
				if (!vd.tex[j][i1].equals(vd.tex[j][i2]))	return FALSE;
		}

		if (vd.color)
			if (vd.color[i1]!=vd.color[i2])	return FALSE;

		if (vd.weight)
			if (!vd.weight[i1].equals(vd.weight[i2]))	return FALSE;

		if (vd.boneindex0)
			if (vd.boneindex0[i1]!=vd.boneindex0[i2])	return FALSE;
		if (vd.boneindex1)
			if (vd.boneindex1[i1]!=vd.boneindex1[i2])	return FALSE;

		if (vd.colorF)
			if (vd.colorF[i1]!=vd.colorF[i2])	return FALSE;

		//XXXXX:More VtxData Element
	}

	return TRUE;

}
BOOL MeshData::VtxFrames::FrameSame(DWORD f1,DWORD f2)
{
	int i,j;

	MeshData::VtxData &vd1=(*this)[f1];
	MeshData::VtxData &vd2=(*this)[f2];

	if ((vd1.pos==NULL)!=(vd2.pos==NULL))
		return FALSE;
	if ((vd1.normal==NULL)!=(vd2.normal==NULL))
		return FALSE;
	if ((vd1.binormal==NULL)!=(vd2.binormal==NULL))
		return FALSE;
	if ((vd1.tangent==NULL)!=(vd2.tangent==NULL))
		return FALSE;
	if ((vd1.color==NULL)!=(vd2.color==NULL))
		return FALSE;
	if ((vd1.weight==NULL)!=(vd2.weight==NULL))
		return FALSE;
	if ((vd1.boneindex0==NULL)!=(vd2.boneindex0==NULL))
		return FALSE;
	if ((vd1.boneindex1==NULL)!=(vd2.boneindex1==NULL))
		return FALSE;
	for (j=0;j<sizeof(vd1.tex)/sizeof(vd1.tex[0]);j++)
	{
		if ((vd1.tex[j]==NULL)!=(vd2.tex[j]==NULL))
			return FALSE;
	}
	if ((vd1.colorF==NULL)!=(vd2.colorF==NULL))
		return FALSE;
	//XXXXX:More VtxData Element

	for (i=0;i<m_nVtx;i++)
	{
		if (vd1.pos)
		{
			if (!vd1.pos[i].equals(vd2.pos[i]))
				return FALSE;
		}
		if (vd1.normal)
		{
			if (!vd1.normal[i].equals(vd2.normal[i]))
				return FALSE;
		}
		if (vd1.binormal)
		{
			if (!vd1.binormal[i].equals(vd2.binormal[i]))
				return FALSE;
		}
		if (vd1.tangent)
		{
			if (!vd1.tangent[i].equals(vd2.tangent[i]))
				return FALSE;
		}
		if (vd1.color)
		{
			if (vd1.color[i]!=vd2.color[i])
				return FALSE;
		}
		if (vd1.weight)
		{
			if (!vd1.weight[i].equals(vd2.weight[i]))
				return FALSE;
		}
		if (vd1.boneindex0)
		{
			if (vd1.boneindex0[i]!=vd2.boneindex0[i])
				return FALSE;
		}
		if (vd1.boneindex1)
		{
			if (vd1.boneindex1[i]!=vd2.boneindex1[i])
				return FALSE;
		}
		for (j=0;j<sizeof(vd1.tex)/sizeof(vd1.tex[0]);j++)
		{
			if (vd1.tex[j])
			{
				if (!vd1.tex[j][i].equals(vd2.tex[j][i]))
					return FALSE;
			}
		}
		if (vd1.colorF)
		{
			if (vd1.colorF[i]!=vd2.colorF[i])
				return FALSE;
		}

		//XXXXX:More VtxData Element
	}

	return TRUE;
}

void VtxFrames_Save(CDataPacket &dp,MeshData::VtxFrames&vf)
{
	dp.Data_NextDword()=vf.m_nVtx;

	dp.Data_NextDword()=(DWORD)vf.size();

	if (vf.size()<=0)
		return;

	DWORD nVtx;
	nVtx=vf.m_nVtx;
	int i,j;
	for (i=0;i<vf.size();i++)
	{
		MeshData::VtxData &vd=vf[i];

		DP_WriteData(dp,vd.pos,nVtx*sizeof(*vd.pos));
		DP_WriteData(dp,vd.normal,nVtx*sizeof(*vd.normal));
		DP_WriteData(dp,vd.binormal,nVtx*sizeof(*vd.binormal));
		DP_WriteData(dp,vd.tangent,nVtx*sizeof(*vd.tangent));
		DP_WriteData(dp,vd.color,nVtx*sizeof(*vd.color));
		DP_WriteData(dp,vd.weight,nVtx*sizeof(*vd.weight));
		DP_WriteData(dp,vd.boneindex0,nVtx*sizeof(*vd.boneindex0));
		DP_WriteData(dp,vd.boneindex1,nVtx*sizeof(*vd.boneindex1));

		for (j=0;j<ARRAY_SIZE(vd.tex);j++)
			DP_WriteData(dp,vd.tex[j],nVtx*sizeof(*vd.tex[j]));

		DP_WriteData(dp,vd.colorF,nVtx*sizeof(*vd.colorF));

		for (j=0;j<ARRAY_SIZE(vd.reserve);j++)
			DP_WriteData(dp,vd.reserve[j],nVtx*sizeof(*vd.reserve[j]));
		
		
		//XXXXX:More VtxData Element
	}

}

void VtxFrames_Load(CDataPacket &dp,MeshData::VtxFrames&vf,DWORD ver)
{
	vf.Clean();

	vf.m_nVtx=dp.Data_NextDword();

	DWORD sz;
	sz=dp.Data_NextDword();

	if (sz<=0)
		return;

	vf.resize(sz);

	int i,j;
	for (i=0;i<sz;i++)
	{
		MeshData::VtxData &vd=vf[i];

		DP_ReadDataNS(dp,vd.pos);
		DP_ReadDataNS(dp,vd.normal);
		DP_ReadDataNS(dp,vd.binormal);
		DP_ReadDataNS(dp,vd.tangent);
		DP_ReadDataNS(dp,vd.color);
		DP_ReadDataNS(dp,vd.weight);
		DP_ReadDataNS(dp,vd.boneindex0);
		DP_ReadDataNS(dp,vd.boneindex1);

		for (j=0;j<ARRAY_SIZE(vd.tex);j++)
			DP_ReadDataNS(dp,vd.tex[j]);

		DP_ReadDataNS(dp,vd.colorF);

		for (j=0;j<ARRAY_SIZE(vd.reserve);j++)
			DP_ReadDataNS(dp,vd.reserve[j]);

		//XXXXX:More VtxData Element
	}
}

//////////////////////////////////////////////////////////////////////////
//MeshData::FabricData
void MeshData::FabricData::Save(CDataPacket &dp)
{
    dp.Data_WriteString(nm.c_str());

    DP_WriteVector(vertices);
    DP_WriteVector(indices);
    DP_WriteVector(borders);
    DP_WriteVector(bones);

    cooked.Save(dp);
}

void MeshData::FabricData::Load(CDataPacket &dp)
{
    dp.Data_ReadString(nm);

    DP_ReadVector(vertices);
    DP_ReadVector(indices);
    DP_ReadVector(borders);
    DP_ReadVector(bones);

    cooked.Load(dp);
}

void MeshData::FabricData::Clear()
{
    vertices.clear();
    indices.clear();
    borders.clear();
    bones.clear();

    cooked.Clear();
}



//////////////////////////////////////////////////////////////////////////
//MeshData

IMPLEMENT_CLASS(MeshData);

MeshData::MeshData()
{
	Zero();
}
MeshData::~MeshData()
{
	Clean();
}

void MeshData::Zero()
{

	nWeight=0;
	flag=0;

	nSkeletonBones=0;

	aabb.reset(0,0,0);

}

void MeshData::Clean()
{
	vtxframes.Clean();
	frames.clear();
	lodInfos.Clean();
	bones.clear();
	atlases.clear();

	Zero();
}


ResType MeshData::GetType()
{
	return Res_Mesh;
}
const char *MeshData::GetTypeName()
{
	return "Mesh";
}


void AppendBoneString(std::string &s,int tab,SkeletonInfo &bones,DWORD iBone)
{
	int i;
	for (i=0;i<tab;i++)
		AppendFmtString(s,"  ");
	AppendFmtString(s,"%s\r\n",bones[iBone].name);

	tab++;
	for (i=iBone+1;i<bones.size();i++)
	{
		if (bones[i].iParent==iBone)
			AppendBoneString(s,tab,bones,i);
	}
	tab--;
}

void MeshData::CalcContent(std::string &s)
{
	s="";
	AppendFmtString(s,
		"Mesh Data Content:\r\n");

	int k;
	AppendFmtString(s,	
		"    %d vertice,%d indice(%d primitives),\r\n"
		"          max weight %d,actual frames:%d\r\n"
		"          tangent info: %s \r\n",
			vtxframes.m_nVtx,lodInfos[0].indice.size(),lodInfos[0].indice.size()/3,
			nWeight,vtxframes.size(),
			HasTangentInfo()?"Yes":"No"
			);
//,double sided: %s
	if (atlases.size()>0)
	{
		AppendFmtString(s,
		"          UV Atlas in channel ");
		for (int i=0;i<atlases.size();i++)
		{
			if (i==0)
				AppendFmtString(s,"%d",atlases[i].channel);
			else
				AppendFmtString(s,",%d",atlases[i].channel);
		}
		AppendFmtString(s,"\r\n");
	}
	else
		AppendFmtString(s,
		"          No UV Atlas\r\n");


	if (bones.size()>0)
	{
		AppendFmtString(s,
		"      --affected by %d bones in a %d-bone skeleton\r\n",bones.size(),nSkeletonBones);
	}
	if (lodInfos.size()>0)
	{
		AppendFmtString(s,
		"      --total %d primitives seperated to %d segments:\r\n",lodInfos[0].indice.size()/3,lodInfos[0].segs.size());
		for (k=0;k<lodInfos[0].segs.size();k++)
		{
			AppendFmtString(s,
		"        segment %d: primitive %d -- %d; %d bones: ",
				k,
				lodInfos[0].segs[k].ps,
				lodInfos[0].segs[k].ps+lodInfos[0].segs[k].pc-1,
				lodInfos[0].segs[k].bc);
			AppendFmtString(s,"\r\n");
		}
	}
	
	AppendFmtString(s,"total %d Lods(primitive count): ",lodInfos.size());
	for(int i = 0;i<lodInfos.size();i++){
		AppendFmtString(s," %d:%02f ",lodInfos[i].indice.size()/3,lodInfos[i].dist);
	}
}


#pragma message("Maybe we need a function to remove all the real frame data that was not referenced at all")
BOOL MeshData::RemoveFrame(DWORD iFrame)
{
	if (iFrame>=frames.size())
		return FALSE;
	frames.erase(frames.begin()+iFrame);//Only remove the frame index,the real frame data will never be removed
	return TRUE;
}

void MeshData::CollapseDupeVtx()
{
	const int  lod = 0;
	lodInfos[lod].indice.clear();

	DWORD  c_nVtxCount=vtxframes.m_nVtx;
	lodInfos[lod].indice.resize(c_nVtxCount);

	for(DWORD i=0;i<lodInfos[lod].indice.size();i++)
		lodInfos[lod].indice[i]=0xffff;
	int c_subMark=0;  //mark the  decrease factor.
	for(int i=0;i<c_nVtxCount;i++)
	{	
		WORD  c_val=lodInfos[lod].indice[i];
		if(c_val!=0xffff)
		{
			vtxframes[0].normal[i].x=-100.0f;
			c_subMark++;  continue;
		}

		lodInfos[lod].indice[i]=i-c_subMark;
		for(int j=i+1;j<c_nVtxCount;j++)
		{
			WORD c_val= lodInfos[lod].indice[j];
			if(c_val!=0xffff) continue;
			if(vtxframes.VertexSame(i,j))
				lodInfos[lod].indice[j]=lodInfos[lod].indice[i];
		}
	}
	//delete the invalid data
	DWORD c=0;
	for(DWORD i=0;i<vtxframes.m_nVtx;i++)
	{
		//the color is not -1 ,is a valid data.
		if(vtxframes[0].normal[i].x!=-100.0f)
			vtxframes.VertexCopyFrom(vtxframes,c++,i);
	}

	vtxframes.m_nVtx=c_nVtxCount-c_subMark;
}

struct AdjVtx {
	AdjVtx(DWORD i){reset();idx = i;}
	AdjVtx(){reset();}
	bool operator <(const AdjVtx & oth)const {return idx<oth.idx;}
	void reset(){idx = 0;memset(b,0,sizeof(b));idxCopyTo = -1;}
	DWORD idx;       //���λ��
	BYTE  b[4];      //ԭʼ�Ĺ�����������
	int   idxCopyTo; //�õ��Ƿ���Ҫ������
};

bool CanMerge(MeshData & m,
			  std::set<BYTE> &seg,
			  WORD * pFace)
{
	MeshData::VtxData &vtx = m.vtxframes[0];
	std::set<BYTE>::iterator itBones;
	bool bCanMerge = true;

	for(int t = 0;t<3;t++){
		BYTE * b  = (BYTE *)(vtx.boneindex0 + pFace[t]);
		int k = 0;
		for(;k<4;k++){
			itBones = seg.find(b[k]);
			if(itBones==seg.end()){
				bCanMerge = false;
				break;
			}
		}
		if(!bCanMerge)
			break;
	}

	return bCanMerge;
}
void AdjFace(MeshData &m ,
			 WORD * pIndices,
			 DWORD segBase,
			 std::set<AdjVtx>&curRefVtx,
			 std::set<AdjVtx>&vtxUsed,
			 std::vector<AdjVtx> &segVtxTable)
{
	MeshData::VtxData & vtx = m.vtxframes[0];
	std::set<AdjVtx>::iterator itVtxUsed;

	for(int i = 0;i<3;i++) {//����ϲ��Ķ���
		AdjVtx v;
		v.idx = pIndices[i];
		memcpy(v.b,vtx.boneindex0 + v.idx,sizeof(DWORD));

		itVtxUsed = curRefVtx.find(v);
		if(itVtxUsed!=curRefVtx.end()){  //�Ѿ�������
			if(itVtxUsed->idxCopyTo>0)
				pIndices[i] = itVtxUsed->idxCopyTo;
			else
				pIndices[i] = WORD(itVtxUsed->idx);
		}
		else{
			itVtxUsed = vtxUsed.find(v);
			if(itVtxUsed!=vtxUsed.end()){ //�õ�������������Segment���ù�
				v.idxCopyTo = segBase + segVtxTable.size(); //������Ķ���λ��
				pIndices[i] = v.idxCopyTo;
				segVtxTable.push_back(v); //�����Ҫ���Ƶĵ�
			}
			else{
				v.idxCopyTo = -1; //�õ�ֻ��͵��޸ģ�������������޸�
			}
			curRefVtx.insert(v);
		}
	}
}
void SegCompile(MeshData & m,int lod,std::set<BYTE> &seg,
				std::set<AdjVtx> &curRefVtx,std::set<AdjVtx> &vtxUsed,
				std::vector<AdjVtx> &segVtxAdjsTable,
				MeshData::SegInfo & segInfo,DWORD *segIdxOff,
				DWORD segBase)
{
	DWORD nLod = m.lodInfos.size();
	MeshData::VtxData &vtx = m.vtxframes[0];
	
	std::set<AdjVtx>::iterator itVtxUsed;
	std::set<BYTE>::iterator itBones;
	//������LOD����
	for(int subLod = lod;subLod<nLod;subLod++){	
		if(subLod!=lod)
			segInfo.ps = (WORD)(segIdxOff[subLod]/3);

		DWORD i_start = segIdxOff[subLod];
		if(i_start>=m.lodInfos[subLod].indice.size())
			continue;

		WORD * p0 = &(m.lodInfos[subLod].indice[i_start]);
		WORD * p1 = p0;
		DWORD primcount = 0;

		for(int j = 0;j<(m.lodInfos[subLod].indice.size()-i_start)/3;j++){
			//���Ա��ϲ��������������ƶ�������������λ��
			if(CanMerge(m,seg,p1)){
				primcount++;
				AdjFace(m,p1,segBase, 
						curRefVtx,vtxUsed,
						segVtxAdjsTable);
				if(p0!=p1)
					for(int t = 0;t<3;t++){ //�ƶ�������ʹͬһsegment������
						WORD tmp = p0[t];
						p0[t] = p1[t];
						p1[t] = tmp;
					}
					p0 += 3;
			}
			p1 += 3; //�����һ����
		}
		
		//����µ�Segment
		DWORD c = 0;
		if(lod==subLod)
			c = (segIdxOff[lod] + 2 - 3*segInfo.ps)/3 + primcount;
		else 
			c = primcount;

		segIdxOff[subLod] += 3*primcount;
		segInfo.pc = WORD(c);
		m.lodInfos[subLod].segs.push_back(segInfo);
	}
}

void SegBoneAdj(MeshData &m,std::set<BYTE> &seg,
				std::set<AdjVtx> &curRefVtx,
				std::set<AdjVtx> &vtxUsed,
				std::vector<AdjVtx> &segVtxTable,
				std::vector<AdjVtx> &vtxAdjTable)
{
	std::set<BYTE>::iterator itBones;
	std::map<WORD,WORD> adjs;
	std::map<WORD,WORD>::iterator itAdjBone;
	std::set<AdjVtx>::iterator itVtxUsed;
	
	//����segbones�������õ�������
	WORD iSeg = 0;
	for(itBones = seg.begin();itBones!=seg.end();itBones++){
		WORD b2bones = *itBones;
		adjs[*itBones] = iSeg++; //������
		WORD sb = m.bones[*itBones];
		m.segbones.push_back(sb);
	}
	
	MeshData::VtxData & vtx = m.vtxframes[0];

	// �����������õĶ�����뵽�������б�,���������������е���
	for(itVtxUsed=curRefVtx.begin();itVtxUsed!=curRefVtx.end();itVtxUsed++){
		AdjVtx v = *(itVtxUsed);
		if(v.idxCopyTo<0){	
			float w = 0.0f;
			i_math::weight3f * pw = vtx.weight+v.idx;
			const float ws[] = {
									pw->x,pw->y,
									pw->z,
									1.0f-pw->x-pw->y-pw->z
								};

			for(int t = 0;t<4;t++){
				itAdjBone = adjs.find(v.b[t]);
				assert(itAdjBone!=adjs.end());
				v.b[t] = BYTE((*itAdjBone).second);
				w += ws[t];
				if(w>0.9999f){
					for(int r = t+1;r<4;r++){
						v.b[r] = 0;
					}
					break;
				}
			}
			vtxUsed.insert(v);
		}
	}

	//�������ѵ�Ĺ�������
	for(int j = 0;j<segVtxTable.size();j++){
		AdjVtx & v = segVtxTable[j];
		float w = 0.0f;
		i_math::weight3f * pw = vtx.weight+v.idx;
		const float ws[] = {
								pw->x,pw->y,
								pw->z,
								1.0f-pw->x-pw->y-pw->z
							};

		for(int t = 0;t<4;t++){
			itAdjBone = adjs.find(v.b[t]);
			assert(itAdjBone!=adjs.end());
			v.b[t] = BYTE((*itAdjBone).second);
			w += ws[t];
			if(w>0.9999f){
				for(int r = t+1;r<4;r++){
					v.b[r] = 0;
				}
				break;
			}
		}
		vtxAdjTable.push_back(v); //�����Ҫ�����ĵ㣬�õ�Ĺ��������Ѿ�����Ϊ��ȷ��ֵ
	}
}

bool FaceAdd(MeshData &m ,std::set<BYTE>&bones,
			 std::set<BYTE>&seg,bool bTest,
			 WORD * pFace,int minBone)
{
	MeshData::VtxData &vtx = m.vtxframes[0];
	std::set<BYTE>::iterator itVtxUsed;

	for(int j = 0;j<3;j++){
		BYTE * b = (BYTE *)(vtx.boneindex0 + pFace[j]);
		for(int k = 0;k<4;k++){
			if(bTest){ //������Ϊ����ļ������������
				itVtxUsed = seg.find(b[k]);
				if(itVtxUsed==seg.end())
					bones.insert(b[k]);
			}
			else
				bones.insert(b[k]);
		}
	}

	bool bOverFlow = bTest&&((bones.size() + seg.size())>minBone);

	return bOverFlow;
}
void SegVtxConstruct(MeshData & m,std::set<AdjVtx> vtxUsed,
				  std::vector<AdjVtx> &vtxAdjTable)
{
	DWORD nVtx = m.vtxframes.m_nVtx;
	DWORD finalSz = nVtx + vtxAdjTable.size();
	std::set<AdjVtx>::iterator itVtxUsed;
	
	MeshData::VtxData & vtx = m.vtxframes[0];
	//����з��ѵ���Ҫ���춥����
	if(vtxAdjTable.size()>0)
		m.ResizeTo(vtx,nVtx,finalSz);

	m.vtxframes.m_nVtx = finalSz;
	vtx.boneindex1 = new DWORD[finalSz];

	//��ӷ��ѵ�
	for(int i = 0;i<vtxAdjTable.size();i++){
		AdjVtx & v = vtxAdjTable[i];
		assert(v.idxCopyTo>=nVtx&&v.idx<nVtx);
		m.vtxframes.VertexCopyFrom(m.vtxframes,v.idxCopyTo,v.idx);
		memcpy(vtx.boneindex1+v.idxCopyTo,v.b,sizeof(DWORD));
	}
	
	//�͵ص�����
	for(int i = 0;i<nVtx;i++){
		AdjVtx v(i);
		itVtxUsed = vtxUsed.find(v);
		if(itVtxUsed!=vtxUsed.end()){
			const AdjVtx & adjV = *(itVtxUsed);
			memcpy(vtx.boneindex1+i,adjV.b,sizeof(DWORD));
		}
		else{
			//һ���ܲ������ĵ�
			memcpy(vtx.boneindex1+i,vtx.boneindex0+i,sizeof(DWORD));
		}
	}

}

void MeshData::MakeSegBone(DWORD minbone)
{
	int const lod = 0;
	
	//first clear all the existing
	for(int i=0;i<vtxframes.size();i++)
		SAFE_DELETE(vtxframes[i].boneindex1);

	if (bones.size()<=minbone)
		return;//No need for seg bone
	
	if (!vtxframes[0].boneindex0)
		return;

	std::set<AdjVtx> vtxUsed;   //������Segment���õĵ㣬����Segment��ԭʼ���ݲ�����Ϊ�Ƿ���Ҫ���Ѷ��������
	std::set<AdjVtx>::iterator itVtxUsed;
	VtxData & vtx = vtxframes[0];

	DWORD nLod = lodInfos.size();
	std::set<BYTE> seg;
	std::set<BYTE>::iterator itBones;

	std::vector<AdjVtx> vtxAdjTable; //���������
	DWORD nVtx = vtxframes.m_nVtx;

	std::vector<DWORD> curSegOffs(nLod);
	memset(&curSegOffs[0],0,nLod*sizeof(WORD));
	
	DWORD segBase = nVtx;

	std::set<AdjVtx> curRefVtx; // ��ǰSegment,���õ��Ķ���
	std::vector<AdjVtx> subVtxAdjsTable; //���������

	//��ÿ��LOD���У���Ϊ��ʼ���½���segment�ϲ�
	for(int lod = 0;lod<nLod;lod++){
		
		//��һ��δ�ϲ��ĵ�
		DWORD idx_start = curSegOffs[lod];
		WORD * pIndices = &(lodInfos[lod].indice[idx_start]);
		
		//��ʼ��
		SegInfo curSeg;
		memset(&curSeg,0,sizeof(curSeg));
		curSeg.ps = WORD(idx_start/3); // Segment��Ƭ��ʼ
		
		//��ʼһ��segment�Ľ���
		DWORD nFaces = lodInfos[lod].indice.size()/3;
		for(int i = idx_start/3;i< nFaces;i++){
			//�Ƿ����Ϊ����ļ����ʹ��������������
			bool bTest = (curRefVtx.size()>minbone-3); 
			std::set<BYTE> refBones;
			//�ҵ�������Ҫ���õ��Ĺ���
			bool bOverflow = FaceAdd(*this,refBones,seg,
									 bTest,pIndices,
									 minbone);
	
			if(!bOverflow){
				for(itBones=refBones.begin();itBones!=refBones.end();itBones++)
					seg.insert(*itBones);

				AdjFace(*this,pIndices,segBase,
					curRefVtx,vtxUsed,
					subVtxAdjsTable);
			}
			
			bool bEnd = (i==nFaces-1);
//			if(bEnd){
//				lastSeg.pc = WORD(nFaces - curSeg.ps);
//				lastSeg.ps = curSeg.ps;
//				lastSeg.bs = segbones.size();
//				if(lod!=nLod-1){
//					bHasStoreSeg = TRUE;
//					break;
//				}
//			}

			//�ﵽһ��Segbone���ɵļ��޸���
			if(bEnd||bOverflow){
				assert(seg.size()<=minbone);
				curSeg.bs = segbones.size();
				curSeg.bc = seg.size();	
				curSegOffs[lod] = i*3;
				
				//���ݵ�ǰ�Ĺ���Segment�б����ո�����档
				SegCompile(*this,lod,seg,curRefVtx,
							vtxUsed,subVtxAdjsTable,
							curSeg,&curSegOffs[0],
							segBase);
				
				SegBoneAdj(*this,seg,curRefVtx,
						   vtxUsed,subVtxAdjsTable,
						   vtxAdjTable);
				
//				if(bHasStoreSeg){
//					assert(lod>0);
//					lastSeg.bc = seg.size();
//					lodInfos[lod-1].segs.push_back(lastSeg);
//					bHasStoreSeg = FALSE;
//				}

				//��ʼ��һ���µ�Segment	
				i = curSegOffs[lod]/3;
				curSeg.ps = i;
				pIndices = (&(lodInfos[lod].indice[0]) + curSegOffs[lod]);

				curSeg.ps = i;
				curSeg.pc = 0;
				curSeg.bs = segbones.size();
				curSeg.bc = 0;

				subVtxAdjsTable.clear();
				curRefVtx.clear();
				segBase = vtxAdjTable.size() + nVtx;
				seg.clear();

				continue;
			}
			
			pIndices += 3;
		}
	}

	//�����齨������
	SegVtxConstruct(*this,vtxUsed,vtxAdjTable);
}
void MeshData::ResizeTo(VtxData &vtx,DWORD szSrc,DWORD szDst)
{
	assert(szDst>szSrc);
#define COPY_RESERVE(elm,dtype){							\
	if(vtx.elm){											\
		const dtype * p = vtx.elm;							\
		vtx.elm = new dtype[szDst];							\
		memcpy(vtx.elm,p,szSrc*sizeof(dtype));				\
		delete []p;									\
	}														\
}
	COPY_RESERVE(pos,i_math::vector3df);
	COPY_RESERVE(normal,i_math::vector3df);
	COPY_RESERVE(binormal,i_math::vector3df);
	COPY_RESERVE(tangent,i_math::vector3df);
	COPY_RESERVE(weight,i_math::weight3f);
	COPY_RESERVE(boneindex0,DWORD);
	COPY_RESERVE(boneindex1,DWORD);
	COPY_RESERVE(color,DWORD);
	for(int i = 0;i<ARRAY_SIZE(vtx.tex);i++){
		COPY_RESERVE(tex[i],i_math::texcoordf);
	}
	for(int i = 0;i<ARRAY_SIZE(vtx.reserve);i++){
		COPY_RESERVE(reserve[i],DWORD);
	}
}

void MeshData::CalcAABB()
{
	BOOL bFirst=TRUE;

	for (int i=0;i<vtxframes.size();i++)
	{
		VtxData *p=&vtxframes[i];
		if (p->pos)
		{
			for (int j=0;j<vtxframes.m_nVtx;j++)
			{
				if (bFirst)
				{
					aabb.reset(p->pos[j]);
					bFirst=FALSE;
				}
				else
					aabb.addInternalPoint(p->pos[j]);
			}
		}
	}

}


BOOL MeshData::HasTangentInfo()
{
	return (vtxframes[0].tangent!=NULL);
}
BOOL MeshData::RemoveTangentInfo()
{
	for (int k=0;k<vtxframes.size();k++)
	{
		SAFE_DELETE(vtxframes[k].tangent);
		SAFE_DELETE(vtxframes[k].binormal);
	}
	return TRUE;
}

void UpdateTangent(MeshData::VtxData &vd,int v0, int v1, int v2)
{
	if(!vd.tex[0]){
		SAFE_DELETE(vd.tangent);
		SAFE_DELETE(vd.binormal);
		return;
	}

	i_math::texcoordf *tex=vd.tex[0];
	i_math::vector3df *pos=vd.pos;
	i_math::vector3df *tan=vd.tangent;
	i_math::vector3df *normal=vd.normal;

	// Step 1. Compute the approximate tangent vector.
	double du1 = tex[v1].u - tex[v0].u;
	double dv1 = tex[v1].v - tex[v0].v;
	double du2 = tex[v2].u - tex[v0].u;
	double dv2 = tex[v2].v - tex[v0].v;

	double prod1 = (du1*dv2-dv1*du2);
	double prod2 = (du2*dv1-dv2*du1);
	if ((fabs(prod1) < 0.000001)||(fabs(prod2) < 0.000001)) return;

	double x = dv2/prod1;
	double y = dv1/prod2;

	i_math::vector3df vec1 = pos[v1]- pos[v0];
	i_math::vector3df vec2 = pos[v2]- pos[v0];
	i_math::vector3df tangent = (vec1 * ((float)x)) + (vec2 * ((float)y));

	// Step 2. Orthonormalize the tangent.
	double component = tangent.dotProduct(normal[v0]);
	tangent -= (normal[v0]*((float)component));
	tangent.normalize();

	// Step 3: Add the estimated tangent to the overall estimate for the vertex.
	tan[v0]+=tangent;
}

BOOL MeshData::BuildTangentInfo()
{
	DWORD nVtx=vtxframes.m_nVtx;
	for (int k=0;k<vtxframes.size();k++)
	{
		VtxData &vd=vtxframes[k];

		if (!vd.tangent)
			vd.tangent=new i_math::vector3df[nVtx];
		if (!vd.binormal)
			vd.binormal=new i_math::vector3df[nVtx];
		
		memset(vd.tangent,0,nVtx*sizeof(vd.tangent[0]));
		memset(vd.binormal,0,nVtx*sizeof(vd.binormal[0]));

		for (int i=0;i<lodInfos[0].indice.size();i+=3)
		{
			UpdateTangent(vd,lodInfos[0].indice[i+0],lodInfos[0].indice[i+1],lodInfos[0].indice[i+2]);
			UpdateTangent(vd,lodInfos[0].indice[i+1],lodInfos[0].indice[i+2],lodInfos[0].indice[i+0]);
			UpdateTangent(vd,lodInfos[0].indice[i+2],lodInfos[0].indice[i+0],lodInfos[0].indice[i+1]);
		}

		for (int i=0;i<nVtx;i++)
		{
			if(vd.tangent)
				vd.tangent[i].normalize();
			if(vd.binormal)
				vd.binormal[i]=vd.normal[i].crossProduct(vd.tangent[i]);
		}
	}
	return TRUE;
}

template <class T,int bFlip>
void DoubleVtxElem(T *&buf,DWORD nVtx)
{
	if (buf)
	{
		T *bufT=new T[nVtx*2];
		memcpy(bufT,buf,nVtx*sizeof(T));
		if (!bFlip)
			memcpy(bufT+nVtx,buf,nVtx*sizeof(T));
		else
		{
#pragma warning(disable:4146)
			for (int i=0;i<nVtx;i++)
				bufT[i+nVtx]=-buf[i];
#pragma warning(default:4146)
		}
		SAFE_DELETE(buf);
		buf=bufT;
	}
}

BOOL MeshData::MakeAtlasInfo(DWORD ch)
{
	if (ch>=ARRAY_SIZE(vtxframes[0].tex))
		return FALSE;
	if (!vtxframes[0].tex[ch])
		return FALSE;

	MeshData::AtlasInfo *ai=FindAtlasInfo(ch);
	if (!ai)
	{
		atlases.resize(atlases.size()+1);
		ai=&atlases[atlases.size()-1];
	}

	ai->channel=(WORD)ch;
	ai->gutter=1.0f;

	float ext=(float)aabb.getExtent().getLength();

	DWORD len=1<<i_math::fastmaxlog2((DWORD)(ext*4));

	len=i_math::clamp_u(len,32,512);

	ai->w=(WORD)len;
	ai->h=(WORD)len;

	return TRUE;
}



#define MESHDATA_VER 1
void MeshData::Save(CDataPacket &dp)
{
	dp.Data_NextDword()=MESHDATA_VER;

	DP_WriteVar(dp,aabb);
	DP_WriteVector(dp,frames);
	dp.Data_NextDword()=nWeight;
	dp.Data_NextDword()=flag;
	dp.Data_NextDword()=nSkeletonBones;

	DP_WriteVector(dp,bones);
	DP_WriteVector(dp,atlases);
	DP_WriteVector(dp,segbones);

	DWORD sz = lodInfos.size();
	dp.Data_NextDword() = sz;
	for(int i = 0;i<sz;i++)
		lodInfos[i].Save(dp);

	VtxFrames_Save(dp,vtxframes);

}

void MeshData::SaveHeader(CDataPacket &dp)
{
	dp.Data_NextDword()=MESHDATA_VER;

	DP_WriteVar(dp,aabb);
	DP_WriteVector(dp,frames);
	dp.Data_NextDword()=nWeight;
	dp.Data_NextDword()=flag;
	dp.Data_NextDword()=nSkeletonBones;
}

void MeshData::Load(CDataPacket &dp)
{
	Clean();

	DWORD ver=dp.Data_NextDword();

	DP_ReadVar(dp,aabb);
	DP_ReadVector(dp,frames);
	nWeight=dp.Data_NextDword();
	flag=dp.Data_NextDword();
	nSkeletonBones=dp.Data_NextDword();

	DP_ReadVector(dp,bones);
	DP_ReadVector(dp,atlases);
	DP_ReadVector(dp,segbones);

	DWORD sz = dp.Data_NextDword();
	lodInfos.resize(sz);
	for(int i = 0;i<sz;i++)
		lodInfos[i].Load(dp);

	VtxFrames_Load(dp,vtxframes,ver);
}

void MeshData::LoadHeader(CDataPacket &dp)
{
	Clean();

	DWORD ver=dp.Data_NextDword();

	DP_ReadVar(dp,aabb);
	DP_ReadVector(dp,frames);
	nWeight=dp.Data_NextDword();
	flag=dp.Data_NextDword();
	nSkeletonBones=dp.Data_NextDword();
}

struct VtxElem{
	VtxElem(){vtx = NULL;i = -1;}
	VtxElem(MeshData::VtxData * v,int index){vtx = v;i = index;}
	MeshData::VtxData * vtx;
	int i;
	static void setLod(int lod){getLod() = lod;}
	static int &getLod(){ static int iLod = 0; return iLod;}
};

template<class T>
int isVec3Less(const i_math::vector3d<T> & v1,const i_math::vector3d<T> &v2){
	if (v1.x < v2.x) return -1;
	if (v1.x == v2.x && v1.y < v2.y) return -1;
	if (v1.x == v2.x && v1.y == v2.y && v1.z < v2.z) return -1;
	if (v1.x == v2.x && v1.y == v2.y && v1.z == v2.z) return 0;
	return 1;
}

template<class T>
int isVec2Less(const i_math::vector2d<T> & v1,const i_math::vector2d<T> &v2){
	if (v1.x < v2.x) return -1;
	if (v1.x == v2.x && v1.y < v2.y) return -1;
	if (v1.x == v2.x && v1.y == v2.y) return 0;
	return 1;
}
struct vtxhash
{
	size_t operator()(const VtxElem & v0) const
	{
		int i = v0.i;
		MeshData::VtxData * m_v0 = v0.vtx;
		size_t hashKey =(size_t)(10.0f*m_v0->pos[i].z);
		return hashKey;
	}
};

//std::string g_boneNames[100];
//#include <fstream>
//#include <string>
//MeshData * gdata = NULL;
//void print(const VtxElem * v0,const VtxElem * v1,const char * reason)
//{
//	static std::ofstream ofs("d:\\vtx.txt");
//	
//	if(v0==NULL||v1==NULL){
//		ofs.flush();
//		ofs.close();
//		return;
//	}
//
//	if(!ofs.is_open()){
//		ofs.open("d:\\vtx.txt",std::ios_base::out);
//	}
//	
//	const VtxElem * v[] = {v0,v1};
//	if(ofs.is_open()){
//		ofs<<"-----------------------"<<std::endl;
//		for(int i = 0;i<2;i++){
//			float w0 = v[i]->vtx->weight[v[i]->i].x;
//			float w1 = v[i]->vtx->weight[v[i]->i].y;
//			float w2 = v[i]->vtx->weight[v[i]->i].z;
//			float w3 = 1.0f - w0 - w1 - w2;
//			ofs<<"v"<<i<<":\t";
//			ofs<<w0<<","<<w1<<","<<w2<<","<<w3<<"  \t";
//			
//			BYTE * b = NULL;
//			if(v[i]->vtx->boneindex0)
//				b = (BYTE *)(v[i]->vtx->boneindex0 + v[i]->i);
//			else if(v[i]->vtx->boneindex1)
//				b = (BYTE *)(v[i]->vtx->boneindex1 + v[i]->i);
//
//			const WORD * bones = &(gdata->bones[0]);
//			
//			int b0 = bones[b[0]];
//			std::string n0 = g_boneNames[b0];
//		
//			int b1 = bones[b[1]];
//			std::string n1 = g_boneNames[b1];
//
//			int b2 = bones[b[2]];
//			std::string n2 = g_boneNames[b2];
//
//			int b3 = bones[b[3]];
//			std::string n3 = g_boneNames[b3];
//
//			ofs<<int(b[0])<<","<<int(b[1])<<","<<int(b[2])<<","<<int(b[3])<<"\t{";
//			ofs<<n0<<","<<n1<<","<<n2<<","<<n3<<"}"<<"\t";
//			ofs<<reason<<std::endl;
//			ofs.flush();
//		}
//	}
//}

struct vtxequal
{
	bool operator()(const VtxElem & v0,const VtxElem &v1) const
	{
		int i = v0.i ,j = v1.i;
		MeshData::VtxData * m_v0 = v0.vtx;
		MeshData::VtxData * m_v1 = v1.vtx;

		bool bSame = true;
		std::string strReason;

#define VTXEUQAL_CHECK_RESULT(result,reason)\
	if(result!=0){				\
		bSame = false;			\
		strReason = reason;		\
		goto _rt;				\
	}

 		assert(m_v0->pos);		
		//λ��
		if( abs(m_v0->pos[i].x-m_v1->pos[j].x)>0.001f||
			abs(m_v0->pos[i].y-m_v1->pos[j].y)>0.001f||
			abs(m_v0->pos[i].z-m_v1->pos[j].z)>0.001f)
			return false;
		
		//����
		float tolerace[] = {0.865f,0.70f,0.6f};//degree:30 ,45.6, 53.15
		int lod = (VtxElem::getLod()>2)?2:VtxElem::getLod();
		if(m_v0->normal){
			float fDot = m_v0->normal[i].dotProduct(m_v1->normal[j]);
			if(fDot<tolerace[lod]) return false;
		}	
		
		//����ɫ
		if(m_v0->color){
			int bc = int(m_v0->color[i])- int(m_v1->color[j]);
			VTXEUQAL_CHECK_RESULT(bc,"Vtx color");
		}
		
		//�������ݱȽ�
		if(TRUE){
			BYTE * b0 = NULL,* b1 = NULL;
			if(m_v0->boneindex0){
				b0 = (BYTE *)(m_v0->boneindex0+i);
				b1 = (BYTE *)(m_v1->boneindex0+j);	
			}
			else if(m_v0->boneindex1){
				b0 = (BYTE *)(m_v0->boneindex1+i);
				b1 = (BYTE *)(m_v1->boneindex1+j);
			}

			if(b0&&b1){
				float w0[] = {
								m_v0->weight[i].x,
								m_v0->weight[i].y,
								m_v0->weight[i].z,
								1.0f-m_v0->weight[i].x-m_v0->weight[i].y-m_v0->weight[i].z
							 };

				float w1[] = {
								m_v1->weight[j].x,
								m_v1->weight[j].y,
								m_v1->weight[j].z,
								1.0f-m_v1->weight[j].x-m_v1->weight[j].y-m_v1->weight[j].z
							 };
				
				float w = 0;
				for(int l = 0;l<4;l++){
					int k = 0;
					for(;k<4;k++){
						if(b0[l]==b1[k]){ //����Ȩ�ز�һ��
							if(w0[l]!=w1[k]){
//								return false;
								strReason = "vtx weight";
								bSame = false;
								goto _rt;
							}
							else
								break;
						}
					}
					if(k>=4){	//����Ӱ��Ĺ�����ͬ
					//	return false;
						bSame = false;
						strReason = "vtx bone";
						goto _rt;
					}

					// һ�������ȽϽ���
					w += w0[l];
					if(w>0.9999f)
						break;
				}
			}
		}

		//��������
		for (int k=0;k<sizeof(m_v0->tex[i])/sizeof(m_v0->tex[i][0]);k++){

			if(m_v0->tex[i]){
				int r = isVec2Less<i_math::f32>(m_v0->tex[i][k],m_v1->tex[j][k]);
				VTXEUQAL_CHECK_RESULT(r,"texture coord");
			}
		}
		
_rt:
		if(!bSame){
//			print(&v0,&v1,strReason.c_str());
		}
		return bSame;
	}
};
bool CheckData(MeshData &m0,MeshData &m1)
{
	MeshData::VtxData &v0 = m0.vtxframes[0];
	MeshData::VtxData &v1 = m1.vtxframes[0];

#define CHECKDATA_ELEM(elm){			\
	if((v0.elm!=NULL&&v1.elm==NULL)		\
	||v0.elm==NULL&&v1.elm!=NULL)		\
		return false;					\
}
	CHECKDATA_ELEM(pos);
	CHECKDATA_ELEM(normal);
	CHECKDATA_ELEM(binormal);
	CHECKDATA_ELEM(tangent);
	CHECKDATA_ELEM(color);
	CHECKDATA_ELEM(weight);
	CHECKDATA_ELEM(boneindex0);
	CHECKDATA_ELEM(boneindex1);

	for(int i = 0;i<ARRAY_SIZE(v0.tex);i++)
		CHECKDATA_ELEM(tex[i]);

	for(int i = 0;i<ARRAY_SIZE(v0.reserve);i++)
		CHECKDATA_ELEM(reserve[i]);

	return true;
}
bool MeshData::Merge(MeshData * data)
{
//	gdata = this;

	if(!CheckData(*this,*data))//������ɲ�ͬ
		return false;

	//�Ѿ�������LOD��Ϣ�������Ի��ں�һ��������Mesh
	if(data->lodInfos.size()>1)
		return false;


	
	std::map<WORD,WORD> adjs; 
	std::map<WORD,WORD>::iterator itAdj;

	//����SegBones�е�����
	assert(data->lodInfos.size()==1);
	LodMeshInfo & lod = data->lodInfos[0];

	//��Ҫ�����Ĺ�������
	if(data->bones.size()){
		for(int i = 0;i< data->bones.size();i++){
			if(data->bones[i]!=bones[i]){
				int j = 0;
				for(;j<bones.size()&&bones[j]!=data->bones[i];j++);
				if(j>=bones.size())
					bones.push_back(data->bones[i]); //�ͼ�LOD�ܸ���Ĺ���Ӱ�죨�����棬ֵ������)
				adjs[i] = j; // ��Ҫ����������
			}
		}
		
		//���������й���������Ϣ
		for(int k = 0;k< data->vtxframes.size();k++){
			VtxData * vtxs = &(data->vtxframes[k]);
			if(NULL==vtxs->boneindex0)
				continue;

			DWORD * p = vtxs->boneindex0;
			i_math::vector3df * pw = vtxs->weight;
			for(int i = 0;i<data->vtxframes.m_nVtx;i++){
				BYTE * b = (BYTE*)p;
				float w = 0;
				for(int j = 0;j<4;j++){
					WORD v = b[j];
					itAdj = adjs.find(v);
					if(itAdj!=adjs.end()){
						*(b+j) = BYTE((*itAdj).second);
					}
					w += ((float *)(pw))[j];
					if(w>0.9999f) //�����Ĳ����ٸ���
						break;
				}
				p++;
				pw++;
			}
		}
	}
	
	
	VtxData * v0 = &(vtxframes[0]);
	VtxData * v1 = &(data->vtxframes[0]);	

	//ȷ����Mesh������LOD����
	float dist = data->lodInfos[0].dist;
	lodInfos.resize(lodInfos.size()+1);	
	int iLod = lodInfos.size() -1; //ָ�����һ��Ԫ��
	for(;iLod>0;iLod--){
		if(dist<lodInfos[iLod-1].dist){
			lodInfos[iLod] = lodInfos[iLod-1];
		}
		else{
			lodInfos[iLod] = data->lodInfos[0];
			break;
		}
	}
		
	std::hash_set<VtxElem,vtxhash,vtxequal> setFind;
	for(int i = 0;i<vtxframes.m_nVtx;i++){
		setFind.insert(VtxElem(v0,i));
	}

	//ȡ�õ���IndexBuffer������
	adjs.clear(); // ��¼������ϵ
	VtxElem::setLod(iLod);
	std::hash_set<VtxElem,vtxhash,vtxequal>::iterator itFind;
	std::vector<WORD> lostVtxs; //��¼�Ҳ����Ķ���
	for(int i = 0;i<data->vtxframes.m_nVtx;i++){
		itFind = setFind.find(VtxElem(v1,i));
		if(itFind!=setFind.end()){
			WORD k = itFind->i;
			if(k!=i)
				adjs[i] = k;
		}
		else{
			adjs[i] = vtxframes.m_nVtx + lostVtxs.size();
			lostVtxs.push_back(i);
		}
	}
	
//	print(NULL,NULL,NULL);
//���·��䶥������
	DWORD nVtxs = vtxframes.m_nVtx;			//ԭ���Ķ�����
	DWORD nSz = nVtxs + lostVtxs.size();	//�����󶥵���
	
#define MERGE_REALLOC_VTX(elem,etype){							\
	if(v0->elem){												\
		etype * pOld = v0->elem;								\
		v0->elem = new etype[nSz];								\
		memcpy(v0->elem,pOld,nVtxs*sizeof(etype));				\
		delete []pOld;											\
		etype * p = v0->elem + nVtxs;							\
		for(int ii = 0;ii<lostVtxs.size();ii++){				\
			*(p++) = v1->elem[lostVtxs[ii]];					\
		}														\
	}															\
}
	//����λ��
	MERGE_REALLOC_VTX(pos,i_math::vector3df);
	//����
	MERGE_REALLOC_VTX(normal,i_math::vector3df);
	//������
	MERGE_REALLOC_VTX(binormal,i_math::vector3df);
	//����
	MERGE_REALLOC_VTX(tangent,i_math::vector3df);
	//����ɫ
	MERGE_REALLOC_VTX(color,DWORD);
	//��������0
	MERGE_REALLOC_VTX(boneindex0,DWORD);
	//��������1
	MERGE_REALLOC_VTX(boneindex1,DWORD);
	//��ͼ����
	for(int i = 0;i< sizeof(v0->tex)/sizeof(v0->tex[0]);i++){
		MERGE_REALLOC_VTX(tex[i],i_math::texcoordf);
	}
	//����Ȩ��
	MERGE_REALLOC_VTX(weight,i_math::weight3f);
	//����Ԫ��
	for(int i = 0;i< sizeof(v0->reserve)/sizeof(v0->reserve[0]);i++){
		MERGE_REALLOC_VTX(reserve[i],DWORD);
	}

// ��������
		
	//��������
	LodMeshInfo & lodInfo = lodInfos[iLod];
	for(int i = 0;i<lodInfo.indice.size();i++){
		WORD idx = lodInfo.indice[i];
		itAdj = adjs.find(idx);
		if(itAdj!=adjs.end()){
			lodInfo.indice[i] = itAdj->second;
		}
	}
	
	vtxframes.m_nVtx = nSz; //�����󶥵���

	return true;		
}
//////////////////////////////////////////////////////////////////////////
void MeshData::LodMeshInfo::Load(CDataPacket &dp)
{	
	DP_ReadVector(dp,segs);
	DP_ReadVector(dp,indice);
	dist = dp.Data_NextFloat();
}
void MeshData::LodMeshInfo::Save(CDataPacket &dp)
{
	DP_WriteVector(dp,segs);
	DP_WriteVector(dp,indice);
	dp.Data_NextFloat() = dist;
}
void MeshData::LodMeshInfo::Clear()
{
	segs.clear();
	indice.clear();
	dist = 50.0f;
}
//////////////////////////////////////////////////////////////////////////
void MeshData::MeshLod::Clean()
{
	for(int i = 0;i<size();i++)
		(*this)[i].Clear();
	clear();
	resize(1);
}




