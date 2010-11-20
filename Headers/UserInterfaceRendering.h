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
	RenderingAPIWrapper* m_pWrapper;
};

}

#endif