#ifndef VIRTUA_TEXTURE_GENERATOR__INCLUDED
#define VIRTUA_TEXTURE_GENERATOR__INCLUDED

#include "header.h"

#include <SDK/EditableImage.h>

#include "VTPacker.h"
#include "dependancies\sqlite3.h"
#include "boost/filesystem.hpp" 
#include <stdio.h>
#define FREEIMAGE_LIB
#include <FreeImage.h>

namespace HAGE {


	class SparseVirtualTextureGenerator : private ISVTFileDataSource
	{
	public:
		
		/* Public Types */

		class PlacedTexture
		{
		public:
			f32 GetUBegin() const{return (f32)packing.GetX() / (float)base->GetSize();}
			f32 GetUSize() const{return (f32)packing.GetXSize() / (float)base->GetSize();}
			f32 GetVBegin() const{return (f32)packing.GetY() / (float)base->GetSize();}
			f32 GetVSize() const{return (f32)packing.GetYSize() / (float)base->GetSize();}
		private:
			PlacedTexture(const SparseVirtualTextureGenerator* parent,const VTPacker::Packing& pack,u32 imageIndex) : 
			   base(parent),packing(pack),savedImage(imageIndex) {}

			const SparseVirtualTextureGenerator* base;
			const VTPacker::Packing	packing;
			const u32				savedImage;

			friend class SparseVirtualTextureGenerator;
		};

		typedef const PlacedTexture& TextureReference;
		typedef std::vector<std::pair<TextureReference,f32>> RelationArray;

		/* Public Constructors */

		SparseVirtualTextureGenerator();
		~SparseVirtualTextureGenerator();

		/* Public Functions */
		
		bool New(u32 pagesize,u32 pagedepth, u32 bordersize);
		void FinalizePlacement();
		void Commit();
		void Save(const char* targetFilename);
		TextureReference PlaceTexture(const ISVTDataLayer* pData,RelationArray relationArray);
		void ResolveTexturePlacement();

		inline u32 GetSize() const{
			return (1<<(_settings.pageDepth-1))*getInnerPageSize();
		}

		/* Interface functions */

		
		void GetSettings(SVTSettings* pSettings) const{ *pSettings = _settings; }
		void ReadPage(u64 index,SVTPage* out) const {_pageDataStorage->RetrievePage(index,out);}
		u32 SharedDataSize(u32 index) const {return _sharedDataStorage->SharedDataSize(index);}
		const void* AccessSharedData(u32 index) const {return _sharedDataStorage->AccessSharedData(index);}
		void ReleaseSharedData(u32 index) const {return _sharedDataStorage->ReleaseSharedData(index);}

		u32 AllocateSharedData(u32 size,const void* pData) {return _sharedDataStorage->AllocateSharedData(size,pData);}

	private:

		/* Private Types */

		class RandomTempStorage
		{
		public:
			RandomTempStorage();
			~RandomTempStorage();
			void SaveData(u32 index,u32 numBytes,const u8* pData);
			u32 GetDataSize(u32 index) const;
			u32 LoadData(u32 index,u32 maxNumBytes,u8* pDataOut) const;
		private:
			mutable sqlite3* storage;
			mutable sqlite3_stmt* save_stmt;
			mutable sqlite3_stmt* retrieve_stmt;
			mutable sqlite3_stmt* get_size_stmt;
		};

		class SequentialTempStorage
		{
		public:
			SequentialTempStorage();
			~SequentialTempStorage();
			u32 SaveData(u32 numBytes,const u8* pData);
			u32 GetDataSize(u32 index) const;
			u32 LoadData(u32 index,u32 maxNumBytes,u8* pDataOut) const;
		private:
			struct Entry
			{
				u64 offset;
				u32 compressed_size;
				u32 uncompressed_size;
			};
			mutable FILE*		_tempfile;
			std::string			_tempfilename;
			std::vector<Entry>	_entries;
		};

		class TempImageStorage : private SequentialTempStorage
		{
		public:
			TempImageStorage(const ISVTSharedDataStorage* pShared);
			~TempImageStorage();
			u32 StoreImage(const ISVTDataLayer* pData);
			ISVTDataLayer* RetrieveImage(u32 Index) const;
		private:
			struct metrics
			{
				u32 sizeX;
				u32 sizeY;
				u32 encoding;
			};

			std::vector<metrics> _metrics;
			const ISVTSharedDataStorage* const _shared;
		};
					
		class TempSharedStorage : private SequentialTempStorage, public ISVTSharedDataStorage
		{
		public:
			TempSharedStorage();
			~TempSharedStorage();

			u32 SharedDataSize(u32 index) const;
			const void* AccessSharedData(u32 index) const;
			void ReleaseSharedData(u32 index) const;
			u32 AllocateSharedData(u32 size,const void* pData);
		private:

			static const int max_items = 100;

			void garbageCollect() const;
						
			struct dataEntry
			{
				void* pData;
				u32 refCount;
			};

			mutable std::map<u32,dataEntry>			dataCache;
		};

		
		class TempPageStorage : private RandomTempStorage
		{
		public:
			TempPageStorage(u32 pageSize,u32 maxDepth,const ISVTSharedDataStorage* shared);
			~TempPageStorage();
			void StorePage(u32 pageIndex,const SVTPage* pData);
			void RetrievePage(u32 Index,SVTPage* pDataOut) const;
		private:
			struct metrics
			{
				SVTPage::SVTPageHeader pageHeader;
			};

			static const u32 maxPageSize = 100*1024;

			const u32 _pageSize;
			const u32 _maxDepth;
			std::vector<metrics> _metrics;
			mutable std::array<u8,maxPageSize> _tempData;
			const ISVTSharedDataStorage* const _shared;
		};

		struct Color
		{
			u8 r,g,b,a;
		};
		
		static void OutputImage(char* filename,const Color* pData,u32 xSize,u32 ySize)
		{
			FIBITMAP* b =FreeImage_Allocate(xSize,ySize,24);

			for(u32 iy = 0; iy < ySize;iy++)
				for(u32 ix = 0; ix < xSize;ix++)
					FreeImage_SetPixelColor(b,ix,iy,(RGBQUAD*)&pData[iy*xSize+ix]);

			BOOL res = FreeImage_Save(FIF_PNG,b,filename);
			FreeImage_Unload(b);
		}

		class BorderedImageReader
		{
		public:
			BorderedImageReader(const Color* pData,const std::vector<bool>& PageStatus,u32 xInputPages,u32 yInputPages,u32 PageOuterSize,u32 PageInnerSize) :
				_data(pData),
				_pageStatus(PageStatus),
				_xInputPages(xInputPages),
				_yInputPages(yInputPages),
				_pageOuterSize(PageOuterSize),
				_pageInnerSize(PageInnerSize),
				_pageBorderSize((PageOuterSize - PageInnerSize)/2),
				_rowStride(xInputPages*PageOuterSize)
			{
			}

			inline const Color& GetPixelFromInner(u32 absX,u32 absY) const
			{
				u32 xPage = absX/_pageInnerSize;
				u32 yPage = absY/_pageInnerSize;
				u32 xPixel = absX%_pageInnerSize;
				u32 yPixel = absY%_pageInnerSize;
				return GetPixelFromInner(xPage,yPage,xPixel,yPixel);
			}

			inline const Color& GetPixelFromInner(u32 xPage,u32 yPage,u32 xPixel,u32 yPixel) const
			{
				static const Color Black = {0,0,0,0};

				assert(xPage < _xInputPages);
				assert(yPage < _yInputPages);
				assert(xPixel < _pageInnerSize);
				assert(yPixel < _pageInnerSize);

				if(_pageStatus[_xInputPages*yPage + xPage])
				{
					const u32 xAbsOut = xPage * _pageOuterSize + xPixel + _pageBorderSize;
					const u32 yAbsOut = yPage * _pageOuterSize + yPixel + _pageBorderSize;
					return _data[yAbsOut*_rowStride + xAbsOut];
				}
				else
					return Black;
			}

			inline const Color& GetPixelFromOuter(u32 absX,u32 absY) const
			{
				const u32 xPage = absX/_pageOuterSize;
				const u32 yPage = absY/_pageOuterSize;
				const u32 xPixel = absX%_pageOuterSize;
				const u32 yPixel = absY%_pageOuterSize;
				return GetPixelFromOuter(xPage,yPage,xPixel,yPixel);
			}

			inline const Color& GetPixelFromOuter(u32 xPage,u32 yPage,u32 xPixel,u32 yPixel) const
			{
				const u32 yInPixel = (_pageInnerSize + yPixel - _pageBorderSize)% _pageInnerSize;
				const u32 xInPixel = (_pageInnerSize + xPixel - _pageBorderSize)% _pageInnerSize;
				const u32 yInPage = yPage + (_pageInnerSize + yPixel - _pageBorderSize)/ _pageInnerSize - 1;
				const u32 xInPage = xPage +(_pageInnerSize + xPixel - _pageBorderSize)/ _pageInnerSize - 1;
				return GetPixelFromInner(xInPage,yInPage,xInPixel,yInPixel);
			}

			u32 GetInnerX() const {return _xInputPages*_pageInnerSize;}
			u32 GetInnerY() const {return _yInputPages*_pageInnerSize;}
			u32 GetOuterX() const {return _xInputPages*_pageOuterSize;}
			u32 GetOuterY() const {return _yInputPages*_pageOuterSize;}
			u32 GetPageOuter() const{return _pageOuterSize;}
			u32 GetPageBorder() const{return _pageBorderSize;}
			u32 GetPageInner() const{return _pageInnerSize;}
			u32 GetPagesX() const{return _xInputPages;}
			u32 GetPagesY() const{return _yInputPages;}
			u64 GetInnerTotal() const{return (u64)GetInnerX()*(u64)GetInnerY();}
			u64 GetOuterTotal() const{return (u64)GetOuterX()*(u64)GetOuterY();}
		private:
			const Color* _data;
			const std::vector<bool>& _pageStatus;
			const u32 _xInputPages;
			const u32 _yInputPages;
			const u32 _rowStride;
			const u32 _pageOuterSize;
			const u32 _pageBorderSize;
			const u32 _pageInnerSize;
		};
		
		/* Private Functions */

		void writePlacedTextures();
		/*
		static void applyBoxFilter(const BorderedImageReader& source,u32 InputBorder,Color* pOutput,u32 xOutputSize,u32 yOutputSize);
		void splitPagesFromImage(i64 xPageBegin,u64 xPageEnd,u64 yPageBegin,i64 yPageEnd,u32 depth,const Color* pIn,const std::vector<bool>* pageStatus,std::vector<Color>& tempBuffer,u32 flags = 0,u64 inputOffset =0, u64 inputStride = 0);
		bool mergePagesToImage(i32 xPageBegin,i32 xPageEnd,i32 yPageBegin,i32 yPageEnd,u32 depth,Color* pOut,std::vector<bool>* pageStatus,std::vector<Color>& tempBuffer,u32 flags = 0);
		*/
		bool finalizeDepthLayer(u32 depth);
		//static void generateBorders(Color* pDataOut,	const BorderedImageReader& imageReader);

		void writeImageToMipLayer(const ISVTDataLayer* data,u32 data_layer,u32 mip_layer,Vector2<u32> offset);

		inline u32 getNumXPagesAtDepth(u32 depth) const {return 1 << depth;}
		inline u32 getXPages() const {return 1 << (_settings.pageDepth-1);}
		inline u32 getYPages() const {return getXPages();}
		inline u32 getNumLayers() const {return _settings.pageDepth;}
		inline u32 getInnerPageSize() const {return _settings.pageInnerSize;}
		inline u32 getOuterPageSize() const {return _settings.pageSize;}
		inline u32 getPageBorderSize() const {return _settings.borderSize;}
		inline u32 getPageInnerPixelCount() const {return getInnerPageSize()*getInnerPageSize();}
		inline u32 getPageOuterPixelCount() const {return getOuterPageSize()*getOuterPageSize();}
		
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

		inline static u32 getPageIndex(u16 x,u16 y,u8 l)
		{
			return bitShuffle16bit(x,y) + (u32)Exp2SquaredSeries(l);
		}

		/* Private Members */
		
		TempImageStorage*			_sourceDataStorage;
		std::list<PlacedTexture>	_textureList;
		VTPacker*					_vtPacker;

		SVTSettings				_settings;
		TempSharedStorage*		_sharedDataStorage;
		TempPageStorage*		_pageDataStorage;

		friend class TempPageStorage;
	};

	}
#endif