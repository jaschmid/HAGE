#include "header.h"
#include "RActor.h"

namespace HAGE {

	RenderingActor* RenderingActor::CreateSub(const guid& ObjectId,const MemHandle& h,const guid& source,const ActorRInit* pInit)
	{
		return new RenderingActor(ObjectId,h,source,pInit);
	}

	RenderingActor::RenderingActor(const guid& ObjectId,const MemHandle& h,const guid& source,const ActorRInit* pInit) :
		ObjectBase<RenderingActor>(ObjectId,h,source),scale(pInit->scale),_init(*pInit)
	{
		
		_mesh = GetResource()->OpenResource<IDrawableMesh>(pInit->mesh);
		if(strcmp(pInit->mesh,"Box")==0)
			_texture = GetResource()->OpenResource<ITextureImage>("WoodBox.png");
		else
			_texture = GetResource()->OpenResource<ITextureImage>("Null");
		if(Input1::IsReady() )
		{
			position = Input1::Get();
		}
	}
		
	int RenderingActor::Draw(EffectContainer* pEffect,const position_constants& c,APIWConstantBuffer* pBuffer)
	{
		if(Input1::IsReady())
		{
			position = Input1::Get();
				
			position_constants pc = c;
			pc.model =					(Matrix4<>::Translate(position)*Matrix4<>::Scale(scale)).Transpose();
			pc.modelview =				(c.modelview.Transpose()*pc.model.Transpose()).Transpose();
			pc.inverse_modelview =		(Matrix4<>::Translate(-position)*c.inverse_modelview.Transpose()).Transpose();
			pc.modelview_projection =	(c.modelview_projection.Transpose()*pc.model.Transpose()).Transpose();
			pBuffer->UpdateContent(&pc);
			pEffect->SetTexture("DiffuseTexture",_texture->GetTexture());
			pEffect->Draw(0,_mesh->GetVertexArray());
		}
		return 1;
	}

	RenderingActor::~RenderingActor()
	{
	}

	DEFINE_CLASS_GUID(RenderingActor);
}
