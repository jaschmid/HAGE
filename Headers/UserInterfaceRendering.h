#ifndef USER__INTERFACERENDERING__INCLUDED
#define USER__INTERFACERENDERING__INCLUDED

#include "header.h"
#include "RenderingDomain.h"

namespace HAGE {

class RenderingDomain;

class UserInterfaceRendering : protected DomainMember<RenderingDomain>
{
public:
	UserInterfaceRendering(RenderingAPIWrapper* pWrapper);
	~UserInterfaceRendering();
	bool ProcessUserInterfaceMessage(const Message* pMessage);

	void Draw();
private:
	RenderingAPIWrapper*	m_pWrapper;

	Vector2<>				m_vMousePosition;
	bool					m_bMouseVisible;

	bool					m_bDebug;
	
	APIWVertexBuffer*		m_pVBSquare;
	APIWVertexArray*		m_pVASquare;
	APIWConstantBuffer*		m_pConstants;
	APIWEffect*				m_pEffect2D;
	TResourceAccess<IVirtualTexture>			_VT;
};

}

#endif