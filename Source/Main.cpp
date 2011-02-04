#include "header.h"
#include "LogicDomain.h"
#include "AIDomain.h"
#include "GraphicsDomain.h"
#include "RenderingDomain.h"
#include "SoundDomain.h"

#include "SettingsLoader.h"

namespace HAGE {

extern const char* GetDomainName(const guid& guid)
{
	if(guid == guid_of<LogicDomain>::Get())
		return guid_of<LogicDomain>::Name();
	else if(guid == guid_of<GraphicsDomain>::Get())
		return guid_of<GraphicsDomain>::Name();
	else if(guid == guid_of<RenderingDomain>::Get())
		return guid_of<RenderingDomain>::Name();
	else if(guid == guid_of<SoundDomain>::Get())
		return guid_of<SoundDomain>::Name();
	else if(guid == guid_of<AIDomain>::Get())
		return guid_of<AIDomain>::Name();
	else if(guid == guid_of<InputDomain>::Get())
		return guid_of<InputDomain>::Name();
	else if(guid == guid_of<ResourceDomain>::Get())
		return guid_of<ResourceDomain>::Name();
}

class HAGEMain : public IMain
{
public:
	HAGEMain()
	{
		initSettings();
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
