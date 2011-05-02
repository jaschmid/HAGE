#include "header.h"
#include "DataProcessor.h"
#include "GenerationDomain.h"
#include <SDK/EditableImage.h>
#include <fstream>
#include "../Source/Core/SVTPageEncoding.h"

namespace HAGE
{
	DataProcessor::DataProcessor(u32 xStart,u32 xEnd,u32 yStart,u32 yEnd) :
		_bDone(false),
		_xBegin(xStart),_xEnd(xEnd),
		_yBegin(yStart),_yEnd(yEnd)
	{
		hsvt.New(128,11,4);

		for(u32 y=yStart;y<yEnd;++y)
			for(u32 x=xStart;x<xEnd;++x)
			{
				printf("queued: %i (%i) %i (%i)\n",x,xEnd,y,yEnd);
				_queuedItems.push_back(DataItem(x,y));
			}
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
				if(bProcessMesh)
					ProcessMesh(*it);
				if(bProcessTexture)
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

		hsvt.FinalizePlacement();

		if(bProcessMesh)
		{

			printf("Generate HGEO\n");
			_mesh.Compact();
			
			printf("Updating Texture coordinates\n");
			
			for(int i = 0; i < _mesh.GetNumVertexIndices(); ++i)
			{
				MeshType::Vertex v = _mesh.GetVertex(i);
				u32 mat = v->Material;

				Vector2<> scale(materials[mat].GetUSize(),materials[mat].GetVSize());
				Vector2<> bias(materials[mat].GetUBegin(),materials[mat].GetVBegin());
					
				Vector2<>& texcoord = v->TexCoord;

				if(texcoord.x > 1.0f || texcoord.x < 0.0f)
					texcoord.x -= floor(texcoord.x);

				if(texcoord.y > 1.0f || texcoord.y < 0.0f)
					texcoord.y -= floor(texcoord.y);
				/*
				assert(texcoord.x >= 0.0f && texcoord.x <= 1.0f);
				assert(texcoord.y >= 0.0f && texcoord.y <= 1.0f);*/
				texcoord.x *= scale.x;
				texcoord.y *= scale.y;
				texcoord.x += bias.x;
				texcoord.y += bias.y;
			}


			//mergeMeshVertices(_mesh);
			printf("Reduce Poly Count'from %i\n",_mesh.GetNumFaces());
			_mesh.InitializeDecimate();
			_mesh.DecimateToError(0.0001f,DecimateUpdate);
			_mesh.Compact();
			printf("Reduced Poly Count to %i\n",_mesh.GetNumFaces());
			writeOutputMesh();
			printf("Done HGEO\n");
		}
		if(bProcessTexture)
		{
			printf("Generate HSVT\n");
			hsvt.Commit();
			printf("Saving HSVT\n");
			hsvt.Save("landscape.hsvt");
			printf("Done HSVT\n");
		}
		_bDone = true;
	}
	
	struct MinMaxTex
	{
		Vector2<> min;
		Vector2<> max;
		std::array<Vector2<>,2> adjustment;
		bool set;
	};

	void DataProcessor::loadMesh(DataProcessor::MeshType& mesh,const TResourceAccess<IMeshData>& data,const Matrix4<>& transform)
	{
		const u8* pIndexData;
		u32 nIndices = data->GetIndexData(&pIndexData);
		mesh.ImportRawIndexData<MeshType::RAW_INDEX_TRIANGLE_LIST,u32>(nIndices/3,(const u32*)pIndexData);

		const u8* positionData;
		const u8* texcoordData;
		u32 stridePos;
		u32 strideTex;
		u32 nVertices = data->GetVertexData((const u8**)&positionData,stridePos,IMeshData::POSITION);
		data->GetVertexData((const u8**)&texcoordData,strideTex,IMeshData::TEXCOORD0);

		u32 textureIndexOffset = materials.size();

		

		for(u32 i = 0; i < nVertices; ++i)
		{
			MeshType::Vertex v = mesh.GetVertex(i);
			if(v == MeshType::nullVertex)
				break;
			v->Position = (transform*Vector4<>(*(const Vector3<>*)&positionData[stridePos*i],1.0f)).xyz();
			v->TexCoord = *(const Vector2<>*)&texcoordData[strideTex*i];
			v->Material = textureIndexOffset;
		}

		for(u32 i = nVertices; i < mesh.GetNumVertexIndices(); ++i)
		{
			MeshType::Vertex v = mesh.GetVertex(i);
			if(v == MeshType::nullVertex)
				break;
			v->Material = textureIndexOffset;
		}

		u32 maxMaterial = 1;

		const u32* materialData = (const u32*)	data->GetExtendedData(IMeshData::TRIANGLE_MATERIAL_INFO);
		if(materialData)
		{
			const u32* piData = (const u32*)pIndexData;
			for(int i = 0; i < nIndices/3; ++i)
				for(int i2 = 0; i2 < 3 ; i2++)
				{
					MeshType::Vertex v = mesh.GetVertex(piData[i*3+i2]);
					assert(v->Material == textureIndexOffset || v->Material == materialData[i] +textureIndexOffset);
					v->Material = materialData[i] +textureIndexOffset;
					if(materialData[i] >= maxMaterial)
						maxMaterial = materialData[i] + 1;
				}
		}


		std::vector<MinMaxTex> textureMinMax;
		textureMinMax.resize(maxMaterial);

		//set texture minmax to unlikely numbers
		memset(textureMinMax.data(),0,sizeof(MinMaxTex)*textureMinMax.size());

		//get minmax values
		for(u32 i = 0; i < mesh.GetNumVertexIndices(); ++i)
		{
			MeshType::Vertex v = mesh.GetVertex(i);
			if(v == MeshType::nullVertex)
				break;
			Vector2<>& texcoords = v->TexCoord;
			MinMaxTex& minmax= textureMinMax[v->Material-textureIndexOffset];

			if(minmax.min.x > texcoords.x || !minmax.set)
				minmax.min.x = texcoords.x;
			if(minmax.min.y > texcoords.y || !minmax.set)
				minmax.min.y = texcoords.y;
			if(minmax.max.x < texcoords.x || !minmax.set)
				minmax.max.x = texcoords.x;
			if(minmax.max.y < texcoords.y || !minmax.set)
				minmax.max.y = texcoords.y;

			minmax.set = true;
		}

		const TResourceAccess<IImageData>* curr = nullptr;
		int index = 0;

		//put the textures into the hsvt generator
		while((curr = (const TResourceAccess<IImageData>*) data->GetTexture(index)) != nullptr && index < textureMinMax.size())
		{
			assert( materials.size() - textureIndexOffset == index );

			SparseVirtualTextureGenerator::RelationArray arr;
			u32 xSize = (*curr)->GetImageWidth();
			u32 ySize = (*curr)->GetImageHeight();
			const u8* data = (const u8*)(*curr)->GetImageData();
			
			if((*curr)->GetImageFormat() == IImageData::R8G8B8A8)
			{
				textureMinMax[index].adjustment = packTexture(textureMinMax[index].min,textureMinMax[index].max,xSize,ySize,(const u32*)data,arr);
			}
			else if((*curr)->GetImageFormat() == IImageData::DXTC1)
			{
				ImageData<DXTC1> compressed(xSize,ySize,data);
				ImageData<R8G8B8A8> decompressed(compressed);
				textureMinMax[index].adjustment = packTexture(textureMinMax[index].min,textureMinMax[index].max,xSize,ySize,(const u32*)decompressed.GetData(),arr);
			}
			else if((*curr)->GetImageFormat() == IImageData::DXTC3)
			{
				ImageData<DXTC3> compressed(xSize,ySize,data);
				ImageData<R8G8B8A8> decompressed(compressed);
				textureMinMax[index].adjustment = packTexture(textureMinMax[index].min,textureMinMax[index].max,xSize,ySize,(const u32*)decompressed.GetData(),arr);
			}
			else if((*curr)->GetImageFormat() == IImageData::DXTC5)
			{
				ImageData<DXTC5> compressed(xSize,ySize,data);
				ImageData<R8G8B8A8> decompressed(compressed);
				textureMinMax[index].adjustment = packTexture(textureMinMax[index].min,textureMinMax[index].max,xSize,ySize,(const u32*)decompressed.GetData(),arr);
			}
			else
			{
				std::vector<u32> tempBuffer;
				tempBuffer.resize(xSize*ySize);
				textureMinMax[index].adjustment = packTexture(textureMinMax[index].min,textureMinMax[index].max,xSize,ySize,(const u32*)tempBuffer.data(),arr);
			}

			++index;
		}
		
		//adjust coordinates
		for(u32 i = 0; i < mesh.GetNumVertexIndices(); ++i)
		{
			MeshType::Vertex v = mesh.GetVertex(i);
			if(v == MeshType::nullVertex)
				break;

			Vector2<>& texcoords = v->TexCoord;
			MinMaxTex& minmax= textureMinMax[v->Material-textureIndexOffset];
			
			texcoords = (( ( texcoords - minmax.min ) | ( minmax.max - minmax.min) ) & ( minmax.adjustment[1] - minmax.adjustment[0]) ) + minmax.adjustment[0];
		}

	}

	
	std::array<Vector2<>,2> DataProcessor::packTexture(Vector2<> mincoord,Vector2<> maxcoord,u32 xSize,u32 ySize,const u32* pData,const SparseVirtualTextureGenerator::RelationArray& arr)
	{

		std::array<Vector2<>,2> result;

		i32 xBegin = floorf(xSize * mincoord.x - 0.5f);
		i32 yBegin = floorf(ySize * mincoord.y - 0.5f);
		i32 xEnd = ceilf(xSize * maxcoord.x + 0.5f);
		i32 yEnd = ceilf(ySize * maxcoord.y + 0.5f);
		
		u32 newXSize = xEnd- xBegin;
		u32 newYSize = yEnd- yBegin;

		result[0] = Vector2<>((mincoord.x*(f32)xSize - (f32)xBegin)/(f32)newXSize,(mincoord.y*(f32)ySize - (f32)yBegin)/(f32)newYSize);
		result[1] = Vector2<>((maxcoord.x*(f32)xSize - (f32)xBegin)/(f32)newXSize,(maxcoord.y*(f32)ySize - (f32)yBegin)/(f32)newYSize);

		assert(result[0].x >= 0.0f && result[0].x <= 1.0f);
		assert(result[0].y >= 0.0f && result[0].y <= 1.0f);
		assert(result[1].x >= 0.0f && result[1].x <= 1.0f);
		assert(result[1].y >= 0.0f && result[1].y <= 1.0f);
		
		ImageData<R8G8B8A8> image_data(newXSize,newYSize);


		for(i32 iy = yBegin; iy < yEnd ; iy++)
			for(i32 ix = xBegin; ix < xEnd ; ix++)
				image_data(ix-xBegin,iy-yBegin).SetData(pData[ (iy%ySize)*xSize + (ix%xSize) ]);

		SVTDataLayer_PNGish raw_image;
		raw_image.Initialize(image_data);
		materials.push_back(hsvt.PlaceTexture(&raw_image,arr));

		return result;
	}

	void DataProcessor::ProcessMesh(const DataItem& item)
	{
		
		Vector3<> offset = Vector3<>(0.0f,0.0f,0.0f);//Vector3<>( (float)((float)item.GetX() - (float)(_xEnd + _xBegin) / 2) * 2.0f, 0.0f , (float)((float)item.GetY() - (float)(_yEnd + _yBegin) / 2) * 2.0f);
		
		Matrix4<> baseTransform = Matrix4<>::Translate(offset);

		MeshType mesh;

		loadMesh(mesh,item.GetMeshData(),baseTransform);

		printf("height mesh loaded\n");
		printf("\t%i vertices\n",mesh.GetNumVertices());
		printf("\t%i faces\n",mesh.GetNumFaces());
		printf("\t%i edges\n",mesh.GetNumEdges());
		
		mergeMeshVertices(mesh);
		mesh.Compact();
		mesh.InitializeDecimate();
		mesh.DecimateToError(0.000001f,DecimateUpdate);
		mesh.Compact();
		
		u32 nChildMeshes = 0;

		const u32* pnChildMeshes = (const u32*)item.GetMeshData()->GetExtendedData(IMeshData::CHILD_COUNT);
		if(pnChildMeshes)
			nChildMeshes = *pnChildMeshes;

		const IMeshData::ChildObject* pChildMeshes = (const IMeshData::ChildObject*) item.GetMeshData()->GetExtendedData(IMeshData::CHILD_LIST);
		for(int i = 0; i < nChildMeshes; ++i)
		{
			MeshType child;
			Matrix4<> childTransform = baseTransform  * pChildMeshes[i].transformation;
			loadMesh(child,pChildMeshes[i].childMesh,childTransform);
			child.DebugValidateMesh();
			mesh.ImportMesh(child);
			mesh.DebugValidateMesh();
		}

		
		printf("child meshes loaded\n");
		printf("\t%i vertices\n",mesh.GetNumVertices());
		printf("\t%i faces\n",mesh.GetNumFaces());
		printf("\t%i edges\n",mesh.GetNumEdges());
		


		_mesh.ImportMesh(mesh);
	}

	void DataProcessor::ProcessTexture(const DataItem& item)
	{
		//this is no longer needed ProcessMesh now does everything

		/*
		static int numLoaded = 0;
		++numLoaded;
		char temp[256];
		sprintf(temp,"@world.MPQ\\world\\maps\\Azeroth\\Azeroth_%i_%i_tex0.adt",item.GetX(),item.GetY());
		const TResourceAccess<IImageData>& curr = *item.GetMeshData()->GetTexture(0);
		printf("texture data \"%s\" ready!\n",temp);
		hsvt.WriteImageToVirtualTexture((item.GetX()-_xBegin)*4096,(item.GetY()-_yBegin)*4096,curr->GetImageWidth(),curr->GetImageHeight(),curr->GetImageData());
		printf("texture data \"%s\" processed!\n",temp);
		printf("%i of %i \n",numLoaded,(_xEnd - _xBegin)*(_yEnd-_yBegin));*/
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
			
		//get etents
		Vector3<> min,max;//extents of the mesh
		Vector3<> first = _mesh.GetVertexData(_mesh.GetVertex(0)).Position;
		min=first;max=first;
		for(int i = 1; i < _mesh.GetNumVertexIndices(); ++i)
		{
			Vector3<> v = _mesh.GetVertexData(_mesh.GetVertex(i)).Position;
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

		fwrite(&nVertices,sizeof(u32),1,f);
		for(u64 i = 0; i< nVertices; ++i)
		{
			Vector3<> out = _mesh.GetVertex(i);
			fwrite(&out,sizeof(Vector3<>),1,f);
			Vector2<> texcoord = _mesh.GetVertex(i)->TexCoord;
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
		/*
		std::fstream os;

		os.open ("landscape.obj", std::fstream::out);

	    os << "# HalfEdgeMesh obj streamer\n# M&A 2008\n\n";
		os << "# Vertices\n";
		for(unsigned int i=0; i<_mesh.GetNumVertexIndices(); i++){
			os << "v " << 
				_mesh.GetVertex(i)->Position.x
			<< " " << 
				_mesh.GetVertex(i)->Position.y
			<< " " <<  
				_mesh.GetVertex(i)->Position.z
			<< "\n";
		}
		os << "\n# Faces\n";
		for(unsigned int i=0; i<_mesh.GetNumFaceIndices(); i++){
			MeshType::VertexTriple vt = _mesh.GetFaceVertices(_mesh.GetFace(i));
		  os << "f " << vt[0].Index()+1 << " "
			 << vt[1].Index()+1
			 << " " <<  vt[2].Index()+1 << "\n";
		}*/
	}
	
	DataProcessor::MeshType::VertexType DataProcessor::DecimateUpdate(const DataProcessor::MeshType::VertexPair& vp,const DataProcessor::MeshType::Edge& e)
	{
		MeshType::VertexType result = MeshType::DefaultMergeFunction(vp,e);

		assert(vp[0]->Material == vp[1]->Material);

		result.Material = vp[0]->Material;

		Vector3<> diff = (vp[1]->Position - vp[0]->Position);

		int max = 0;

		for(int i = 1; i <3; ++i)
		{
			if(fabs(diff.c[i]) > fabs(diff.c[max]))
				max = i;
		}
		
		f32 t = (result.Position.c[max] - vp[0]->Position.c[max]) / diff.c[max];
		
		result.TexCoord = vp[0]->TexCoord + ((vp[1]->TexCoord - vp[0]->TexCoord) * t);
		return result;
	}

	void DataProcessor::mergeMeshVertices(MeshType& mesh)
	{
		printf("Merging...\n");
		Vector3<> min,max;//extents of the mesh
		Vector3<> first = mesh.GetVertexData(mesh.GetVertex(0)).Position;
		min=first;max=first;
		for(int i = 1; i < mesh.GetNumVertexIndices(); ++i)
		{
			Vector3<> v = mesh.GetVertexData(mesh.GetVertex(i)).Position;
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
		printf("Generating Tree\n");
		myTree tree(myTree::SplitterType(min,max));

		for(int i = 0; i < mesh.GetNumVertexIndices(); ++i)
		{
			MeshType::Vertex v = mesh.GetVertex(i);
			if(v!=MeshType::nullVertex)
				tree.Insert(v);
		}

		printf("Done generating Tree\n");

		const myTree::TreeNode* cur = tree.TopNode();
		while(cur)
		{
			if(cur->IsLeaf())
			{
				auto elements = cur->LeafElements();
				if(elements.size() > 1000)
					printf("\t%i Element Node Merging\n",elements.size());
				for(size_t i = 0; i < elements.size(); ++i)
					for(size_t i2 = i+1; i2 < elements.size(); ++i2)
					{
						MeshType::Vertex v1 = elements[i];
						MeshType::Vertex v2 = elements[i2];
						if(v1 != MeshType::nullVertex && v2 != MeshType::nullVertex)
						{
							Vector3<> pos1 = mesh.GetVertexData(v1).Position;
							Vector3<> pos2 = mesh.GetVertexData(v2).Position;
							float distance_sq = !( pos1 - pos2);
							if(distance_sq < 0.00001f)
							{
								MeshType::Vertex v = MeshType::nullVertex;
								if((v = mesh.MergeVertex(mesh.MakePair(v1,v2))) == MeshType::nullVertex)
								{
									printf("Vertex Merge Failed:\n");
									printf("\t%f - %f - %f\n",v1->Position.x,v1->Position.y,v1->Position.z);
									printf("\t%f - %f - %f\n",v2->Position.x,v2->Position.y,v2->Position.z);
								}
								break;
							}
						}
					}
			}
			cur = cur->GetNextNode();
		}
					
		printf("Validate Mesh\n");

		mesh.DebugValidateMesh();

		printf("vertices merged\n");
		printf("%i vertices\n",mesh.GetNumVertices());
		printf("%i faces\n",mesh.GetNumFaces());
		printf("%i edges\n",mesh.GetNumEdges());
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
			if(DataProcessor::bProcessMesh)
			{
				sprintf(temp,"@world.MPQ\\world\\maps\\Azeroth\\Azeroth_%i_%i.adt",GetX(),GetY());
				_mesh = GetResource()->OpenResource<IMeshData>(temp);
				printf("requesting %s\n",temp);
			}
			_bLoading = true;
		}
		else if( (!DataProcessor::bProcessMesh || _mesh.getStage() > 0) )
			_bDone = true;
	}

	}