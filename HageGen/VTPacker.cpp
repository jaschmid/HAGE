#include "VTPacker.h"
#include <set>

namespace HAGE {

/* Public Constructors */

VTPacker::VTPacker(u32 PageSize,u8 nLayers) : _pageSize(PageSize),_nLayers(nLayers),_wastedPixels(0),_packedPixels(0),_packingQuality(0.0f),
	_maxSize(Vector2<u32>( 1 << (nLayers-1), 1 << (nLayers-1) )),_freeSpace(Vector2<u32>( (1 << (nLayers-1))*PageSize, (1 << (nLayers-1))*PageSize)),_wasteCounter(nLayers),_baseAffinity(0.0f)
{
	for(u32 i = 0; i< _wasteCounter.size(); ++i)
		_wasteCounter[i] = 0xffffffff; //indicates unused
}

VTPacker::~VTPacker()
{
}

/* Public Functions */

VTPacker::Packing VTPacker::AddItem(const PackingRelationship& relationship,u32 xSize,u32 ySize)
{
	PackingData dataItem;
	dataItem.xLocation = -1;
	dataItem.yLocation = -1;
	dataItem.xSize = xSize;
	dataItem.ySize = ySize;

	for(auto it = relationship.begin(); it!= relationship.end(); ++it)
		dataItem.packingRelationship.push_back(std::make_pair(it->first.getIndex(),it->second));

	for(auto it = dataItem.packingRelationship.begin(); it != dataItem.packingRelationship.end(); ++it)
		_data[it->first].packingRelationship.push_back(std::make_pair(_data.size(),it->second));

	_data.push_back(dataItem);

	_unpackedItems.push_back((u32)_data.size()-1);

	return Packing((u32)_data.size() -1,this);
}

void VTPacker::ResolvePackingLocations()
{
	preparePacking();
	while(_unpackedItems.size() != 0)
	{
		auto item = getNextItemToPack();
		u32 index = *item;
		_unpackedItems.erase(item);

		u32 xLoc,yLoc;

		findBestPackingLocation(index,xLoc,yLoc);

		//assert(xLoc < 0x4000 && yLoc < 0x4000);

		packItem(index,xLoc,yLoc);
	}
}


i32 VTPacker::getWasteChangeForNode(u16 x,u16 y,u8 layer,u32 Waste) const
{
	u32 node = _wasteCounter.GetNodeIndex(x,y,layer);
	const u32 available = _pageSize*_pageSize;
	assert(available >= Waste);

	if(_wasteCounter[node] == 0xffffffff)
		return Waste;
	else
	{
		u32 oldWaste = _wasteCounter[node];
		assert(oldWaste + Waste >= available);
		return (i32)Waste - (i32)(available*_baseAffinity);
	}
}
i32 VTPacker::updateWasteForNode(u16 x,u16 y,u8 layer,u32 Waste)
{
	u32 node = _wasteCounter.GetNodeIndex(x,y,layer);
	const u32 available = _pageSize*_pageSize;
	assert(available >= Waste);

	if(_wasteCounter[node] == 0xffffffff)
	{
		_wasteCounter[node] = Waste;
		return Waste;
	}
	else
	{
		u32 oldWaste = _wasteCounter[node];
		assert(oldWaste + Waste >= available);
		_wasteCounter[node] -= available - Waste;
		return (i32)Waste - (i32)(available*_baseAffinity);
	}
}

/* Private Functions */

i32	VTPacker::getWastedPixelsForPacking(u32 item,u32 xLoc,u32 yLoc) const
{
	u32 xSize = _data[item].xSize;
	u32 ySize = _data[item].ySize;

	u8 currentLayer	= (u8)_nLayers;

	i32 wastage = getRelationshipWastage(item,xLoc,yLoc);

	while(currentLayer != 0)
	{
		currentLayer --;
		
		const u16 currentXBegin	= (u16)(xLoc / _pageSize);
		const u16 currentYBegin	= (u16)(yLoc / _pageSize);
		const u16 currentXLast	= (u16)(std::max<i32>((i32)(xLoc + xSize)-1,xLoc) / _pageSize);
		const u16 currentYLast	= (u16)(std::max<i32>((i32)(yLoc + ySize)-1,yLoc) / _pageSize);

		u32 xMinMargin	= xLoc - currentXBegin*_pageSize;
		u32 yMinMargin	= yLoc - currentYBegin*_pageSize;
		u32 xMaxMargin  = (currentXLast+1)*_pageSize - (xLoc + xSize);
		u32 yMaxMargin  = (currentYLast+1)*_pageSize - (yLoc + ySize);

		if(currentXBegin == currentXLast)
			xMinMargin = xMinMargin+xMaxMargin;
		if(currentYBegin == currentYLast)
			yMinMargin = yMinMargin+yMaxMargin;
		
		//top left corner
		wastage += getWasteChangeForNode(currentXBegin,currentYBegin,currentLayer,(xMinMargin*_pageSize)+(yMinMargin*_pageSize)-xMinMargin*yMinMargin);
			
		//left column
		for(u16 y = currentYBegin + 1; y < currentYLast; ++y)
			wastage += getWasteChangeForNode(currentXBegin,y,currentLayer,xMinMargin*_pageSize);

		//top row
		for(u16 x = currentXBegin +1; x < currentXLast; ++x)
			wastage += getWasteChangeForNode(x,currentYBegin,currentLayer,yMinMargin*_pageSize);	

		if(currentXBegin != currentXLast)
		{
			//top right corner
			wastage += getWasteChangeForNode(currentXLast,currentYBegin,currentLayer,(xMaxMargin*_pageSize)+(yMinMargin*_pageSize)-xMaxMargin*yMinMargin);
			
			//right column
			for(u16 y = currentYBegin+ 1; y < currentYLast; ++y)
				wastage += getWasteChangeForNode(currentXLast,y,currentLayer,xMaxMargin*_pageSize);
		}

		if(currentYBegin != currentYLast)
		{
			//bottom left corner
			wastage += getWasteChangeForNode(currentXBegin,currentYLast,currentLayer,(xMinMargin*_pageSize)+(yMaxMargin*_pageSize)-xMinMargin*yMaxMargin);
			
			//bottom column 
			for(u16 x = currentXBegin +1; x < currentXLast; ++x)
				wastage += getWasteChangeForNode(x,currentYLast,currentLayer,yMaxMargin*_pageSize);
		}
		
		//bottom right corner
		if(currentXBegin != currentXLast && currentYBegin != currentYLast)
			wastage += getWasteChangeForNode(currentXLast,currentYLast,currentLayer,(xMaxMargin*_pageSize)+(yMaxMargin*_pageSize)-xMaxMargin*yMaxMargin);
		
		if(currentLayer == 0)
		{
			assert(currentXBegin == 0);
			assert(currentYBegin == 0);
			assert(currentXLast == 0);
			assert(currentYLast == 0);
		}

		xLoc/=2;
		yLoc/=2;
		xSize/=2;
		ySize/=2;
	}
		

	return wastage;
}

i32 VTPacker::getRelationshipWastage(u32 item,u32 xLoc,u32 yLoc) const
{
	i32 wastage = 0;
	const u32 xSize = _data[item].xSize;
	const u32 ySize = _data[item].ySize;

	for(auto it = _data[item].packingRelationship.begin(); it != _data[item].packingRelationship.end(); ++it)
	{
		const u32 alt_xLoc = _data[it->first].xLocation;
		const u32 alt_yLoc = _data[it->first].yLocation;
		const u32 alt_xSize = _data[it->first].xSize;
		const u32 alt_ySize = _data[it->first].ySize;

		if(alt_xLoc == 0xffffffff || alt_yLoc == 0xffffffff)
			continue;
		
		u8 currentLayer	= 0;

		while(currentLayer != _nLayers)
		{

			u32 yBegin = std::max(yLoc>>currentLayer,alt_yLoc>>currentLayer)/_pageSize;
			u32 yEnd = std::min((yLoc+ySize)>>currentLayer,(alt_yLoc+alt_ySize)>>currentLayer)/_pageSize;
			u32 xBegin = std::max(xLoc>>currentLayer,alt_xLoc>>currentLayer)/_pageSize;
			u32 xEnd = std::min((xLoc+xSize)>>currentLayer,(alt_xLoc+alt_xSize)>>currentLayer)/_pageSize;
			
			for(u32 yPage = yBegin; yPage <= yEnd; ++yPage)
				for(u32 xPage = xBegin; xPage <= xEnd; ++xPage)
				{
					i32 xInPage = (i32)std::min((xLoc+xSize)>>currentLayer,_pageSize*(xPage+1)) - (i32)std::max(xLoc>>currentLayer,_pageSize*xPage);
					i32 yInPage = (i32)std::min((yLoc+ySize)>>currentLayer,_pageSize*(yPage+1)) - (i32)std::max(yLoc>>currentLayer,_pageSize*yPage);
					assert(xInPage >= 0 );
					assert(yInPage >= 0 );
					i32 change = xInPage*yInPage*(it->second-_baseAffinity);
					//printf("wastage change: %i (%i, %i, %f, %f)\n",change,xInPage,yInPage,it->second,_baseAffinity);
					wastage -= change;
				}
				
			currentLayer ++;
		}
	}

	return wastage;
}

i32	VTPacker::updateWastedPixelsForPacking(u32 item,u32 xLoc,u32 yLoc)
{
	u32 xSize = _data[item].xSize;
	u32 ySize = _data[item].ySize;
	
	u8 currentLayer	= (u8)_nLayers;

	i32 wastage = getRelationshipWastage(item,xLoc,yLoc);

	while(currentLayer != 0)
	{
		currentLayer --;
		
		const u16 currentXBegin	= (u16)(xLoc / _pageSize);
		const u16 currentYBegin	= (u16)(yLoc / _pageSize);
		const u16 currentXLast	= (u16)(std::max<i32>((i32)(xLoc + xSize)-1,xLoc) / _pageSize);
		const u16 currentYLast	= (u16)(std::max<i32>((i32)(yLoc + ySize)-1,yLoc) / _pageSize);
		
		u32 xMinMargin	= xLoc - currentXBegin*_pageSize;
		u32 yMinMargin	= yLoc - currentYBegin*_pageSize;
		u32 xMaxMargin  = (currentXLast+1)*_pageSize - (xLoc + xSize);
		u32 yMaxMargin  = (currentYLast+1)*_pageSize - (yLoc + ySize);

		if(currentXBegin == currentXLast)
			xMinMargin = xMinMargin+xMaxMargin;
		if(currentYBegin == currentYLast)
			yMinMargin = yMinMargin+yMaxMargin;
		
		//top left corner
		wastage += updateWasteForNode(currentXBegin,currentYBegin,currentLayer,(xMinMargin*_pageSize)+(yMinMargin*_pageSize)-xMinMargin*yMinMargin);
			
		//left column
		for(u16 y = currentYBegin + 1; y < currentYLast; ++y)
			wastage += updateWasteForNode(currentXBegin,y,currentLayer,xMinMargin*_pageSize);

		//top row
		for(u16 x = currentXBegin +1; x < currentXLast; ++x)
			wastage += updateWasteForNode(x,currentYBegin,currentLayer,yMinMargin*_pageSize);	

		if(currentXBegin != currentXLast)
		{
			//top right corner
			wastage += updateWasteForNode(currentXLast,currentYBegin,currentLayer,(xMaxMargin*_pageSize)+(yMinMargin*_pageSize)-xMaxMargin*yMinMargin);
			
			//right column
			for(u16 y = currentYBegin+ 1; y < currentYLast; ++y)
				wastage += updateWasteForNode(currentXLast,y,currentLayer,xMaxMargin*_pageSize);
		}

		if(currentYBegin != currentYLast)
		{
			//bottom left corner
			wastage += updateWasteForNode(currentXBegin,currentYLast,currentLayer,(xMinMargin*_pageSize)+(yMaxMargin*_pageSize)-xMinMargin*yMaxMargin);
			
			//bottom column 
			for(u16 x = currentXBegin +1; x < currentXLast; ++x)
				wastage += updateWasteForNode(x,currentYLast,currentLayer,yMaxMargin*_pageSize);
		}
		
		//bottom right corner
		if(currentXBegin != currentXLast && currentYBegin != currentYLast)
			wastage += updateWasteForNode(currentXLast,currentYLast,currentLayer,(xMaxMargin*_pageSize)+(yMaxMargin*_pageSize)-xMaxMargin*yMaxMargin);
		
		if(currentLayer == 0)
		{
			assert(currentXBegin == 0);
			assert(currentYBegin == 0);
			assert(currentXLast == 0);
			assert(currentYLast == 0);
		}

		xLoc/=2;
		yLoc/=2;
		xSize/=2;
		ySize/=2;
	}

	wastage = (i32)((f32)wastage*_baseAffinity);

	return wastage;
}

void	VTPacker::preparePacking()
{
	// umm *drool*
}

VTPacker::unpackedItemsType::const_iterator		VTPacker::getNextItemToPack() const
{
	//oh just pack randomly for now
	return _unpackedItems.begin();
}

void	VTPacker::findBestPackingLocation(u32 item,u32& xLoc,u32& yLoc)
{
	bool	bFound = false;
	i32		lowestWaste;
	
	std::multiset<u32> currentLines;
	
	u32 xSize = _data[item].xSize;
	u32 ySize = _data[item].ySize;

	std::vector<Vector2<u32>> possibleLocations;

	_freeSpace.GetPossibleLocations(Vector2<u32>(xSize,ySize),possibleLocations);

	for(auto it = possibleLocations.begin(); it != possibleLocations.end(); ++it)
	{
		i32 thisWaste = getWastedPixelsForPacking(item,it->x,it->y);

		if(!bFound || thisWaste < lowestWaste)
		{
			lowestWaste = thisWaste;
			xLoc = it->x;
			yLoc = it->y;
		}
			
		bFound = true;
	}

	assert(bFound);
}

void	VTPacker::packItem(u32 item,u32 xLoc,u32 yLoc)
{
	assert(xLoc <= 0x7ffffff);
	assert(yLoc <= 0x7ffffff);

	u32 xSize = _data[item].xSize;
	u32 ySize = _data[item].ySize;

	_wastedPixels += updateWastedPixelsForPacking(item,xLoc,yLoc);
	_packedPixels += xSize * ySize;
	_packingQuality = (f32)_packedPixels / (f32)(_wastedPixels+_packedPixels);

	_freeSpace.InsertRectangle(Vector2<u32>(xSize,ySize),Vector2<u32>(xLoc,yLoc));

	_data[item].xLocation = (i32)xLoc;
	_data[item].yLocation = (i32)yLoc;

	_packedItems.push_back(item);
}


/* public constructors */
VTPacker::FreeSpaceManager::FreeSpaceManager(const Vector2<u32>& totalSize)
{
	insertFreeRectangle(Vector4<u32>(0,0,totalSize.x,totalSize.y));
}

VTPacker::FreeSpaceManager::~FreeSpaceManager()
{
}

/*public functions*/
bool VTPacker::FreeSpaceManager::GetPossibleLocations(const Vector2<u32>& size,std::vector<Vector2<u32>>& locations)
{
	_currVector.clear();
	getLargerRectangles(size,_currVector);
	for(auto it = _currVector.begin(); it!= _currVector.end(); ++it)
	{
		Vector4<u32> pos = (*it)->GetPosition();
		locations.push_back(Vector2<u32>(pos.x,pos.y));

		if((*it)->GetSize().x > size.x)
		{
			locations.push_back(Vector2<u32>(pos.z-size.x,pos.y));
			if((*it)->GetSize().y > size.y)
				locations.push_back(Vector2<u32>(pos.z-size.x,pos.w-size.y));
		}
		else if((*it)->GetSize().y > size.y)
		{
			locations.push_back(Vector2<u32>(pos.x,pos.w-size.y));
		}
	}

	if(locations.size() != 0)
		return true;

	return false;
}
bool VTPacker::FreeSpaceManager::InsertRectangle(const Vector2<u32>& size,const Vector2<u32>& location)
{
	Vector4<u32> newRect(location.x,location.y,size.x+location.x,size.y+location.y);
	_currVector.clear();
	getOverlappingRectangles(newRect,_currVector);
	std::vector<Vector4<u32>> newFreeRects;
	for(auto it = _currVector.begin(); it!= _currVector.end(); ++it)
	{
		(*it)->GenerateSplit(newFreeRects,newRect);
		removeFreeRectangle(*it);
	}
	for(auto it = newFreeRects.begin(); it!= newFreeRects.end();++it)
		insertFreeRectangle(*it);
	return true;
}

/*private functions*/
VTPacker::FreeSpaceManager::RectRef VTPacker::FreeSpaceManager::insertFreeRectangle(const Vector4<u32> location)
{
	FreeRectangle r(location);
	for(auto it = _storage.begin(); it!= _storage.end();)
	{
		if(r.Contains(*it))
		{
			auto old = it;
			++it;
			_storage.erase(old);
		}
		else if(it->Contains(r))
		{
			return _storage.end();
			++it;
		}
		else
			++it;
	}

	_storage.push_front(r);
	RectRef ref = _storage.begin();
	return ref;
}

void VTPacker::FreeSpaceManager::removeFreeRectangle(RectRef rect)
{
	_storage.erase(rect);
}

void VTPacker::FreeSpaceManager::getLargerRectangles(const Vector2<u32>& size, std::vector<RectRef>& largerRectangles)
{
	for(auto it = _storage.begin(); it!= _storage.end(); ++it)
	{
		Vector2<u32> this_size = it->GetSize();
		if(this_size.x>=size.x && this_size.y >= size.y)
			largerRectangles.push_back(it);
	}
}
void VTPacker::FreeSpaceManager::getOverlappingRectangles(const Vector4<u32>& position, std::vector<RectRef>& overlappingRectangles)
{
	for(auto it = _storage.begin(); it!= _storage.end(); ++it)
	{
		if(it->Overlaps(FreeRectangle(position)))
			overlappingRectangles.push_back(it);
	}
}

}