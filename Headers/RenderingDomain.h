#ifndef RENDERING__DOMAIN__INCLUDED
#define RENDERING__DOMAIN__INCLUDED

#include "header.h"
#include "GenericLight.h"

namespace HAGE {

class UserInterfaceRendering;

struct position_constants
{
	Matrix4<>	model;
	Matrix4<>	inverse_modelview;
	Matrix4<>	modelview;
	Matrix4<>	modelview_projection;
};

class RenderingDomain : public DomainBase<RenderingDomain>
{
	public:
		RenderingDomain();
		~RenderingDomain();
		void DomainStep(t64 time);

		const Matrix4<>& GetInvViewMatrix();
		const Matrix4<>& GetViewMatrix();
		const Matrix4<>& GetProjectionMatrix();
		const GLightOut& GetLight(u32 n);
		RenderingAPIWrapper* GetWrapper(){return pWrapper;}

	private:
		virtual bool MessageProc(const Message* pMessage);

		float										fCameraX,fCameraY,fCameraZ;
		
		RenderingAPIWrapper*						pWrapper;

		UserInterfaceRendering*						pInterface;
		Matrix4<>		_invViewMatrix;
		Matrix4<>		_viewMatrix;
		Matrix4<>		_projectionMatrix;
		GLightOut		_light[3];
		APIWTexture*	_lightCubeDepth[3];

		APIWConstantBuffer*							_pConstants;
		APIWConstantBuffer*							_pShadowcubeConstants;
		APIWConstantBuffer*							_pLightConstants;
		EffectContainer*							_pEffect;
		EffectContainer*							_pShadowmapEffect;

		TResourceAccess<IMeshData>					_Map;
};

}

#endif