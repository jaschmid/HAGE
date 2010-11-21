#ifndef DEBUG_USER__INTERFACERENDERING__INCLUDED
#define DEBUG_USER__INTERFACERENDERING__INCLUDED

#include "HAGE.h"

namespace HAGE {
class DebugUI;

class RenderDebugUI
{
public:
	RenderDebugUI(RenderingAPIWrapper* pWrapper);
	~RenderDebugUI();

	void Draw();
private:
	RenderingAPIWrapper*	m_pWrapper;

	Vector2<>				m_vMousePosition;

	bool					m_bVisible;
	
	APIWVertexBuffer*		m_pVBSquare;
	APIWVertexArray*		m_pVASquare;
	APIWConstantBuffer*		m_pConstantsBackdrop;
	APIWConstantBuffer*		m_pConstantsPointer;
	APIWEffect*				m_pEffect2D;

	friend class			HAGE::DebugUI;

	//non synced debug UI input
	static StaticMessageQueue<1024*128>	DebugUIControl;
};

}

#endif