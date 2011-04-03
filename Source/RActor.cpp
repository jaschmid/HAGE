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
		
		
		if(strcmp(pInit->mesh,"Box")==0)
		{
			_mesh = GetResource()->OpenResource<IDrawableMesh>("t_landscape.hgeo");
			//@world.MPQ\\world\\maps\\Azeroth\\Azeroth_38_40.adt
			//art.mpq\\World\\Azeroth\\Elwynn\\BUILDINGS\\BlackSmith\\BlackSmithBrick01.blp
			//@world.MPQ\\world\\maps\\Azeroth\\Azeroth_38_40_tex0.adt
			_texture = GetResource()->OpenResource<ITextureImage>("t_landscape.hsvt");
		}
		else
		{
			_mesh = GetResource()->OpenResource<IDrawableMesh>(pInit->mesh);
			_texture = GetResource()->OpenResource<ITextureImage>("Null");
		}
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
			if(_mesh->GetTexture(0))
				pEffect->SetTexture("DiffuseTexture",_mesh->GetTexture(0));
			else
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
