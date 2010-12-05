/********************************************************/
/* FILE: traits.h								        */
/* DESCRIPTION: Defines traits for HAGE templates       */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/

#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef __TRAITS_BASE_H__
#define __TRAITS_BASE_H__

namespace HAGE {

class VoidTraits
{
};

template<class _Final> class get_traits
{
	// abstract class
};

template<> class get_traits<void> : VoidTraits
{
	// abstract class
};

template<class _Final> class guid_of<get_traits<_Final>> : public guid_of<_Final>
{
};

template<class _Final> class domain_access<get_traits<_Final>> : public domain_access<_Final>
{
};


template<class _Domain> class DomainOutputTraits
{
	public:
		typedef _Domain		Domain;
		typedef DomainOutputTraits<Domain> Traits;
};
template<> class DomainOutputTraits<void> : public VoidTraits
{
	public:
		typedef void		Domain;
		typedef VoidTraits	Traits;
};

template<class _Domain,bool bEnabled> class DomainOutputTraitsWrapper
{
	//abstract
};

template<class _Domain> class DomainOutputTraitsWrapper<_Domain,true>
{
	public:
		typedef typename DomainOutputTraits<_Domain>::Traits	Traits;
};
template<class _Domain> class DomainOutputTraitsWrapper<_Domain,false>
{
	public:
		typedef typename DomainOutputTraits<void>::Traits	Traits;
};

template<class _Input,i32 _Delay> class InputDelay
{
};
template<class _Domain,class _Input> class DomainInputTraits
{
	public:
		typedef _Domain				    Domain;
		typedef _Input					SourceDomain;
		typedef DomainInputTraits<Domain,SourceDomain> Traits;
		static	const i32				SInputDelay = 0;
};
template<class _Domain,class _Input,i32 _Delay> class DomainInputTraits<_Domain,InputDelay<_Input,_Delay>>
{
	public:
		typedef _Domain				    Domain;
		typedef _Input					SourceDomain;
		typedef DomainInputTraits<Domain,InputDelay<_Input,_Delay>> Traits;
		static	const i32				SInputDelay = _Delay;
};

template<u32 _Index> class _VoidInput
{
public:
	typedef void	Domain;
};

template<class _Domain> class DomainInputTraits<_Domain,void>
{
	public:
		typedef void				    Domain;
		typedef void					SourceDomain;
		typedef _VoidInput<0>			Traits;
		static	const i32				SInputDelay = 0;
};

template<class _Domain,u32 i> class DomainInputTraits<_Domain,_VoidInput<i>>
{
	public:
		typedef void				    Domain;
		typedef void					SourceDomain;
		typedef _VoidInput<i>			Traits;
		static	const i32				SInputDelay = 0;
};

template<class _Domain> class DomainBase;
template<class _Domain,bool bOutput = false,class _Input1 = _VoidInput<1>,class _Input2 = _VoidInput<2>> class DomainTraits
{
	public:
		typedef typename DomainOutputTraitsWrapper<_Domain,bOutput>::Traits		OutputTraits;
		typedef typename DomainInputTraits<_Domain,_Input1>::Traits			Input1Traits;
		typedef typename DomainInputTraits<_Domain,_Input2>::Traits			Input2Traits;
		typedef _Domain												Domain;
		typedef	DomainBase<_Domain>									DomainBaseType;
};


template<class _Domain,class _OutputType> class ObjectOutputTraits
{
	public:
		typedef _OutputType Type;
		typedef _Domain		Domain;
		typedef ObjectOutputTraits<Domain,Type> Traits;
};
template<class _Domain> class ObjectOutputTraits<_Domain,void> : public VoidTraits
{
	public:
		typedef VoidTraits Traits;
};

template<class _Domain,class _Input> class ObjectInputTraits
{
	public:
		typedef _Domain				    Domain;
		typedef _Input					SourceClass;
		typedef ObjectInputTraits<Domain,SourceClass> Traits;
};

template<class _Domain> class ObjectInputTraits<_Domain,void>
{
	public:
		typedef _VoidInput<0> Traits;
};
template<class _Domain,u32 i> class ObjectInputTraits<_Domain,_VoidInput<i>>
{
	public:
		typedef _VoidInput<i> Traits;
};
template<class _Final> class ObjectBase;
template<class _Final,class _Domain,class _InitType=void,class _OutputType = void,class _Input1 = _VoidInput<1>,class _Input2 = _VoidInput<2>> class ObjectTraits
{
	public:
		typedef typename ObjectOutputTraits<_Domain,_OutputType>::Traits		OutputTraits;
		typedef typename ObjectInputTraits<_Domain,_Input1>::Traits			Input1Traits;
		typedef typename ObjectInputTraits<_Domain,_Input2>::Traits			Input2Traits;
		typedef _Final												ObjectType;
		typedef _InitType											ObjectInitType;
		typedef _Domain												Domain;
		typedef	ObjectBase<_Final>									ObjectBaseType;
};

template<class _Type, class _Stage0Loader,class _Stage1Loader = void,class _Stage2Loader = void> class ResourceTraits
{
	public:
		typedef _Type	ResourceType;
		typedef _Stage0Loader Stage0Loader;
		typedef _Stage1Loader Stage1Loader;
		typedef _Stage2Loader Stage2Loader;
};

}

#endif
