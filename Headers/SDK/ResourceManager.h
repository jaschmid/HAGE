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

struct SStagedResource
{
	u32		nCurrentStage;
	void*	pCurrentStage;
	u32		nRefCount;
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
	MessageResourceNotifyLoaded(SStagedResource* pMaster,const std::string& name,const guid& type) : MessageHelper<MessageResourceNotifyLoaded,MessageResourceUnknown>(id),
		_type(type),_Master(pMaster)
	{ assert(name.size() <= 255); strcpy(_name,name.c_str());}
	const char* GetName() const{return _name;}
	const guid& GetType() const{return _type;}
	const SStagedResource* GetMaster() const{return _Master;}
private:
	char				_name[256];
	guid				_type;
	SStagedResource*	_Master;
	static const u32 id = MESSAGE_RESOURCE_NOTIFY_LOADED;
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
			_data->nRefCount++;
	}

	~TResourceAccess()
	{
		if(_data)
			_data->nRefCount--;
	}

	inline TResourceAccess& operator =(const TResourceAccess& other)
	{
		if(_data)
			_data->nRefCount--;
		_data=other._data;
		if(_data)
			_data->nRefCount++;
		return *this;
	}

	inline const _T* operator ->() const {return (const _T*)_data->pCurrentStage;}
	inline u32 getStage() const {return _data->nCurrentStage;}

private:

	TResourceAccess(SStagedResource& data) : _data(&data)
	{
		if(_data)
			_data->nRefCount++;
	}

	SStagedResource* _data;

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
		return TResourceAccess<_T>(_OpenResource(name,guid_of<_T>::Get()));
	}
private:


	void ProcessMessages();
	SStagedResource& _OpenResource(const std::string& name, const guid& guid);

	const std::unordered_map<guid,IResource*,guid_hasher>&					_remoteStage0Database;

	typedef StaticMessageQueue<LOCAL_RESOURCE_QUEUE_SIZE>			QueueInType;
	typedef StaticMessageQueue<RESOURCE_DOMAIN_QUEUE_SIZE>			QueueOutType;
	QueueInType						_queueIn;
	QueueOutType					_queueOut;

	std::unordered_map<tResourceKey,SStagedResource,key_hasher>			_localResourceMap;
	friend class ResourceDomain;
	friend class SharedDomainBase;
};

}

#endif
