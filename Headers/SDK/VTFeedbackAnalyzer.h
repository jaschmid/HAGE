#include <HAGE.h>

namespace HAGE {

	
	class VirtualTextureFeedbackAnalyzer
	{
	public:
		VirtualTextureFeedbackAnalyzer(u32 CacheSize,u32 nLayers) : _cacheSize(CacheSize),_quadTree(nLayers) {}
		~VirtualTextureFeedbackAnalyzer() {}
		
			
		struct element
		{
			union
			{
				struct
				{
					u16 x;
					u16 y;
				};
				u32 index;
			};
			u16 level;
			u16 priority;
		};

		struct cache_element
		{
			u32 page_index;
			u32 cache_index;
		};

		
		void AdjustCacheSize(u32 new_cache)
		{
			_currentData.clear();
			_cacheSize = new_cache;
		}

		void AnalyzeFeedback(const element* pFeedbackData,u32 nFeedback,std::vector<cache_element>& changes,std::vector<u32>& removed_pages)
		{

			_quadTree.ParseElements(pFeedbackData,nFeedback,_newDataBuffer);

			if(_newDataBuffer.size() > _cacheSize)
				_newDataBuffer.resize(_cacheSize);

			std::sort(_newDataBuffer.begin(),_newDataBuffer.end(),_eIdxCompare);
			
			changes.clear();
			_oldData.clear();

			auto ni = _newDataBuffer.begin();
			auto oi = _currentData.begin();

			while(oi != _currentData.end() && ni != _newDataBuffer.end())
			{
				if(oi->page_index == ni->index)
				{
					++oi;
					++ni;
				}
				else if(oi->page_index < ni->index)
				{
					//not in new list anymore
					_oldData.push_back(&*oi);
					std::push_heap(_oldData.begin(),_oldData.end(),_cRIdxCompare);
					++oi;
				}
				else
				{
					//new item
					cache_element new_cache;
					new_cache.cache_index = 0xffff;//currently don't know, will update this later
					new_cache.page_index = ni->index;
					changes.push_back(new_cache);
					++ni;
				}
			}

			while(oi != _currentData.end())
			{
				//not in new list anymore
				_oldData.push_back(&*oi);
				std::push_heap(_oldData.begin(),_oldData.end(),_cRIdxCompare);
				++oi;
			}

			while(ni != _newDataBuffer.end())
			{
				//new item
				cache_element new_cache;
				new_cache.cache_index = 0xffff;//currently don't know, will update this later
				new_cache.page_index = ni->index;
				changes.push_back(new_cache);
				++ni;
			}

			//now resort the _oldData  so we discard old elements first

			std::sort_heap(_oldData.begin(),_oldData.end(),_cRIdxCompare);

			auto it_old = _oldData.begin();

			u32 nOldElementsToReplace = (u32)(( (_currentData.size() + changes.size() )> _cacheSize )?(_currentData.size() + changes.size()  - _cacheSize):0);

			removed_pages.clear();

			for(auto it = changes.begin(); it!=changes.end();++it)
			{
				if(nOldElementsToReplace)
				{
					removed_pages.push_back((*it_old)->page_index);
					it->cache_index = (*it_old)->cache_index;
					(*it_old)->page_index = it->page_index;
					++it_old;
					--nOldElementsToReplace;
				}
				else
				{
					cache_element new_cache;
					new_cache.cache_index = (u32)_currentData.size();
					new_cache.page_index = it->page_index;
					it->cache_index = new_cache.cache_index;
					_currentData.push_back(new_cache);
				}
			}

			//and resort our current data
			std::sort(_currentData.begin(),_currentData.end(),_cIdxCompare);



		}


		struct element_compare_priority
		{
			bool operator()(const element& _1,const element& _2)
			{
				return _1.priority > _2.priority;
			}
		};
		struct element_compare_index
		{
			bool operator()(const element& _1,const element& _2)
			{
				return _1.index < _2.index;
			}
		};
		struct cache_compare_index
		{
			bool operator()(const cache_element& _1,const cache_element& _2)
			{
				return _1.page_index < _2.page_index;
			}
		};
		struct cache_compare_reverse_index
		{
			bool operator()(const cache_element* _1,const cache_element* _2)
			{
				return _1->page_index > _2->page_index;
			}
		};

	private:
		class StaticRefCountQuadTree
		{
		public:
			StaticRefCountQuadTree(u32 nLayers) : 
				_pNodes(new tree_node[Exp2SquaredSeries(nLayers)]),
				_nNodes((u32)Exp2SquaredSeries(nLayers)),
				_nLayers(nLayers)
			{
			}
			~StaticRefCountQuadTree()
			{
			}

			void ParseElements(const element* pElements,u32 nElements,std::vector<element>& out)
			{
				// remove old data
				memset(_pNodes,0,_nNodes*sizeof(tree_node));


				for(u32 i = 0; i<nElements;++i)
				{
					const element& e = pElements[i];	
					assert(e.level < _nLayers);
					assert(e.x < (1<<e.level));
					assert(e.y < (1<<e.level));
					u32 x = e.x;
					u32 y = e.y;
					u32 l = e.level;
					u32 n = GetNodeLocation(e.x,e.y,e.level);
					assert(n < _nNodes);
					
					safeIncrement(_pNodes[n].count_this);

					while(n != 0)
					{
						u32 check = GetNodeLocation(x,y,l);
						assert(n == check);
						safeIncrement(_pNodes[n].count_children);
						n = GetParentNode(n);
						x /= 2;
						y /= 2;
						l--;
					}

					safeIncrement(_pNodes[n].count_children);

				}

				out.clear();

				pushNodeToHeap(0,out);
				
				std::sort_heap(out.begin(),out.end(),prio_compare);
			}

		private:
			element_compare_priority prio_compare;

			void pushNodeToHeap(u32 n,std::vector<element>& out)
			{
				element e;
				e.index = n;
				e.level = 0;
				e.priority = _pNodes[n].count_this;
				out.push_back(e);
				std::push_heap(out.begin(),out.end(),prio_compare);

				u32 firstChild = (n << 2) +1;

				if(firstChild < _nNodes)
					for(int i = 0; i <4; ++i)
						if(_pNodes[firstChild+i].count_children > 0)
							pushNodeToHeap(firstChild+i,out);
			}
			
			inline static void safeIncrement(u16& value)
			{
				value = value + !((((u32)value) + 1) & 0x00010000);
			}

			inline static u64 Exp2SquaredSeries(u32 n)
			{
				const static u64 base = 0x5555555555555555L;
				return base & ((1 << ((u64)n<<1))-1);
			}

			inline static u32 bitShuffle8bit(u32 low,u32 high)
			{
					
				static const u16 bitShuffleLookup[256] = 
				{
					0x0000,0x0001,0x0004,0x0005,0x0010,0x0011,0x0014,0x0015,0x0040,0x0041,0x0044,0x0045,0x0050,0x0051,0x0054,0x0055,
					0x0100,0x0101,0x0104,0x0105,0x0110,0x0111,0x0114,0x0115,0x0140,0x0141,0x0144,0x0145,0x0150,0x0151,0x0154,0x0155,
					0x0400,0x0401,0x0404,0x0405,0x0410,0x0411,0x0414,0x0415,0x0440,0x0441,0x0444,0x0445,0x0450,0x0451,0x0454,0x0455,
					0x0500,0x0501,0x0504,0x0505,0x0510,0x0511,0x0514,0x0515,0x0540,0x0541,0x0544,0x0545,0x0550,0x0551,0x0554,0x0555,
					0x1000,0x1001,0x1004,0x1005,0x1010,0x1011,0x1014,0x1015,0x1040,0x1041,0x1044,0x1045,0x1050,0x1051,0x1054,0x1055,
					0x1100,0x1101,0x1104,0x1105,0x1110,0x1111,0x1114,0x1115,0x1140,0x1141,0x1144,0x1145,0x1150,0x1151,0x1154,0x1155,
					0x1400,0x1401,0x1404,0x1405,0x1410,0x1411,0x1414,0x1415,0x1440,0x1441,0x1444,0x1445,0x1450,0x1451,0x1454,0x1455,
					0x1500,0x1501,0x1504,0x1505,0x1510,0x1511,0x1514,0x1515,0x1540,0x1541,0x1544,0x1545,0x1550,0x1551,0x1554,0x1555,
					0x4000,0x4001,0x4004,0x4005,0x4010,0x4011,0x4014,0x4015,0x4040,0x4041,0x4044,0x4045,0x4050,0x4051,0x4054,0x4055,
					0x4100,0x4101,0x4104,0x4105,0x4110,0x4111,0x4114,0x4115,0x4140,0x4141,0x4144,0x4145,0x4150,0x4151,0x4154,0x4155,
					0x4400,0x4401,0x4404,0x4405,0x4410,0x4411,0x4414,0x4415,0x4440,0x4441,0x4444,0x4445,0x4450,0x4451,0x4454,0x4455,
					0x4500,0x4501,0x4504,0x4505,0x4510,0x4511,0x4514,0x4515,0x4540,0x4541,0x4544,0x4545,0x4550,0x4551,0x4554,0x4555,
					0x5000,0x5001,0x5004,0x5005,0x5010,0x5011,0x5014,0x5015,0x5040,0x5041,0x5044,0x5045,0x5050,0x5051,0x5054,0x5055,
					0x5100,0x5101,0x5104,0x5105,0x5110,0x5111,0x5114,0x5115,0x5140,0x5141,0x5144,0x5145,0x5150,0x5151,0x5154,0x5155,
					0x5400,0x5401,0x5404,0x5405,0x5410,0x5411,0x5414,0x5415,0x5440,0x5441,0x5444,0x5445,0x5450,0x5451,0x5454,0x5455,
					0x5500,0x5501,0x5504,0x5505,0x5510,0x5511,0x5514,0x5515,0x5540,0x5541,0x5544,0x5545,0x5550,0x5551,0x5554,0x5555
				};
				assert((bitShuffleLookup[low] & (bitShuffleLookup[high]<<1)) == 0);
				return bitShuffleLookup[low] | (bitShuffleLookup[high]<<1);
			}
			inline static u32 bitShuffle16bit(u32 low,u32 high)
			{
				return bitShuffle8bit(low&0xff,high&0xff) | (bitShuffle8bit((low&0xff00)>>8,(high&0xff00)>>8) << 16);
			}

			inline static u32 GetNodeLocation(u32 x,u32 y,u32 l)
			{
				return bitShuffle16bit(x,y) + (u32)Exp2SquaredSeries(l);
			}

			inline static u32 GetParentNode(u32 n)
			{
				return ((n-1)>>2);
			}
			
			struct tree_node
			{
				u16 count_this;
				u16 count_children;
			};

			tree_node* const	_pNodes;
			const u32			_nNodes;
			const u32			_nLayers;

		};

		element_compare_index		_eIdxCompare;
		cache_compare_index			_cIdxCompare;
		cache_compare_reverse_index	_cRIdxCompare;

		u32							_cacheSize;

		StaticRefCountQuadTree		_quadTree;

		std::vector<element>		_newDataBuffer;

		std::vector<cache_element*>	_oldData;

		std::vector<cache_element>	_currentData;
	};

}