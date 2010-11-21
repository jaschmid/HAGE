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
		void DomainInit(u64 step);
		void DomainStep(u64 step);
		void DomainShutdown(u64 step);

		static const guid& id;
		static const bool continuous;
	private:
		virtual bool MessageProc(const Message* pMessage);

		InputPin<GraphicsDomain,RenderingDomain>	Input;
		InputPin<LogicDomain,RenderingDomain>		InputDirect;

		float										fCameraX,fCameraY,fCameraZ;
		
		RenderingAPIWrapper*						pWrapper;

		UserInterfaceRendering*						pInterface;

		APIWVertexBuffer*							pVertexBuffer;
		APIWVertexArray*							pVertexArray;
		APIWConstantBuffer*							pConstants;
		APIWEffect*									pEffect;
};

}

#endif