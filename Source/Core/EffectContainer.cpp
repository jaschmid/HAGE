#include "HAGE.h"
#include "EffectContainer.h"

namespace HAGE {

EffectContainer::EffectContainer(RenderingAPIWrapper* pWrapper,const char* pProgram,const APIWRasterizerState* pRasterizerState, const APIWBlendState* pBlendState,
		const u32 nBlendStates, bool AlphaToCoverage) :
	_pWrapper(pWrapper)
{
	_pEffect = _pWrapper->CreateEffect(pProgram,pRasterizerState,pBlendState,nBlendStates,AlphaToCoverage);
}

void EffectContainer::SetConstant(u32 index,const APIWConstantBuffer* pBuffer)
{
	if(index+1 > _pConstants.size())
		_pConstants.resize(index+1);
	_pConstants[index] = pBuffer;
}

void EffectContainer::SetTexture(u32 index,const APIWTexture* pTexture)
{
	if(index+1 > _pTextures.size())
		_pTextures.resize(index+1);
	_pTextures[index] = pTexture;
}

void EffectContainer::Draw(u32 pass, const APIWVertexArray* pArray) const
{
	_pEffect->Draw(const_cast<APIWVertexArray*>(pArray),const_cast<HAGE::APIWConstantBuffer* const *>(_pConstants.data()),_pConstants.size(),const_cast<HAGE::APIWTexture* const *>(_pTextures.data()),_pTextures.size());
}

}