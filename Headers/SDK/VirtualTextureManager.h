/********************************************************/
/* FILE: VirtualTextureManager.h                        */
/* DESCRIPTION: Implements the managing backbone for    */
/*              Virtual textures                        */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/

#include <HAGE.h>

#ifndef __INTERNAL_VIRTUAL_TEXTURE_MANAGER_H__
#define __INTERNAL_VIRTUAL_TEXTURE_MANAGER_H__

namespace HAGE {

	class VirtualTextureManager : public IStreamingResourceProvider
	{
	public:
		virtual void ProcessResourceAccess(IResource* feedback);
		virtual void CreateResourceAccessSet(std::vector<IResource*>& accesses);
		virtual bool QueryDependancies(const ResourceAccess* dependanciesIn,const std::pair<std::string,guid>** pDependanciesOut,u32& nDependanciesInOut){return false;}

		static IStreamingResourceProvider* LoadVirtualTexture(IDataStream* stream);

		void NotifyFeedbackBufferDeleted(u32 idx);

	private:

		const static int DefaultCacheSize = 1024*1024*64;

		VirtualTextureManager();
		~VirtualTextureManager();
		bool Load(IDataStream* stream);

		class FeedbackBuffer : public IVirtualTexture
		{
		public:
			// public functions
			virtual const APIWTexture* GetCurrentVTCache() const {return _cache;}
			virtual const APIWTexture* GetCurrentVTRedirection() const {return _redirection;}
			virtual u32 GetCacheSize() const {return _currentCacheSize;}
			virtual void AdjustCacheSize(u32 newSize) const {_requestedCacheSize=newSize;}
			virtual void ProvideFeedback() const{++_nFeedback;}

			//internal functions

			FeedbackBuffer(const APIWTexture* Cache,const APIWTexture* Redirection,u32 CacheSize,u32 InternalIndex,VirtualTextureManager* Parent);
			~FeedbackBuffer();

			u32 GetRequestedCacheSize(){return _requestedCacheSize;}
			void SetCacheSize(u32 CacheSize){_requestedCacheSize = CacheSize; _currentCacheSize=CacheSize;}

			u32 GetInternalIndex(){return _index;}

		private:
			mutable	int				_nFeedback;
			mutable	u32				_requestedCacheSize;
			u32						_currentCacheSize;
			const u32				_index;
			const APIWTexture*		_cache;
			const APIWTexture*		_redirection;
			VirtualTextureManager*	_parent;
		};

		typedef std::pair<APIWTexture*,u32> UpdatableTexture;

		struct FeedbackElement
		{
			FeedbackBuffer*		buffer;
			UpdatableTexture	cache;
			UpdatableTexture	redirection;
			u32					elementIndex;
			u32					setIndex;
		};

		struct PageData
		{
			u32 nBytes;
			u32 nPageIndex;
			u32 nPageX;
			u32 nPageY;
			u32 nLevel;
			u8*	pData;
		};

		class PageCache
		{
		public:
			PageCache(u32 CacheSizeBytes,u32 PageSize,u32 nXPages,u32 nYPages);
			~PageCache();

			u32 GetNumCachePages() {return nCachePagesX*nCachePagesY;}
			void LoadPage(const PageData* page,u32 CacheIndex);

			UpdatableTexture CreateCacheTexture() const;
			void UpdateCacheTexture(UpdatableTexture& cacheTexture) const;

			class PageRedirection
			{
			public:
				PageRedirection()
				{
					DestXBegin=0.0f;
					DestXEnd=0.0f;
					DestYBegin=0.0f;
					DestYEnd=0.0f;
				}

			private:
				PageRedirection(f32 xBegin,f32 xEnd,f32 yBegin,f32 yEnd)
				{
					DestXBegin=xBegin;
					DestXEnd=xEnd;
					DestYBegin=yBegin;
					DestYEnd=yEnd;
				}

				f32 DestXBegin;
				f32 DestXEnd;
				f32 DestYBegin;
				f32 DestYEnd;
			};

			void UpdateCacheSize(u32 CacheSizeBytes);

			PageRedirection CreateRedirection(u32 CacheIndex,f32 xBegin,f32 xEnd,f32 yBegin,f32 yEnd) const;
		private:

			struct Pixel
			{
				u8 r,g,b,a;
			};

			std::vector<PageData>	pageData;
			u32						nCachePagesX, nCachePagesY;

			std::vector<Pixel>		cacheData;
			u32						nCacheX, nCacheY;

			u32						nDataVersion;

			APIWTexture*			pTextureBuffer;

			const u32				nPageSizeX,nPageSizeY;
			const u32				nPagesX,nPagesY;
		};

		typedef PageCache CacheType;

		template<class _TCache> class MipMappedPageRedirectionTable
		{		

		public:
			MipMappedPageRedirectionTable(u32 nMaxXPages,u32 nMaxYPages,const _TCache* Cache,u32 nLevel = 0) : 
				_nPagesX(1 << nLevel),
				_nPagesY(1 << nLevel),
				_redirectionTable((1 << nLevel)*(1 << nLevel)),
				_redirectionTableIndices((1 << nLevel)*(1 << nLevel)),
				_subPage(AllocateSubTable(nMaxXPages,nMaxYPages,Cache,nLevel)),
				_cache(Cache),
				_Level(nLevel)
			{
				for(int i = 0; i < _redirectionTable.size(); ++i)
					_redirectionTableIndices[i] = 0xffffffff;
			}

			void UnloadPage(u32 nPageX,u32 nPageY,u32 nLevel)
			{
				assert(nLevel > 0);

				if(nLevel - 1 > _Level)
					_subPage->UnloadPage(nPageX,nPageY,nLevel);

				assert(nLevel == _Level+1);

				u32 newIndex = _redirectionTableIndices[(nPageY/2) * _nPagesX + (nPageX/2)];

				_subPage->replacePage(nPageX,nPageY,0,0xffffffff,newIndex);
			}

			void LoadPage(u32 nPageX,u32 nPageY,u32 nLevel,u32 CacheIndex)
			{
				if(nLevel > _Level)
					_subPage->LoadPage(nPageX,nPageY,nLevel,CacheIndex);

				assert(nLevel == _Level);

				u32 oldIndex = _redirectionTableIndices[nPageY * _nPagesX + nPageX];

				replacePage(nPageX,nPageY,0,oldIndex,CacheIndex);
			}
			
			UpdatableTexture CreateRedirectionTexture() const
			{
				UpdatableTexture result;
				return result;
			}
			void UpdateRedirectionTexture(UpdatableTexture& redirectionTexture) const
			{
			}

		private:
			static MipMappedPageRedirectionTable* AllocateSubTable(u32 nXPages,u32 nYPages,const PageCache* cache,u32 nLevel)
			{
				if( (((u32)1 << nLevel) > nXPages) &&  (((u32)1 << nLevel) > nYPages) )
					return nullptr;
				return new MipMappedPageRedirectionTable(nXPages,nYPages,cache,nLevel+1);
			}
			
			void replacePage(u32 nPageX,u32 nPageY,u32 nSplitDepth,u32 OldCacheIndex,u32 NewCacheIndex)
			{
				u32 numSplit = 1 << nSplitDepth;
				u32 xBegin = nPageX << nSplitDepth;
				u32 yBegin = nPageY << nSplitDepth;
				f32 splitSize = 1.0f / (f32)(numSplit);
				for(int iy = 0; iy < numSplit; iy++)
					for(int ix = 0; ix < numSplit; ix++)
					{

						if(OldCacheIndex == 0xffffffff)
						{
							assert(numSplit == 1);
							OldCacheIndex = _redirectionTableIndices[(yBegin+iy) * _nPagesX + xBegin + ix];
						}

						//only update lower detail loaded data
						if(_redirectionTableIndices[(yBegin+iy) * _nPagesX + xBegin + ix] == OldCacheIndex)
						{
							PageCache::PageRedirection& ref = _redirectionTable[(yBegin+iy) * _nPagesX + xBegin + ix];
							_redirectionTableIndices[(yBegin+iy) * _nPagesX + xBegin + ix] = NewCacheIndex;
							ref = _cache->CreateRedirection(NewCacheIndex,ix *splitSize,(ix+1) *splitSize,iy *splitSize,(iy+1) *splitSize);
						}
					}

				if(_subPage)
					_subPage->replacePage(nPageX,nPageY,nSplitDepth+1,OldCacheIndex,NewCacheIndex);
			}

			std::vector<PageCache::PageRedirection>	_redirectionTable;
			std::vector<u32>				_redirectionTableIndices;

			const _TCache* const			_cache;
			MipMappedPageRedirectionTable * const	_subPage;
			const u32						_nPagesX,_nPagesY;
			const u32						_Level;
		};

		u32				nTextureX, nTextureY;
		u32				nPageSizeX,nPageSizeY;
		u32				nPagesX,nPagesY;

		u32				_cacheSize;
		u32				_nCurrentVersion;

		//our page Cache
		CacheType*		_pageCache;

		//and our redirection table
		MipMappedPageRedirectionTable<PageCache>* _redirectionTable;

		//our access elements
		std::vector<FeedbackElement> _feedbackElements;

		//data source
		IDataStream*		_pStreamSource;
		SparseVirtualTextureFile* _pTextureSource;
	};

};

#endif