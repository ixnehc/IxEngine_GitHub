#include "stdh.h"
#include ".\meshexporthandler.h"
#include "math/matrix43.h"
#include <IGameModifier.h>
#include <IGame.h>
#include <IGame/IGameType.h>
#include "MaxDecator.h"
#include "stringparser/stringparser.h"
#include <set>

#include "Interface/InterfaceInstantiate.h"

#include "PhysicsSystem/IPhysicsSystem.h"

#include "../common/resdata/DtrData.h"

#include "commondefines/general_stl.h"



//max index size of sub mesh 
#define   MAX_INDEX_SIZE  4  //maxindex=2^32
void NAN_FLOAT_TEST(float *fValue)
{
	static BOOL s_bNanFound=FALSE;
	if(0 != _isnan(*fValue) || *fValue != *fValue)
	{
		if(s_bNanFound == false)
		{
			MessageBox(NULL, "Exporter found a NAN float value and has set it to 0.0f.  The resulting exported-file may not be accurate.", "ERROR: NAN FLOAT", MB_OK|MB_ICONERROR);
		}
		s_bNanFound= true;
		*fValue= 0.0f;
	}
}

MeshExportHandler::MeshExportHandler(void)
{
}

MeshExportHandler::~MeshExportHandler(void)
{
}

int MeshExportHandler::GetHandlerType()
{
	return  IExportHandler::MeshHandler;
}

void  MeshExportHandler::Init(MeshNode *t_gameNode)
{
		t_gameNode->m_bInit=TRUE;

		FillKeyFrame(t_gameNode->m_transformkeys,t_gameNode->m_object,GEOM_CHANNEL);
		IGameSkin  *pSkin=t_gameNode->m_object->GetIGameSkin();

		IGameMesh   *meshObject=NULL;
		if(pSkin!=NULL)
		{
			t_gameNode->m_boneAffect=TRUE;
			t_gameNode->m_SkeletonInfo.clear();
			meshObject= (IGameMesh*)pSkin->GetInitialPose();
		}
		else
			meshObject= (IGameMesh*)t_gameNode->m_object;
}

BOOL MeshExportHandler::DoExport(GameNode * node0,TimeValue time)
{
	MeshNode * gameNode=(MeshNode*)node0;
	IGameMesh   *meshObject=NULL;
	meshObject= (IGameMesh*)gameNode->m_object;

	if(!CheckValid(meshObject,gameNode->m_gameNode))
	{
		MessageBox(NULL,"the node is already considered as an bone in skeleton ,can't be affected by any bone yet.","IxEngine",WS_VISIBLE);
		return  FALSE;
	}
	if(IsBoneAffect(meshObject,gameNode->m_gameNode))  
			gameNode->m_boneAffect=TRUE;

	BOOL  bFirstTime=FALSE;

	if(!gameNode->m_bInit){
		Init(gameNode);
		bFirstTime=TRUE;
	}

	int i=0;
	for(;i<gameNode->m_transformkeys.size();i++)
		if(gameNode->m_transformkeys[i]==time) break;

	if(i>=gameNode->m_transformkeys.size())
		return FALSE;
	
	std::string nodeName = gameNode->m_gameNode->GetName();

	_CheckMesh(meshObject,nodeName.c_str());

	IGameNode * node = gameNode->m_gameNode;
	IGameMaterial * mtrl = node->GetNodeMaterial();
	
	// 含有材质的情况 按材质的不同将模型 分为若干个子模型
	if(mtrl)
    {
		int nSub = mtrl->GetSubMaterialCount();

		if(0==nSub)
        {//没有子材质
			MeshData * m = gameNode->m_subMeshContainer.getMesh(-1);
			if(!BuildSubMesh(-1,gameNode->m_BoneNodes,gameNode,m,bFirstTime,time))
				gameNode->m_subMeshContainer.Remove(-1);
		}
		else
        { //含有子材质
			for(int i = 0;i<nSub;i++)
            {
				MeshData * m = gameNode->m_subMeshContainer.getMesh(i); 
				if(!BuildSubMesh(i,gameNode->m_BoneNodes,gameNode,m,bFirstTime,time))		
					gameNode->m_subMeshContainer.Remove(i);
			}
		}
	}
	else
    {
		MeshData * m = gameNode->m_subMeshContainer.getMesh(-1);
		if(!BuildSubMesh(-1,gameNode->m_BoneNodes,gameNode,m,bFirstTime,time))
			gameNode->m_subMeshContainer.Remove(-1);
	}

	return  TRUE;
}

BOOL MeshExportHandler::_CheckMesh(IGameMesh * mesh,const char * name)
{
	assert(mesh);

	//检测uv 是否正确
	Point3 p;
	if(!mesh->GetMapVertex(1,0,p)){
		std::string s;
		FormatString(s,"用作[diffuse map]的uv通道1数据为空。[mesh:%s]",name);
		AfxMessageBox(s.c_str());	
	}

	return TRUE;
}

BOOL MeshExportHandler::BuildSubMesh(int mtrlID, NodeVector &vecBones,GameNode *gameNode,MeshData *resData,BOOL bFirstTime/*=FALSE*/,TimeValue time/*=0*/)
{
	IGameMesh * mesh  = (IGameMesh *)gameNode->m_object;
	if (FALSE)
	if (mesh)
	{
		IGameSkin  *pSkin=mesh->GetIGameSkin();
		if (pSkin)
		{
			if (pSkin->GetInitialPose())
				mesh=pSkin->GetInitialPose();
		}
	}

	IGameNode * node = gameNode->m_gameNode;

	Tab<FaceEx *> faces;
	DWORD nVtx = 0;//顶点数

	if(mtrlID>=0){
		faces = mesh->GetFacesFromMatID(mtrlID);
		nVtx  = 3*faces.Count();
	}
	else{
		nVtx = 3*mesh->GetNumberOfFaces();
	}
	
	if(nVtx==0)
		return FALSE;

	// 初始化模型属性数据
	if(bFirstTime){
		resData->flag=0;
		resData->nWeight= 0;
		IGameSkin * pSkin = mesh->GetIGameSkin();
		if(pSkin){
			int maxWeight = 0;
			if(mtrlID>=0){		
				int nFace = faces.Count(); 
				for(int i = 0;i<nFace;i++){
					FaceEx * face = faces[i];
					for(int c = 0;c<3;c++){
						int vtxIdx = face->vert[c];
						int num = pSkin->GetNumberOfBones(vtxIdx);
						maxWeight = max(num,maxWeight);
					}
				}
			}
			else
			{
				int count = mesh->GetNumberOfVerts();
				for(int i = 0;i<count;i++){
					int num = pSkin->GetNumberOfBones(i);
					maxWeight = max(num,maxWeight);
				}
			}
			resData->nWeight= maxWeight;
		}
		else if(node->GetChildCount()||
				node->GetNodeParent())
			resData->nWeight=1;
		

		resData->nSkeletonBones=0;

		//set bounding box 
// 		if(TRUE){
// 			Box3  boundingBox;
// 			mesh->GetBoundingBox(boundingBox);
// 			Matrix3 matObj   = node->GetObjectTM(0).ExtractMatrix3();
// 			Matrix3 matWld   = node->GetWorldTM(0).ExtractMatrix3();
// 			Matrix3 matLocal = Inverse(matWld)*matObj;
// 
// 			i_math::matrix43f mat;
// 			matrix43f_From_Matrix3(mat,matWld);
// 			
// 			i_math::aabbox3df aabb;
// 			memcpy(&aabb,&boundingBox,sizeof(Box3));
// 			
//  			mat.transformBoxEx(aabb);
// 
// 			resData->aabb.MinEdge.x = aabb.MinEdge.x;
// 			resData->aabb.MinEdge.y = aabb.MinEdge.z;
// 			resData->aabb.MinEdge.z = aabb.MinEdge.y;
// 
// 			resData->aabb.MaxEdge.x = aabb.MaxEdge.x;
// 			resData->aabb.MaxEdge.y = aabb.MaxEdge.z;
// 			resData->aabb.MaxEdge.z = aabb.MaxEdge.y;
// 		}

		resData->aabb.resetInvalid();
		resData->vtxframes.m_nVtx = nVtx;
		resData->lodInfos.Clean();
	}

	
	size_t sz = resData->vtxframes.size();
	resData->frames.push_back(sz);
	resData->vtxframes.resize(sz + 1);

	//加入一批顶点数据	
	AllocteMemory(resData,mesh,node,nVtx,resData->nWeight);

	BuildFrame(mtrlID,resData,vecBones,mesh,gameNode->m_gameNode,time,resData->nWeight);

	
	return TRUE;
}

void MeshExportHandler::AllocteMemory(MeshData *resData,IGameMesh * mesh,IGameNode *gameNode,int nVtx,const int nBones)
{		
	MeshData::VtxData & frame = resData->vtxframes.back();

	memset(&frame,0,sizeof(frame));
	frame.pos=new i_math::vector3df[nVtx];
	frame.normal=new i_math::vector3df[nVtx];
	frame.boneindex1= NULL;//new DWORD[4*t_vertexCount];
	frame.binormal = new i_math::vector3df[nVtx];
	frame.tangent = new i_math::vector3df[nVtx];
	if (!GetConfig().bFloatColor)
		frame.color=new DWORD[nVtx];
	else
		frame.colorF=new i_math::vector4df[nVtx];

	if(nBones){
		frame.weight=new i_math::weight3f[nVtx];
		frame.boneindex0=new DWORD[nBones*nVtx];
	}
	else if(gameNode->GetChildCount()||
			gameNode->GetNodeParent()){
		frame.weight=new i_math::weight3f[nVtx];
		frame.boneindex0=new DWORD[nBones*nVtx];
	}

	for(int c = 1;c<9;c++){
		Point3 p;
		frame.tex[c-1] = NULL;
		if(mesh->GetMapVertex(c,0,p))
			frame.tex[c-1] = new i_math::texcoordf[nVtx];
	}
}

void MeshExportHandler:: BuildFrame(int mtrlID,MeshData *resData, NodeVector &vecBones,IGameMesh * mesh,IGameNode * gameNode,TimeValue time, int nBones,BOOL bFabric)
{
	MeshData::VtxData &vtxFrame = resData->vtxframes.back();
	
	Tab<FaceEx *> faces ;
	DWORD nFaces = 0;

	// 材质ID为mtrlID 所有面的集合
	if(mtrlID>=0)
	{	
		faces = mesh->GetFacesFromMatID(mtrlID);
		nFaces = faces.Count();
	}
	else
	{
		//没有材质时Mesh不被分割
		nFaces = mesh->GetNumberOfFaces();
	}

	const int corner[]={0,2,1};
	Matrix3 toNodeMat, worldTM,worldMat3,objMat; //到局部空间的转换矩阵
	if(TRUE)
	{
		objMat = gameNode->GetObjectTM(time).ExtractMatrix3();
		worldTM = gameNode->GetWorldTM(time).ExtractMatrix3();
		worldMat3 = Inverse(worldTM);
		toNodeMat = worldMat3*objMat; 
	}

	DWORD bColorVtx=FALSE;

	extern Config & GetConfig();
	float scaleMesh=GetConfig().scaleMesh;
	BOOL bFloatColor=GetConfig().bFloatColor;
	int convertUVW=GetConfig().convertUVW;

	for(int i=0;i<nFaces;i++)
	{
		//得到当前面
		FaceEx * face = NULL;
		face = (mtrlID>=0)?faces[i]:mesh->GetFace(i);
		int idx = -1;
		Point3 pos,normal,binormal,tangent,posO;
		DWORD col;
		i_math::vector4df colF;

		for(int c=0;c<3;c++)
		{
			idx =  face->vert[c];
			pos = mesh->GetVertex(idx,false);
//			pos = toNodeMat.VectorTransform(pos);
			pos*=scaleMesh;

			//normal
			if(mesh->GetNormal(face,c,posO))
				normal = Normalize(posO);
			else
				normal.Set(0,0,0);				//法线为非法
			
			//binormal
			idx = mesh->GetFaceVertexTangentBinormal(face->meshFaceIndex,c);
			if(mesh->GetBinormal(idx,posO))
				binormal = Normalize(posO);
			else
				binormal.Set(0,0,0);

			//tangent
			if(mesh->GetTangent(idx,posO))
				tangent = Normalize(posO);
			else
				tangent.Set(0,0,0);

			if(CMatrixUtil::IsParity(worldTM))
			{
				normal = toNodeMat.VectorTransform(normal);	
				binormal = toNodeMat.VectorTransform(binormal);	
				tangent = toNodeMat.VectorTransform(tangent);	
				
				normal = -normal;
				binormal = -binormal;
				tangent = -tangent;
			}
			else
			{
				normal = toNodeMat.VectorTransform(normal);	
				binormal = toNodeMat.VectorTransform(binormal);	
				tangent = toNodeMat.VectorTransform(tangent);	
			}

			//alpha
            if (!bFabric)
            {
                if (!bFloatColor)
                {
                    idx = face->color[c];
                    Point3 ptColor = mesh->GetColorVertex(idx);
                    col = (BYTE)(i_math::clamp_f(ptColor.x*255.0f, 0.0f, 255.0f));
                    if (col >= 254)
                        col = 255;
                    col = (col << 24) | 0xffffff;
                    if (col != 0xffffffff)
                        bColorVtx = TRUE;
                }
                else
                {
                    idx = face->color[c];
                    Point3 ptColor = mesh->GetColorVertex(idx);
                    idx = face->alpha[c];
                    float alpha = mesh->GetAlphaVertex(idx);
                    colF.x = ptColor.x;
                    colF.y = ptColor.y;
                    colF.z = ptColor.z;
                    colF.w = alpha;
                    if (!((colF.x >= 0.999f) && (colF.y >= 0.999f) && (colF.z >= 0.999f) && (colF.w >= 0.999f)))
                        bColorVtx = TRUE;
                }
            }
            else
            {
                float border = 0.0f;
                float stiffness = 0.0f;
                float constraint = 0.0f;
                {
                    DWORD idx = face->color[c];
                    Point3 ptColor = mesh->GetColorVertex(idx);
                    if (ptColor.x > 0.0f || ptColor.y > 0.0f || ptColor.z > 0.0f)
                        border = 1.0f;

                    idx = face->alpha[c];
                    stiffness = mesh->GetAlphaVertex(idx);

                    idx = face->illum[c];
                    contraint = mesh->GetIllumVertex(idx);
                }

                colF.x = border;
                colF.y = stiffness;
                colF.z = constraint;
                colF.w = 1.0f;
                bColorVtx = TRUE;
            }
			
			int idxTarget = 3*i + corner[c];
			if(CMatrixUtil::IsParity(worldTM))
				idxTarget = 3*i + c;

			WRITE_POINT3(&vtxFrame.pos[idxTarget],pos);
			WRITE_POINT3(&vtxFrame.normal[idxTarget],normal);
			WRITE_POINT3(&vtxFrame.binormal[idxTarget],binormal);
			WRITE_POINT3(&vtxFrame.tangent[idxTarget],tangent);
			if (!bFloatColor)
				vtxFrame.color[idxTarget]=col;
			else
				vtxFrame.colorF[idxTarget]=colF;
			
			//调整aabb
			resData->aabb.addInternalPoint(vtxFrame.pos[idxTarget]);

			//text
			for(int ch = 1;ch<9;ch++)
			{
				if(!(vtxFrame.tex[ch-1]))
					continue;
				int  texIndex = mesh->GetFaceTextureVertex(face->meshFaceIndex,c,ch);
				Point3 texCoord;
				mesh->GetMapVertex(ch,texIndex,texCoord);
				texCoord.y=1-texCoord.y;
				texCoord.z=1-texCoord.z;
				switch(convertUVW)
				{
					case 0:
						memcpy(&(vtxFrame.tex[ch-1][idxTarget]),&texCoord,2*sizeof(float));
						break;
					case 1:
					{
						float *src=(float*)&texCoord;
						float *dest=(float*)&(vtxFrame.tex[ch-1][idxTarget]);
						dest[0]=src[0];
						dest[1]=src[2];
						break;
					}
				}
			}
			
			IGameSkin * pSkin = mesh->GetIGameSkin();
			idx =  face->vert[c];
			if(pSkin&&(nBones>0))
			{
				int numBones= pSkin->GetNumberOfBones(idx);
				DWORD   boneIndex[4];
				float   boneWeight[4];
				memset(boneIndex,0,sizeof(boneIndex));
				memset(boneWeight,0,sizeof(boneWeight));

				if(numBones>4)  
					MessageBox(NULL,"Too many bone affect one vertex!","IxEngine",WS_VISIBLE);
				
				assert(numBones<=nBones);
				//顶点受骨骼影响的
				float w = 0;
				int t = 0;
				for(int b=0;b<numBones;b++)
				{	
					DWORD idxBone =(DWORD)(pSkin->GetBone(idx,b));// INode Ptr
					float w = pSkin->GetWeight(idx,b);
					
					//无效骨骼
					if(w==0) 
						continue; 	
					
					int k = t;
					for(;(k>0)&&(w>boneWeight[k-1]);k--)
					{
						boneWeight[k] = boneWeight[k-1];
						boneIndex[k] = boneIndex[k-1];
					}						
					boneWeight[k] = w;
					boneIndex[k] = idxBone;
					t++;
				}
				// copy it to current frame buffer.
				memcpy(vtxFrame.boneindex0+nBones*idxTarget , boneIndex,nBones*sizeof(DWORD));
				memcpy(vtxFrame.weight + idxTarget , boneWeight,3*sizeof(float));
			}
			else if(gameNode->GetChildCount()||gameNode->GetNodeParent())
			{
				vtxFrame.boneindex0[idxTarget]=(DWORD)gameNode->GetMaxNode();
				vtxFrame.weight[idxTarget].w0 = 1.0f;
				vtxFrame.weight[idxTarget].w1 = 0.0f;
				vtxFrame.weight[idxTarget].w2 = 0.0f;
			}
		}
	}

	//检查color是否都是0xffffffff,如果是的话,我们不需要这些数据
	if (!bColorVtx)
	{
		if (vtxFrame.color)
			delete []vtxFrame.color;
		vtxFrame.color=NULL;
		if (vtxFrame.colorF)
			delete []vtxFrame.colorF;
		vtxFrame.colorF=NULL;
	}

}


BOOL MeshExportHandler::CheckValid(IGameMesh * mesh,IGameNode * gameNode)
{
	  BOOL  bInSkeleton = gameNode->GetChildCount()||gameNode->GetNodeParent();
	  BOOL  bAffectByBone = (mesh->GetIGameSkin()!=NULL);
	  if(bAffectByBone&&bInSkeleton)  return FALSE;
	 return  TRUE;
}

struct ItemMesh{
	ItemMesh(std::string name){strName = name;}
	ItemMesh(std::string name,float d,int iMesh){strName = name; add(d,iMesh);}
	std::string strName;
	struct _Item{
		_Item(int iM,float d){iMesh = iM; dist = d;}
		int iMesh;float dist; // 模型的部位 模型的Lod
		bool operator <(const _Item &v)const{ return (dist<v.dist)||(dist==v.dist&&iMesh<v.iMesh);}
	};
	std::set<_Item> meshDatas;
	typedef std::set<_Item>::const_iterator itMeshSet;
	void add(int iMesh,float d)const {
		const_cast<ItemMesh *>(this)->meshDatas.insert(_Item(iMesh,d));
	}
	bool operator<(const ItemMesh & v1) const{
		return (strName.compare(v1.strName)<0);
	}
};

struct MeshBody
{
	struct _Item
    {
		_Item(MeshData * m,int mtrl){mesh = m; mtrlID = mtrl;}
		MeshData * mesh; int mtrlID;
		bool operator <(const _Item & v) const{return mtrlID<v.mtrlID;}
	};
	std::set<_Item> components;
	typedef std::set<_Item>::iterator itComponent;
	bool add(int mtrlID,MeshData *mesh)
    {
		itComponent i = components.find(_Item(mesh,mtrlID));
		if(i==components.end()){
			components.insert(_Item(mesh,mtrlID));
		}
		else{
			MeshData * m = const_cast<MeshData *>(i->mesh);
			assert(m);
			m->Merge(mesh);
			return false;
		}
		return true;
	}
};

//从{dtr}XXXX_YYYY_ZZZZ中找出XXXX_YYYY
//从{dtr}YYYY_ZZZZ中找出YYYY
//从{dtr}YYYY中找出""
//这个函数假定传入的字符串是以{dtr}开头的
const char *CullDtrPrefix(const char *path)
{
	static std::string s;
	s=GetFileName(std::string(path));
	RemoveBlank(s);
	s=s.c_str()+5;
	int idx=StringReverseFind(s.c_str(),'_');
	if (idx<0)
		return "";
	s=LEFT_STRING(s,idx);
	return s.c_str();
}

//从{dtr}XXXX_YYYY_ZZZZ中找出XXXX_YYYY_ZZZZ
//从{dtr}YYYY_ZZZZ中找出YYYY_ZZZZ
//从{dtr}YYYY中找出YYYY
//这个函数假定传入的字符串是以{dtr}开头的
const char *CullDtrName(const char *path)
{
	static std::string s;
	s=GetFileName(std::string(path));
	RemoveBlank(s);
	s=s.c_str()+5;
	return s.c_str();
}


//符合{dtr}XXXXX
BOOL IsDtr(const char *path)
{
	static std::string s;
	s=GetFileName(std::string(path));
	RemoveBlank(s);
	int idx=s.find("{dtr}");
	if (idx!=0)
		return FALSE;
	return TRUE;
}

//符合{rb}XXXXX
BOOL IsRb(const char *path)
{
	static std::string s;
	s=GetFileName(std::string(path));
	RemoveBlank(s);
	int idx=s.find("{rb}");
	if (idx!=0)
		return FALSE;
	return TRUE;
}

BOOL CheckFabric(const char *path,std::string &nmMesh,std::string &nmFabric,int &idMtrl)
{
    nmFabric = "";
    nmMesh = "";
    idMtrl = -1;

    static std::string s;
    s = GetFileName(std::string(path));
    RemoveBlank(s);
    int idx = s.find("{fabric,");
    if (idx == 0)
    {
        const char *p = s.c_str() + 8;
        std::string s2;
        while (*p)
        {
            if (*p == '}')
                break;
            s2 += *p;
            p++;
        }

        std::vector<std::string> pieces;
        SplitStringBy(",", s2, &pieces);
        if (pieces.size() > 0)
            nmMesh = pieces[0];
        if (pieces.size()>1)
            idMtrl = IntFromString(pieces[1].c_str());
        if (*p)
            nmFabric = p + 1;
        return TRUE;
    }
    else
        return FALSE;
}


struct MeshDataEntry
{
	MeshData *data;
	int idMtrl;
	int iPiece;
};

struct DtrPiecesEntry
{
	DEFINE_CLASS(DtrPiecesEntry);

	DtrPiecesEntry()
	{
		idx=-1;
	}
	std::string nodePath;
	std::string prefix;
	DtrPiecesData data;
	std::vector<std::string> nameSubs;

	int idx;
};

BOOL CheckDtrPiecesParent(DtrPiecesEntry *parent,DtrPiecesEntry *sub,int &idxSub)
{
	idxSub=-1;
	for (int j=0;j<parent->nameSubs.size();j++)
	{
		if (StringEqualNoCase(sub->prefix.c_str(),parent->nameSubs[j].c_str()))
		{
			idxSub=j;
			return TRUE;
		}
	}
	return FALSE;
}

DtrPiecesEntry *FindDtrPiecesParent(DtrPiecesEntry *pieces,std::vector<DtrPiecesEntry *>&piecesAll,int &idxSub)
{
	idxSub=-1;
	for (int i=0;i<piecesAll.size();i++)
	{
		DtrPiecesEntry *piecesTest=piecesAll[i];
		if (piecesTest==pieces)
			continue;

		if (CheckDtrPiecesParent(piecesTest,pieces,idxSub))
			return piecesTest;
	}

	return NULL;
}


int CompMeshDataEntry(const void *l,const void *r)
{
	if (((MeshDataEntry *)l)->idMtrl<((MeshDataEntry *)r)->idMtrl)
		return -1;
	return 1;
}

struct FabricMeshInfo
{
    MeshNode *node;
    int idMtrl;
    std::string nmMesh;
    std::string nmFabric;
};

int FindClosestVertexInMeshData(MeshData *data, i_math::vector3df &pos)
{
    int iClosest = -1;
    float dist2Min = 1000000000.0f;
    if (data->vtxframes.size() > 0)
    {
        for (int i = 0;i < data->vtxframes.m_nVtx;i++)
        {
            float dist2 = pos.getDistanceFromSQ(data->vtxframes[0].pos[i]);
            if (dist2 < dist2Min)
            {
                iClosest = i;
                dist2Min = dist2;
                if (dist2 < 0.001f*0.001f)
                    return iClosest;
            }
        }
    }
    return iClosest;
}

void FindNeighbourVertices(MeshData *data, WORD iVtx,std::set<WORD>&indicesNeighbour)
{
    indicesNeighbour.clear();

    if (data->lodInfos.size() <= 0)
        return;

    std::vector<WORD>&indices = data->lodInfos[0].indice;

    for (int i = 0;i < indices.size;i += 3)
    {
        if (indices[i] == iVtx)
        {
            indicesNeighbour.insert(indices[i] + 1);
            indicesNeighbour.insert(indices[i] + 2);
        }
        if (indices[i + 1] == iVtx)
        {
            indicesNeighbour.insert(indices[i] + 0);
            indicesNeighbour.insert(indices[i] + 2);
        }
        if (indices[i + 2] == iVtx)
        {
            indicesNeighbour.insert(indices[i] + 0);
            indicesNeighbour.insert(indices[i] + 1);
        }
    }
}

//Fill fabric data in dataMesh,with dataFabricMesh
void BuildMeshDataFabric(MeshData *dataMesh, MeshData *dataFabricMesh,const char *nmFabric,BonesData2 *boneData,IPhysicsSystem *pPS)
{
    if (dataFabricMesh->vtxframes.size() > 0)
    {
        dataMesh->fabrics.resize(dataMesh->fabrics.size() + 1);
        MeshData::FabricData &dataFabric = dataMesh->fabrics[dataMesh->fabrics.size() - 1];

        dataFabric.vertices.resize(dataFabricMesh->vtxframes.m_nVtx);
        for (int i = 0;i < dataFabricMesh->vtxframes.m_nVtx;i++)
        {
            dataFabric.vertices[i].pos = dataFabricMesh->vtxframes[0].pos[i];
            i_math::vector4df &colF = dataFabricMesh->vtxframes[0].colorF[i];
            BOOL bBorder = colF..x > 0.5f;
            dataFabric.vertices[i].stiffness = colF.y;
            dataFabric.vertices[i].constraint = colF.z;

            if (TRUE)
            {
                int iVtx = FindClosestVertexInMeshData(dataMesh, dataFabric.vertices[i].pos);
                assert(iVtx != -1);

                memcpy(&dataFabric.vertices[i].boneindex[0], &dataMesh->vtxframes[0].boneindex0[iVtx], 4);
                dataFabric.vertices[i].weight = dataMesh->vtxframes[0].weight[iVtx];
            }

            if (bBorder)
            {
                MeshData::FabricData::BorderLink border;
                border.iVtx = i;
                dataFabric.borders.push_back(border);
            }
        }

        dataFabric.nWeights = dataMesh->nWeight;

        if (TRUE)
        {
            i_math::matrix43f *mats;
            boneData->skeleton.GetDefMatrix(mats);

            float scaleMesh = GetConfig().scaleMesh;

            std::set<WORD>neighbours;

            for (int i = 0;i < boneData->skeleton.size();i++)
            {
                BoneInfo &bi = boneData->skeleton[i];
                static std::string s;
                s = bi.name;
                RemoveBlank(s);
                if (0 == s.find("fabric_"))
                {
                    i_math::vector3df pos = mats[i].getTranslation();
                    pos *= scaleMesh;

                    int iVtx = FindClosestVertexInMeshData(dataFabricMesh, pos);
                    FindNeighbourVertices(dataFabricMesh, iVtx, neighbours);

                    i_math::vector3df front(0.0f, 0.0f, 1.0f);
                    i_math::vector3df back(0.0f, 0.0f, -1.0f);
                    i_math::vector3df left(-1.0f, 0.0f, 0.0f);
                    i_math::vector3df right(1.0f, 0.0f, 0.0f);

                    i_math::vector3df dirs[4];
                    dirs[0] = front;
                    dirs[1] = back;
                    dirs[2] = left;
                    dirs[3] = right;

                    WORD indices[4];
                    for (int j = 0;j < 4;j++)
                    {
                        i_math::vector3df dirT;
                        mats[i].rotateVect(dirs[j], dirT);
                        dirT.normalize();

                        indices[j] = 0xffff;

                        if (TRUE)
                        {
                            float dMax = -10000000.0f;
                            std::set<WORD>::iterator it;
                            for (it = neighbours.begin();it != neighbours.end();it++)
                            {
                                WORD idx = (*it);
                                i_math::vector3df posNeighbour = dataFabricMesh->vtxframes[0].pos[idx];
                                i_math::vector3df dirNeighbour = posNeighbour - pos;
                                dirNeighbour.normalize();

                                float d = dirNeighbour.dotProduct(dirT);
                                if (d < cos(45.0f*i_math::GRAD_PI2))
                                    continue;
                                if (d > dMax)
                                {
                                    dMax = d;
                                    indices[j] = idx;
                                }
                            }
                        }
                    }

                    if (((indices[0] == 0xffff) && (indices[1] == 0xffff)) ||
                        ((indices[2] == 0xffff) && (indices[3] == 0xffff)))
                    {
                        std::string s;
                        FormatString(s, "Could not locate bone(%s) on fabric!", bi.name);
                        MessageBox(NULL, "", s.c_str();MB_OK);
                        continue;
                    }

                    MeshData::FabricData::BoneLink bonelink;
                    bonelink.bondindex = i;
                    memcpy(bonelink.bonename, bi.name, sizeof(bonelink.bonename));

                    bonelink.center = iVtx;
                    bonelink.front = indices[0];
                    bonelink.back = indices[1];
                    bonelink.left = indices[2];
                    bonelink.right = indices[3];

                    dataFabric.bones.push_back(bonelink);
                }
            }
        }

        dataFabric.indices = dataFabricMesh->lodInfos[0].indice;

        //Cook
        if (pPS)
        {
            PhysFabricMesh mesh;
            PhysFabricData data;

            mesh.pos = &dataFabric.vertices[0].pos;
            mesh.stiffness= &dataFabric.vertices[0].stiffness;
            mesh.strideStiffness = mesh.stridePos = sizeof(dataFabric.vertices[0]);
            mesh.indices = &dataFabric.indices[0];
            mesh.nIndices = dataFabric.indices.size();

            if (pPS->BuildFabric(mesh, data))
            {
                DWORD szData = 0;
                szData += data.PhaseIndices.GetSize();
                szData += data.PhaseTypes.GetSize();
                szData += data.Sets.GetSize();
                szData += data.Restvalues.GetSize();
                szData += data.StiffnessValues.GetSize();
                szData += data.Indices.GetSize();
                szData += data.Anchors.GetSize();
                szData += data.TetherLengths.GetSize();
                szData += data.Triangles.GetSize();

                dataFabric.cooked.NumParticles = data.NumParticles;
                dataFabric.cooked.buf.reserve(szData);

                dataFabric.cooked.PhaseIndices = dataFabric.cooked.Add(data.PhaseIndices.begin, data.PhaseIndices.end);
                dataFabric.cooked.PhaseTypes = dataFabric.cooked.Add(data.PhaseTypes.begin, data.PhaseTypes.end);
                dataFabric.cooked.Sets = dataFabric.cooked.Add(data.Sets.begin, data.Sets.end);
                dataFabric.cooked.Restvalues = dataFabric.cooked.Add(data.Restvalues.begin, data.Restvalues.end);
                dataFabric.cooked.StiffnessValues = dataFabric.cooked.Add(data.StiffnessValues.begin, data.StiffnessValues.end);
                dataFabric.cooked.Indices = dataFabric.cooked.Add(data.Indices.begin, data.Indices.end);
                dataFabric.cooked.Anchors = dataFabric.cooked.Add(data.Anchors.begin, data.Anchors.end);
                dataFabric.cooked.TetherLengths = dataFabric.cooked.Add(data.TetherLengths.begin, data.TetherLengths.end);
                dataFabric.cooked.Triangles = dataFabric.cooked.Add(data.Triangles.begin, data.Triangles.end);
            }
        }

        dataFabric.nm = nmFabric;
    }

}

BOOL MeshExportHandler::Packet(ResGroup &resGrp, GameNodeContainer &nodeContainer)
{
    IPhysicsSystem * pPS = NULL;
    II_AccuirePS(pPS);

    if (pPS)
        pPS->Init();

	std::set<ItemMesh> meshItems;
	std::set<ItemMesh>::iterator itMesh;

	std::vector<MeshNode*> dtrs;
	std::vector<MeshNode*> rbs;
    std::deque<FabricMeshInfo>fabrics;
	for(int i = 0;i<nodeContainer.size();i++)
	{
		MeshNode * curNode = (MeshNode*)nodeContainer[i];
		std::string strName = curNode->m_nodePath;

		if (IsDtr(strName.c_str()))
		{
			dtrs.push_back(curNode);
			continue;
		}
		if (IsRb(strName.c_str()))
		{
			rbs.push_back(curNode);
			continue;
		}
        if (TRUE)
        {
            std::string nmMesh,nmFabric;
            int idMtrl;
            if (CheckFabric(strName.c_str(),nmMesh,nmFabric,idMtrl))
            {
                fabrics.resize(fabrics.size() + 1);
                FabricMeshInfo &info= fabrics[fabrics.size()-1];
                info.node = curNode;
                info.idMtrl = idMtrl;
                info.nmMesh = nmMesh;
                info.nmFabric = nmFabric;

                continue;
            }
        }

		int pos = strName.find("{");
		std::string uniName = strName;
		float dist = 0;
		
		if(pos!=std::string::npos)
		{
			uniName = strName.substr(0,pos);
			pos = strName.find("lod=");
			int pos2 = strName.find("}");
			if(pos!=std::string::npos&&pos2!=std::string::npos)
			{
				std::string strNum = strName.substr(pos+4,pos2);
				dist = (float)atof(strNum.c_str());
			}
		}

		extern void RemoveBlank(std::string &s);
		RemoveBlank(uniName);

		itMesh = meshItems.find(ItemMesh(uniName));
		if(itMesh!=meshItems.end())
		{
			itMesh->add(i,dist);
		}
		else
		{
			meshItems.insert(ItemMesh(uniName,i,dist));
		}
	}

    for (int i = 0;i < fabrics.size();i++)
    {
        MeshNode * gameNode = (MeshNode*)fabrics[i].node;
        CollapseDupeVtx(gameNode);
    }

    for(itMesh = meshItems.begin();itMesh!=meshItems.end();itMesh++)
	{
		std::string path = itMesh->strName;
        std::string filename = GetFileName(path);

		DWORD nLods = itMesh->meshDatas.size();
		if(0==nLods)
			continue;

		ItemMesh::itMeshSet lm = itMesh->meshDatas.begin();

		MeshBody body;

		MeshNode * gameNode = NULL;
		int iMesh = -1;
		for(;lm!=itMesh->meshDatas.end();lm++)
		{
			iMesh = lm->iMesh;
			gameNode = (MeshNode*)nodeContainer[iMesh];

            BOOL bFabric = FALSE;
            if (TRUE)
            {
                std::hash_map<MeshNode *, FabricMeshInfo>::iterator it = fabrics.find(gameNode);
                if (it != fabrics.end())
                {
                    bFabric = TRUE;
                }
            }

			int nSz = gameNode->m_subMeshContainer.size();
			CollapseDupeVtx(gameNode);
		
			bool bLodMain = false;
			for(int i = 0;i<nSz;i++)
			{
				int mtrlID = gameNode->m_subMeshContainer[i].mtrlID;
				MeshData * mesh = gameNode->m_subMeshContainer[i].pmesh;
// 				mesh->BuildTangentInfo();
				mesh->MakeAtlasInfo(1);
				mesh->lodInfos[0].dist = lm->dist;
				bLodMain = body.add(mtrlID,mesh);
				
				if(!bLodMain)
				{
					ResData_Delete(gameNode->m_subMeshContainer[i].pmesh);
				}
			}	
			
			gameNode->bMeshExport=TRUE;
		}
		
		MeshBody::itComponent i;
		for(i = body.components.begin();i!=body.components.end();i++)
		{
			MeshData * m = i->mesh;	

			m->MakeSegBone(GetConfig().maxBones); 

			std::string compName = path;
            if (TRUE)
            {
                std::string name = GetFileName(compName);
                for (int j = 0;j < fabrics.size();j++)
                {
                    if (fabrics[j].nmMesh == name)
                    {
                        if ((fabrics[j].idMtrl == i->mtrlID) || (fabrics[j].idMtrl == -1))
                        {
                            if (fabrics[j].node->m_subMeshContainer.size() > 0)
                            {
                                BonesData2 *boneData = resGrp.findFirstBoneData();
                                if (boneData)
                                    BuildMeshDataFabric(m, fabrics[j].node->m_subMeshContainer[0].pmesh, fabrics[j].nmFabric.c_str(),boneData);
                            }
                            break;
                        }
                    }
                }
            }

			if(body.components.size()>1)
			{
				char  index[MAX_INDEX_SIZE+5];
				sprintf(index,"_%d",i->mtrlID);
				compName.append(index);
			}
			
			resGrp.push(compName.c_str(),m);
		}
	}

	if ((dtrs.size()>0)&&(GetConfig().bExpDtr))
	{
		if(pPS)
		{
			std::vector<DtrPiecesEntry*> piecesAll;			

			std::vector<MeshNode *>buf;
			std::vector<MeshDataEntry>entries;
			while(dtrs.size()>0)
			{
				MeshNode *node=dtrs[0];
				DtrPiecesEntry *piecesNew=Class_New2(DtrPiecesEntry);
				piecesNew->prefix=CullDtrPrefix(node->m_nodePath.c_str());
				piecesNew->nodePath=node->m_nodePath.c_str();
				DtrPiecesData *data=&piecesNew->data;

				buf.clear();
				if (TRUE)
				{
					int i=0;
					while(i<dtrs.size())
					{
						if (piecesNew->prefix==CullDtrPrefix(((MeshNode *)dtrs[i])->m_nodePath.c_str()))
						{
							buf.push_back((MeshNode *)dtrs[i]);
							dtrs.erase(dtrs.begin()+i);
							continue;
						}
						i++;
					}
				}

				//Build pieces
				if (TRUE)
				{
					entries.clear();
					data->pieces.clear();

					std::vector<vector3df>verticesRB;
					std::vector<WORD>indicesRB;
					for (int i=0;i<buf.size();i++)
					{
						MeshNode *node=buf[i];
						MeshNode *nodeRB=NULL;
						if (TRUE)
						{
							std::string s=node->m_nodePath;
							s=ReplaceString(s.c_str(),"dtr","rb");

							for (int i=0;i<rbs.size();i++)
							{
								if (StringEqualNoCase(rbs[i]->m_nodePath.c_str(),s.c_str()))
								{
									nodeRB=rbs[i];
									break;
								}
							}

						}

						std::string nmSub=CullDtrName(node->m_nodePath.c_str());

						CollapseDupeVtx(node);

						if (nodeRB)
							CollapseDupeVtx(nodeRB);

						verticesRB.clear();
						indicesRB.clear();
						if (nodeRB)
						{
							int nSub=nodeRB->m_subMeshContainer.size();
							for(int j=0;j<nSub;j++)
							{
								if (TRUE)
								{
									MeshData *mdata=nodeRB->m_subMeshContainer[j].pmesh;
									if (mdata->vtxframes.size()<=0)
										continue;
									if (mdata->lodInfos.size()<=0)
										continue;

									MeshData::VtxData *vdata=&mdata->vtxframes[0];
									if (!vdata->pos)
										continue;

									int nVtx=mdata->vtxframes.m_nVtx;
									WORD iStartVtx=(WORD)verticesRB.size();

									VEC_APPEND_BUFFER(verticesRB,vdata->pos,nVtx);

									MeshData::LodMeshInfo*lod=&mdata->lodInfos[0];
									for (int k=0;k<lod->indice.size();k++)
									{
										WORD idx=lod->indice[k]+iStartVtx;
										indicesRB.push_back(idx);
									}
								}
							}	
						}
						else
						{
							//根据node里的顶点数据计算一个convex hull
							std::vector<i_math::vector3df>vertices;
							int nSub=node->m_subMeshContainer.size();
							for(int j=0;j<nSub;j++)
							{
								MeshData *mdata=node->m_subMeshContainer[j].pmesh;
								if (mdata->vtxframes.size()>0)
									VEC_APPEND_BUFFER(vertices,mdata->vtxframes[0].pos,mdata->vtxframes.m_nVtx);
							}

							if (vertices.size()>0)
							{
								ConvexHullParam paramHull;
								ConvexHull hull;

								paramHull.vertices=&vertices[0];
								paramHull.nVertices=vertices.size();

								if (pPS->BuildConvexHull(paramHull,hull))
								{
									VEC_SET_BUFFER(verticesRB,hull.vertices,hull.nVertices);
									VEC_SET_BUFFER(indicesRB,hull.indices,hull.nIndices);
								}
							}

						}

						if ((verticesRB.size()<=0)||(indicesRB.size()<=0))
							continue;

						if (TRUE)
						{
							int nSub=node->m_subMeshContainer.size();
							for(int j=0;j<nSub;j++)
							{
								MeshDataEntry entry;
								entry.data=node->m_subMeshContainer[j].pmesh;
								entry.idMtrl=node->m_subMeshContainer[j].mtrlID;
								entry.iPiece=data->pieces.size();
								entries.push_back(entry);
							}
						}

						piecesNew->nameSubs.push_back(nmSub);

						data->pieces.resize(data->pieces.size()+1);
						DtrPieceInfo *piece=&data->pieces[data->pieces.size()-1];

						//求出piece的中心点
						if (TRUE)
						{
							double x=0.0,y=0.0,z=0.0;
							for (int i=0;i<verticesRB.size();i++)
							{
								x+=(double)(verticesRB[i].x);
								y+=(double)(verticesRB[i].y);
								z+=(double)(verticesRB[i].z);
							}

							piece->posBase.x=(float)(x/(double)verticesRB.size());
							piece->posBase.y=(float)(y/(double)verticesRB.size());
							piece->posBase.z=(float)(z/(double)verticesRB.size());
						}


						//顶点位置转换到piece的local 空间
						for (int i=0;i<verticesRB.size();i++)
							verticesRB[i]-=piece->posBase;

						//Build mopp 数据
						if (TRUE)
						{
							MoppMesh moppMesh;
							moppMesh.code = NULL;
							moppMesh.indices  = &indicesRB[0];
							moppMesh.vertices = (float *)&verticesRB[0];
							moppMesh.nIndice = indicesRB.size();
							moppMesh.nVertices = verticesRB.size();

							pPS->BuildMoppCode(moppMesh);

							DWORD szOrg=data->codes.size();
							data->codes.resize(szOrg+moppMesh.szCode);
							
							moppMesh.code = &(data->codes[szOrg]);
							pPS->BuildMoppCode(moppMesh);
							piece->tpBuild=moppMesh.buildtype;
							piece->offset=moppMesh.off;
							piece->szCode=moppMesh.szCode;
							piece->startCode=szOrg;

							szOrg=data->verticesShp.size();
							VEC_APPEND(data->verticesShp,verticesRB);
							piece->startVerticesShp=szOrg;
							piece->szVerticesShp=verticesRB.size();

							szOrg=data->indicesShp.size();
							VEC_APPEND(data->indicesShp,indicesRB);
							piece->startIndicesShp=szOrg;
							piece->szIndicesShp=indicesRB.size();
						}
					}
				}

				//按照mtrl id 排序
				qsort(&entries[0],entries.size(),sizeof(MeshDataEntry),CompMeshDataEntry);

				//复制顶点数据,并且Build part 信息
				int idMtrlCur=-100000;
				int iPart=-1;
				data->nParts=0;
				for (int i=0;i<entries.size();i++)
				{
					MeshDataEntry *entry=&entries[i];
					MeshData *mdata=entry->data;
					if (mdata->vtxframes.size()<=0)
						continue;
					if (mdata->lodInfos.size()<=0)
						continue;

					MeshData::VtxData *vdata=&mdata->vtxframes[0];

					int nVtx=mdata->vtxframes.m_nVtx;
					WORD iStartVtx=(WORD)data->vertices.size();
					DWORD iStartIdx=data->indices.size();

					if (idMtrlCur!=entry->idMtrl)
					{
						idMtrlCur=entry->idMtrl;
						iPart=data->nParts;
						data->nParts++;
						data->primsParts.resize(data->nParts*data->pieces.size());//为这个part增加记录每个piece的prim的数组空间
						memset(&data->primsParts[iPart*data->pieces.size()],0,data->pieces.size()*sizeof(DtrPiecesData::PrimRange));
					}

					i_math::vector3df posBase;
					posBase=data->pieces[entry->iPiece].posBase;
					for (int k=0;k<nVtx;k++)
					{
						VtxDtr vtx;
						if (vdata->pos)
							vtx.pos=vdata->pos[k]-posBase;
						if (vdata->normal)
							vtx.normal=vdata->normal[k];
						if (vdata->binormal)
							vtx.binormal=vdata->binormal[k];
						if (vdata->tangent)
							vtx.tangent=vdata->tangent[k];
						if (vdata->tex[0])
							vtx.uv=vdata->tex[0][k];
						vtx.boneindex=entry->iPiece;
						data->vertices.push_back(vtx);
						data->aabb.addInternalPoint(vdata->pos[k]);
					}

					DtrPiecesData::PrimRange *prims=&data->primsParts[iPart*data->pieces.size()+entry->iPiece];
					prims->ps=data->indices.size()/3;

					MeshData::LodMeshInfo*lod=&mdata->lodInfos[0];
					for (int k=0;k<lod->indice.size();k++)
					{
						WORD idx=lod->indice[k]+iStartVtx;
						data->indices.push_back(idx);
					}

					prims->pc=lod->indice.size()/3;
				}

				piecesAll.push_back(piecesNew);
			}

			//根据piecesAll的内容构建资源数据
			if (TRUE)
			{
				//首先去掉前缀为空的pieces
				if (TRUE)
				{
					int c=0;
					for (int i=0;i<piecesAll.size();i++)
					{
						DtrPiecesEntry *entry=piecesAll[i];
						if (entry->prefix=="")
						{
							Safe_Class_Delete(entry);
							continue;
						}
						piecesAll[c]=entry;
						c++;
					}
					piecesAll.resize(c);
				}

				while(piecesAll.size()>0)
				{
					DtrPiecesEntry *root=NULL;
					if (TRUE)//先找一个root
					{
						for (int i=0;i<piecesAll.size();i++)
						{
							DtrPiecesEntry *entry=piecesAll[i];
							int idxSub;
							if (!FindDtrPiecesParent(entry,piecesAll,idxSub))
							{
								root=entry;
								piecesAll.erase(piecesAll.begin()+i);
								break;
							}
						}
					}

					if (!root)
						break;//不应该执行到这里

					DtrData *dataDtr=(DtrData*)ResData_New(Res_Dtr);
					root->idx=dataDtr->piecesAll.size();
					root->data.iParent=-1;
					root->data.iParentSub=-1;
					dataDtr->piecesAll.push_back(root->data);

					std::string path=GetFileFolderPath(root->nodePath)+"\\"+root->prefix+GetFileSuffix(root->nodePath);

					std::deque<DtrPiecesEntry *>qu;
					qu.push_back(root);
					while(qu.size()>0)
					{
						DtrPiecesEntry *parent=qu[0];
						qu.pop_front();

						//找出所有parent的sub
						if (TRUE)
						{
							int i=0;
							while(i<piecesAll.size())
							{
								DtrPiecesEntry *pieces=piecesAll[i];
								int idxSub;
								if (CheckDtrPiecesParent(parent,pieces,idxSub))
								{//是sub,加到资源中去
									piecesAll.erase(piecesAll.begin()+i);
									pieces->idx=dataDtr->piecesAll.size();
									pieces->data.iParent=parent->idx;
									pieces->data.iParentSub=idxSub;
									dataDtr->piecesAll.push_back(pieces->data);
									qu.push_back(pieces);//加到队列中去
									continue;
								}
								i++;
							}
						}

						Safe_Class_Delete(parent);
					}

					//资源收集数据完毕,保存
					resGrp.push(path.c_str(),dataDtr);

				}

			}


			for (int i=0;i<piecesAll.size();i++)
			{
				Safe_Class_Delete(piecesAll[i]);
			}
			piecesAll.clear();

		}

	}

    if (pPS)
        pPS->UnInit();

	return TRUE;
}

void MeshExportHandler::CollapseDupeVtx(MeshNode * meshNode)
{
	int nSubMesh = meshNode->m_subMeshContainer.size();
	for(int i=0;i<nSubMesh;i++){
		MeshData * mesh= meshNode->m_subMeshContainer[i].pmesh;
		mesh->CollapseDupeVtx();
	}
}

BOOL MeshExportHandler::IsBoneAffect(IGameMesh * mesh,IGameNode * gameNode)
{
	  if(mesh->GetIGameSkin()||gameNode->GetChildCount()||gameNode->GetNodeParent())
		  return  TRUE;
	  return FALSE;
}




