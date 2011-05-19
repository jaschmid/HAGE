#include "header.h"
#include "RLight.h"

namespace HAGE {
	RenderingLight* RenderingLight::CreateSub(const guid& ObjectId,const MemHandle& h,const guid& source)
	{
		return new RenderingLight(ObjectId,h,source);
	}

	RenderingLight::RenderingLight(const guid& ObjectId,const MemHandle& h,const guid& source) :
		ObjectBase<RenderingLight>(ObjectId,h,source)
	{
		
		_mesh = GetResource()->OpenResource<IDrawableMesh>("sphere.ply");
		_texture = GetResource()->OpenResource<ITextureImage>("sun.png");
	}

	GLightOut RenderingLight::Step(RenderingDomain* pRendering)
	{
		_data = Input1::Get();
		return _data;
	}
	
	int RenderingLight::Draw(EffectContainer* pEffect,const position_constants& c,APIWConstantBuffer* pBuffer,bool bShadow,bool bOrbit)
	{
		if(Input1::IsReady())
		{
			//hack inverse

			Vector3<> position = _data.Position;
			Matrix4<> world = Matrix4<>::Translate(position)*Matrix4<>::Scale(Vector3<>(0.1f,0.1f,0.1f));

			Matrix4<> inv_world =world.Invert();
							
			position_constants pc = c;
			pc.model =					(world).Transpose();
			pc.modelview =				(c.modelview.Transpose()*pc.model.Transpose()).Transpose();
			pc.inverse_modelview =		(inv_world*c.inverse_modelview.Transpose()).Transpose();
			pc.modelview_projection =	(c.modelview_projection.Transpose()*pc.model.Transpose()).Transpose();

			
			pc.diffuseFactor = 0.0f;
			pc.ambientFactor = 1.0f;

			pBuffer->UpdateContent(&pc);

			pEffect->SetTexture("DiffuseTexture",_texture->GetTexture());//pEffect->SetTexture("DiffuseTexture",_virtTex->GetCurrentVTRedirection());
			pEffect->Draw(0,_mesh->GetVertexArray());
		}
		return 1;
	}

	RenderingLight::~RenderingLight()
	{
	}

	DEFINE_CLASS_GUID(RenderingLight);
}
