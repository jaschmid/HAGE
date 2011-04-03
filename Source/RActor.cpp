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
		
		_meshCircle = GetResource()->OpenResource<IDrawableMesh>("torus.ply");
		_meshPoles = GetResource()->OpenResource<IDrawableMesh>("cylinder.ply");
		_textureLines = GetResource()->OpenResource<ITextureImage>("Null");

		if(strcmp(pInit->mesh,"Box")==0)
		{
			_mesh = GetResource()->OpenResource<IDrawableMesh>("t_landscape.hgeo");
			//_mesh = GetResource()->OpenResource<IDrawableMesh>("landscape.hgeo");
			//@world.MPQ\\world\\maps\\Azeroth\\Azeroth_38_40.adt
			//art.mpq\\World\\Azeroth\\Elwynn\\BUILDINGS\\BlackSmith\\BlackSmithBrick01.blp
			//@world.MPQ\\world\\maps\\Azeroth\\Azeroth_38_40_tex0.adt
			_texture = GetResource()->OpenResource<ITextureImage>("t_landscape.hsvt");
		}
		else
		{
			_mesh = GetResource()->OpenResource<IDrawableMesh>(pInit->mesh);
			_texture = GetResource()->OpenResource<ITextureImage>(pInit->texture);
		}
		if(Input1::IsReady() )
		{
			data = Input1::Get();
		}
	}
		
	int RenderingActor::Draw(EffectContainer* pEffect,const position_constants& c,APIWConstantBuffer* pBuffer,bool bShadow,bool bOrbit)
	{
		if(bShadow && !_init.bCastShadow)
			return 0;
		if(Input1::IsReady())
		{
			data = Input1::Get();

			//hack inverse

			Vector3<> position = Vector3<>(data.worldMatrix.Row(0).w,data.worldMatrix.Row(1).w,data.worldMatrix.Row(2).w);

			Matrix4<> inv_world =(Matrix4<>::Translate(-position)*data.worldMatrix).Transpose()* Matrix4<>::Translate(-position);

			Matrix4<> check = data.worldMatrix * inv_world;

				
			position_constants pc = c;
			pc.model =					(data.worldMatrix*Matrix4<>::Scale(scale)).Transpose();
			pc.modelview =				(c.modelview.Transpose()*pc.model.Transpose()).Transpose();
			pc.inverse_modelview =		((Matrix4<>::Scale(-scale)*inv_world)*c.inverse_modelview.Transpose()).Transpose();
			pc.modelview_projection =	(c.modelview_projection.Transpose()*pc.model.Transpose()).Transpose();

			
			if(_init.bCastShadow)
			{	
				pc.diffuseFactor = 1.0f;
				pc.ambientFactor = 0.0f;
			}
			else
			{
				pc.diffuseFactor = 0.0f;
				pc.ambientFactor = 1.0f;
			}

			pBuffer->UpdateContent(&pc);
			if(_mesh->GetTexture(0))
				pEffect->SetTexture("DiffuseTexture",_mesh->GetTexture(0));
			else
				pEffect->SetTexture("DiffuseTexture",_texture->GetTexture());
			pEffect->Draw(0,_mesh->GetVertexArray());
			
			if(!bShadow && _init.behavior == HAGE::ACTOR_BEHAVIOR_PLANET)
			{
				
				pc.model =					(data.worldMatrix * (Matrix4<>::Scale(scale * 3.0f) * Matrix4<>::AngleRotation(Vector3<>(1.0f,0.0f,0.0f),3.14f/2.0f))).Transpose();
				pc.modelview =				(c.modelview.Transpose()*pc.model.Transpose()).Transpose();
				pc.inverse_modelview =		(inv_world*c.inverse_modelview.Transpose()).Transpose();
				pc.modelview_projection =	(c.modelview_projection.Transpose()*pc.model.Transpose()).Transpose();

				pc.diffuseFactor = 0.0f;
				pc.ambientFactor = 1.0f;
				pBuffer->UpdateContent(&pc);
				pEffect->SetTexture("DiffuseTexture",_textureLines->GetTexture());
				pEffect->Draw(0,_meshPoles->GetVertexArray());
				
				if(bOrbit)
				{
				
					pc.model =					(data.orbitWorldMatrix* (Matrix4<>::Scale(Vector3<>(_init.initial_distance,_init.initial_distance,_init.initial_distance))* Matrix4<>::AngleRotation(Vector3<>(1.0f,0.0f,0.0f),3.14f/2.0f))).Transpose();
					pc.modelview =				(c.modelview.Transpose()*pc.model.Transpose()).Transpose();
					pc.inverse_modelview =		(inv_world*c.inverse_modelview.Transpose()).Transpose();
					pc.modelview_projection =	(c.modelview_projection.Transpose()*pc.model.Transpose()).Transpose();

					pc.diffuseFactor = 0.0f;
					pc.ambientFactor = 1.0f;
					pBuffer->UpdateContent(&pc);
					pEffect->SetTexture("DiffuseTexture",_textureLines->GetTexture());
					pEffect->Draw(0,_meshCircle->GetVertexArray());
				}
			}
		}
		return 1;
	}

	RenderingActor::~RenderingActor()
	{
	}

	DEFINE_CLASS_GUID(RenderingActor);
}
