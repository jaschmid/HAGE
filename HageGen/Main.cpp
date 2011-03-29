#include "header.h"

#include "GenerationDomain.h"

namespace HAGE {

extern const char* GetDomainName(const guid& guid)
{
	if(guid == guid_of<GenerationDomain>::Get())
		return guid_of<GenerationDomain>::Name();
}

class HAGEMain : public IMain
{
public:
	HAGEMain()
	{
		m_pGen	= DomainCreator<GenerationDomain>();
	}
	~HAGEMain()
	{
	}
private:
	const GenerationDomain*		m_pGen;
};

extern IMain* HAGECreateMain(void)
{
	return new class HAGEMain;
}

DEFINE_DOMAIN(GenerationDomain);

}
