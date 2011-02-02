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
		const u32 nBlendStates = 1, bool AlphaToCoverage = false,const APIWSampler* pSamplers=nullptr,u32 nSamplers=0);

		// create effect from file

		// add later

		//set functions
		void SetConstant(const char* name,const APIWConstantBuffer* pBuffer);
		void SetTexture(const char* name,const APIWTexture* pTexture);

		//drawing functions
		void Draw(u32 pass, const APIWVertexArray* pArray) const;
	private:
		RenderingAPIWrapper*				_pWrapper;
		APIWEffect*							_pEffect;
	};
};

#endif