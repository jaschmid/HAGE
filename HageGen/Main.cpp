#include "header.h"

namespace HAGE {

extern const char* GetDomainName(const guid& guid)
{
}

class HAGEMain : public IMain
{
public:
	HAGEMain()
	{
	}
	~HAGEMain()
	{
	}
private:
};

extern IMain* HAGECreateMain(void)
{
	return new class HAGEMain;
}

}
