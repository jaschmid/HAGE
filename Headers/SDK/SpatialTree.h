/********************************************************/
/* FILE: SpatialTree.h                                  */
/* DESCRIPTION: Defines a Class for Spatial Trees (Quad)*/
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/

#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef __SPATIAL_TREE_H__
#define __SPATIAL_TREE_H__

#include <vector>
#include <array>

namespace HAGE {

	template<class _source,class _dest> class ConversionFunctor
	{
	public:
		_dest operator()(_source& s) const
		{
			return (_dest)s;
		}
		const _dest operator()(const _source& s) const
		{
			return (const _dest)s;
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
				yMin = parent.split_location.y;
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
			if(elements.size() > max_elements && std::min(std::min(xMax-xMin,yMax-yMin),zMax-zMin) > 0.0001f)
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

		OctSplitter() {}
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
		

}

#endif