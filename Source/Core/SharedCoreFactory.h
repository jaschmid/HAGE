#ifndef __SHARED_COREFACTORY_H__
#define __SHARED_COREFACTORY_H__

#include <HAGE.h>

#include <map>

namespace HAGE {

class SharedCoreFactory
{
public:
	SharedCoreFactory();
	~SharedCoreFactory();

	result CreateObject(guid ObjectId, guid InterfaceId, void** ppInterface);
};

}

#endif
