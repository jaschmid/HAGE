#include <HAGE.h>
#include "VirtualTextureManager.h"

namespace HAGE {

	class NullVirtualTexture : public IVirtualTexture
	{
	public:
		virtual const APIWTexture* GetCurrentVTCache() const
		{
			return cacheTexture;
		}
		virtual const APIWTexture* GetCurrentVTRedirection() const
		{
			return redirectionTexture;
		}
		virtual u32 GetCacheSize() const
		{
			return 0;
		}
		
		virtual void AdjustCacheSize(u32 newSize) const
		{
			return;
		}
		virtual void ProvideFeedback() const
		{
			return;
		}

		NullVirtualTexture()
		{
			RenderingAPIAllocator* pAlloc = RenderingAPIAllocator::QueryAPIAllocator();

			pAlloc->BeginAllocation();

			u32 Black = 0;

			cacheTexture = pAlloc->CreateTexture(1,1,1,HAGE::APIWFormat::R8G8B8A8_UNORM,0,&Black);
			redirectionTexture = pAlloc->CreateTexture(1,1,1,HAGE::APIWFormat::R8G8B8A8_UNORM,0,&Black);

			pAlloc->EndAllocation();
		}

		~NullVirtualTexture()
		{
			delete cacheTexture;
			delete redirectionTexture;
		}
	private:
		APIWTexture* cacheTexture;
		APIWTexture* redirectionTexture;
	};

	class NullVirtualTextureManager : public IStreamingResourceProvider
	{
	public:
		~NullVirtualTextureManager(){}

		virtual void ProcessResourceAccess(IResource* feedback){return;}
		virtual void CreateResourceAccessSet(std::vector<IResource*>& accesses){accesses.push_back(new NullVirtualTexture);}
		virtual bool QueryDependancies(const ResourceAccess* dependanciesIn,const std::pair<std::string,guid>** pDependanciesOut,u32& nDependanciesInOut){return false;}

	};
	
	IStreamingResourceProvider* VirtualTextureManager::LoadVirtualTexture(IDataStream* stream)
	{
		if(stream->GetIdentifierString() == "Null")
		{
			return new NullVirtualTextureManager;
		}
		else
		{
			VirtualTextureManager* result;
			result = new VirtualTextureManager;
			if(!result->Load(stream))
			{
				delete result;
				return nullptr;
			}
			else
				return result;
		}
	}

	
	void VirtualTextureManager::ProcessResourceAccess(IResource* feedback)
	{
		FeedbackBuffer* pFeedback = (FeedbackBuffer*)feedback;
	}

	void VirtualTextureManager::CreateResourceAccessSet(std::vector<IResource*>& accesses)
	{
		size_t oldSize = _feedbackElements.size();
		size_t newSize = oldSize +2;
		_feedbackElements.resize(newSize);
		for(size_t i = oldSize; i < newSize; ++i)
		{
			_feedbackElements[i].elementIndex = i - oldSize;
			_feedbackElements[i].setIndex = oldSize /2;
			_feedbackElements[i].redirection = _redirectionTable->CreateRedirectionTexture();
			_feedbackElements[i].cache = _pageCache->CreateCacheTexture();
			_feedbackElements[i].buffer = new FeedbackBuffer(
				_feedbackElements[i].cache.first,
				_feedbackElements[i].redirection.first,
				_cacheSize,	i, this);

			accesses.push_back(_feedbackElements[i].buffer);
		}
	}
	

	VirtualTextureManager::VirtualTextureManager() : _pageCache(nullptr),_redirectionTable(nullptr),
		_pStreamSource(nullptr),_pTextureSource(nullptr),nTextureX(0),nTextureY(0),
		nPageSizeX(0),nPageSizeY(0),nPagesX(0),nPagesY(0),_nCurrentVersion(0),_cacheSize(0)
	{
	}

	VirtualTextureManager::~VirtualTextureManager()
	{
		if(_pageCache)
			delete _pageCache;
		if(_redirectionTable)
			delete _redirectionTable;
		if(_pTextureSource)
			delete _pTextureSource;
		if(_pStreamSource)
			_pStreamSource->Close();
	}

	bool VirtualTextureManager::Load(IDataStream* stream)
	{
		_pStreamSource = stream;
		_pTextureSource = new SparseVirtualTextureFile();
		if(!_pTextureSource->Open(stream))
		{
			delete _pTextureSource;
			_pTextureSource= nullptr;
			return false;
		}

		_cacheSize = VirtualTextureManager::DefaultCacheSize;

		nPagesX = _pTextureSource->GetNumXPagesAtDepth(_pTextureSource->GetMaxDepth());
		nPagesY = nPagesX;

		nPageSizeX = _pTextureSource->GetPageOuterSize();
		nPageSizeY = nPageSizeX;

		nTextureX = nPagesX*nPageSizeX;
		nTextureY = nPagesY*nPageSizeY;

		_pageCache = new CacheType(_cacheSize,nPageSizeX,nPagesX,nPagesY);

		_redirectionTable = new MipMappedPageRedirectionTable<CacheType>(nPagesX,nPagesY,_pageCache,0);

		return true;
	}

	void VirtualTextureManager::NotifyFeedbackBufferDeleted(u32 idx)
	{
		_feedbackElements[idx].elementIndex = 0;
		_feedbackElements[idx].setIndex = 0;
		delete _feedbackElements[idx].redirection.first;
		delete _feedbackElements[idx].cache.first;
		_feedbackElements[idx].redirection.first = nullptr;
		_feedbackElements[idx].cache.first = nullptr;
		_feedbackElements[idx].buffer = 0;
	}

	VirtualTextureManager::FeedbackBuffer::FeedbackBuffer(const APIWTexture* Cache,const APIWTexture* Redirection,u32 CacheSize,u32 InternalIndex,VirtualTextureManager* Parent)
		: _nFeedback(0),_requestedCacheSize(CacheSize),_currentCacheSize(CacheSize),_index(InternalIndex),_cache(Cache),_redirection(Redirection),_parent(Parent)
	{
	}
	VirtualTextureManager::FeedbackBuffer::~FeedbackBuffer()
	{
		_parent->NotifyFeedbackBufferDeleted(_index);
	}

	static u32 RoundDownToPowerOfTwo(u32 v)
	{

		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v = v+1;
		return v >> 1;
	}

	static u32 FindLog2(u32 v)
	{
		u32 r = 0; // r will be lg(v)

		while (v >>= 1) // unroll for more speed...
		{
			r++;
		}

		return r;
	}

	VirtualTextureManager::PageCache::PageCache(u32 CacheSizeBytes,u32 PageSize,u32 nXPages,u32 nYPages) :
		nDataVersion(0),nPageSizeX(PageSize),nPageSizeY(PageSize),nPagesX(nXPages),nPagesY(nYPages)
	{
		UpdateCacheSize(CacheSizeBytes);
	}

	void VirtualTextureManager::PageCache::UpdateCacheSize(u32 CacheSizeBytes)
	{
		u32	oldCachePages = pageData.size();

		static const u32 PixelSize = sizeof(Pixel); //when we'll use compressed textures this will change
		u32 nTotalCacheLog = FindLog2(RoundDownToPowerOfTwo(CacheSizeBytes/PixelSize));

		if(nTotalCacheLog%2)
			nCacheX = 1<<(nTotalCacheLog/2+1);
		else
			nCacheX = 1<<(nTotalCacheLog/2);

		nCacheY = 1<<(nTotalCacheLog/2);

		assert(nCacheX*nCacheY <= CacheSizeBytes);
		assert(nCacheX != 0 && nCacheY != 0);

		assert(nCacheX % nPageSizeX == 0);
		assert(nCacheY % nPageSizeY == 0);

		nCachePagesX = nCacheX / nPageSizeX;
		nCachePagesY = nCacheY / nPageSizeY;

		u32 newCachePages = nCachePagesX*nCachePagesY;
		u32 newCachePixels = nCacheX*nCacheY;

		std::vector<PageData> newPageData(newCachePages);
		std::vector<Pixel> newCacheData(newCachePixels);

		for(int i = 0; i< std::min(oldCachePages,newCachePages);++i)
			newPageData[i] = pageData[i];

		pageData = newPageData;
		cacheData = newCacheData;
	}

	VirtualTextureManager::PageCache::~PageCache()
	{
	}
	
	void VirtualTextureManager::PageCache::LoadPage(const PageData* page,u32 CacheIndex)
	{
	}

	VirtualTextureManager::UpdatableTexture VirtualTextureManager::PageCache::CreateCacheTexture() const
	{
		RenderingAPIAllocator* pAlloc = RenderingAPIAllocator::QueryAPIAllocator();
		assert(pAlloc);
		pAlloc->BeginAllocation();
		UpdatableTexture result;

		result.first = pAlloc->CreateTexture(nCacheX,nCacheY,1,APIWFormat::R8G8B8A8_UNORM,0,nullptr,0);
		result.second = 0;

		pAlloc->EndAllocation();
		return result;
	}

	void VirtualTextureManager::PageCache::UpdateCacheTexture(UpdatableTexture& cacheTexture) const
	{
	}
	
	VirtualTextureManager::PageCache::PageRedirection VirtualTextureManager::PageCache::CreateRedirection(u32 CacheIndex,f32 xBegin,f32 xEnd,f32 yBegin,f32 yEnd) const
	{
		PageRedirection redirection;
		return redirection;
	}
}