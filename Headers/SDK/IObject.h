#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef __IOBJECT_H__
#define __IOBJECT_H__

namespace HAGE {

class IObject
{
public:
	virtual bool MessageProc(const MessageObjectUnknown* pMessage) = 0;
protected:
	virtual result Destroy() = 0;
	friend class CoreFactory;
};

}

#endif