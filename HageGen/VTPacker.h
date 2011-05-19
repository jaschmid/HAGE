#include "header.h"
#include <list>
#include <boost/scoped_array.hpp>

namespace HAGE {

template<class _element> class StaticQuadTree
{
public:
	/*public types*/
	typedef _element value_type;


	StaticQuadTree(u8 nLayers) : 
		_nodes(new value_type[Exp2SquaredSeries(nLayers)]),
		_nNodes(Exp2SquaredSeries(nLayers)),
		_nLayers(nLayers)
	{
		assert(nLayers<16);
	}
	~StaticQuadTree()
	{
	}

	u32 GetNodeIndex(u16 x,u16 y,u8 layer) const
	{
		assert(layer < _nLayers);
		assert(x < (1 << layer) );
		assert(y < (1 << layer) );
		return getNodeIndex(x,y,layer);
	}

	u32 GetParentNode(u32 node) const
	{
		assert(node < _nNodes);
		return getParentNode(node);
	}

	u32 GetFirstChildNode(u32 node) const
	{
		assert(node < _nNodes);
		return (node>>2)+1;
	}
	u32 GetLastChildNode(u32 node) const
	{
		assert(node < _nNodes);
		return (node>>2)+4;
	}
	
	const value_type& operator[](u32 index) const
	{
		assert(index < _nNodes);
		return _nodes[index];
	}

	value_type& operator[](u32 index)
	{
		assert(index < _nNodes);
		return _nodes[index];
	}

	u32 size() const
	{
		return _nNodes;
	}

private:

	inline static u32 Exp2SquaredSeries(u32 n)
	{
		const static u32 base = 0x55555555;
		return base & ((1 << ((u64)n<<1))-1);
	}

	inline static u32 bitShuffle8bit(u8 low,u8 high)
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
	inline static u32 bitShuffle16bit(u16 low,u16 high)
	{
		return bitShuffle8bit(low&0xff,high&0xff) | (bitShuffle8bit((low&0xff00)>>8,(high&0xff00)>>8) << 16);
	}

	inline static u32 getNodeIndex(u16 x,u16 y,u8 l)
	{
		return bitShuffle16bit(x,y) + (u32)Exp2SquaredSeries(l);
	}

	inline static u32 getParentNode(u32 n)
	{
		return ((n-1)>>2);
	}

	boost::scoped_array<value_type>	_nodes;
	const u32			_nNodes;
	const u32			_nLayers;

};

class VTPacker
{
public:

	/* Public Types */

	class Packing
	{
	public:

		Packing(const Packing& p) : _packer(p._packer),_index(p._index) {}

		u32 GetX() const {return _packer->getPackingXLocation(_index);}
		u32 GetY() const {return _packer->getPackingYLocation(_index);}
		u32 GetXSize() const {return _packer->getPackingXSize(_index);}
		u32 GetYSize() const {return _packer->getPackingYSize(_index);}

	private:

		u32 getIndex() const 
			{return _index;}

		Packing(u32 i,const VTPacker* p) :
			_packer(p),_index(i) {}

		const VTPacker* const	_packer;
		const u32				_index;

		friend class VTPacker;
	};

	typedef std::vector<std::pair<Packing,float>> PackingRelationship;

	/* Public Constructors */

	VTPacker(u32 PageSize,u8 nLayers);
	~VTPacker();

	/* Public Functions */

	Packing AddItem(const PackingRelationship& relationship,u32 xSize,u32 ySize);
	void ResolvePackingLocations();

private:
	
	/* Private Types */

	struct PackingData
	{
		i32 xLocation;
		i32 yLocation;
		u32 xSize;
		u32 ySize;
		std::vector< std::pair<u32,float> > packingRelationship;
	};

	class FreeRectangle
	{
	public:

		/* public constructors */

		FreeRectangle(u32 xBegin,u32 yBegin,u32 xSize,u32 ySize) : _position(xBegin,yBegin,xBegin+xSize,yBegin+ySize),_size(xSize,ySize)
		{}
		FreeRectangle(const Vector4<u32>& location) : _position(location),_size(location.z-location.x,location.w-location.y)
		{}

		~FreeRectangle(){}

		/* public functions */

		bool Contains(const FreeRectangle& other) const 
		{
			if(other._position.x >= _position.x && other._position.z <= _position.z && other._position.y >= _position.y && other._position.w <= _position.w)
				return true;
			else
				return false;
		}

		bool Overlaps(const FreeRectangle& other) const
		{
			Vector2<i32> distance = Vector2<i32>((i32)other._position.x - (i32)_position.x,(i32)other._position.y - (i32)_position.y);

			bool xIntersect = (distance.x == 0) || (distance.x > 0 && ((u32)distance.x) < _size.x) || (distance.x < 0 && ((u32)(-distance.x)) < other._size.x);
			bool yIntersect = (distance.y == 0) || (distance.y > 0 && ((u32)distance.y) < _size.y) || (distance.y < 0 && ((u32)(-distance.y)) < other._size.y);

			return xIntersect && yIntersect;
		}

		void GenerateSplit(std::vector<Vector4<u32>>& sub,const Vector4<u32>& rect) const
		{
			if(rect.x > _position.x)
				sub.push_back(Vector4<u32>(_position.x,_position.y,rect.x,_position.w));
			if(rect.z < _position.z)
				sub.push_back(Vector4<u32>(rect.z,_position.y,_position.z,_position.w));
			if(rect.y > _position.y)
				sub.push_back(Vector4<u32>(_position.x,_position.y,_position.z,rect.y));
			if(rect.w < _position.w)
				sub.push_back(Vector4<u32>(_position.x,rect.w,_position.z,_position.w));
		}

		bool operator ==(const FreeRectangle& other) const
		{
			return _position.x == other._position.x && _position.y == other._position.y && _position.z == other._position.z && _position.w == other._position.w;
		}

		bool operator !=(const FreeRectangle& other) const
		{
			return !(operator ==(other));
		}

		const Vector2<u32>& GetSize() const
		{
			return _size;
		}
		
		const Vector4<u32>& GetPosition() const
		{
			return _position;
		}

		operator const Vector2<u32>&()
		{
			return _size;
		}

		operator const Vector4<u32>&()
		{
			return _position;
		}

	private:

		/* private members */
		const Vector4<u32> _position;
		const Vector2<u32> _size;
	};

	class FreeSpaceManager
	{
	public:

		/* public constructors */
		FreeSpaceManager(const Vector2<u32>& totalSize);
		~FreeSpaceManager();

		/* public functions */
		bool GetPossibleLocations(const Vector2<u32>& size,std::vector<Vector2<u32>>& locations);
		bool InsertRectangle(const Vector2<u32>& size,const Vector2<u32>& location);
	private:

		/*private types*/
		typedef std::list<FreeRectangle> RectStorage;
		typedef RectStorage::iterator RectRef;

		/*private functions*/
		RectRef insertFreeRectangle(const Vector4<u32> location);
		void removeFreeRectangle(RectRef rect);

		void getLargerRectangles(const Vector2<u32>& size, std::vector<RectRef>& largerRectangles);
		void getOverlappingRectangles(const Vector4<u32>& position, std::vector<RectRef>& overlappingRectangles);

		/*private members*/
		RectStorage _storage;

		std::vector<RectRef> _currVector;
	};

	friend class Packing;

	typedef std::list<u32> unpackedItemsType;

	/* Private Functions */

	u32 getPackingXLocation(u32 index) const { assert(_data[index].xLocation >= 0 ); return (u32)_data[index].xLocation; }
	u32 getPackingYLocation(u32 index) const { assert(_data[index].yLocation >= 0 ); return (u32)_data[index].yLocation; }
	u32 getPackingXSize(u32 index) const { return _data[index].xSize; }
	u32 getPackingYSize(u32 index) const { return _data[index].ySize; }
	
	i32		getRelationshipWastage(u32 item,u32 xLoc,u32 yLoc) const;
	i32		getWastedPixelsForPacking(u32 item,u32 xLoc,u32 yLoc) const;
	i32		updateWastedPixelsForPacking(u32 item,u32 xLoc,u32 yLoc);
	void	preparePacking();
	unpackedItemsType::const_iterator		getNextItemToPack() const;
	void	findBestPackingLocation(u32 item,u32& xLoc,u32& yLoc);
	void	packItem(u32 item,u32 xLoc,u32 yLoc);
	i32 getWasteChangeForNode(u16 x,u16 y,u8 layer,u32 Waste) const;
	i32 updateWasteForNode(u16 x,u16 y,u8 layer,u32 Waste);
	/* Private Members */

	std::vector<PackingData> _data;
	unpackedItemsType _unpackedItems;
	std::list<u32> _packedItems;
	i64		_wastedPixels;
	u64		_packedPixels;
	f32		_packingQuality;  // == packed / (packed + wasted) 
	StaticQuadTree<u32>		_wasteCounter;
	
	FreeSpaceManager		_freeSpace;
	const u32				_pageSize;
	const u32				_nLayers;
	const f32				_baseAffinity;
	const Vector2<u32>		_maxSize;
};

}