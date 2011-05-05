/********************************************************/
/* FILE: SparseVirtualTextureFile.h                     */
/* DESCRIPTION: Declares a class intended to aid in the */
/*              usage of Spars Virtual Texture Files    */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/

#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef __SPARSE_VIRTUAL_TEXTURE_FILE__
#define __SPARSE_VIRTUAL_TEXTURE_FILE__

#include "EditableImage.h"

namespace HAGE {


	enum SVTLAYERS
	{
		SVTLAYER_UNDEFINED			= 0x00000000,
		SVTLAYER_DIFFUSE_COLOR		= 0x00000001,
		SVTLAYER_ALPHA				= 0x00000002,
		SVTLAYER_SPECULAR_COLOR	= 0x00000004,
		SVTLAYER_SPECULAR_POWER	= 0x00000008,
		SVTLAYER_EMISSIVE_COLOR	= 0x00000010,
		SVTLAYER_NORMAL_MAP		= 0x00000040,
	};
	
	class ISVTSharedDataStorage
	{
	public:
		virtual u32 SharedDataSize(u32 index) const = 0;
		virtual const void* AccessSharedData(u32 index) const = 0;
		virtual void ReleaseSharedData(u32 index) const = 0;
		virtual u32 AllocateSharedData(u32 size,const void* pData) = 0;
	};

	class ISVTDataLayer
	{
	public:

		virtual ~ISVTDataLayer() {}

		virtual Vector2<u32> GetSize() const = 0;

		virtual u32	GetEncodingId() const = 0;
		virtual bool GetImageData(const Vector2<u32>& offset,u32 LayerId,ImageRect<R8G8B8A8>& dataOut) const = 0;
		virtual u32 Serialize(u32 maxBytes,u8* pOutBytes) const = 0; 
		virtual u32 EstimateSerializeSize() const = 0;

		//initialization functions
		virtual void Empty(const Vector2<u32>& size) = 0;
		virtual void GenerateMipmap(const Vector2<u32>& size,const std::array<const ISVTDataLayer*,16>& parents,u32 borderSize) = 0;
		virtual u32 Deserialize(const Vector2<u32>& size,u32 numBytes,const u8* pInBytes,const ISVTSharedDataStorage* pShared) = 0;

		//update function
		virtual ISVTDataLayer* WriteRect(Vector2<i32> offset,u32 layer,const ISVTDataLayer* data) = 0;

		static const u32		MaxLayerSerializedSizeFactor	= 4; // max size is Factor * x_size * y_size
		static ISVTDataLayer* CreateLayer(u32 encodingId);
	};

	enum
	{
		LAYER_ENCODING_UNCOMPRESSED_RAW	= 0,
		LAYER_ENCODING_PNGISH_RAW		= 1,
		LAYER_ENCODING_JPEG_XR_RAW		= 2,
		LAYER_ENCODING_COMPOSITE		= 3,
		LAYER_ENCODING_BLENDED			= 4
	};
	
	class SVTPage
	{
	public:

		enum PAGE_FLAGS
		{
			PAGE_COMPRESSED_NONE		= 0,
			PAGE_COMPRESSED_DEFLATE		= 1,
			PAGE_COMPRESSED_LZMA		= 2,
			PAGE_COMPRESSED_SNAPPY		= 4,
		};

		struct SVTPageHeader
		{
			u16 flags;
			u16 layers;
		};
		
		SVTPage(u32 pageSize);
		~SVTPage();

		u32 LayerMask();
		const ISVTDataLayer* GetLayer(u32 Layer) const;
		bool UpdateLayer(u32 Layer,ISVTDataLayer*);
		u32 Serialize(u32 maxBytes,u8* pOutBytes,SVTPageHeader* headerOut,PAGE_FLAGS requested_compression = PAGE_COMPRESSED_DEFLATE) const;

		//initialization functions
		void Empty();
		void GenerateMipmap(const std::array<const SVTPage*,16>& parents,u32 borderSize);
		void WriteRect(Vector2<i32> offset,u32 layer,const ISVTDataLayer* data);
		bool Deserialize(u32 pageSize,const SVTPageHeader* header, u32 numBytes,const u8* pInBytes,const ISVTSharedDataStorage* pShared);
	private:
		struct LayerHeader
		{
			u32 layerSize;
			u16	layerEncoding;
			u16	layerFlags;
		};

		const u32 _pageSize;
		u32 _layerMask;

		static u32 getLayerIndex(u32 Layer);
		static u32 getLayerMask(u32 Layer);

		void reset(); // clears current contents

		static u32 compress_layer(const void* pInData,u32 pInDataSize,void* pOutData,u32 maxOutSize,PAGE_FLAGS compression);
		static u32 decompress_layer(const void* pInData,u32 pInDataSize,void* pOutData,u32 maxOutSize,PAGE_FLAGS compression);

		enum
		{
			LAYER_INDEX_DIFFUSE = 0,
			LAYER_INDEX_ALPHA = 1,
			LAYER_INDEX_SPECULAR_COLOR = 2,
			LAYER_INDEX_SPECULAR_POWER = 3,
			LAYER_INDEX_EMISSIVE_COLOR = 4,
			LAYER_INDEX_NORMAL_MAP = 5
		};

		static const u32 nNumLayers = 6;
		static const u32 defaultMixedEncoding = LAYER_ENCODING_UNCOMPRESSED_RAW;
		static const u32 nMaxStackBuffer = 1024*128;

		std::array<ISVTDataLayer*,nNumLayers> _layers;
	};


	class ISVTFileDataSource : public ISVTSharedDataStorage
	{
	public:
		
		struct SVTSettings
		{
			u32 pageSize;
			u32 pageInnerSize;
			u32 borderSize;
			u32 pageDepth;
			u32 nSharedDataEntries;
		};

		virtual void GetSettings(SVTSettings* pSettings) const = 0;
		virtual void ReadPage(u64 index,SVTPage* out) const = 0;
		virtual u32 SharedDataSize(u32 index) const = 0;
		virtual const void* AccessSharedData(u32 index) const = 0;
		virtual void ReleaseSharedData(u32 index) const = 0;

		virtual u32 AllocateSharedData(u32 size,const void* pData) = 0;
	};
	
	class SparseVirtualTextureFile
	{
	public:
		SparseVirtualTextureFile();
		~SparseVirtualTextureFile();

		void Open(const ISVTFileDataSource* source);
		void Open(IDataStream* dataStream);

		void Save(const char* destFile) const;
		
		bool ReadPage(u64 PageIndex,const Vector2<u32>& offset,ImageRect<R8G8B8A8>& pDataOut) const;
		void ReadImageFromVirtualTexture(u64 vtXOffset, u64 vtYOffset,ImageRect<R8G8B8A8>& pDataOut,int level = -1) const;

		//inline functions
		u64 GetPageIndex(u64 xLocation,u64 yLocation,u32 depth) const
		{
			if(depth == 0)
			{
				assert(xLocation == 0);
				assert(yLocation == 0);
				return 0;
			}
			else
			{
				u64 result = GetFirstSubPage(GetPageIndex(xLocation/2,yLocation/2,depth-1));
				if(xLocation%2)
					result += 1;
				if(yLocation%2)
					result +=2;
				return result;
			}
		}

		inline u32 GetSize() const
		{
			return GetPageInnerSize() * GetNumXPagesAtDepth(_fileHeader.pagedepth-1);
		}

		inline u64 GetTotalNumPages() const
		{
			return _pageHeaders.size();
		}

		inline u64 GetMaxDepthPages() const
		{
			return 1i64 << (((u64)GetMaxDepth())*2i64);
		}

		inline u32 GetPageInnerSize() const
		{
			return _fileHeader.pagesize-2*_fileHeader.bordersize;
		}
		inline u32 GetPageInnerPixelCount() const
		{
			return GetPageInnerSize()*GetPageInnerSize();
		}

		inline u32 GetPageOuterSize() const
		{
			return _fileHeader.pagesize;
		}

		inline u32 GetPageOuterPixelCount() const
		{
			return GetPageOuterSize()*GetPageOuterSize();
		}

		inline u32 GetNumLayers() const
		{
			return _fileHeader.pagedepth;
		}

		inline u32 GetMaxDepth() const
		{
			return GetNumLayers()-1;
		}
				
		inline static u64 GetFirstSubPage(u64 page)
		{
			return page*4+1;
		}
		inline static u64 GetParentPage(u64 page)
		{
			return (page-1)/4;
		}
		inline static u64 GetFirstPageOfSubpageBlock(u64 page)
		{
			return GetFirstSubPage(GetParentPage(page));
		}

		inline static void GetPageLocation(u64 page,u32& xPage,u32& yPage,u32& Level)
		{
			if(page == 0)
			{
				xPage = 0;
				yPage = 0;
				Level = 0;
			}
			else
			{
				GetPageLocation(GetParentPage(page),xPage,yPage,Level);
				++Level;
				xPage*=2;
				yPage*=2;
				switch((page - 1)%4)
				{
					case 0: break;	
					case 1: ++xPage; break;
					case 2: ++yPage; break;
					case 3: ++xPage;++yPage; break;
				}

			}
		}
		
		inline static u32 GetNumXPagesAtDepth(u32 pagedepth)
		{
			return 1 << (pagedepth);
		}
		inline static u64 GetNumPagesAtDepth(u32 pagedepth)
		{
			return 1i64 << ((u64)pagedepth*2i64);
		}

		inline static u64 GetTotalNumPagesUntilDepth(u32 pagedepth)
		{
			u64 numPages = 0;
			for(int i = 0 ; i<(int)pagedepth+1;++i)
				numPages |= GetNumPagesAtDepth(i);
			return numPages;
		}

		inline static u64 GetFirstPageAtDepth(u32 pagedepth)
		{
			if(pagedepth == 0)
				return 0;
			return GetTotalNumPagesUntilDepth(pagedepth -1);
		}
	private:

		struct FileHeader
		{
			char fourCC[4];//HSVT
			u16 majorVersion; // 0 currently
			u16 minorVersion; // 2 currently
			u32 pagesize;
			u32 bordersize;
			u32 pagedepth; // how deep does the rabbit hole go?

			u32 nPageHeaders;
			u64 offsetPageHeaders;
			u32 nSharedDataHeaders;
			u64 offsetSharedDataHeaders;
		};

		struct PageHeader
		{
			u64 file_offset;			
			u32 page_size;
			SVTPage::SVTPageHeader pageHeader;
		};

		struct SharedDataHeader
		{
			u64 file_offset;
			u32 data_size;
			u16 data_flags;
			u16 importance;//number of pages that reference the shared data not implemented
		};

		class SharedDataCache : public ISVTSharedDataStorage
		{
		public:
			SharedDataCache(const ISVTFileDataSource* source);
			SharedDataCache(const std::vector<SharedDataHeader>& sharedEntries,IDataStream* dataStream);
			~SharedDataCache();

			u32 SharedDataSize(u32 index) const;
			const void* AccessSharedData(u32 index) const;
			void ReleaseSharedData(u32 index) const;
			u32 AllocateSharedData(u32 size,const void* pData)
			{ 
				return -1;//not implemented for this one 
			}
		private:
			u32 writeHeaders(FILE* file,std::vector<SharedDataHeader>& headers) const;
			u32 writeSharedData(FILE* file,std::vector<SharedDataHeader>& headers) const;
			void garbageCollect() const;

			static const u32 max_items = 100;

			struct dataEntry
			{
				void* pData;
				u32 refCount;
			};

			std::vector<SharedDataHeader>			_sharedDataHeaders;
			mutable std::map<u32,dataEntry>			_dataCache;
			mutable IDataStream*					_dataStream;
			const ISVTFileDataSource*				_dataSource;

			friend class SparseVirtualTextureFile;
		};

		const static int maxPageSize = 128*1024;

		void getPage(u64 index,SVTPage* pPageOut) const;

		const ISVTFileDataSource* _dataSource;
		IDataStream* _dataStream;

		bool _bOpen;

		FileHeader _fileHeader;
		std::vector<PageHeader> _pageHeaders;
		SharedDataCache* _dataCache;
	};
}

#endif