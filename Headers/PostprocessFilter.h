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

	private:
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