#include "stdh.h"
#include "ExportControl.h"
#include "../Common/resdata/MeshData.h"
#include "../Common/Log/LogFile.h"
#include "./MaxDecator.h"
#include "stringparser/stringparser.h"
#include "FourStateDlg.h"
#include "resource.h"

CExportControl::CExportControl(void)
{
}

CExportControl::~CExportControl(void)
{
	Clean();
}

void CExportControl::Clean()
{
	m_nodeContainer.Clean();
	m_boneContainer.Clean();
	m_dummyContainer.Clean();
	m_moppContainer.Clean();
	m_spgContainer.Clean();
	m_resGroup.clean();

	_boneExp.Clean();
	_dummiesExp.Clean();
	_meshExp.Clean();
	_moppExp.Clean();
	_mtrlExp.Clean();
	_spgExp.Clean();
	_animUVExp.Clean();
	_xfmExp.Clean();
}

CExportControl::CExportControl(std::string folderPath,BOOL bOverwrite)
{
		m_floderPath = folderPath;
		m_floderPath.append("\\");
		m_bInit=FALSE;
		_bOverwrite = bOverwrite;
}
//the major task is to enumerate all the node  and choice 
//appropriate exporter handler to do exporting.
void CExportControl::SetNodes(TopNode * node,int n)
{
	_nodes.clear();
	for(int i=0;i<n;i++){
		BOOL bState = node[i].bEnable;
		_nodes.push_back(node[i]);
	}
}

BOOL CExportControl::_IsEnable(GameNode * node)
{
	INode * maxNode = node->m_maxNode;
	return _IsEnable(maxNode);
}

BOOL CExportControl::_IsEnable(INode *node)
{
	BOOL bEnable = TRUE;
	for(int i = 0;i<_nodes.size();i++)
		if(_nodes[i].node==node){
			bEnable = _nodes[i].bEnable;
			break;
		}
	return bEnable;
}

void CExportControl::SetFileName(const char * folderPath)
{
	m_floderPath = folderPath;
	m_floderPath.append("\\");
}

void CExportControl::DoExport(IGameScene * gameScene,TimeValue time)
{
		_gameSence = gameScene;

		_TranslateNode(gameScene);

		if(GetConfig().bExpMesh||GetConfig().bExpDtr)
		{
			_meshExp.Clean();
			for(int i=0;i<m_nodeContainer.size();)
			{
				if(_IsEnable(m_nodeContainer[i]))
				{
					if(!_meshExp.DoExport(m_nodeContainer[i],time))
					{
						m_nodeContainer.erase(m_nodeContainer.begin()+i);
						continue;
					}
				}
				i++;
			}
		}
	
		//////////////////////////////////////////////////////////////////////////
		//the bone information export only at first frame.
		if(time==0){
			int sz = 0;
			int boneCount=m_boneContainer.size();
			if((GetConfig().bExpABone||GetConfig().bExpMesh||GetConfig().bExpDummy)){
				_boneExp.Clean();
				for(int i=0;i<boneCount;i++){	
					BoneNode * bone=(BoneNode *)m_boneContainer[i];
					_boneExp.DoExport(bone,time);
				}
			}

			sz = m_moppContainer.size();
			if(GetConfig().bExpMopp){
				_moppExp.Clean();
				for(int i=0;i<sz;i++){	
					GameNode * node = m_moppContainer[i];
					if(_IsEnable(node))
						_moppExp.DoExport(node,time);
				}
			}

			sz = m_spgContainer.size();
			if(GetConfig().bExpSpg){
				_spgExp.Clean();
				for (int i = 0;i<sz;i++) {
					GameNode * node = m_spgContainer[i];
					if(_IsEnable(node))
						_spgExp.DoExport(node,time);
				}
			}
		}
}

//export the node from root ,transient all the node in the tree.
BOOL  CExportControl::_DoExportFromRoot(IGameNode * gameNode)
{
		//export the current Node.
		IGameObject * object= gameNode->GetIGameObject();
		object->InitializeData();
		GameNode  * node=NULL;	
		switch(object->GetIGameType())
		{
		case IGameObject::IGAME_MESH:
			{
				Object * obj = object->GetMaxObject();
				if(!obj->IsParticleSystem())//取得Mesh 资源节点 该资源可能 是(Mesh,Mopp,Dummy)
					node=_FecthMeshNode(gameNode);
				break;
			}	
		case IGameObject::IGAME_BONE:
			{
				node=_FecthBoneNode(gameNode);
				break;
			}
		case IGameObject::IGAME_HELPER:
			{
				node=_FecthBoneNode(gameNode);
				break;
			}
		default:break;
		}
		
		if(node){
			std::string name = gameNode->GetName();
			int pos = name.find("{",0);

			if(node){
				node->m_object = object;
				node->m_gameNode = gameNode;
			}

			INode  * maxNode = gameNode->GetMaxNode();
			int nChilds = maxNode->NumberOfChildren();
			int childNodeCount = gameNode->GetChildCount();

			//与IGame 导出存在差异
			if(nChilds!= childNodeCount){
				assert(nChilds>childNodeCount);
				for(int i = 0;i< nChilds;i++){
					INode * ch = maxNode->GetChildNode(i);
					assert(ch&&_gameSence);
					if(!_gameSence->GetIGameNode(ch))
						_RepairExport(ch);
				}
			}

			for(int i=0;i<childNodeCount;i++){
				IGameNode  *node=NULL;
				node = gameNode->GetNodeChild(i);
				
				if (node->IsTarget()||
				   IsUserNode(node->GetMaxNode(),FALSE))  
					continue;

				if(!_DoExportFromRoot(node))
					return FALSE;
			}
		}
	
		return TRUE;
}

void CExportControl::_RepairExport(INode * node)
{
	/* 
		对footsteep 类型的节点做处理
	*/
	int idx = m_dummyContainer.find(node);
	if(idx>0)
		return;

	INode * nodeParent = node->GetParentNode();
	assert(nodeParent);
	if(node->NumberOfChildren()!=1) //子节点不唯一
		return;

	INode * dumNode = node->GetChildNode(0);
	std::string dumname = dumNode->GetName();

	if(dumname.substr(0,1)=="{"&&dumname.substr(dumname.length()-1,1)=="}")
	{
		DummyNode * dummy=(DummyNode*)NodeFactory::newNode(GameNode::NODE_DUMMY);
		dummy->m_gameNode = NULL;
		dummy->m_object = NULL;
		dummy->m_nodePath= m_floderPath + node->GetName();
		dummy->m_maxNode= dumNode;
		dummy->parentBone= nodeParent;
		dummy->boneContainer=&m_boneContainer;
		dummy->owner=&m_dummyContainer;
		m_dummyContainer.push_back(dummy);
	}
}

void CExportControl::SetInstance(HINSTANCE  hinstance)
{
	m_instance=hinstance;
}

void CExportControl::SetMtrlOverwrite(BOOL bOverwrite)
{
	_bOverwrite = bOverwrite;
}

void CExportControl::FlushToFile()
{
		//write data to file .
		IFileSystem  * pFS=NULL;
	    IUtilRS  * pUtil=NULL;
		II_AccuireFS01(pFS);
		II_AccuireUtilRS(pUtil);
		pUtil->SetFS(pFS);
		
		BOOL bAll = FALSE; // 全覆盖
		for(int i = 0;i<m_resGroup.items.size();i++)
		{
			std::string path = m_resGroup.items[i].path; 
			std::string suffix = m_resGroup.items[i].data->GetTypeSuffix();

			std::string filePath = path;
			MakeFileSuffix(filePath,m_resGroup.items[i].data->GetTypeSuffix());
			BOOL bExsit = pFS->ExistFile(filePath.c_str());
			
			if(suffix.compare("mtl")==0&&!_bOverwrite&&bExsit){ //材质存在并且采用不覆盖
				continue;
			}
			
			if(bExsit&&!bAll){
				CFourStateDlg dlg(filePath.c_str(),"IxEngine");
				dlg.SetInsteadFile(filePath.c_str());
				DWORD retID = dlg.DoModal();
				if(retID==CFourStateDlg::Ok){
					_SaveRes(pUtil,pFS,path.c_str(),m_resGroup.items[i].data);
				}
				else if(retID==CFourStateDlg::OkAll){
					bAll = TRUE;
					_SaveRes(pUtil,pFS,path.c_str(),m_resGroup.items[i].data);
				}
				else if(retID==CFourStateDlg::No){
					// do nothing
				}
				else if(retID==CFourStateDlg::Cancel){
					break;
				}
			}
			else{
				_SaveRes(pUtil,pFS,path.c_str(),m_resGroup.items[i].data);
			}
		}
}
BOOL CExportControl::_SaveRes(IUtilRS * pUtil,IFileSystem * pFS,const char * namePath,ResData * pData)
{
	if(!pUtil||!pFS)
		return FALSE;
	
	std::string path = namePath;
	path.append(".");
	path.append(pData->GetTypeSuffix());

	FileAttr atr = pFS->GetFileAttr(path.c_str());

	if(atr==File_ReadOnly){
		std::string msgDump;
		FormatString(msgDump,"文件{ %s }为只读文件,不能被覆盖 !",path.c_str());
		::MessageBox(NULL,msgDump.c_str(),NULL,MB_OK);
	}

	return pUtil->SaveRes(namePath,pData);
}
void  CExportControl::_TranslateNode(IGameScene *t_gameScene)
{
	int c_topNodeCount=t_gameScene->GetTopLevelNodeCount();
	for(int i=0;i<c_topNodeCount;i++)
	{
		IGameNode  * node = t_gameScene->GetTopLevelNode(i);
		if(node->IsTarget())
			continue;

		if(IsUserNode(node->GetMaxNode(),FALSE))
			continue;

		if(FALSE == _DoExportFromRoot(node))
			return;
	}
}
BOOL CExportControl::_CheckName(const char * name,DWORD handlerType)
{
	if(!name||strlen(name)<4)
		return FALSE;

	std::string t = name;
	if(t.substr(0,1).compare("{")!=0||t.substr(t.length()-1,1).compare("}")!=0)
		return FALSE;

	int pos = t.find_first_of("_",0);

	if(pos==std::string::npos)
		return FALSE;

	std::string t2 = t.substr(1,pos-1);

	if((t2.compare("sph")==0||t2.compare("bb")==0||t2.compare("pt")==0)&&handlerType==IExportHandler::DummiesHandler)
		return TRUE;

	if(t2.compare("mopp")==0&&handlerType==IExportHandler::MoppHandler)
		return TRUE;
	
	if(t2.compare("spg")==0&&handlerType==IExportHandler::SpgHandler)
		return TRUE;

	return FALSE;
}

BOOL CExportControl::_FecthSpgNode(GameNode ** node,IGameNode * gameNode/*,const std::string & nodePath*/)
{
	*node=NULL;
	int idx=m_spgContainer.find(gameNode->GetMaxNode());
	if(idx>=0) //已经在SPG 容器找到改节点
	{
		*node=m_spgContainer[idx];
		return TRUE;
	}
	
	std::string nameNode;
	nameNode = gameNode->GetName();
	IGameNode * parent = gameNode->GetNodeParent();

	//从命名规则上判断该节点是否是 Spg
	if(FALSE == _CheckName(nameNode.c_str(),IExportHandler::SpgHandler))
		return FALSE;
	
	SpgNode * spg=(SpgNode*)NodeFactory::newNode(GameNode::NODE_SPG);
	
	int p0 = nameNode.find_first_of("_",0)+1;
	int c = nameNode.length() - p0 -1;
	spg->m_nodePath= m_floderPath  + nameNode.substr(p0,c);

	spg->m_maxNode=gameNode->GetMaxNode();
	m_spgContainer.push_back(spg);
	*node=spg;

	return TRUE;
}	

//suceess return TRUE (if *t_node==NULL ,continue else return).
BOOL CExportControl::_FecthDummyNode(GameNode **node,IGameNode * gameNode/*,const std::string  &t_nodePath*/)
{
	//already in dummy container.
	*node=NULL;
	int idx=m_dummyContainer.find(gameNode->GetMaxNode());
	if(idx>=0)
	{
		*node=m_dummyContainer[idx];
		return TRUE;
	}
	
	std::string cNodeName;
	cNodeName= gameNode->GetName();
	IGameNode *parent= gameNode->GetNodeParent();

	//从命名规则上判断该节点是否是 Dummy
	if(FALSE == _CheckName(cNodeName.c_str(),IExportHandler::DummiesHandler))
		return FALSE;

			
	if(gameNode->GetChildCount())
	{
		LogFile::Prompt("[Dummy: %s]不能含有子节点",gameNode->GetName());
		return TRUE;
	}

	if((!parent))
	{
		LogFile::Prompt("[Dummy: %s]没有挂接到任何父节点上",gameNode->GetName());
		return TRUE;
	}

	DummyNode * dummy =(DummyNode*)NodeFactory::newNode(GameNode::NODE_DUMMY);
	dummy->m_nodePath =  gameNode->GetName();
	dummy->m_maxNode = gameNode->GetMaxNode();
	dummy->parentBone = parent->GetMaxNode();
	dummy->boneContainer = &m_boneContainer;
	dummy->owner = &m_dummyContainer;
	m_dummyContainer.push_back(dummy);
	*node = dummy;

	return  TRUE;
}

BOOL CExportControl::_FecthMoppNode(GameNode **t_node,IGameNode * gameNode/*,const std::string  &nodePath*/)
{
	*t_node = NULL;

	int idx=m_moppContainer.find(gameNode->GetMaxNode());
	if(idx>=0){
		*t_node=m_moppContainer[idx];
		return TRUE;
	}

	std::string nameNode;
	nameNode = gameNode->GetName();
	IGameNode *parent=gameNode->GetNodeParent();

	//从命名规则上判断该节点是否是 Mopp
	if(!_CheckName(nameNode.c_str(),IExportHandler::MoppHandler))
		return FALSE;


	MoppNode * mopp=(MoppNode*)NodeFactory::newNode(GameNode::NODE_MOPP);

	int p0 = nameNode.find_first_of("_",0)+1;
	int c = nameNode.length() - p0 -1;
	mopp->m_nodePath= m_floderPath + nameNode.substr(p0,c);
	
	mopp->m_maxNode=gameNode->GetMaxNode();
	m_moppContainer.push_back(mopp);
	*t_node=mopp;
	
	return TRUE;
}

GameNode* CExportControl::_FecthMeshNode(IGameNode * gameNode)
{
	GameNode  *node=NULL;
	
	//在Mopp资源容器中查找 如果该节点已经存在则返回 
	// TRUE : 改节点为Mopp节点
	if(TRUE == _FecthMoppNode(&node,gameNode))
		return node;
	
	if(TRUE==_FecthSpgNode(&node,gameNode))
		return node;

	//在Dummy资源容器中查找 如果该节点已经存在则返回 
	// TRUE : 改节点为Dummy节点
	if(TRUE == _FecthDummyNode(&node,gameNode))
		return node;

	if (std::string("{anchor}")==gameNode->GetName())
	{
		m_boneContainer.nodeAnchor=gameNode;
		return NULL;
	}
	if (std::string("{root}")==gameNode->GetName())
	{
		m_boneContainer.nodeOverrideRoot=gameNode;
		return NULL;
	}

	
	//在Mesh资源容器中查找 如果该节点已经存在则返回
	int c_index=m_nodeContainer.find(gameNode->GetMaxNode());
	if(c_index>=0) 
	{
		node=m_nodeContainer[c_index];
		return node;
	}


	
	// if not a dummy ,then put it into mesh node container.
	if(TRUE)
	{
		node=NodeFactory::newNode(GameNode::NODE_MESH);
		m_nodeContainer.push_back(node);
		node->m_maxNode=gameNode->GetMaxNode();
		node->m_type=GameNode::NODE_MESH;

		node->m_nodePath = m_floderPath;
		node->m_nodePath.append(gameNode->GetName());
	}
	
	//if not a sole node,also put it into a bone container . 
	
	if(gameNode->GetChildCount()||gameNode->GetNodeParent())
	{
		BoneNode * newboneNode=(BoneNode *)NodeFactory::newNode(GameNode::NODE_BONE);
		newboneNode->m_boneContainer=&m_boneContainer;
		//decide the bone type
		if(TRUE)
		{
			std::string cNodeName=gameNode->GetName();
			if(cNodeName.substr(0,1)=="{"&&cNodeName.substr(cNodeName.length()-1,1)=="}")
				newboneNode->m_type=GameNode::NODE_DUMMY;
			else
				newboneNode->m_type=GameNode::NODE_MESH;
		}				
		newboneNode->m_refContainer=&m_nodeContainer;
		newboneNode->m_nodePath= m_floderPath + gameNode->GetName()/*+C_RES_LINKER*/;
		newboneNode->m_maxNode=gameNode->GetMaxNode();
		newboneNode->m_boneInfo.flag=BoneInfo::ForceDword;
		newboneNode->m_gameNode= gameNode;
		newboneNode->m_object= gameNode->GetIGameObject();
		Matrix3 objMat_max = CMatrixUtil::ToLHS(gameNode->GetObjectTM(0).ExtractMatrix3());
		i_math::matrix43f objMat;
		matrix43f_From_Matrix3(objMat,objMat_max);
		newboneNode->m_boneInfo.matOff = objMat;
		strcpy(newboneNode->m_boneInfo.name,gameNode->GetName());
		m_boneContainer.push_back(newboneNode);
	}

	return  node;
}

GameNode * CExportControl::_FecthBoneNode(IGameNode * gameNode)
{
		BoneNode *  node = NULL;
		int index=m_boneContainer.find(gameNode->GetMaxNode());
		if(index>=0){
			node=(BoneNode *)m_boneContainer[index];
			return node;
		}
		
		//check if it is a dummy,the node name has the feature like this (node name) and has parent but no childerns.
		if(TRUE)
		{
			GameNode *node=NULL;
			if(TRUE == _FecthDummyNode(&node,gameNode))
				return node;
		}

		//not found the bone in bone container,then add a new.
		if(TRUE)
		{
			std::string name = gameNode->GetName();
			int ret=m_boneContainer.find(name.c_str());
			if(ret!=-1)  
			{
				LogFile::Prompt("[Bone: %s]存在两个节点重名。",gameNode->GetName());
				return NULL;
			}
			node=(BoneNode *)NodeFactory::newNode(GameNode::NODE_BONE);
			m_boneContainer.push_back(node);
			node->m_maxNode = gameNode->GetMaxNode();
			node->m_nodePath = m_floderPath + name;
			strcpy(node->m_boneInfo.name,name.c_str());
			node->m_boneContainer = &m_boneContainer;
			node->m_refContainer  = &m_nodeContainer;
			node->m_boneInfo.flag=BoneInfo::ForceDword;
		}

		return node;
}
BOOL CExportControl::_IsSole(GameNode * node)
{
	IGameNode * gameNode = node->m_gameNode;
	assert(gameNode);

	//含有父节点 则必定受骨骼影响
	if(gameNode->GetNodeParent())
		return FALSE;

	DWORD sz = gameNode->GetChildCount();
	
	//如果含有子节点 且 子节点不是Dummy UserData Mopp 则说明受骨骼影响
	for(int i = 0;i<sz;i++)
	{
		IGameNode * tn = gameNode->GetNodeChild(i);
		INode * maxNode = tn->GetMaxNode();

		if(m_moppContainer.getNode(maxNode)) 
			continue;
		if(m_dummyContainer.getNode(maxNode)) 
			continue;
		if(IsUserNode(tn->GetMaxNode(),FALSE))
			continue;
		return FALSE;
	}

	return TRUE;
}
// the bind funcion is very important, the node in mesh container has a pointer to the bone node.
// bind will integer the information that mesh data need such as bone index ,skeletonInfo.
void	CExportControl::_Bind()
{
#pragma warning(disable:4312)
	_MakeSkeleton();
	 //pool information from m_boneContainer.
	for(int i=0;i<m_nodeContainer.size();i++)
	{
		 MeshNode * mesh = (MeshNode *)m_nodeContainer[i];
		 if(!(mesh->m_boneAffect))
			 continue;
		
		 MeshData::VtxData *pVtxData = NULL;
		 IGameSkin *pSkin = NULL;

		 for(int mc = 0; mc < mesh->m_subMeshContainer.size();mc++)
		 {
			 MeshData  * meshData = (mesh->m_subMeshContainer[mc]).pmesh;
			 if(meshData->vtxframes.empty()) 
				 continue;

			pVtxData = &((meshData->vtxframes)[0]);
		
			//to check the mesh whether be affected by one skeleton.
			int  sklID = -1;  
			pSkin = mesh->m_object->GetIGameSkin();

			if(!_IsSole(mesh))
			{		
			   for(int n = 0; n<meshData->vtxframes.m_nVtx; ++n)
			   {
				    INode * maxNode = (INode *)pVtxData->boneindex0[n];
					BoneNode  *boneNode = (BoneNode*)m_boneContainer.getNode(maxNode);
					if(!boneNode) 
						continue;

					WORD idx = 0;
					for(;idx<meshData->bones.size();++idx){
					  if(meshData->bones[idx]==(boneNode->m_boneId)){
						  pVtxData->boneindex0[n] = idx;
						break;
					  }
					}

					if(idx>=meshData->bones.size()){
					  pVtxData->boneindex0[n] = meshData->bones.size();
					  meshData->bones.push_back(boneNode->m_boneId);
					}
					
					if(sklID<0)
						sklID = boneNode->m_skeletonId;
					else{
						if(sklID !=boneNode->m_skeletonId)
							continue;
					}
			   }
			   meshData->nSkeletonBones=_FindBoneCount(sklID);
			 }       // the mesh be affected by skin.
			 else if(pSkin&&meshData->nWeight>0)
			 {		
				 for(int n =0;n<meshData->vtxframes.m_nVtx;n++){
					for(int c = 0;c < meshData->nWeight;c++)
					{
						INode  *maxNode = (INode *)pVtxData->boneindex0[(meshData->nWeight)*n+c];
						BoneNode *boneNode=(BoneNode *)m_boneContainer.getNode(maxNode);
						if(!boneNode)  	
							continue;
							
						WORD idx = 0;
						for(; idx<meshData->bones.size();++idx){
							if(meshData->bones[idx]==boneNode->m_boneId){
								pVtxData->boneindex0[(meshData->nWeight)*n+c]=idx;
								break;
							}
						}
						
						if(idx>=meshData->bones.size()){
							pVtxData->boneindex0[(meshData->nWeight)*n+c] = meshData->bones.size();
							meshData->bones.push_back(boneNode->m_boneId);
						}

						//check the bone system be legal.
						if(sklID<0) 
							sklID = boneNode->m_skeletonId;

						if(TRUE){
							i_math::matrix43f meshWld,boneWld;
							GMatrix wm = mesh->m_gameNode->GetWorldTM();
							Matrix3 m43mesh = CMatrixUtil::ToLHS(wm.ExtractMatrix3());
							memcpy(&(meshWld),m43mesh.GetAddr(),sizeof(i_math::matrix43f));
							boneNode->m_boneInfo.matOff = meshWld;
						}
					}
				 }

				meshData->nSkeletonBones = _FindBoneCount(sklID);
				
				for(int i = 0; i< meshData->vtxframes.m_nVtx;++i){
					BYTE  indice[4] = {0};
					memset(indice,0,sizeof(indice));
					for(int b=0;b<meshData->nWeight;b++)
						indice[b] =  pVtxData->boneindex0[(meshData->nWeight)*i+b];
					memcpy(pVtxData->boneindex0+i,indice,sizeof(indice));
				}
			 }
			 else
			 {
				 SAFE_DELETE(pVtxData->boneindex0);
				 meshData->nWeight = 0;
				 meshData->nSkeletonBones = 0;
				 mesh->m_boneAffect = FALSE;
			 }
			
			//复制
			for(int f=1;f<meshData->vtxframes.size();f++){
				MeshData::VtxData * pVtxData1 = &((meshData->vtxframes)[f]);
				if(!_IsSole(mesh)) 
					memcpy(pVtxData1->boneindex0,pVtxData->boneindex0,(meshData->vtxframes.m_nVtx)*sizeof(DWORD));
				else
					memcpy(pVtxData1->boneindex0,pVtxData->boneindex0,(meshData->nWeight)*(meshData->vtxframes.m_nVtx)*sizeof(DWORD));
			}
		}
	}
}


//the function will seperate the bone node by skeleton id .
//bones have the same skeleton id indicates in the same skeleton.
void  CExportControl::_MakeSkeleton()
{
		int sklID=0;
		//split the bone node by skeleton id.
	 	for(int i=0;i<m_boneContainer.size();i++)
		{
			 int boneID =0;
			 BoneNode  * cur=(BoneNode *)m_boneContainer[i];
			 BoneNode  * curp=NULL;
			 if(cur->m_skeletonId<0) {
				  int p = cur->m_boneInfo.iParent;

				  if(p >= 0){
					  curp=(BoneNode *)m_boneContainer[p];
					  if(curp->m_skeletonId>=0)
						  cur->m_skeletonId= curp->m_skeletonId;
					  else{
						  cur->m_skeletonId = sklID;
						  curp->m_skeletonId = sklID;
						  sklID++;
					  }	
				  }
				  else{
					  cur->m_skeletonId = sklID;
					  ++sklID;
				  }
			 }
			 else{
				 int p = cur->m_boneInfo.iParent;
				 if(p>=0)
				 {
					 curp=(BoneNode *)m_boneContainer[p];
					 if(curp->m_skeletonId<0)
						 curp->m_skeletonId = cur->m_skeletonId;
					 else{
						int m = max(cur->m_skeletonId,curp->m_skeletonId);
						int n = min(cur->m_skeletonId,curp->m_skeletonId);
						for(int r = 0; r<m_boneContainer.size(); ++r){
							BoneNode * b = (BoneNode*)m_boneContainer[r];
							if(b->m_skeletonId==m)
								b->m_skeletonId=n;
						}
					 }	
				 }
			 }     
		}
		//  generate bone id by skeleton id.
		int * pBoneId=new int[sklID];
		memset(pBoneId,0,sklID* sizeof(int));
		for(int r = 0; r<m_boneContainer.size(); ++r){
			BoneNode * cur = (BoneNode *)m_boneContainer[r];
			cur->m_boneId = (pBoneId[cur->m_skeletonId])++;
		}
		delete [] pBoneId;	

		//  generate parent id  according to bone id.
		for(int r = 0;r<m_boneContainer.size(); ++r){
			 BoneNode * cur = (BoneNode *)m_boneContainer[r];
			 int p = cur->m_boneInfo.iParent;
			 if(p<0) 
				 continue;
			 BoneNode * curP = (BoneNode *)m_boneContainer[p];
			 cur->m_boneInfo.iParent = curP->m_boneId;
		}
}

int   CExportControl::_FindBoneCount(int t_skeletonId)
{	
	 int count = 0;

	 for(int r=0;r<m_boneContainer.size();r++){
		 BoneNode *node =(BoneNode *)m_boneContainer[r];
			if(node->m_skeletonId==t_skeletonId) 
				++count;
	 }

	 return  count;
}

void CExportControl::_AfterExport()
{
	//binding ,integer the information bone and mesh.
	//collect mesh resource .
	IExportHandler  * handler=NULL;

	//材质
	if(GetConfig().bExpMtrl){
		_mtrlExp.Clean();
		for(int i=0;i<m_nodeContainer.size();i++)
		{
			if(FALSE==_IsEnable(m_nodeContainer[i]))
				continue;
			_mtrlExp.DoExport(m_nodeContainer[i],0);
		}
	}
	
	//节点动画
	if(GetConfig().bExpAXfm){
		_xfmExp.Clean();
		for(int i=0;i<m_nodeContainer.size();i++){
			if(FALSE==_IsEnable(m_nodeContainer[i]))
				continue;
			_xfmExp.DoExport(m_nodeContainer[i],0);
		}
	}

	//挂接点
	if(GetConfig().bExpDummy){
		_dummiesExp.Clean();
		for(int i=0;i<m_dummyContainer.size();i++){
			if(FALSE==_IsEnable(m_dummyContainer[i]))
				continue;
			_dummiesExp.DoExport(m_dummyContainer[i],0);
		}
	}
	
	//纹理动画
	if(GetConfig().bExpAUv){
		_animUVExp.Clean();
		for(int i = 0;i< m_nodeContainer.size();i++){
			if(FALSE==_IsEnable(m_nodeContainer[i]))
				continue;
			_animUVExp.DoExport(m_nodeContainer[i],0);
		}
	}
}

void  CExportControl::CollectRes()
{
	_Bind();
	_AfterExport();

	IExportHandler  * handler = NULL;

	//骨骼动画
	if(m_boneContainer.size()&&GetConfig().bExpABone)
	{
		_boneExp.Packet(m_resGroup,m_boneContainer);
	}

	//模型
	if(m_nodeContainer.size()&&(GetConfig().bExpDtr||GetConfig().bExpMesh))
	{
		_meshExp.Packet(m_resGroup,m_nodeContainer);
	}
	
	//材质	
	if(GetConfig().bExpMtrl){
		_mtrlExp.Packet(m_resGroup,m_nodeContainer);//NULL
	}
	
	//节点动画	
	if(GetConfig().bExpAXfm){
		_xfmExp.Packet(m_resGroup,m_nodeContainer);//NULL
	}

	//挂节点
	if(m_dummyContainer.size()&&GetConfig().bExpDummy){
		_dummiesExp.Packet(m_resGroup,m_dummyContainer);
	}

	//纹理动画
	if(m_nodeContainer.size()&&GetConfig().bExpAUv){
		_animUVExp.Packet(m_resGroup,m_nodeContainer);
	}

	//物理简模
	if(m_moppContainer.size()&&GetConfig().bExpMopp){
		_moppExp.Packet(m_resGroup,m_moppContainer);
	}
	
	//小植被
	if(m_spgContainer.size()&&GetConfig().bExpSpg){
		_spgExp.Packet(m_resGroup,m_spgContainer);
	}
}

const GameNodeContainer * CExportControl::GetBoneNodes(IGameScene * gameScene)
{
	DoExport(gameScene,0);
	_MakeSkeleton();
	return &m_boneContainer;
}








