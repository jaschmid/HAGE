#include "header.h"
#include "DataProcessor.h"
#include "GenerationDomain.h"

namespace HAGE
{
	DataProcessor::DataProcessor(u32 xStart,u32 xEnd,u32 yStart,u32 yEnd) :
		_bDone(false),
		_xBegin(xStart),_xEnd(xEnd),
		_yBegin(yStart),_yEnd(yEnd)
	{
		hsvt.New("landscape.hsvt",128,9,4);

		for(u32 y=yStart;y<yEnd;++y)
			for(u32 x=xStart;x<xEnd;++x)
				_queuedItems.push_back(DataItem(x,y));
	}

	DataProcessor::~DataProcessor()
	{
	}
	
	bool DataProcessor::Process()
	{
		if(_bDone)
			return true;

		while(_queuedItems.size() > 0  &&  _loadingItems.size() <4 )
		{
			_loadingItems.push_back(_queuedItems.front());
			_queuedItems.pop_front();
		}

		for(auto it = _loadingItems.begin();it!=_loadingItems.end();)
		{
			it->TryLoad();
			if(it->IsReady())
			{
				ProcessMesh(*it);
				ProcessTexture(*it);
				auto last = it;
				++it;
				_loadingItems.erase(last);
			}
			else
				++it;
		}

		if(_loadingItems.empty() && _queuedItems.empty() )
		{
			Finalize();
			return true;
		}

		return false;
	}

	void DataProcessor::Finalize()
	{
		printf("Generate HSVT\n");
		hsvt.Commit();
		printf("Done HSVT\n");
		printf("Generate HGEO\n");
		mergeMeshVertices();
		writeOutputMesh();
		printf("Done HGEO\n");
		_bDone = true;
	}
	
	void DataProcessor::ProcessMesh(const DataItem& item)
	{/*
		const u8* pData;
		u32 nIndices = item.GetMeshData()->GetIndexData(&pData);

		u32 preVertices = (u32)_mesh.GetNumVertexIndices();

		_mesh.ImportRawIndexData<MeshType::RAW_INDEX_TRIANGLE_LIST,u32>(nIndices/3,(const u32*)pData);

		const u8* positionData;
		u32 stride;
		u32 nVertices = item.GetMeshData()->GetVertexData((const u8**)&positionData,stride,IMeshData::POSITION);

		Vector3<> offset = Vector3<>( (float)(item.GetX() - (_xEnd + _xBegin) / 2) * 2.0f, 0.0f , (float)(item.GetY() - (_yEnd + _yBegin) / 2) * 2.0f);

		for(u32 i = 0; i < nVertices; ++i)
			_mesh.GetVertexData(_mesh.GetVertex(i + preVertices)) = *(const Vector3<>*)&positionData[stride*i] + offset;
		printf("mesh loaded\n");
		printf("%i vertices\n",_mesh.GetNumVertices());
		printf("%i faces\n",_mesh.GetNumFaces());
		printf("%i edges\n",_mesh.GetNumEdges());*/
	}

	void DataProcessor::ProcessTexture(const DataItem& item)
	{
		static int numLoaded = 0;
		++numLoaded;
		char temp[256];
		sprintf(temp,"@world.MPQ\\world\\maps\\Azeroth\\Azeroth_%i_%i_tex0.adt",item.GetX(),item.GetY());
		const TResourceAccess<IImageData>& curr = item.GetTextureData();
		printf("texture data \"%s\" ready!\n",temp);
		hsvt.WriteImageToVirtualTexture((item.GetX()-_xBegin)*4096,(item.GetY()-_yBegin)*4096,curr->GetImageWidth(),curr->GetImageHeight(),curr->GetImageData());
		printf("texture data \"%s\" processed!\n",temp);
		printf("%i of %i \n",numLoaded,(_xEnd - _xBegin)*(_yEnd-_yBegin));
	}

	void DataProcessor::writeOutputMesh()
	{
		printf("Writing...\n");
		FILE* f = fopen("landscape.hgeo","wb");
		char header[] = "HGEO";
		fwrite(header,4,1,f);

		_mesh.Compact();

		u64 nVertices = _mesh.GetNumVertices();
		u64 nPrimitives = _mesh.GetNumFaces();
			
		fwrite(&nVertices,sizeof(u32),1,f);
		for(u64 i = 0; i< nVertices; ++i)
		{
			Vector3<> out = _mesh.GetVertex(i);
			fwrite(&out,sizeof(Vector3<>),1,f);
			Vector2<> texcoord = Vector2<>((out.x-min.x)/(max.x-min.x),(out.z-min.z)/(max.z-min.z));
			fwrite(&texcoord,sizeof(Vector2<>),1,f);
		}
			
		fwrite(&nPrimitives,sizeof(u32),1,f);
		for(u64 i = 0; i< nPrimitives; ++i)
		{
			MeshType::VertexTriple vt = _mesh.GetFaceVertices(_mesh.GetFace(i));
			u32 index[3];
			for(int i2 = 0; i2 <3;++i2)
				index[i2] =	(u32)_mesh.GetIndex(vt[i2]);
			fwrite(index,3*sizeof(u32),1,f);
		}

		fclose(f);
		printf("Done Writing\n");

	}

	void DataProcessor::mergeMeshVertices()
	{
		printf("Merging...\n");
		Vector3<> first = _mesh.GetVertexData(_mesh.GetVertex(0));
		min=first;max=first;
		for(int i = 1; i < _mesh.GetNumVertexIndices(); ++i)
		{
			Vector3<> v = _mesh.GetVertexData(_mesh.GetVertex(i));
			if(v.x < min.x)
				min.x = v.x;
			if(v.x > max.x)
				max.x = v.x;
			if(v.y < min.y)
				min.y = v.y;
			if(v.y > max.y)
				max.y = v.y;
			if(v.z < min.z)
				min.z = v.z;
			if(v.z > max.z)
				max.z = v.z;
		}

		typedef SpatialTree<MeshType::Vertex> myTree;

		myTree tree(myTree::SplitterType(min,max));

		Vector3<> test1 = _mesh.GetVertexData(_mesh.GetVertex(8));
		Vector3<> test2 = _mesh.GetVertexData(_mesh.GetVertex(9*9+8*8));
		for(int i = 0; i < _mesh.GetNumVertexIndices(); ++i)
		{
			MeshType::Vertex v = _mesh.GetVertex(i);
			if(v!=MeshType::nullVertex)
				tree.Insert(v);
		}

		const myTree::TreeNode* cur = tree.TopNode();
		while(cur)
		{
			if(cur->IsLeaf())
			{
				auto elements = cur->LeafElements();
				for(size_t i = 0; i < elements.size(); ++i)
					for(size_t i2 = i+1; i2 < elements.size(); ++i2)
					{
						MeshType::Vertex v1 = elements[i];
						MeshType::Vertex v2 = elements[i2];
						if(v1 != MeshType::nullVertex && v2 != MeshType::nullVertex)
						{
							Vector3<> pos1 = _mesh.GetVertexData(v1);
							Vector3<> pos2 = _mesh.GetVertexData(v2);
							float distance_sq = !( pos1 - pos2);
							if(distance_sq < 0.000001f)
							{
								assert(_mesh.MergeVertex(_mesh.MakePair(v1,v2)) != MeshType::nullVertex);
								break;
							}
						}
					}
			}
			cur = cur->GetNextNode();
		}
					
		_mesh.DebugValidateMesh();

		printf("vertices merged\n");
		printf("%i vertices\n",_mesh.GetNumVertices());
		printf("%i faces\n",_mesh.GetNumFaces());
		printf("%i edges\n",_mesh.GetNumEdges());
	}

	DataProcessor::DataItem::DataItem(u32 x,u32 y) :
	_x(x),_y(y),
	_bDone(false),_bLoading(false)
	{
	}

	DataProcessor::DataItem::~DataItem()
	{
	}

	void DataProcessor::DataItem::TryLoad()
	{
		if(!_bLoading)
		{
			char temp[256];
			sprintf(temp,"@world.MPQ\\world\\maps\\Azeroth\\Azeroth_%i_%i.adt",GetX(),GetY());
			_mesh = GetResource()->OpenResource<IMeshData>(temp);
			sprintf(temp,"@world.MPQ\\world\\maps\\Azeroth\\Azeroth_%i_%i_tex0.adt",GetX(),GetY());
			_texture = GetResource()->OpenResource<IImageData>(temp);
			_bLoading = true;
		}
		else if(_mesh.getStage() > 0 && _texture.getStage() > 0)
			_bDone = true;
	}
}