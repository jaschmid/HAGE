#include "header.h"
#include "LogicDomain.h"
#include "AIDomain.h"
#include "GraphicsDomain.h"
#include "RenderingDomain.h"
#include "SoundDomain.h"

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
};

extern IMain* HAGECreateMain(void)
{
	return new class HAGEMain;
}

DEFINE_DOMAIN(AIDomain);
DEFINE_DOMAIN(LogicDomain);
DEFINE_DOMAIN(GraphicsDomain);
DEFINE_DOMAIN(SoundDomain);
DEFINE_DOMAIN(RenderingDomain);

}
