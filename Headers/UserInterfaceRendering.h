#ifndef USER__INTERFACERENDERING__INCLUDED
#define USER__INTERFACERENDERING__INCLUDED

#include "header.h"
#include "RenderingDomain.h"

namespace HAGE {

class RenderingDomain;

class UserInterfaceRendering : protected ObjectBase<RenderingDomain>
{
public:
	UserInterfaceRendering(RenderingAPIWrapper* pWrapper);
	~UserInterfaceRendering();
	bool ProcessUserInterfaceMessage(const Message* pMessage);

	void Draw();
private:
	RenderingAPIWrapper*	m_pWrapper;

	Vector2<>				m_vMousePosition;
	
	APIWVertexBuffer*		m_pVBSquare;
	APIWVertexArray*		m_pVASquare;
	APIWConstantBuffer*		m_pConstants;
	APIWEffect*				m_pEffect2D;
};

}

#endif