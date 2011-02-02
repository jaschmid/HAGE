#include "HAGE.h"
#include "EffectContainer.h"

namespace HAGE {

EffectContainer::EffectContainer(RenderingAPIWrapper* pWrapper,const char* pProgram,const APIWRasterizerState* pRasterizerState, const APIWBlendState* pBlendState,
		const u32 nBlendStates, bool AlphaToCoverage,const APIWSampler* pSamplers,u32 nSamplers) :
	_pWrapper(pWrapper)
{
	_pEffect = _pWrapper->CreateEffect(pProgram,pRasterizerState,pBlendState,nBlendStates,AlphaToCoverage,pSamplers,nSamplers);
}

void EffectContainer::SetConstant(const char* pName,const APIWConstantBuffer* pBuffer)
{
	_pEffect->SetConstant(pName,pBuffer);
}

void EffectContainer::SetTexture(const char* pName,const APIWTexture* pTexture)
{
	_pEffect->SetTexture(pName,pTexture);
}

void EffectContainer::Draw(u32 pass, const APIWVertexArray* pArray) const
{
	_pEffect->Draw(const_cast<APIWVertexArray*>(pArray));
}

}