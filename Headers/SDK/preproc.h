/********************************************************/
/* FILE: preproc.h                                      */
/* DESCRIPTION: Defines common preprocessor macros.     */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/ 

#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef HAGE__PREPROCESSOR
#define HAGE__PREPROCESSOR

#undef NDEBUG

#ifndef _Windows
 #if defined(_WIN32)
  #define _Win32
  #define _Windows
 #elif defined(WIN32)
  #define _Win32
  #define _Windows
 #elif defined(__WIN32__)
  #define _Win32
  #define _Windows
 #elif defined(__Win32__)
  #define _Win32
  #define _Windows
 #elif defined(_WINDOWS)
  #define _Windows
 #elif defined(__INTEL__) && defined(__MWERKS__)
 // Metrowerks CodeWarrior doesn't build anything other than Win32 on INTEL, no DOS
  #define _Windows
  #define _Win32
 #endif
#else
 #if defined __Win32__ || defined _WIN32
  #ifndef _Win32
   #define _Win32
  #endif
 #endif
#endif

#if defined(_Windows)
    #define TARGET_WINDOWS
#elif defined(__unix__) || defined(unix)
    #define TARGET_LINUX
#else
    #define TARGET_MACOS
#endif

#ifdef _MSC_VER
    #define COMPILER_MSVC
    #pragma warning( disable :4996)
#elif defined(__GNUC__)
    #define COMPILER_GCC
	//olol gcc does not support nullptr
	const                        // this is a const object...
	class {
	public:
	  template<class T>          // convertible to any type
		operator T*() const      // of null non-member
		{ return 0; }            // pointer...
	  template<class C, class T> // or any type of null
		operator T C::*() const  // member pointer...
		{ return 0; }
	private:
	  void operator&() const;    // whose address can't be taken
	} nullptr = {};
#else
    #error UNKNOWN COMPILER
#endif

#if defined(_WIN64) || defined(__LP64__)
    #define TARGET_X64
#else
    #define TARGET_X86
#endif

#endif
