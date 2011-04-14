/********************************************************/
/* FILE: ResourceManager.h                              */
/* DESCRIPTION: Defines the ResourceManager class for   */
/*              Controlling the domain level resources  */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/

#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__

#include <unordered_map>

#include <boost/functional/hash.hpp>

namespace HAGE {

const u32 RESOURCE_DOMAIN_QUEUE_SIZE	=	1024*32;
const u32 LOCAL_RESOURCE_QUEUE_SIZE		=	1024*32;

const u32 RESOURCE_STAGE_STREAMING		=	0xffffffff;

class IResource;


class StagedResource
{
public:
	
	u32 GetCurrentStage() const{ return nCurrentStage;}
	const IResource* GetResource() const{ return  pCurrentStage;}
	void AddRef() const{assert(nRefCount!=RESOURCE_STAGE_STREAMING);_InterlockedIncrement(&nRefCount);}
	void Release() const {assert(nRefCount!=RESOURCE_STAGE_STREAMING);_InterlockedDecrement(&nRefCount);}

protected:

	StagedResource(u32 nStage,const IResource* pStage,i32 ref) :
		 nCurrentStage(nStage),pCurrentStage(pStage),nRefCount(ref)
		 {
		 }
	
	StagedResource(StagedResource& other)
		 {
		 }

	StagedResource(){}
	~StagedResource(){}
	
	u32		nCurrentStage;
	const IResource*	pCurrentStage;
	mutable volatile i32		nRefCount;
};

class MessageResourceUnknown : public Message
{
public:
	MessageResourceUnknown(u32 MessageCode,u32 Size) : Message(MessageCode,Size) {}
private:
	static const u32 id = MESSAGE_RESOURCE_UNKNOWN;
};

class MessageResourceRequestLoad : public MessageHelper<MessageResourceRequestLoad,MessageResourceUnknown>
{
public:
	MessageResourceRequestLoad(const std::string& name,const guid& type) : MessageHelper<MessageResourceRequestLoad,MessageResourceUnknown>(id),_type(type)
	{ assert(name.size() <= 255); strcpy(_name,name.c_str());}
	const char* GetName() const{return _name;}
	const guid& GetType() const{return _type;}
private:
	char				_name[256];
	guid				_type;
	static const u32 id = MESSAGE_RESOURCE_REQUEST_LOAD;
};

struct SStagedResource;

class MessageResourceNotifyLoaded : public MessageHelper<MessageResourceNotifyLoaded,MessageResourceUnknown>
{
public:
	MessageResourceNotifyLoaded(const StagedResource* pMaster,const std::string& name,const guid& type) : MessageHelper<MessageResourceNotifyLoaded,MessageResourceUnknown>(id),
		_type(type),_Master(pMaster)
	{ assert(name.size() <= 255); strcpy(_name,name.c_str());}
	const char* GetName() const{return _name;}
	const guid& GetType() const{return _type;}
	const StagedResource* GetMaster() const{return _Master;}
private:
	char				_name[256];
	guid				_type;
	const StagedResource*	_Master;
	static const u32 id = MESSAGE_RESOURCE_NOTIFY_LOADED;
};

class MessageResourceNotifyStreamOpen : public MessageHelper<MessageResourceNotifyStreamOpen,MessageResourceUnknown>
{
public:
	MessageResourceNotifyStreamOpen(const StagedResource* pMaster,const StagedResource* pStreamingMaster,const std::string& name,const guid& type) : MessageHelper<MessageResourceNotifyStreamOpen,MessageResourceUnknown>(id),
		_type(type),_Master(pMaster),_StreamingMaster(pStreamingMaster)
	{ assert(name.size() <= 255); strcpy(_name,name.c_str());}
	const char* GetName() const{return _name;}
	const guid& GetType() const{return _type;}
	const StagedResource* GetMaster() const{return _Master;}
	const StagedResource* GetStreamingMaster() const{return _StreamingMaster;}
private:
	char				_name[256];
	guid				_type;
	const StagedResource*	_Master;
	const StagedResource*	_StreamingMaster;
	static const u32 id = MESSAGE_RESOURCE_NOTIFY_STREAM_OPEN;
};

class MessageResourceNotifyStreamIn : public MessageHelper<MessageResourceNotifyStreamIn,MessageResourceUnknown>
{
public:
	MessageResourceNotifyStreamIn(const StagedResource* pStream,const std::string& name,const guid& type) : MessageHelper<MessageResourceNotifyStreamIn,MessageResourceUnknown>(id),
		_type(type),_Stream(pStream)
	{ assert(name.size() <= 255); strcpy(_name,name.c_str());}
	const char* GetName() const{return _name;}
	const guid& GetType() const{return _type;}
	const StagedResource* GetStream() const{return _Stream;}
private:
	char				_name[256];
	guid				_type;
	const StagedResource*	_Stream;
	static const u32 id = MESSAGE_RESOURCE_NOTIFY_STREAM_IN;
};

class MessageResourceNotifyStreamOut : public MessageHelper<MessageResourceNotifyStreamOut,MessageResourceUnknown>
{
public:
	MessageResourceNotifyStreamOut(const StagedResource* pStream,const std::string& name,const guid& type) : MessageHelper<MessageResourceNotifyStreamOut,MessageResourceUnknown>(id),
		_type(type),_Stream(pStream)
	{ assert(name.size() <= 255); strcpy(_name,name.c_str());}
	const char* GetName() const{return _name;}
	const guid& GetType() const{return _type;}
	const StagedResource* GetStream() const{return _Stream;}
private:
	char				_name[256];
	guid				_type;
	const StagedResource*	_Stream;
	static const u32 id = MESSAGE_RESOURCE_NOTIFY_STREAM_OUT;
};

class MessageResourceRequestStreamClose : public MessageHelper<MessageResourceRequestStreamClose,MessageResourceUnknown>
{
public:
	MessageResourceRequestStreamClose(const std::string& name,const guid& type) : MessageHelper<MessageResourceRequestStreamClose,MessageResourceUnknown>(id),
		_type(type)
	{ assert(name.size() <= 255); strcpy(_name,name.c_str());}
	const char* GetName() const{return _name;}
	const guid& GetType() const{return _type;}
private:
	char				_name[256];
	guid				_type;
	static const u32 id = MESSAGE_RESOURCE_REQUEST_STREAM_CLOSE;
};

class ResourceAccess
{
public:

	ResourceAccess() : _data(nullptr)
	{
	}


	ResourceAccess(const ResourceAccess& in) : _data(in._data)
	{
		if(_data)
			_data->AddRef();
	}

	~ResourceAccess()
	{
		if(_data)
			_data->Release();
	}

	inline ResourceAccess& operator =(const ResourceAccess& other)
	{
		if(_data)
			_data->Release();
		_data=other._data;
		if(_data)
			_data->AddRef();
		return *this;
	}

	inline const IResource* operator ->() const {return (const IResource*)_data->GetResource();}
	inline u32 getStage() const {return _data->GetCurrentStage();}
	inline operator const IResource*() const {return (const IResource*)_data->GetResource();}

private:

	ResourceAccess(StagedResource& data) : _data(&data)
	{
		if(_data)
			_data->AddRef();
	}

	StagedResource* _data;

	friend class ResourceDomain;
	template<class _T> friend class TResourceAccess;
};

template<class _T> class TResourceAccess
{
public:

	TResourceAccess() : _data(nullptr)
	{
	}

	TResourceAccess(const TResourceAccess<_T>& in) : _data(in._data)
	{
		if(_data)
			_data->AddRef();
	}

	TResourceAccess(const ResourceAccess& in) : _data(in._data)
	{
		if(_data)
			_data->AddRef();
	}

	~TResourceAccess()
	{
		if(_data)
			_data->Release();
	}

	inline TResourceAccess& operator =(const TResourceAccess& other)
	{
		if(_data)
			_data->Release();
		_data=other._data;
		if(_data)
			_data->AddRef();
		return *this;
	}

	inline TResourceAccess& operator =(const ResourceAccess& other)
	{
		if(_data)
			_data->Release();
		_data=other._data;
		if(_data)
			_data->AddRef();
		return *this;
	}

	inline const _T* operator ->() const {return (const _T*)_data->GetResource();}
	inline u32 getStage() const {return _data->GetCurrentStage();}

private:

	TResourceAccess(StagedResource& data) : _data(&data)
	{
		if(_data)
			_data->AddRef();
	}

	StagedResource* _data;

	friend class CResourceManager;
};

class IResource;

struct guid_hasher
{
	size_t operator()(const guid& g) const
	{
		return (size_t)(g.ll[0] ^ g.ll[1]);
	}
};


typedef	std::pair<std::string,guid> tResourceKey;

struct key_hasher
{
	size_t operator()(const tResourceKey& key) const
	{
		size_t seed = 0xbadf00d;
		boost::hash_combine<std::string>(seed,key.first);
		boost::hash_combine<HAGE::u64>(seed,key.second.ll[0] ^ key.second.ll[1]);

		return seed;
	}
};

class CResourceManager
{
public:
	CResourceManager();
	~CResourceManager();
	template<class _T> TResourceAccess<_T> OpenResource(const std::string& name)
	{
		return TResourceAccess<_T>(*_OpenResource(name,guid_of<_T>::Get()));
	}

	void RunGarbageCollection();
private:
	
	class StagedResourceLocal : public StagedResource
	{
	public:
		
		~StagedResourceLocal(){ pMaster->Release(); }

		StagedResourceLocal(const StagedResource* rMaster, const StagedResource* rStreamingMaster = nullptr) :
			StagedResource((rStreamingMaster)?(rStreamingMaster->GetCurrentStage()):(rMaster->GetCurrentStage()),
				(rStreamingMaster)?(rStreamingMaster->GetResource()):(rMaster->GetResource()),0),pMaster(rMaster),pStreamingMaster(rStreamingMaster) {pMaster->AddRef();}
		StagedResourceLocal(StagedResourceLocal& other) :
			StagedResource(other.nCurrentStage,other.pCurrentStage,0),pMaster(other.pMaster),pStreamingMaster(other.pStreamingMaster) {pMaster->AddRef();}
		StagedResourceLocal& operator = (const StagedResourceLocal& other) 
		{
			pMaster->Release(); 
			nCurrentStage = other.nCurrentStage; 
			pCurrentStage = other.pCurrentStage; 
			pMaster = other.pMaster;
			pMaster->AddRef();
			pStreamingMaster = other.pStreamingMaster;
		}
		u32 GetRefCount() const{return nRefCount;}
		void Update(const StagedResource* pUpdate)
		{
			pMaster->Release();
			pMaster=pUpdate;
			nCurrentStage=pUpdate->GetCurrentStage();
			pCurrentStage=pUpdate->GetResource();
			pMaster->AddRef();
		}
		const StagedResource* UpdateStreaming(const StagedResource* pUpdate)
		{
			const StagedResource* result = pStreamingMaster;
			pStreamingMaster = pUpdate;
			if(pUpdate)
			{
				nCurrentStage=pUpdate->GetCurrentStage();
				pCurrentStage=pUpdate->GetResource();
			}
			return result;
		}
	
		bool IsStreaming() const{return pStreamingMaster!=nullptr;}
		bool HasMaster() const{return pMaster!=nullptr;}
		
	private:
		const StagedResource*	pMaster;
		const StagedResource*	pStreamingMaster;
	};

	void ProcessMessages();
	StagedResource* _OpenResource(const std::string& name, const guid& guid);

	const std::unordered_map<guid,StagedResource*,guid_hasher>&					_remoteStage0Database;

	typedef StaticMessageQueue<LOCAL_RESOURCE_QUEUE_SIZE>			QueueInType;
	typedef StaticMessageQueue<RESOURCE_DOMAIN_QUEUE_SIZE>			QueueOutType;
	QueueInType						_queueIn;
	QueueOutType					_queueOut;

	std::unordered_map<tResourceKey,StagedResourceLocal,key_hasher>			_localResourceMap;
	friend class ResourceDomain;
	friend class SharedDomainBase;
};

}

#endif
