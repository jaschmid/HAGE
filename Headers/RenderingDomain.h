#ifndef RENDERING__DOMAIN__INCLUDED
#define RENDERING__DOMAIN__INCLUDED

#include "header.h"

namespace HAGE {

class UserInterfaceRendering;

class RenderingDomain : public DomainBase<RenderingDomain>
{
	public:
		RenderingDomain();
		~RenderingDomain();
		void DomainStep(u64 step);

		const Matrix4<>& GetViewMatrix();
		const Matrix4<>& GetProjectionMatrix();
		RenderingAPIWrapper* GetWrapper(){return pWrapper;}

	private:
		virtual bool MessageProc(const Message* pMessage);

		float										fCameraX,fCameraY,fCameraZ;
		
		RenderingAPIWrapper*						pWrapper;

		UserInterfaceRendering*						pInterface;
		Matrix4<>		_viewMatrix;
		Matrix4<>		_projectionMatrix;


		TResourceAccess<IDrawableMesh>						Ico;
};

}

#endif