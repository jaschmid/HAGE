#ifndef __SVT_PAGE_ENCODING_H__
#define __SVT_PAGE_ENCODING_H__

namespace HAGE {


	class SVTDataLayer_Raw : public ISVTDataLayer
	{
	public:
		SVTDataLayer_Raw() : _imageData(nullptr) {}
		virtual ~SVTDataLayer_Raw(){reset();}

		void Initialize(const ImageData<R8G8B8A8>& data) { _imageData = new ImageData<R8G8B8A8>(data); _size = Vector2<u32>(data.XSize(),data.YSize()); }
		
		virtual Vector2<u32> GetSize() const {return _size;}

		virtual u32	GetEncodingId() const {return LAYER_ENCODING_UNCOMPRESSED_RAW;}
		virtual bool GetImageData(const Vector2<u32>& offset,u32 LayerId,ImageRect<R8G8B8A8>& dataOut) const;
		virtual u32 Serialize(u32 maxBytes,u8* pOutBytes) const { if(_imageData == nullptr)return 0; memcpy(pOutBytes,_imageData->GetData(),_imageData->GetDataSize()); return _imageData->GetDataSize(); }
		virtual u32 EstimateSerializeSize() const {return _size.x*_size.y*sizeof(u32);}

		//initialization functions
		virtual void Empty(const Vector2<u32>& size) { reset(); _size = size;}
		virtual void GenerateMipmap(const Vector2<u32>& size,const std::array<const ISVTDataLayer*,16>& parents,u32 borderSize);
		virtual u32 Deserialize(const Vector2<u32>& size,u32 numBytes,const u8* pInBytes,const ISVTSharedDataStorage* pShared);
		
		virtual ISVTDataLayer* WriteRect(Vector2<i32> offset,u32 layer,const ISVTDataLayer* data);
	protected:

		static Pixel<R8G8B8A8> ColorAverage(const Pixel<R8G8B8A8>& _1,const Pixel<R8G8B8A8>& _2, const Pixel<R8G8B8A8>& _3, const Pixel<R8G8B8A8>& _4);

		void reset(){if(_imageData){delete _imageData;_imageData = nullptr;}}

		ImageData<R8G8B8A8>* _imageData;
		Vector2<u32>		_size;
	};

	class SVTDataLayer_PNGish : public SVTDataLayer_Raw
	{
	public:
		virtual u32	GetEncodingId() const {return LAYER_ENCODING_PNGISH_RAW;}
		virtual u32 Serialize(u32 maxBytes,u8* pOutBytes) const;
		virtual u32 Deserialize(const Vector2<u32>& size,u32 numBytes,const u8* pInBytes,const ISVTSharedDataStorage* pShared);
		virtual u32 EstimateSerializeSize() const {return (3*_size.x*_size.y+4);}

	private:
		static const u8 numFilterMethods = 10;

		typedef std::array<u32,numFilterMethods> SumType;

		static u8 getPaeth(u8 left,u8 top,u8 top_left);
		static u8 getPaethX(i32 left,i32 top,i32 top_left);
		static u8 filterByte(u8 method,u8 current,u8 left,u8 top,u8 top_left);
		static u8 predict(u8 method,u8 left,u8 top,u8 top_left);
		static u8 unfilterByte(u8 method,u8 current,u8 left,u8 top,u8 top_left);
		static u32 predDiff(u8 method,u8 current,u8 left,u8 top,u8 top_left);
		static void filterSum(SumType& sum,u8 current,u8 left,u8 top,u8 top_left);
		static u8 getBestFilter(const SumType& sums);
		
		static inline u8 filterBytePaethX(u8 current,u8 left,u8 top,u8 top_left)
		{
			if(top_left > std::max(top,left)) return current - std::min(top,left);
			else if(top_left < std::min(top,left)) return current - std::max(top,left);
			else return current - (u8)((i32)top+(i32)left-(i32)top_left);
		}

		static inline void writePaethXFilteredPixel(u8* pOut,u32& pos,const Pixel<R8G8B8A8>& current,const Pixel<R8G8B8A8>& left,const Pixel<R8G8B8A8>& top,const Pixel<R8G8B8A8>& topleft)
		{
			std::pair<u32,u32> red_minmax = std::minmax<u32>(top.Red(),left.Red());
			std::pair<u32,u32> green_minmax = std::minmax<u32>(top.Green(),left.Green());
			std::pair<u32,u32> blue_minmax = std::minmax<u32>(top.Blue(),left.Blue());

			u32 grad_red = top.Red() + left.Red() - topleft.Red();
			u32 grad_green = top.Green() + left.Green() - topleft.Green();
			u32 grad_blue = top.Blue() + left.Blue() - topleft.Blue();

			u8 red_val = std::max(std::min(grad_red,red_minmax.second),red_minmax.first);
			u8 green_val = std::max(std::min(grad_green,green_minmax.second),green_minmax.first);
			u8 blue_val = std::max(std::min(grad_blue,blue_minmax.second),blue_minmax.first);
			
			u32 filtered  = ((u32)red_val ) |
							(((u32)green_val)<<8) |
							(((u32)blue_val)<<16);
			
			(*(u32*)(&pOut[pos])) = filtered;
			pos+=3;
		}

		static void filterColorSum(SumType& red,SumType& green,SumType& blue,const Pixel<R8G8B8A8>& current,const Pixel<R8G8B8A8>& left,const Pixel<R8G8B8A8>& top,const Pixel<R8G8B8A8>& topleft);
		static void writeFilteredPixel(u8 redFilter,u8 greenFilter,u8 blueFilter,u8* pOut,u32& pos,const Pixel<R8G8B8A8>& current,const Pixel<R8G8B8A8>& left,const Pixel<R8G8B8A8>& top,const Pixel<R8G8B8A8>& topleft);
		static Pixel<R8G8B8A8> readFilteredPixel(u8 redFilter,u8 greenFilter,u8 blueFilter,const u8* pIn,u32& pos,const Pixel<R8G8B8A8>& left,const Pixel<R8G8B8A8>& top,const Pixel<R8G8B8A8>& topleft);
	};

	
	class SVTDataLayer_JpegXR : public SVTDataLayer_Raw
	{
	public:
		virtual u32	GetEncodingId() const {return LAYER_ENCODING_JPEG_XR_RAW;}
		virtual u32 Serialize(u32 maxBytes,u8* pOutBytes) const;
		virtual u32 Deserialize(const Vector2<u32>& size,u32 numBytes,const u8* pInBytes,const ISVTSharedDataStorage* pShared);
		virtual u32 EstimateSerializeSize() const {return (4*(_size.x)*_size.y);}
	private:
		static void static_jxr_read_block(void* image, int mx, int my, int*data);
		static void static_jxr_write_block(void* image, int mx, int my, int*data);

		void jxr_read_block(void* image, int mx, int my, int*data) const;
		void jxr_write_block(void* image, int mx, int my, int*data);
	};
};

#endif