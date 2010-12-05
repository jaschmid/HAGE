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

		void DrawIco(Vector3<> Location);

	private:
		virtual bool MessageProc(const Message* pMessage);

		float										fCameraX,fCameraY,fCameraZ;
		
		RenderingAPIWrapper*						pWrapper;

		UserInterfaceRendering*						pInterface;

		TResourceAccess<IDrawableMesh>						Ico;
};

}

#endif