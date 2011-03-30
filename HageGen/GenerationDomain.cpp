#include "header.h"

#include "GenerationDomain.h"

namespace HAGE {

		GenerationDomain::GenerationDomain() 
		{
			printf("Init Generation\n");
		}

		bool GenerationDomain::MessageProc(const Message* m)
		{
			return SharedDomainBase::MessageProc(m);
		}

		char* str_meshes[] = {
			"@world.MPQ\\world\\maps\\Azeroth\\Azeroth_38_40.adt",
			"@world.MPQ\\world\\maps\\Azeroth\\Azeroth_39_40.adt",
			"@world.MPQ\\world\\maps\\Azeroth\\Azeroth_40_40.adt",
			"\0"
		};

		template<class _source,class _dest> class ConversionFunctor
		{
		public:
			_dest& operator()(_source& s) const
			{
				return (_dest&)s;
			}
			const _dest& operator()(const _source& s) const
			{
				return (const _dest&)s;
			}
		};
		
		template<class _element, size_t max_elements, class _converter = ConversionFunctor<_element,Vector3<>> > class OctSplitter
		{
		public:
			OctSplitter(const Vector3<>& min,const Vector3<>& max) 
			{
				xMin = min.x;
				yMin = min.y;
				zMin = min.z;
				xMax = max.x;
				yMax = max.y;
				zMax = max.z;
			}
			OctSplitter(const OctSplitter& parent,size_t index) 
			{
				if(index < 4)
				{
					xMin = parent.xMin;
					xMax = parent.split_location.x;
				}
				else
				{
					xMin = parent.split_location.x;
					xMax = parent.xMax;
				}

				if(index % 4 < 2)
				{
					yMin = parent.yMin;
					yMax = parent.split_location.y;
				}
				else
				{
					yMin = parent.split_location.x;
					yMax = parent.yMax;
				}
				
				if(index % 2 == 0)
				{
					zMin = parent.zMin;
					zMax = parent.split_location.z;
				}
				else
				{
					zMin = parent.split_location.z;
					zMax = parent.zMax;
				}
			}

			static const size_t Dimension = 8;
						
			bool operator()(const _element& inserted,const std::vector<_element>& elements,size_t depth)
			{
				if(elements.size() > max_elements)
				{
					
					split_location = Vector3<>((xMin+xMax)/2.0f,(yMin+yMax)/2.0f,(zMin+zMax)/2.0f);
					return true;
				}
				else
					return false;
			}

			size_t operator()(const _element& inserted,size_t depth) const
			{
				Vector3<> loc = converter(inserted);
				if(loc.x < split_location.x)
				{
					if(loc.y < split_location.y)
					{
						if(loc.z < split_location.z)
							return 0;
						else
							return 1;
					}
					else
					{
						if(loc.z < split_location.z)
							return 2;
						else
							return 3;
					}
				}
				else
				{
					if(loc.y < split_location.y)
					{
						if(loc.z < split_location.z)
							return 4;
						else
							return 5;
					}
					else
					{
						if(loc.z < split_location.z)
							return 6;
						else
							return 7;
					}
				}
			}
			
		private:
			const _converter converter;
			Vector3<> split_location;
			f32 xMin,xMax;
			f32 yMin,yMax;
			f32 zMin,zMax;
		};

		template<class _element,class _splitter = OctSplitter<_element,10>> class SpatialTree
		{
		private:

			const static size_t _dimension = _splitter::Dimension;
			
		public:

			class TreeNode;

			
			typedef OctSplitter<_element,10> SplitterType;
			typedef _element ElementType;
			typedef std::vector<_element> Container;

			SpatialTree(const _splitter& split)
				:base(new TreeNode(split))
			{
			}

			~SpatialTree()
			{
				delete base;
			}

			void Insert(const _element& element)
			{
				base->Insert(element);
			}

			const TreeNode* TopNode()
			{
				return base;
			}

			template<size_t a,size_t b> class Max
			{
			public:
				static const size_t value = (a>b)?a:b;
			};
			
			class TreeNode
			{
			public:
				
				size_t GetNumElements() const
				{
					if(bLeaf)
					{
						LeafNode* leaf = GetLeaf();
						return leaf->elements.size();
					}
					else
					{
						size_t sum = 0;
						BranchNode* branch = GetBranch();
						for(auto it = branch->subNodes.begin();it!=subNodes.end();++it)
							sum += (*it)->GetNumElements();
						return sum;
					}
				}

				const TreeNode* BranchSubNode(size_t index) const
				{
					assert(!bLeaf && index < _dimension);
					const BranchNode* branch = GetBranch();
					return branch->subNodes[index];
				}

				const Container& LeafElements() const
				{
					assert(bLeaf);
					const LeafNode* leaf = GetLeaf();
					return leaf->elements;
				}

				size_t GetNodeIndex() const
				{
					return _index;
				}

				const TreeNode* GetNextNode() const
				{
					if(IsBranch())
						return BranchSubNode(0);
					else
					{
						const TreeNode* cur = _parent;
						size_t i = _index + 1;
						while(i == _splitter::Dimension && cur != nullptr)
						{
							i = cur->GetNodeIndex() + 1;
							cur = cur->GetParent();
						}
						if(cur)
							return cur->BranchSubNode(i);
						else
							return nullptr;
					}
				}

				const TreeNode* GetParent() const
				{
					return _parent;
				}

				bool IsLeaf() const
				{
					return bLeaf;
				}

				bool IsBranch() const
				{
					return !bLeaf;
				}

			private:

				void Insert(const _element& element)
				{
					if(bLeaf)
					{
						LeafNode* leaf = GetLeaf();
						leaf->elements.push_back(element);
						if( _split(element,leaf->elements,_depth) )
						{
							BranchNode node;
							
							for(auto it = node.subNodes.begin();it!=node.subNodes.end();++it)
								*it = new TreeNode(_depth+1,this,it-node.subNodes.begin());

							for(auto it = leaf->elements.begin();it!=leaf->elements.end();++it)
								node.subNodes[_split(*it,_depth)]->Insert(*it);

							MakeBranch(node);
							
							BranchNode* node2 = GetBranch();
						}
					}
					else
					{
						BranchNode* node = GetBranch();
						node->subNodes[_split(element,_depth)]->Insert(element);
					}
				}
				
				class LeafNode
				{
				public:
					LeafNode(){}
					Container elements;
				};

				class BranchNode
				{
				public:
					BranchNode()
					{
					}
					~BranchNode()
					{
					}
					std::array<TreeNode*,_dimension>	subNodes;
				};

				TreeNode(size_t depth,TreeNode* parent,size_t index) :
				_split(parent->_split,index),_depth(depth),_parent(parent),_index(index)
				{
					bLeaf=true;
					new (_data.data()) LeafNode();
				}
				TreeNode(const _splitter& split) :
				_split(split),_depth(0),_parent(nullptr),_index(0)
				{
					bLeaf=true;
					new (_data.data()) LeafNode();
				}

				~TreeNode()
				{
					if(bLeaf)
						((LeafNode*)_data.data())->~LeafNode();
					else
					{
						BranchNode* node = GetBranch();
						for(auto it = node->subNodes.begin();it!=node->subNodes.end();++it)
							delete *it;
						((BranchNode*)_data.data())->~BranchNode();
					}
				}

				bool bLeaf;
				
				const LeafNode* GetLeaf() const
				{
					assert(bLeaf);
					return (const LeafNode*)_data.data();
				}
				const BranchNode* GetBranch() const
				{
					assert(!bLeaf);
					return (const BranchNode*)_data.data();
				}

				LeafNode* GetLeaf()
				{
					assert(bLeaf);
					return (LeafNode*)_data.data();
				}
				BranchNode* GetBranch()
				{
					assert(!bLeaf);
					return (BranchNode*)_data.data();
				}
				
				void MakeBranch(const BranchNode& node)
				{
					assert(bLeaf);
					((LeafNode*)_data.data())->~LeafNode();
					bLeaf = false;
					new (_data.data()) BranchNode(node);
				}

				void MakeLeaf(const LeafNode& leaf)
				{
					assert(!bLeaf);
					((BranchNode*)_data.data())->~BranchNode();
					bLeaf = true;
					new (_data.data()) LeafNode(leaf);
				}
								
				TreeNode*													_parent;
				size_t														_depth;
				size_t														_index;
				std::array<u8,Max<sizeof(LeafNode),sizeof(BranchNode)>::value>	_data;
				_splitter _split;

				friend class SpatialTree<_element,_splitter>;
			};
			
		private:

			TreeNode* base;
		};
		
		
		void GenerationDomain::writeOutputMesh()
		{
			printf("Writing...\n");
			FILE* f = fopen("landscape.hgeo","wb");
			char header[] = "HGEO";
			fwrite(header,4,1,f);

			_mesh.Compact();

			u32 nVertices = _mesh.GetNumVertices();
			u32 nPrimitives = _mesh.GetNumFaces();
			
			fwrite(&nVertices,sizeof(u32),1,f);
			for(int i = 0; i< nVertices; ++i)
			{
				Vector3<> out = _mesh.GetVertex(i);
				fwrite(&out,sizeof(Vector3<>),1,f);
				Vector2<> texcoord = Vector2<>((out.x-min.x)/(max.x-min.x),(out.z-min.z)/(max.z-min.z));
				fwrite(&texcoord,sizeof(Vector2<>),1,f);
			}
			
			fwrite(&nPrimitives,sizeof(u32),1,f);
			for(int i = 0; i< nPrimitives; ++i)
			{
				MeshType::VertexTriple vt = _mesh.GetFaceVertices(_mesh.GetFace(i));
				u32 index[3];
				for(int i2 = 0; i2 <3;++i2)
					index[i2] =	_mesh.GetIndex(vt[i2]);
				fwrite(index,3*sizeof(u32),1,f);
			}

			fclose(f);
			printf("Done Writing\n");

		}

		void GenerationDomain::mergeMeshVertices()
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
									assert(_mesh.MergeVertex(_mesh.MakePair(v1,v2)));
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

		void GenerationDomain::DomainStep(t64 time)
		{
			static bool done = false;
			static bool load_requested = false;
			static char temp[256];
			static int numLoaded = 0;

			if(!load_requested)
			{
				for(int iy = yBegin; iy <= xEnd; ++iy)
					for(int ix = xBegin; ix <= xEnd; ++ix)
					{						
						sprintf(temp,"@world.MPQ\\world\\maps\\Azeroth\\Azeroth_%i_%i.adt",ix,iy);
						data[ix-xBegin][iy-yBegin] = Resource->OpenResource<IMeshData>(temp);
						loaded[ix-xBegin][iy-yBegin] = false;
					}
				load_requested = true;
			}

			if(!done)
				for(int iy = yBegin; iy <= xEnd; ++iy)
					for(int ix = xBegin; ix <= xEnd; ++ix)
						if(!loaded[ix-xBegin][iy-yBegin] && data[ix-xBegin][iy-yBegin].getStage() > 0)
						{
				
							sprintf(temp,"@world.MPQ\\world\\maps\\Azeroth\\Azeroth_%i_%i.adt",ix,iy);
							TResourceAccess<IMeshData>& curr = data[ix-xBegin][iy-yBegin];
							loaded[ix-xBegin][iy-yBegin] = true;
							printf("data %s loaded!\n",temp);
							{
								const u8* pData;
								u32 nIndices = curr->GetIndexData(&pData);

								u32 preVertices = _mesh.GetNumVertexIndices();

								_mesh.ImportRawIndexData<MeshType::RAW_INDEX_TRIANGLE_LIST,u32>(nIndices/3,(const u32*)pData);

								const u8* positionData;
								u32 stride;
								u32 nVertices = curr->GetVertexData((const u8**)&positionData,stride,IMeshData::POSITION);

								Vector3<> offset = Vector3<>( (float)(ix - (xEnd + xBegin) / 2) * 2.0f, 0.0f , (float)(iy - (yEnd + yBegin) / 2) * 2.0f);

								for(int i = 0; i < nVertices; ++i)
									_mesh.GetVertexData(_mesh.GetVertex(i + preVertices)) = *(const Vector3<>*)&positionData[stride*i] + offset;
								printf("data loaded\n");
								printf("%i vertices\n",_mesh.GetNumVertices());
								printf("%i faces\n",_mesh.GetNumFaces());
								printf("%i edges\n",_mesh.GetNumEdges());

					
								_mesh.DebugValidateMesh();
							}

							numLoaded++;
							if(numLoaded == numToLoad)
							{
								mergeMeshVertices();
								writeOutputMesh();
								done = true;
							}
						}
			//printf("Time %f, Elapsed Time: %f\n",GetTime().toSeconds(),GetElapsedTime().toSeconds());

		}

		GenerationDomain::~GenerationDomain()
		{

			printf("Destroy GenerationDomain\n");
		}
}
