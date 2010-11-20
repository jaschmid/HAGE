#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef MAIN_CLASS_H_INCLUDED
#define MAIN_CLASS_H_INCLUDED

namespace HAGE {

	class  IMain
	{
	public:
		virtual ~IMain(){};
		virtual void MessageProc(const Message& m){}
	};
	extern IMain* HAGECreateMain();

}

#endif