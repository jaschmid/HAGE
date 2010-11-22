#include "header.h"
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
		m_pLogic	= DomainCreator<LogicDomain>();
		m_pAI		= DomainCreator<AIDomain>();
		m_pGraphics = DomainCreator<GraphicsDomain>();
		m_pRendering= DomainCreator<RenderingDomain>();
		m_pSound	= DomainCreator<SoundDomain>();
		m_pResource = DomainCreator<ResourceDomain>();
	}
	~HAGEMain()
	{
	}
private:
	const LogicDomain*		m_pLogic;
	const AIDomain*			m_pAI;
	const GraphicsDomain*		m_pGraphics;
	const RenderingDomain*	m_pRendering;
	const SoundDomain*		m_pSound;
	const ResourceDomain*		m_pResource;
};

extern IMain* HAGECreateMain(void)
{
	return new class HAGEMain;
}

}