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
		
void test()
{
	typedef CEditableMesh<> mesh ;
	mesh m;
	mesh::Face f=m.InsertFace(mesh::MakeTriple(m.InsertVertex(),m.InsertVertex(),m.InsertVertex()));
	mesh::VertexTriple vt = m.GetFaceVertices(f);
	mesh::Face other = m.GetFace(vt);
	assert(f==other);
	mesh::Vertex peak = m.InsertVertex();
	mesh::Face f2=m.InsertFace(mesh::MakeTriple(vt[1],vt[0],peak));
	mesh::Face f3=m.InsertFace(mesh::MakeTriple(vt[2],vt[1],peak));
	mesh::Face f4=m.InsertFace(mesh::MakeTriple(vt[0],vt[2],peak));
	m.InvalidateEdge(m.GetEdge(mesh::MakePair(f2,f3)));
	m.Compact();
	assert(m.GetNumVertices() == 4);
	assert(m.GetNumFaces() == 2);
	assert(m.GetNumEdges() == 5);
}
		TResourceAccess<IMeshData> data;

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
		
		template<class _element, class _converter = ConversionFunctor<_element,Vector3<>> > class OctDecider
		{
		public:

			bool operator()(const _element& inserted,const std::vector<_element>& elements,size_t depth)
			{
				if(elements.size() > 10)
					return true;
				else
					return false;
			}
		};

		template<class _element, class _converter = ConversionFunctor<_element,Vector3<>> > class OctSplitter
		{
		public:
			static const size_t Dimension = 8;

			OctSplitter(const OctDecider<_element,_converter>& decider,const std::vector<_element>& elements)
			{
				split_location = Vector3<>(0.0f,0.0f,0.0f);
				for(int i = 0 ; i < elements.size();++i)
					split_location+=converter(elements[i]);

				split_location = split_location*(1.0f/(float)elements.size());
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
		};

		template<class _element,class _decider = OctDecider<_element>, class _splitter = OctSplitter<_element>,size_t _dimension = _splitter::Dimension> class SpatialTree
		{
		public:
			SpatialTree()
				:base(new TreeNode(0))
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

		private:
			template<size_t a,size_t b> class Max
			{
			public:
				static const size_t value = (a>b)?a:b;
			};
			
			class TreeNode
			{
			private:
				
				class LeafNode
				{
				public:
					LeafNode() :decider() {}
					std::vector<_element> elements;
					_decider decider;
				};

				class BranchNode
				{
				public:
					BranchNode(const _decider& decider,const std::vector<_element>& elements,size_t depth) : splitter(decider,elements) 
					{
						for(auto it = subNodes.begin();it!=subNodes.end();++it)
							*it = new TreeNode(depth+1);
					}
					~BranchNode()
					{
						
						for(auto it = subNodes.begin();it!=subNodes.end();++it)
							delete *it;
					}
					std::array<TreeNode*,_dimension>	subNodes;
					const _splitter splitter;
				};

			public:
				TreeNode(size_t depth) :
				  _depth(depth)
				{
					bLeaf=true;
					new (_data.data()) LeafNode();
				}

				~TreeNode()
				{
					if(bLeaf)
						((LeafNode*)_data.data())->~LeafNode();
					else
						((BranchNode*)_data.data())->~BranchNode();
				}

				void Insert(const _element& element)
				{
					if(bLeaf)
					{
						LeafNode* leaf = GetLeaf();
						leaf->elements.push_back(element);
						if( leaf->decider(element,leaf->elements,_depth) )
						{
							BranchNode node(leaf->decider,leaf->elements,_depth);
							MakeBranch(node);
						}
					}
					else
					{
						BranchNode* node = GetBranch();
						node->subNodes[node->splitter(element,_depth)]->Insert(element);
					}
				}

			private:
				bool bLeaf;

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
								
				size_t														_depth;
				std::array<u8,Max<sizeof(LeafNode),sizeof(BranchNode)>::value>	_data;
			};

			TreeNode* base;
		};

		void GenerationDomain::DomainStep(t64 time)
		{
			typedef CEditableMesh<> mesh ;
			static int imesh = 0;
			static bool load = true;
			static bool done = false;
			if(load)
			{
				data = Resource->OpenResource<IMeshData>(str_meshes[imesh]);
				load=false;
			}
			if(data.getStage() > 0 && ! done)
			{
				printf("data %s loaded!\n",str_meshes[imesh]);
				{
					mesh mesh;
					const u8* pData;
					u32 nIndices = data->GetIndexData(&pData);
					mesh.ImportRawIndexData<mesh::RAW_INDEX_TRIANGLE_LIST,u32>(nIndices/3,(const u32*)pData);
					const u8* positionData;
					u32 stride;
					u32 nVertices = data->GetVertexData((const u8**)&positionData,stride,IMeshData::POSITION);
					for(int i = 0; i < mesh.GetNumVertexIndices(); ++i)
						mesh.GetVertexData(mesh.GetVertex(i)) = *(const Vector3<>*)&positionData[stride*i];
					printf("data loaded\n");
					printf("%i vertices\n",mesh.GetNumVertices());
					printf("%i faces\n",mesh.GetNumFaces());
					printf("%i edges\n",mesh.GetNumEdges());

					
					mesh.DebugValidateMesh();

					SpatialTree<mesh::Vertex> tree;

					Vector3<> test1 = mesh.GetVertexData(mesh.GetVertex(8));
					Vector3<> test2 = mesh.GetVertexData(mesh.GetVertex(9*9+8*8));
					for(int i = 0; i < nVertices; ++i)
					{
						mesh::Vertex v = mesh.GetVertex(i);
						if(v!=mesh::nullVertex)
							tree.Insert(v);
						/*
						for(int i2 = i+1; i2 < nVertices; ++i2)
						{
							mesh::Vertex v1 = mesh.GetVertex(i);
							mesh::Vertex v2 = mesh.GetVertex(i2);
							if(v1 != mesh::nullVertex && v2 != mesh::nullVertex)
							{
								Vector3<> pos1 = mesh.GetVertexData(v1);
								Vector3<> pos2 = mesh.GetVertexData(v2);
								float distance_sq = !( pos1-pos2);
								if(distance_sq < 0.000001f)
									mesh.MergeVertex(mesh.MakePair(v1,v2));
							}
						}*/
					}
					
					mesh.DebugValidateMesh();

					printf("vertices merged\n");
					printf("%i vertices\n",mesh.GetNumVertices());
					printf("%i faces\n",mesh.GetNumFaces());
					printf("%i edges\n",mesh.GetNumEdges());
				}

				imesh++;
				if(str_meshes[imesh][0]!='\0')
					load=true;
				else
					done = true;
			}
			//printf("Time %f, Elapsed Time: %f\n",GetTime().toSeconds(),GetElapsedTime().toSeconds());

		}

		GenerationDomain::~GenerationDomain()
		{

			printf("Destroy GenerationDomain\n");
		}
}
