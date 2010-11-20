#include "header.h"
#include "InputDomain.h"
#include "LogicDomain.h"
#include "AIDomain.h"
#include "GraphicsDomain.h"
#include "RenderingDomain.h"
#include "SoundDomain.h"
#include "ResourceDomain.h"

namespace HAGE {

class HAGEMain : public IMain
{
public:
	HAGEMain()
	{
		m_pInput	= new InputDomain();
		m_pLogic	= new LogicDomain();
		m_pAI		= new AIDomain();
		m_pGraphics = new GraphicsDomain();
		m_pRendering= new RenderingDomain();
		m_pSound	= new SoundDomain();
		m_pResource = new ResourceDomain();
	}
	void MessageProc(const Message& m)
	{
		m_pInput->PostInputMessage(m);
	}
	~HAGEMain()
	{
		delete m_pInput;
		delete m_pLogic;
		delete m_pAI;
		delete m_pGraphics;
		delete m_pRendering;
		delete m_pSound;
		delete m_pResource;
	}
private:
	InputDomain*		m_pInput;
	LogicDomain*		m_pLogic;
	AIDomain*			m_pAI;
	GraphicsDomain*		m_pGraphics;
	RenderingDomain*	m_pRendering;
	SoundDomain*		m_pSound;
	ResourceDomain*		m_pResource;
};

extern IMain* HAGECreateMain(void)
{
	return new class HAGEMain;
}

}