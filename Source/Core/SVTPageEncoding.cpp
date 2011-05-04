#include <HAGE.h>
#include <stdio.h>

#include "SVTPageEncoding.h"

namespace HAGE
{

	ISVTDataLayer* ISVTDataLayer::CreateLayer(u32 encodingId)
	{
		switch(encodingId)
		{
		case LAYER_ENCODING_UNCOMPRESSED_RAW:
			return new SVTDataLayer_Raw;
		case LAYER_ENCODING_PNGISH_RAW:
			return new SVTDataLayer_PNGish;
		case LAYER_ENCODING_JPEG_XR_RAW:
			return new SVTDataLayer_JpegXR;
		case LAYER_ENCODING_COMPOSITE:
		case LAYER_ENCODING_BLENDED:
		default:
			assert(!"non supported encoding");
			return nullptr;
		}
	}
	
	bool SVTDataLayer_Raw::GetImageData(const Vector2<u32>& offset,u32 LayerId,ImageRect<R8G8B8A8>& dataOut) const
	{
		assert( offset.x + dataOut.XSize() <= _size.x);
		assert( offset.y + dataOut.YSize() <= _size.y);

		if(_imageData)
			dataOut.CopyFrom(_imageData->GetRect(Vector4<u32>(offset.x,offset.y,offset.x + dataOut.XSize(),offset.y + dataOut.YSize())));
		else
			for(u32 y = 0; y < dataOut.YSize(); ++y)
				for(u32 x = 0; x < dataOut.XSize(); ++x)
					dataOut( x, y ).SetData(0);

		return true;
	}

	void SVTDataLayer_Raw::GenerateMipmap(const Vector2<u32>& size,const std::array<const ISVTDataLayer*,16>& parents,u32 borderSize)
	{
		reset(); 

		u32 outerPageSize = size.x;

		assert(size.x == size.y);

		_size = size;

		ImageData<R8G8B8A8> largeData(outerPageSize*2, outerPageSize*2);

		const u32 r[4] = {outerPageSize - 3*borderSize,	borderSize,						borderSize,						borderSize};
		const u32 s[4] = {2*borderSize,					outerPageSize - 2*borderSize,	outerPageSize -2*borderSize,	2*borderSize};
		const u32 w[4] = {0,							2*borderSize,					outerPageSize,					2*outerPageSize-2*borderSize};

		for(int y= 0; y<4;++y)
			for(int x= 0; x<4;++x)
				if(parents[y*4+x])
					parents[y*4+x]->GetImageData(Vector2<u32>(r[x],r[y]),0xffffffff,largeData.GetRect(Vector4<u32>(w[x],w[y],w[x]+s[x],w[y]+s[y])));

		_imageData = new ImageData<R8G8B8A8>(outerPageSize,outerPageSize);

		for(int y = 0; y<_size.y;++y)
			for(int x = 0; x<_size.x;++x)
				(*_imageData)(x,y) = ColorAverage( largeData(x*2+0,y*2+0), largeData(x*2+1,y*2+0), largeData(x*2+0,y*2+1), largeData(x*2+1,y*2+1));
	}

	u32 SVTDataLayer_Raw::Deserialize(const Vector2<u32>& size,u32 numBytes,const u8* pInBytes,const ISVTSharedDataStorage* pShared)
	{
		reset(); 

		_size = size;

		_imageData = new ImageData<R8G8B8A8>(_size.x,_size.y,(const Pixel<R8G8B8A8>*)pInBytes);

		return _imageData->GetDataSize();
	}

	ISVTDataLayer* SVTDataLayer_Raw::WriteRect(Vector2<i32> offset,u32 layer,const ISVTDataLayer* data)
	{
		if(!_imageData)
			_imageData = new ImageData<R8G8B8A8>(_size.x,_size.y);

		Vector2<u32> sourceOffset ( (u32)std::max<i32>(-offset.x,0) , (u32)std::max<i32>(-offset.y,0) );
		Vector2<u32> targetSize = data->GetSize();

		data->GetImageData(sourceOffset,layer,_imageData->GetRect( Vector4<u32>( (u32)std::max<i32>(offset.x,0) , (u32)std::max<i32>(offset.y,0) ,std::min<u32>(targetSize.x+offset.x,_size.x) , std::min<u32>(targetSize.y+offset.y,_size.y))) );

		return this;
	}

	Pixel<R8G8B8A8> SVTDataLayer_Raw::ColorAverage(const Pixel<R8G8B8A8>& _1,const Pixel<R8G8B8A8>& _2, const Pixel<R8G8B8A8>& _3, const Pixel<R8G8B8A8>& _4)
	{
		Pixel<R8G8B8A8> result;
		result.SetRed((u8)(((u32)_1.Red() + (u32)_2.Red() + (u32)_3.Red() + (u32)_4.Red() + 2)/4));
		result.SetGreen((u8)(((u32)_1.Green() + (u32)_2.Green() + (u32)_3.Green() + (u32)_4.Green() + 2)/4));
		result.SetBlue((u8)(((u32)_1.Blue() + (u32)_2.Blue() + (u32)_3.Blue() + (u32)_4.Blue() + 2)/4));
		result.SetAlpha((u8)(((u32)_1.Alpha() + (u32)_2.Alpha() + (u32)_3.Alpha() + (u32)_4.Alpha() + 2)/4));
		return result;
	}

	
	u32 SVTDataLayer_PNGish::Serialize(u32 maxBytes,u8* pOutBytes) const
	{
		static const Pixel<R8G8B8A8> border(0x00000000);
		if(!_imageData)
			return 0;
		
		u32 pos = 0;

		/*
		SumType redSum; memset(&redSum,0,sizeof(redSum));
		SumType greenSum; memset(&greenSum,0,sizeof(greenSum));
		SumType blueSum; memset(&blueSum,0,sizeof(blueSum));
		*/
		//first row
		{/*
			filterColorSum(redSum,greenSum,blueSum,(*_imageData)(0,0),border,border,border);

			for(u32 x=1;x<_imageData->XSize();++x)
				filterColorSum(redSum,greenSum,blueSum,(*_imageData)(x,0),(*_imageData)(x-1,0),border,border);
				*/
			u8 RedFilter = 4;//getBestFilter(redSum);
			u8 GreenFilter = 4;//getBestFilter(greenSum);
			u8 BlueFilter = 4;//getBestFilter(blueSum);
			
			//pOutBytes[pos++] = 0;
			//pOutBytes[pos++] = RedFilter;
			//pOutBytes[pos++] = GreenFilter;
			//pOutBytes[pos++] = BlueFilter;
			
			//writePaethXFilteredPixel(pOutBytes,pos,(*_imageData)(0,0),border,border,border);
			writeFilteredPixel(RedFilter,GreenFilter,BlueFilter,pOutBytes,pos,(*_imageData)(0,0),border,border,border);
		
			for(u32 x=1;x<_imageData->XSize();++x)
				//writePaethXFilteredPixel(pOutBytes,pos,(*_imageData)(x,0),(*_imageData)(x-1,0),border,border);
				writeFilteredPixel(RedFilter,GreenFilter,BlueFilter,pOutBytes,pos,(*_imageData)(x,0),(*_imageData)(x-1,0),border,border);
		}

		for(u32 y=1;y<_imageData->YSize();++y)
		{/*
			memset(&redSum,0,sizeof(redSum));
			memset(&greenSum,0,sizeof(greenSum));
			memset(&blueSum,0,sizeof(blueSum));

			filterColorSum(redSum,greenSum,blueSum,(*_imageData)(0,y),border,(*_imageData)(0,y-1),border);

			for(u32 x=1;x<_imageData->XSize();++x)
				filterColorSum(redSum,greenSum,blueSum,(*_imageData)(x,y),(*_imageData)(x-1,y),(*_imageData)(x,y-1),(*_imageData)(x-1,y-1));
				*/
			u8 RedFilter = 4;//getBestFilter(redSum);
			u8 GreenFilter = 4;//getBestFilter(greenSum);
			u8 BlueFilter = 4;//getBestFilter(blueSum);

			//pOutBytes[pos++] = 0;
			//pOutBytes[pos++] = RedFilter;
			//pOutBytes[pos++] = GreenFilter;
			//pOutBytes[pos++] = BlueFilter;
			//writePaethXFilteredPixel(pOutBytes,pos,(*_imageData)(0,y),border,(*_imageData)(0,y-1),border);
			writeFilteredPixel(RedFilter,GreenFilter,BlueFilter,pOutBytes,pos,(*_imageData)(0,y),border,(*_imageData)(0,y-1),border);
		
			for(u32 x=1;x<_imageData->XSize();++x)
				//writePaethXFilteredPixel(pOutBytes,pos,(*_imageData)(x,y),(*_imageData)(x-1,y),(*_imageData)(x,y-1),(*_imageData)(x-1,y-1));
				writeFilteredPixel(RedFilter,GreenFilter,BlueFilter,pOutBytes,pos,(*_imageData)(x,y),(*_imageData)(x-1,y),(*_imageData)(x,y-1),(*_imageData)(x-1,y-1));
		}
		
		//debug
		/*
		{
			
			assert(pos == 3*(_size.x+1)*_size.y);

			u32 t_pos = 0;

			{
				u8 RedFilter = pOutBytes[t_pos++];
				u8 GreenFilter = pOutBytes[t_pos++];
				u8 BlueFilter = pOutBytes[t_pos++];

				assert( (*_imageData)(0,0) == readFilteredPixel(RedFilter,GreenFilter,BlueFilter,pOutBytes,t_pos,border,border,border) );
		
				for(u32 x=1;x<_imageData->XSize();++x)
					assert( (*_imageData)(x,0) == readFilteredPixel(RedFilter,GreenFilter,BlueFilter,pOutBytes,t_pos,(*_imageData)(x-1,0),border,border) );
			}
		
			for(u32 y=1;y<_imageData->YSize();++y)
			{
				u8 RedFilter = pOutBytes[t_pos++];
				u8 GreenFilter = pOutBytes[t_pos++];
				u8 BlueFilter = pOutBytes[t_pos++];
			
				assert( (*_imageData)(0,y) == readFilteredPixel(RedFilter,GreenFilter,BlueFilter,pOutBytes,t_pos,border,(*_imageData)(0,y-1),border) );
		
				for(u32 x=1;x<_imageData->XSize();++x)
					assert( (*_imageData)(x,y) == readFilteredPixel(RedFilter,GreenFilter,BlueFilter,pOutBytes,t_pos,(*_imageData)(x-1,y),(*_imageData)(x,y-1),(*_imageData)(x-1,y-1)) );
			}

			assert(pos == t_pos);
		}*/

		return pos;
	}

	u32 SVTDataLayer_PNGish::Deserialize(const Vector2<u32>& size,u32 numBytes,const u8* pInBytes,const ISVTSharedDataStorage* pShared)
	{
		static const Pixel<R8G8B8A8> border(0x00000000);
		reset(); 

		_size = size;

		_imageData = new ImageData<R8G8B8A8>(_size.x,_size.y);

		assert(numBytes == 3*(_size.x)*_size.y);

		u32 pos = 0;

		{
			//pos++;
			u8 RedFilter = 4;//pInBytes[pos++];
			u8 GreenFilter = 4;//pInBytes[pos++];
			u8 BlueFilter = 4;//pInBytes[pos++];

			(*_imageData)(0,0) = readFilteredPixel(RedFilter,GreenFilter,BlueFilter,pInBytes,pos,border,border,border);
		
			for(u32 x=1;x<_imageData->XSize();++x)
				(*_imageData)(x,0) = readFilteredPixel(RedFilter,GreenFilter,BlueFilter,pInBytes,pos,(*_imageData)(x-1,0),border,border);
		}
		
		for(u32 y=1;y<_imageData->YSize();++y)
		{
			//pos++;
			u8 RedFilter = 4;//pInBytes[pos++];
			u8 GreenFilter = 4;//pInBytes[pos++];
			u8 BlueFilter = 4;//pInBytes[pos++];
			
			(*_imageData)(0,y) = readFilteredPixel(RedFilter,GreenFilter,BlueFilter,pInBytes,pos,border,(*_imageData)(0,y-1),border);
		
			for(u32 x=1;x<_imageData->XSize();++x)
				(*_imageData)(x,y) = readFilteredPixel(RedFilter,GreenFilter,BlueFilter,pInBytes,pos,(*_imageData)(x-1,y),(*_imageData)(x,y-1),(*_imageData)(x-1,y-1));
		}

		assert(pos == numBytes);

		return pos;
	}
	
	void SVTDataLayer_PNGish::filterColorSum(SumType& red,SumType& green,SumType& blue,const Pixel<R8G8B8A8>& current,const Pixel<R8G8B8A8>& left,const Pixel<R8G8B8A8>& top,const Pixel<R8G8B8A8>& topleft)
	{
		filterSum(red,current.Red(),left.Red(),top.Red(),topleft.Red());
		filterSum(green,current.Green(),left.Green(),top.Green(),topleft.Green());
		filterSum(blue,current.Blue(),left.Blue(),top.Blue(),topleft.Blue());
	}
	
	void SVTDataLayer_PNGish::writeFilteredPixel(u8 redFilter,u8 greenFilter,u8 blueFilter,u8* pOut,u32& pos,const Pixel<R8G8B8A8>& current,const Pixel<R8G8B8A8>& left,const Pixel<R8G8B8A8>& top,const Pixel<R8G8B8A8>& topleft)
	{
		//pOut[pos++]  = 0;
		pOut[pos++] = filterByte(redFilter,current.Red(),left.Red(),top.Red(),topleft.Red());
		pOut[pos++] = filterByte(greenFilter,current.Green(),left.Green(),top.Green(),topleft.Green());
		pOut[pos++] = filterByte(blueFilter,current.Blue(),left.Blue(),top.Blue(),topleft.Blue());
		/*
		assert( unfilterByte(redFilter,pOut[pos-3],left.Red(),top.Red(),topleft.Red()) == current.Red() );
		assert( unfilterByte(greenFilter,pOut[pos-2],left.Green(),top.Green(),topleft.Green()) == current.Green() );
		assert( unfilterByte(blueFilter,pOut[pos-1],left.Blue(),top.Blue(),topleft.Blue()) == current.Blue() );*/
	}
	
	Pixel<R8G8B8A8> SVTDataLayer_PNGish::readFilteredPixel(u8 redFilter,u8 greenFilter,u8 blueFilter,const u8* pIn,u32& pos,const Pixel<R8G8B8A8>& left,const Pixel<R8G8B8A8>& top,const Pixel<R8G8B8A8>& topleft)
	{
		Pixel<R8G8B8A8> result;
		result.SetAlpha(0xff);
		//pos++;
		result.SetRed(unfilterByte(redFilter,pIn[pos++],left.Red(),top.Red(),topleft.Red()));
		result.SetGreen(unfilterByte(greenFilter,pIn[pos++],left.Green(),top.Green(),topleft.Green()));
		result.SetBlue(unfilterByte(blueFilter,pIn[pos++],left.Blue(),top.Blue(),topleft.Blue()));
		return result;
	}
	
	void SVTDataLayer_PNGish::filterSum(SumType& sum,u8 current,u8 left,u8 top,u8 top_left)
	{
		for(int i = 0; i < numFilterMethods; ++i)
			sum[i] += predDiff(i,current,left,top,top_left);
	}

	u8 SVTDataLayer_PNGish::getBestFilter(const SumType& sums)
	{/*
		static std::array<u32,numFilterMethods> bestFilters = { 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0};
		static u32 count = 0;
		*/
		u32 min = sums[0];
		u8 best = 0;
		for(int i = 1; i < numFilterMethods; ++i)
			if(sums[i] < min)
			{
				min = sums[i];
				best = i;
			}/*
		bestFilters[best] ++;
		++count;

		if(count%10000 == 0)
		{
			printf("Filter statistics:\n");
			for(int i = 0; i < numFilterMethods; ++i)
				printf("\tFilter %i - %u uses\n",i,bestFilters[i]);
		}*/

		return best;
	}
	
	u8 SVTDataLayer_PNGish::getPaeth(u8 left,u8 top,u8 top_left)
	{
		i32 p = (i32)left + (i32) top - (i32) top_left;
		u32 d_l = std::abs((i32)left - p);
		u32 d_t = std::abs((i32)top - p);
		u32 d_tl = std::abs((i32)top_left - p);

		if( d_l < d_t && d_l < d_tl )
			return left;
		else if( d_t < d_tl )
			return top;
		else
			return top_left;
	}

	
	u8 SVTDataLayer_PNGish::getPaethX(i32 left,i32 top,i32 top_left)
	{
		if(top_left > std::max(top,left)) return std::min(top,left);
		else if(top_left < std::min(top,left)) return std::max(top,left);
		else return top+left-top_left;
	}
	
	u8 SVTDataLayer_PNGish::predict(u8 method,u8 left,u8 top,u8 top_left)
	{
		i32 avg = (((i32)top + (i32)left) /2);
		i32 grad = ((i32)top + (i32)left - (i32)top_left);
		switch(method)
		{/*
		case 0:
			return 0x00;
		case 1:
			return left;
		case 2:
			return top;
		case 3:
			return getPaeth(left,top,top_left);*/
		case 4:
			return getPaethX(left,top,top_left);
		/*case 5:
			return avg;
		case 6:
			return ((grad>>2) + ((avg*3)>>2));
		case 7:
			return ((grad>>1) + (avg>>1));
		case 8:
			return ((avg>>2) + ((grad*3)>>2));
		case 9:
			return grad;*/
		default:
			assert(!"Unknown filter method for PNGish layer");
			return 0xff;
		}
	}

	u8 SVTDataLayer_PNGish::filterByte(u8 method,u8 current,u8 left,u8 top,u8 top_left)
	{
		return current - predict(method,left,top,top_left);
	}
	

	u8 SVTDataLayer_PNGish::unfilterByte(u8 method,u8 current,u8 left,u8 top,u8 top_left)
	{
		return current + predict(method,left,top,top_left);
	}

	u32 SVTDataLayer_PNGish::predDiff(u8 method,u8 current,u8 left,u8 top,u8 top_left)
	{
		u8 pred = predict(method,left,top,top_left);
		u8 diff = current - pred;
		if( diff > 128 )
			return 255 - diff;
		else
			return diff;
		//return (u32)std::abs((i32)current - (i32)predict(method,left,top,top_left));
	}
}