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
		virtual u32 EstimateSerializeSize() const {return (4*(_size.x+1)*_size.y);}

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
		
		static void filterColorSum(SumType& red,SumType& green,SumType& blue,const Pixel<R8G8B8A8>& current,const Pixel<R8G8B8A8>& left,const Pixel<R8G8B8A8>& top,const Pixel<R8G8B8A8>& topleft);
		static void writeFilteredPixel(u8 redFilter,u8 greenFilter,u8 blueFilter,u8* pOut,u32& pos,const Pixel<R8G8B8A8>& current,const Pixel<R8G8B8A8>& left,const Pixel<R8G8B8A8>& top,const Pixel<R8G8B8A8>& topleft);
		static Pixel<R8G8B8A8> readFilteredPixel(u8 redFilter,u8 greenFilter,u8 blueFilter,const u8* pIn,u32& pos,const Pixel<R8G8B8A8>& left,const Pixel<R8G8B8A8>& top,const Pixel<R8G8B8A8>& topleft);
	};
};

#endif