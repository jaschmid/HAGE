/********************************************************/
/* FILE: VirtualTextureManager.h                        */
/* DESCRIPTION: Implements the managing backbone for    */
/*              Virtual textures                        */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/

#include <HAGE.h>
#include <deque>

#include "VTFeedbackAnalyzer.h"
#include <boost/circular_buffer.hpp>

#ifndef __INTERNAL_VIRTUAL_TEXTURE_MANAGER_H__
#define __INTERNAL_VIRTUAL_TEXTURE_MANAGER_H__

namespace HAGE {

	class VirtualTextureManager : public IStreamingResourceProvider
	{
	public:
		virtual bool ProcessResourceAccess(IResource* feedback);
		virtual bool CheckFeedbackBufferReady(IResource* feedback);
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
			virtual const APIWTexture* _Debug_GetFeedbackTexture() const {return _feedbackTexture;}
			virtual const APIWConstantBuffer* GetSettings() const { _pVTSettings->UpdateContent(&_settings);return _pVTSettings;}

			virtual u32 GetId() const;
			virtual APIWEffect* BeginFeedback(RenderingAPIWrapper* pRenderer) const;
			virtual void EndFeedback(RenderingAPIWrapper* pRenderer) const;

			//internal functions

			FeedbackBuffer(APIWTexture* Cache,APIWTexture* Redirection,u32 CacheSize,u32 InternalIndex,VirtualTextureManager* Parent);
			~FeedbackBuffer();

			u32 GetRequestedCacheSize(){return _requestedCacheSize;}
			void SetCacheSize(u32 CacheSize){_requestedCacheSize = CacheSize; _currentCacheSize=CacheSize;}

			u32 GetInternalIndex(){return _index;}
			bool GetFeedbackData(const void** ppDataOut);
			bool HasFeedback(){return _bFeedbackProvided;}

			struct VT_Settings
			{
				Vector2<>	cache_inner_page_size;
				Vector2<>	cache_border_size;	
				Vector2<>	cache_outer_page_size;
				Vector2<>	cache_factor;	
			};

			void UpdateSettings(const VT_Settings& settings);

		private:
			mutable	bool			_bFeedbackProvided;
			mutable APIWTexture*	_feedbackTexture;
			mutable APIWTexture*	_feedbackDepth;
			mutable	u32				_requestedCacheSize;

			u32						_currentCacheSize;
			bool					_bFeedbackRecieved;
			const u32				_index;
			VT_Settings				_settings;
			APIWTexture*			_cache;
			APIWTexture*			_redirection;
			VirtualTextureManager*	_parent;
			APIWEffect*				_feedbackEffect;
			APIWConstantBuffer*		_pVTSettings;
		};

		typedef std::pair<APIWTexture*,u32> UpdatableTexture;

		struct FeedbackElement
		{
			FeedbackBuffer*		buffer;
			APIWFence*			write_fence;
			UpdatableTexture	cache;
			UpdatableTexture	redirection;
			u32					elementIndex;
			u32					setIndex;
		};

		struct PageData
		{
			u32 nBytes;
			u32 nCacheIndex;
			u8*	pData;
		};

		class PageCache
		{
		public:
			PageCache(u32 CacheSizeBytes,u32 PageSize,u32 nBorderSize,u32 nXPages,u32 nYPages);
			~PageCache();

			u32 GetNumCachePages() {return nCachePagesX*nCachePagesY;}
			u32 GetCacheXPages() {return nCachePagesX;}
			u32 GetCacheYPages() {return nCachePagesY;}
			PageData* UpdatePage(u32 CacheIndex);

			UpdatableTexture CreateCacheTexture();
			void UpdateCacheTexture(UpdatableTexture& cacheTexture);
			
			Vector2<> GetCachePageInnerSize() 
			{
				return Vector2<>((f32)(nPageSizeX-2*_nBorderSize)/(f32)(nCacheX),(f32)(nPageSizeY-2*_nBorderSize)/(f32)(nCacheY));
			}
			Vector2<> GetCacheBorderSize() 
			{
				return Vector2<>((f32)(_nBorderSize)/(f32)nCacheX,(f32)(_nBorderSize)/(f32)nCacheY);
			}

			class PageRedirection
			{
			public:
				PageRedirection()
				{/*
					OffsetX=0.0f;
					OffsetY=0.0f;
					Exponent=0.0f;*/
					data= 0;
				}

				//static const HAGE::APIWFormat format = HAGE::APIWFormat::R32G32B32A32_FLOAT;
				static const HAGE::APIWFormat format = R16_FLOAT;

			private:
				PageRedirection(u32 xOff,u32 yOff,u32 exponent)
				{
					
					
					assert(xOff < (1<<5));
					assert(yOff < (1<<5));
					assert(exponent < (1<<5 - 1));
					data = (0x0000) | ((exponent+1)<<10) | (yOff<<5) | (xOff<<0);
					
						/*
					OffsetX=xOff;
					OffsetY=yOff;
					Exponent= exponent;*/
				}
				/*
				f32 OffsetY;
				f32 OffsetX;
				f32 Exponent;
				f32 _Padding;
				*/
				u16 data;
				friend class PageCache;
			};

			void UpdateCacheSize(u32 CacheSizeBytes);

			PageRedirection CreateRedirection(u32 CacheIndex,f32 xOff,f32 yOff,f32 scale) const;
		private:

			void GetCachePageInfo(const u32& cacheindex,u32& xBegin,u32& xEnd,u32& yBegin,u32& yEnd) const
			{
				xBegin = (cacheindex % nCachePagesX)*nPageSizeX;
				xEnd = xBegin + nPageSizeX;
				yBegin = (cacheindex / nCachePagesX)*nPageSizeY;
				yEnd = yBegin + nPageSizeY;
			}

			struct Pixel
			{
				u8 r,g,b,a;
			};

			std::vector<PageData>	pageData;//currently cached data
			std::vector<Pixel>		pixelData; //pageData points into this
			typedef std::vector<PageData*> item_changes;
			std::vector<item_changes> changes;

			u32						nCachePagesX, nCachePagesY;
			u32						nCacheX, nCacheY;
			
			const u32				_nBorderSize;
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
				_Level(nLevel),
				_TotalLevels(std::max(FindLog2(nMaxXPages),FindLog2(nMaxYPages))+1)
			{
				for(int i = 0; i < _redirectionTable.size(); ++i)
					_redirectionTableIndices[i].cacheIndex = 0xffffffff;
			}

			void UnloadPage(u32 nPageX,u32 nPageY,u32 nLevel)
			{
				assert(nLevel > 0);

				if(nLevel - 1 > _Level)
				{
					_subPage->UnloadPage(nPageX,nPageY,nLevel);
					return;
				}

				assert(nLevel == _Level+1);
				u32 this_X = nPageX/2;
				u32 this_Y = nPageY/2;

				u32 newIndex = _redirectionTableIndices[this_Y * _nPagesX + this_X].cacheIndex;
				u32 newSplit = _redirectionTableIndices[this_Y * _nPagesX + this_X].nSplit;

				u32 oldIndex = _subPage->_redirectionTableIndices[nPageY * _subPage->_nPagesX + nPageX].cacheIndex;

				//adjust values for new split

				this_X /= (newSplit);
				this_X *= (newSplit*2);
				this_Y /= (newSplit);
				this_Y *= (newSplit*2);

				_subPage->replacePage(this_X,this_Y,newSplit*2,oldIndex,newIndex);
			}

			void LoadPage(u32 nPageX,u32 nPageY,u32 nLevel,u32 CacheIndex)
			{
				if(nLevel > _Level)
				{
					_subPage->LoadPage(nPageX,nPageY,nLevel,CacheIndex);
					return;
				}

				assert(nLevel == _Level);

				u32 oldIndex = _redirectionTableIndices[nPageY * _nPagesX + nPageX].cacheIndex;

				replacePage(nPageX,nPageY,1,oldIndex,CacheIndex);
			}
			
			UpdatableTexture CreateRedirectionTexture() const
			{
				if(_subPage)
					return _subPage->CreateRedirectionTexture();
				else
				{
					RenderingAPIAllocator* pAlloc = RenderingAPIAllocator::QueryAPIAllocator();
					assert(pAlloc);
					pAlloc->BeginAllocation();
					UpdatableTexture result;
					result.first = pAlloc->CreateTexture(_nPagesX,_nPagesY,_TotalLevels,
						typename _TCache::PageRedirection::format,TEXTURE_CPU_WRITE,
						nullptr);
					pAlloc->EndAllocation();
					return result;
				}
			}
			void UpdateRedirectionTexture(UpdatableTexture& redirectionTexture) const
			{
				RenderingAPIAllocator* pAlloc = RenderingAPIAllocator::QueryAPIAllocator();
				assert(pAlloc);
				pAlloc->BeginAllocation();
				pAlloc->UpdateTexture(redirectionTexture.first,0,0,_nPagesX,_nPagesY,_TotalLevels-1-_Level,_redirectionTable.data());
				pAlloc->EndAllocation();
				if(_subPage)
					return _subPage->UpdateRedirectionTexture(redirectionTexture);
				else
					return;
			}

		private:
			static MipMappedPageRedirectionTable* AllocateSubTable(u32 nXPages,u32 nYPages,const PageCache* cache,u32 nLevel)
			{
				if( (((u32)1 << nLevel) >= nXPages) &&  (((u32)1 << nLevel) >= nYPages) )
					return nullptr;
				return new MipMappedPageRedirectionTable(nXPages,nYPages,cache,nLevel+1);
			}
			
			void replacePage(u32 nPageX,u32 nPageY,u32 nPages,u32 OldCacheIndex,u32 NewCacheIndex)
			{
				f32 texelSize = 1.0f / (f32)_nPagesX;
				f32 xOff = (f32)nPageX*texelSize;
				f32 yOff = (f32)nPageY*texelSize;
				PageCache::PageRedirection new_redir = _cache->CreateRedirection(NewCacheIndex,xOff,yOff,(f32)nPages*texelSize);
				for(int iy = 0; iy < nPages; iy++)
					for(int ix = 0; ix < nPages; ix++)
						//only update lower detail loaded data
						if(_redirectionTableIndices[(nPageY+iy) * _nPagesX + nPageX + ix].cacheIndex == OldCacheIndex)
						{
							PageCache::PageRedirection& ref = _redirectionTable[(nPageY+iy) * _nPagesX + nPageX + ix];
							_redirectionTableIndices[(nPageY+iy) * _nPagesX + nPageX + ix].cacheIndex = NewCacheIndex;
							_redirectionTableIndices[(nPageY+iy) * _nPagesX + nPageX + ix].nSplit = nPages;
							ref =new_redir;
						}

				if(_subPage)
					_subPage->replacePage(nPageX*2,nPageY*2,nPages*2,OldCacheIndex,NewCacheIndex);
			}

			struct IndexInformation
			{
				u16 cacheIndex;
				u16 nSplit;
			};
			
			static u32 FindLog2(u32 v)
			{
				u32 r = 0; // r will be lg(v)

				while (v >>= 1) // unroll for more speed...
				{
					r++;
				}

				return r;
			}

			std::vector<typename _TCache::PageRedirection>	_redirectionTable;
			std::vector<IndexInformation>				_redirectionTableIndices;

			const _TCache* const			_cache;
			MipMappedPageRedirectionTable * const	_subPage;
			const u32						_nPagesX,_nPagesY;
			const u32						_Level;
			const u32						_TotalLevels;
		};

		void adjustPageDebug(void* pData,u32 level,u32 x,u32 y);


		const static bool				_bDebugPage = false;

		u32				nTextureX, nTextureY;
		u32				nPageSizeX,nPageSizeY;
		u32				nPagesX,nPagesY;

		u32				_cacheSize;
		u32				_nCurrentVersion;

		VirtualTextureFeedbackAnalyzer* _analyzer;

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