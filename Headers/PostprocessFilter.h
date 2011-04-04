#ifndef RENDERING_POSTPROCESS_FILTER__INCLUDED
#define RENDERING_POSTPROCESS_FILTER__INCLUDED

#include "header.h"

namespace HAGE {

	class PostprocessFilter
	{
	public:
			PostprocessFilter(RenderingAPIWrapper* pWrapper);
			~PostprocessFilter();

			void BeginSceneRendering();
			void EndSceneRendering();

			u32 GetPasses(){return _nPasses;}
			void SetPasses(u32 n){_nPasses = std::min<u32>(12,std::max<u32>(0,n));}

			void NextFilter(){_currentFilter = (_currentFilter +1)%_nFilters;}

	private:
			static const u32							_nFilters = 3;
			u32											_nPasses;
			u32											_currentFilter;
			RenderingAPIWrapper*						_pWrapper;
			APIWTexture*								_pPostprocessBuffer[2];
			APIWTexture*								_pPostprocessDepthBuffer;
			APIWConstantBuffer*							_pPostprocessConstants;
			APIWVertexBuffer*							_pPostprocessQuadBuffer;
			APIWVertexArray*							_pPostprocessQuadArray;
			APIWEffect*									_pPostprocessEffect;
	};

}

#endif