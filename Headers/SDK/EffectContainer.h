#ifndef EFFECT_CONTAINER__INCLUDED
#define EFFECT_CONTAINER__INCLUDED

#include "HAGE.h"
#include "RenderingAPIWrapper.h"

namespace HAGE
{
	class EffectContainer
	{
	public:
		// create effect directly
		EffectContainer(RenderingAPIWrapper* pWrapper,const char* pProgram,const APIWRasterizerState* pRasterizerState = &DefaultRasterizerState, const APIWBlendState* pBlendState = &DefaultBlendState,
		const u32 nBlendStates = 1, bool AlphaToCoverage = false);

		// create effect from file

		// add later

		//set functions
		void SetConstant(u32 index,const APIWConstantBuffer* pBuffer);
		void SetTexture(u32 index,const APIWTexture* pTexture);

		//drawing functions
		void Draw(u32 pass, const APIWVertexArray* pArray) const;
	private:
		RenderingAPIWrapper*				_pWrapper;
		APIWEffect*							_pEffect;
		std::vector<const APIWConstantBuffer*>	_pConstants;
		std::vector<const APIWTexture*>			_pTextures;
	};
};

#endif