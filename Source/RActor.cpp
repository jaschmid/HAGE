#include "header.h"
#include "RActor.h"

namespace HAGE {

	RenderingActor* RenderingActor::CreateSub(const guid& ObjectId,const MemHandle& h,const guid& source)
	{
		return new RenderingActor(ObjectId,h,source);
	}

	RenderingActor::RenderingActor(const guid& ObjectId,const MemHandle& h,const guid& source) :
		ObjectBase<RenderingActor>(ObjectId,h,source)
	{
		char temp[256];
		sprintf(temp,"mesh%i.ply",rand()%3);
		_mesh = GetResource()->OpenResource<IDrawableMesh>(temp);
		if(Input1::IsReady() )
		{
			position = Input1::Get();
		}
	}
		
	int RenderingActor::Draw(EffectContainer* pEffect,const position_constants& c,APIWConstantBuffer* pBuffer)
	{
		if(Input1::IsReady() )
		{
			position = Input1::Get();
			position_constants pc = c;
			pc.model =					Matrix4<>::Translate(position);
			pc.modelview =				c.modelview*pc.model;
			pc.inverse_modelview =		Matrix4<>::Translate(-position)*c.inverse_modelview;
			pc.modelview_projection =	c.modelview_projection*Matrix4<>::Translate(position);
			pBuffer->UpdateContent(&pc);
			pEffect->Draw(0,_mesh->GetVertexArray());
		}
		return 1;
	}

	RenderingActor::~RenderingActor()
	{
	}

	DEFINE_CLASS_GUID(RenderingActor);
}
