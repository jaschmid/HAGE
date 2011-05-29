#include <HAGE.h>
#include "VirtualTextureManager.h"
#include "math.h"

namespace HAGE {
	
	static const char* feedback_program =
		"#define nLights				 3\n"
		"H_CONSTANT_BUFFER_BEGIN(TransformGlobal)\n"
		"    float4x4 model;\n"
		"    float4x4 inverse_modelview;\n"
		"    float4x4 modelview;\n"
		"    float4x4 modelview_projection;\n"
		"H_CONSTANT_BUFFER_END\n"
		"\n"
		"// cbuffer Transform : register(b0)\n"
		"#define Model					 model\n"
		"#define Modelview               modelview\n"
		"#define InverseModelview        inverse_modelview\n"
		"#define ModelviewProjection     modelview_projection\n"
		"\n"
		"VS_IN_BEGIN\n"
		"  DECL_VS_IN(float3,position);\n"
		"  DECL_VS_IN(float2,texcoord);\n"
		"VS_IN_END\n"
		"VS_OUT_BEGIN\n"
		"  DECL_VS_OUT(float4,proj_position);\n"
		"  DECL_VS_OUT(float2,tex);\n"
		"  DECL_VS_POSITION;\n"
		"VS_OUT_END\n"
		"\n"
		"static const float PI =  3.14159265f;\n"
		"VERTEX_SHADER\n"
		"{	\n"
		"\n"
		"   float4 proj_vertex = mul( ModelviewProjection, float4( VS_IN(position), 1.0f ) );\n "
		"	float4 view_vertex = mul( Modelview, float4(VS_IN(position),1.0f) );\n"
		"	float lenxy = length(view_vertex.xy);\n"
		"	float phi = atan2(lenxy, view_vertex.z);\n"
		"	float r = sin( phi );\n"
		"	view_vertex.xy = (normalize(view_vertex.xy) * r);\n"
		"	VS_OUT_POSITION.xy = view_vertex.xy;\n"
		"	float sqlen = dot(view_vertex.xyz,view_vertex.xyz);\n"
		"	VS_OUT(proj_position) = float4(view_vertex.x,view_vertex.y,sqlen,1.0f);\n "
		"	VS_OUT_POSITION.z = 1.0f-1.0f/(1.01f+(sqlen*sign(view_vertex.z)));\n"
		"	VS_OUT_POSITION.w = 1.0f;\n"
		"	VS_OUT(tex) = VS_IN(texcoord);\n"
		"\n"
		"}"
		"FS_IN_BEGIN\n"
		"  DECL_FS_IN(float4,proj_position);\n"
		"  DECL_FS_IN(float2,tex);\n"
		"FS_IN_END\n"
		"FS_OUT_BEGIN\n"
		"  DECL_FS_COLOR;\n"
		"FS_OUT_END\n"
		"\n"
		"float mipmapLevel(float2 tex)\n"
		"{ \n"
		"  float2 dx = ddx(tex);\n"
		"  float2 dy = ddy(tex);\n"
		"  float d = max(dot(dx,dx), dot(dy,dy));\n"
		"  return max(0.0f, ceil(-0.5f*log2(d) -5.0f));\n"
		"} \n"
		"float mipmapLevelAni(float2 tex)\n"
		"{ \n"
		"  float2 dx = ddx(tex);\n"
		"  float2 dy = ddy(tex);\n"
		"  float Pmax = max(dot(dx,dx), dot(dy,dy));\n"
		"  float Pmin = min(dot(dx,dx), dot(dy,dy));\n"
		"  float N = min(ceil(Pmax/Pmin), 8.0f*8.0f);\n"
		"  return min(max(0.0f, ceil(-0.5f*log2(Pmax/N) -5.0f)),16.0f);\n"
		"} \n"
		"float distanceEstimate(float dist)\n"
		"{ \n"
		"  return max(0.0f, 11-0.5*log2(dist));\n"
		"} \n"
		"\n"
		"FRAGMENT_SHADER\n"
		"{\n"
		"  float dist = FS_IN(proj_position).z/FS_IN(proj_position).w;\n"
		"  FS_OUT_COLOR = float4( FS_IN(tex).x, FS_IN(tex).y, mipmapLevelAni(FS_IN(tex).xy) / 16.0f , 1.0f);\n"
		"}\n";

	static const int FeedbackXSize = 256;
	static const int FeedbackYSize = 256;

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
		virtual const APIWConstantBuffer* GetSettings() const
		{
			return nullptr;
		}
		virtual u32 GetCacheSize() const
		{
			return 0;
		}
		
		virtual u32 GetId() const {return 0xffffffff;}
		virtual APIWEffect* BeginFeedback(RenderingAPIWrapper* pRenderer) const  {return nullptr;}
		virtual void EndFeedback(RenderingAPIWrapper* pRenderer) const {return;}
		virtual const APIWTexture* _Debug_GetFeedbackTexture() const
		{
			return redirectionTexture;
		}

		virtual void AdjustCacheSize(u32 newSize) const
		{
			return;
		}
		virtual void ProvideFeedback() const
		{
			return;
		}
		virtual void BeginFeedback() {}
		virtual void EndFeedback() {}

		NullVirtualTexture()
		{
			RenderingAPIAllocator* pAlloc = RenderingAPIAllocator::QueryAPIAllocator();

			pAlloc->BeginAllocation();

			u32 Black = 0;

			cacheTexture = pAlloc->CreateTexture(1,1,1,R8G8B8A8_UNORM,0,&Black);
			redirectionTexture = pAlloc->CreateTexture(1,1,1,R8G8B8A8_UNORM,0,&Black);

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
		
		virtual bool CheckFeedbackBufferReady(IResource* feedback){return true;}
		virtual bool ProcessResourceAccess(IResource* feedback){return true;}
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

	
	bool VirtualTextureManager::ProcessResourceAccess(IResource* feedback)
	{
		FeedbackBuffer* pFeedback = (FeedbackBuffer*)feedback;

		const void* pReadData;

		if(! pFeedback->HasFeedback() )
			return true;
		else if(pFeedback->GetFeedbackData(&pReadData))
		{
			VirtualTextureFeedbackAnalyzer::element* pElementBuffer = new VirtualTextureFeedbackAnalyzer::element[FeedbackXSize*FeedbackYSize];
			const Vector4<u16>* pData = (const Vector4<u16>*)pReadData;

			u32 nElements = 0;

			for(int iy = 0; iy<FeedbackYSize;iy++)
				for(int ix = 0; ix<FeedbackXSize;ix++)
				{
					const auto& v = pData[iy * FeedbackXSize + ix];
					if(v.w == 0)
						continue;
					auto& e = pElementBuffer[nElements];
					e.level = std::min((u16)(floorf((f32)(v.z*16)/(f32)0xffff+0.5f)),(u16)_pTextureSource->GetMaxDepth());
					f32 nPages =  (f32)_pTextureSource->GetNumXPagesAtDepth(e.level);
					e.x = std::min<u16>((u16)(floorf((f32)(v.x)*nPages/(f32)0xffff)+0.5f),(u16)nPages-1);
					e.y = std::min<u16>((u16)(floorf((f32)(v.y)*nPages/(f32)0xffff)+0.5f),(u16)nPages-1);
					
					++nElements;
				}

			std::vector<VirtualTextureFeedbackAnalyzer::cache_element> changes;
			std::vector<u32> removed;

			_analyzer->AnalyzeFeedback(pElementBuffer,nElements,changes,removed);

			delete [] pElementBuffer;
			
			//remove old pages from redirection table
			for(auto it = removed.begin(); it != removed.end(); ++it)
			{
				u32 pageX,pageY,level;
				_pTextureSource->GetPageLocation(*it,pageX,pageY,level);
				_redirectionTable->UnloadPage(pageX,pageY,level);
			}
			

			for(auto it = changes.begin(); it != changes.end(); ++it)
			{
				u32 pageX,pageY,level;
				_pTextureSource->GetPageLocation(it->page_index,pageX,pageY,level);
				u32 pageSize = _pTextureSource->GetPageOuterSize();
				PageData* p = _pageCache->UpdatePage(it->cache_index);
				_pTextureSource->ReadPage(it->page_index,Vector2<u32>(0,0),ImageRect<R8G8B8A8>((void*)p->pData,Vector2<u32>(pageSize,pageSize),pageSize));

				if(_bDebugPage)
					adjustPageDebug(p->pData,level,pageX,pageY);

				_redirectionTable->LoadPage(pageX,pageY,level,it->cache_index);
			}


			u32 index = pFeedback->GetInternalIndex();
			
			_redirectionTable->UpdateRedirectionTexture(_feedbackElements[index].redirection);
			_pageCache->UpdateCacheTexture(_feedbackElements[index].cache);
		
			RenderingAPIAllocator* pAlloc = RenderingAPIAllocator::QueryAPIAllocator();
			assert(pAlloc);
			pAlloc->BeginAllocation();

			if(_feedbackElements[index].write_fence)
				pAlloc->FreeObject(_feedbackElements[index].write_fence);
			_feedbackElements[index].write_fence = pAlloc->CreateStreamingFence();

			pAlloc->EndAllocation();
			return true;
		}
		else
			return false;
	}

	bool VirtualTextureManager::CheckFeedbackBufferReady(IResource* feedback)
	{
		FeedbackBuffer* pFeedback = (FeedbackBuffer*)feedback;
		u32 index = pFeedback->GetInternalIndex();

		if(_feedbackElements[index].write_fence && !_feedbackElements[index].write_fence->CheckStatus())
			return false;
		else
		{
			if(_feedbackElements[index].write_fence)
			{
				delete _feedbackElements[index].write_fence;
				_feedbackElements[index].write_fence = nullptr;
			}
			return true;
		}
	}

	void VirtualTextureManager::CreateResourceAccessSet(std::vector<IResource*>& accesses)
	{
		size_t oldSize = _feedbackElements.size();
		size_t newSize = oldSize +2;
		_feedbackElements.resize(newSize);
		for(size_t i = oldSize; i < newSize; ++i)
		{
			_feedbackElements[i].elementIndex = (u32)(i - oldSize);
			_feedbackElements[i].setIndex = (u32)(oldSize /2);
			_feedbackElements[i].redirection = _redirectionTable->CreateRedirectionTexture();
			_feedbackElements[i].cache = _pageCache->CreateCacheTexture();
			_feedbackElements[i].write_fence = nullptr;
			_feedbackElements[i].buffer = new FeedbackBuffer(
				_feedbackElements[i].cache.first,
				_feedbackElements[i].redirection.first,
				_cacheSize,	(u32)i, this);
			FeedbackBuffer::VT_Settings settings;
			settings.cache_inner_page_size = _pageCache->GetCachePageInnerSize();
			settings.cache_border_size = _pageCache->GetCacheBorderSize();
			settings.cache_outer_page_size = settings.cache_inner_page_size + settings.cache_border_size* 2.0f;
			settings.cache_factor = Vector2<>((f32)nPagesX/(f32)_pageCache->GetCacheXPages(),(f32)nPagesY/(f32)_pageCache->GetCacheYPages());

			_feedbackElements[i].buffer->UpdateSettings(settings);
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
		if(_analyzer)
			delete _analyzer;
		if(_pStreamSource)
			_pStreamSource->Close();
	}

	bool VirtualTextureManager::Load(IDataStream* stream)
	{
		_pStreamSource = stream;
		_pTextureSource = new SparseVirtualTextureFile();
		_pTextureSource->Open(stream);
		/*
		{
			delete _pTextureSource;
			_pTextureSource= nullptr;
			return false;
		}
		*/
		_cacheSize = VirtualTextureManager::DefaultCacheSize;

		nPagesX = _pTextureSource->GetNumXPagesAtDepth(_pTextureSource->GetMaxDepth());
		nPagesY = nPagesX;

		nPageSizeX = _pTextureSource->GetPageOuterSize();
		nPageSizeY = nPageSizeX;

		nTextureX = nPagesX*nPageSizeX;
		nTextureY = nPagesY*nPageSizeY;

		_pageCache = new CacheType(_cacheSize,nPageSizeX,nPagesX,nPagesY,(nPageSizeX-_pTextureSource->GetPageInnerSize())/2);

		_redirectionTable = new MipMappedPageRedirectionTable<CacheType>(nPagesX,nPagesY,_pageCache,0);

		_analyzer = new VirtualTextureFeedbackAnalyzer(_pageCache->GetNumCachePages(),_pTextureSource->GetNumLayers());

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
		if(_feedbackElements[idx].write_fence)
			delete _feedbackElements[idx].write_fence;
		_feedbackElements[idx].write_fence = nullptr;
	}

	VirtualTextureManager::FeedbackBuffer::FeedbackBuffer(APIWTexture* Cache,APIWTexture* Redirection,u32 CacheSize,u32 InternalIndex,VirtualTextureManager* Parent)
		: _bFeedbackProvided(false),_requestedCacheSize(CacheSize),_currentCacheSize(CacheSize),_index(InternalIndex),_cache(Cache),_redirection(Redirection),_parent(Parent),
		_bFeedbackRecieved(false)
	{
		RenderingAPIAllocator* pAlloc = RenderingAPIAllocator::QueryAPIAllocator();
		assert(pAlloc);
		pAlloc->BeginAllocation();

		_feedbackTexture = pAlloc->CreateTexture(FeedbackXSize,FeedbackYSize,1,R16G16B16A16_UNORM,TEXTURE_CPU_READ | TEXTURE_GPU_WRITE,nullptr);
		_feedbackDepth = pAlloc->CreateTexture(FeedbackXSize,FeedbackYSize,1,R16_UNORM,TEXTURE_GPU_DEPTH_STENCIL,nullptr);
		_feedbackEffect = pAlloc->CreateEffect(feedback_program);
		_pVTSettings = pAlloc->CreateConstantBuffer(sizeof(VT_Settings));

		pAlloc->EndAllocation();
	}
	VirtualTextureManager::FeedbackBuffer::~FeedbackBuffer()
	{
		delete _feedbackTexture;
		delete _feedbackDepth;
		delete _feedbackEffect;

		_parent->NotifyFeedbackBufferDeleted(_index);
	}

	void VirtualTextureManager::FeedbackBuffer::UpdateSettings(const VT_Settings& settings)
	{
		_settings = settings;
	}

	u32 VirtualTextureManager::FeedbackBuffer::GetId() const
	{
		return _index;
	}

	APIWEffect* VirtualTextureManager::FeedbackBuffer::BeginFeedback(RenderingAPIWrapper* pRenderer) const
	{
		//assert(!_bFeedbackProvided);
		_bFeedbackProvided = true;
		_feedbackTexture->Clear(Vector4<>(0.0f,0.0f,0.0f,0.0f));
		_feedbackDepth->Clear(true,1.0f);
		pRenderer->SetRenderTarget(_feedbackTexture,_feedbackDepth);
		return _feedbackEffect;
	}

	void VirtualTextureManager::FeedbackBuffer::EndFeedback(RenderingAPIWrapper* pRenderer) const
	{
		assert(_bFeedbackProvided);
		pRenderer->SetRenderTarget(RENDER_TARGET_DEFAULT,RENDER_TARGET_DEFAULT);
		
		/*if(!_bFeedbackRecieved)
		{*/
			_feedbackTexture->StreamForReading(0,0,FeedbackXSize,FeedbackYSize);
			//_bFeedbackRecieved=true;
		//}
	}
			
	bool VirtualTextureManager::FeedbackBuffer::GetFeedbackData(const void** ppDataOut)
	{
		if(!_bFeedbackProvided)
			return false;
		

		RenderingAPIAllocator* pAlloc = RenderingAPIAllocator::QueryAPIAllocator();
		assert(pAlloc);
		pAlloc->BeginAllocation();

		bool res = pAlloc->ReadTexture(_feedbackTexture,ppDataOut);

		pAlloc->EndAllocation();

		if(res)
		{
			_bFeedbackProvided = false;
			_bFeedbackRecieved = false;
		}

		return res;
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

	VirtualTextureManager::PageCache::PageCache(u32 CacheSizeBytes,u32 PageSize,u32 nXPages,u32 nYPages,u32 nBorderSize) :
		nPageSizeX(PageSize),nPageSizeY(PageSize),nPagesX(nXPages),nPagesY(nYPages),_nBorderSize(nBorderSize)
	{
		UpdateCacheSize(CacheSizeBytes);
	}

	void VirtualTextureManager::PageCache::UpdateCacheSize(u32 CacheSizeBytes)
	{
		u32	oldCachePages = (u32)pageData.size();

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

		pixelData.resize(newCachePixels);
		pageData.resize(newCachePages);

		for(u32 i = 0; i< newCachePages;++i)
		{
			pageData[i].nBytes = nPageSizeX*nPageSizeY*sizeof(Pixel);
			pageData[i].nCacheIndex = i;
			pageData[i].pData = (u8*)&pixelData[i*nPageSizeX*nPageSizeY];
		}

	}

	VirtualTextureManager::PageCache::~PageCache()
	{
	}
	
	static u32 ToColor(const Vector4<>& vec4)
	{
		Vector4<u8> vec;
		vec.x = (u8)(std::max(0.0f,std::min(255.0f,vec4.x*255.0f)));
		vec.y = (u8)(std::max(0.0f,std::min(255.0f,vec4.y*255.0f)));
		vec.z = (u8)(std::max(0.0f,std::min(255.0f,vec4.z*25.0f)));
		vec.w = (u8)(std::max(0.0f,std::min(255.0f,vec4.w*255.0f)));
		return *(u32*)&vec;
	}

	VirtualTextureManager::PageData* VirtualTextureManager::PageCache::UpdatePage(u32 CacheIndex)
	{
		for(auto it = changes.begin();it!=changes.end();++it)
		{
			it->push_back(&pageData[CacheIndex]);
			std::push_heap(it->begin(),it->end());
		}
		return &pageData[CacheIndex];
	}

	VirtualTextureManager::UpdatableTexture VirtualTextureManager::PageCache::CreateCacheTexture()
	{
		RenderingAPIAllocator* pAlloc = RenderingAPIAllocator::QueryAPIAllocator();
		assert(pAlloc);
		pAlloc->BeginAllocation();
		UpdatableTexture result;

		result.first = pAlloc->CreateTexture(nCacheX,nCacheY,1,R8G8B8A8_UNORM,TEXTURE_CPU_WRITE,nullptr,0);
		result.second = changes.size();
		changes.resize(changes.size()+1);

		pAlloc->UpdateTexture(result.first,0,0,nCacheX,nCacheY,0,nullptr);

		pAlloc->EndAllocation();
		return result;
	}

	void VirtualTextureManager::PageCache::UpdateCacheTexture(UpdatableTexture& cacheTexture)
	{

		RenderingAPIAllocator* pAlloc = RenderingAPIAllocator::QueryAPIAllocator();
		assert(pAlloc);
		pAlloc->BeginAllocation();

		PageData* last = nullptr;
		
		while(!changes[cacheTexture.second].empty())
		{
			if(changes[cacheTexture.second].front() != last)
			{
				last = changes[cacheTexture.second].front();
				
				u32 xCBegin,xCEnd;
				u32 yCBegin,yCEnd;

				GetCachePageInfo(last->nCacheIndex,xCBegin,xCEnd,yCBegin,yCEnd);

				pAlloc->UpdateTexture(cacheTexture.first,xCBegin,yCBegin,xCEnd-xCBegin,yCEnd-yCBegin,0,last->pData);
			}
			std::pop_heap(changes[cacheTexture.second].begin(),changes[cacheTexture.second].end());
			changes[cacheTexture.second].pop_back();
		}		

		pAlloc->EndAllocation();
	}
	
	VirtualTextureManager::PageCache::PageRedirection VirtualTextureManager::PageCache::CreateRedirection(u32 CacheIndex,f32 xOff,f32 yOff,f32 scale) const
	{
		/*
		u32 xCBegin,xCEnd;
		u32 yCBegin,yCEnd;

		GetCachePageInfo(CacheIndex,xCBegin,xCEnd,yCBegin,yCEnd);

		f32 borderSize = (f32)_nBorderSize / (f32)nCacheX;

		f32 xfBegin = (f32)xCBegin / (f32)nCacheX + borderSize;
		f32 xfSize = (f32)(xCEnd-xCBegin) / (f32)nCacheX -2.0f*borderSize;
		f32 yfBegin = (f32)yCBegin / (f32)nCacheY + borderSize;
		f32 yfSize = (f32)(yCEnd-yCBegin) / (f32)nCacheY - 2.0f*borderSize;

		f32 compScale = xfSize/scale;

		f32 xFIndex = (f32)(CacheIndex % nCachePagesX) /(f32)(nCachePagesX);
		f32 yFIndex = (f32)(CacheIndex / nCachePagesX) /(f32)(nCachePagesY);*/

		f32 exponent = -(std::logf(scale)/std::logf(2));

		u32 xLoc = (u32)((CacheIndex % nCachePagesX) * (1<<5)/nCachePagesX);
		u32 yLoc = (u32)((CacheIndex / nCachePagesX) * (1<<5)/nCachePagesY);
		u32 Exp = ((u32)floorf(exponent+0.5f));
		
		PageRedirection redirection(xLoc,yLoc,Exp);

		return redirection;
	}

	void VirtualTextureManager::adjustPageDebug(void* pData,u32 level,u32 x,u32 y)
	{
		static const u32 level_colors[] = 
		{
			0xffC46210,// level 0
			0xff2E5894,// level 1
			0xff9C2542,// level 2
			0xffBF4F51,// level 3
			0xffA57164,// level 4
			0xff58427C,// level 5
			0xff4A646C,// level 6
			0xff85754E,// level 7
			0xff319177,// level 8
			0xff0A7E8C,// level 9
			0xff9C7C38,// level 10
			0xff8D4E85,// level 11
			0xff8FD400,// level 12
			0xffD98695,// level 13
			0xff757575,// level 14
			0xff0081AB// level 15
		};
		struct Pixel
		{
			u8 b,g,r,a;
		};
		static const Pixel* pixel_colors = (const Pixel*)level_colors;


		Pixel* pPix = (Pixel*)pData;

		for(u32 iy = 0; iy < nPageSizeY; ++iy)
			for(u32 ix = 0; ix < nPageSizeX; ++ix)
			{
				u32 min_dist = std::min(std::min(std::min(iy,ix),(u32)(nPageSizeY-1-iy)),(u32)(nPageSizeX-1-ix));
				Pixel& cur = pPix[iy*nPageSizeX+ix];
				
				if(min_dist < 3)
				{
					cur.a = pixel_colors[level].a;
					cur.r = pixel_colors[level].r;
					cur.g = pixel_colors[level].g;
					cur.b = pixel_colors[level].b;
				}
				else if(ix <4 || iy < 4)
				{
					cur.r = 0x00;
					cur.g = 0x00;
					cur.b = 0xff;
					cur.a = 0xff;
				}
				else if(ix > nPageSizeX-5 || iy > nPageSizeY-5 )
				{
					cur.r = 0x00;
					cur.g = 0xff;
					cur.b = 0x00;
					cur.a = 0xff;
				}
				else if(ix <5 || iy < 5)
				{
					cur.r = 0x00;
					cur.g = 0xff;
					cur.b = 0x00;
					cur.a = 0xff;
				}
				else if(ix > nPageSizeX-6 || iy > nPageSizeY-6 )
				{
					cur.r = 0x00;
					cur.g = 0x00;
					cur.b = 0xff;
					cur.a = 0xff;
				}
				else if(min_dist < 8)
				{
					cur.a = pixel_colors[level].a;
					cur.r = pixel_colors[level].r;
					cur.g = pixel_colors[level].g;
					cur.b = pixel_colors[level].b;
				}
				else
				{
					cur.r /= 2;
					cur.g /= 2;
					cur.b /= 2;
				}
			}
	}
}