#include <HAGE.h>

namespace HAGE {

	CoreFactory::CoreFactory(PinBase*& pOut) : m_pout(pOut)
	{
	}
	CoreFactory::~CoreFactory()
	{
	}
	
	bool CoreFactory::TryCreateObjectWithGuid(const guid& ObjectId, const guid& ObjectTypeId, IObject** ppInterface, bool bMaster)
	{
		// check if already exists
		object_map_type::iterator it = flatObjectList.find(ObjectId);
		if(it != flatObjectList.end())
		{
			if(bMaster || it->second.bMaster)
			{
				*ppInterface = nullptr;
				return false;
			}
			else
			{
				it->second.uRefCounter++;
				*ppInterface = it->second.pObject;
				return true;
			}
		}

		function_map_type::iterator it2 =registeredFunctionCreation.find(ObjectTypeId);
		if(it2 != registeredFunctionCreation.end())
		{
			if(m_pout)
				m_pout->PostMessage(MessageFactoryObjectCreated(ObjectId,ObjectTypeId));
			*ppInterface=it2->second(ObjectId);
		
			if(*ppInterface)
			{
				ObjectRefContainer container;
				container.pObject = *ppInterface;
				container.uRefCounter = 1;
				container.bMaster = bMaster;

				flatObjectList.insert(std::pair<guid,ObjectRefContainer>(ObjectId,container));

				return true;
			}
		}
		return false;
	}

	guid CoreFactory::CreateObject(const guid& ObjectTypeId, IObject** ppInterface)
	{
		guid ObjectId = *(guid*)number_generator().data;
		if(TryCreateObjectWithGuid(ObjectId,ObjectTypeId,ppInterface,true))
			return ObjectId;
		else
			return guidNull;
	}

	result CoreFactory::QueryObject(const guid& ObjectId, IObject** ppInterface)
	{
		return S_OK;
	}

	bool CoreFactory::MessageProc(const MessageFactoryUnknown* pMessage)
	{
		if(pMessage->GetMessageCode() == MESSAGE_FACTORY_OBJECT_CREATED)
		{
			MessageFactoryObjectCreated* pDetailed = (MessageFactoryObjectCreated*)pMessage;
			IObject* pObject;
			TryCreateObjectWithGuid(pDetailed->GetObjectId(),pDetailed->GetObjectTypeId(),&pObject);
			return true;
		}
		else if(pMessage->GetMessageCode() == MESSAGE_FACTORY_OBJECT_DESTROYED)
		{
			MessageFactoryObjectDestroyed* pDetailed = (MessageFactoryObjectDestroyed*)pMessage;
			object_map_type::iterator it = flatObjectList.find(pDetailed->GetData());
			if(it != flatObjectList.end())
			{
				assert(!it->second.bMaster);
				it->second.uRefCounter--;
				if(it->second.uRefCounter == 0)
				{
					it->second.pObject->Destroy();
					if(m_pout)
						m_pout->PostMessage(MessageFactoryObjectDestroyed(it->first));
					flatObjectList.erase(it);
				}
			}
			return true;
		}
		return false;
	}

	bool CoreFactory::DispatchMessage(const MessageObjectUnknown* pMessage)
	{
		object_map_type::iterator it = flatObjectList.find(pMessage->GetTarget());
		if(it != flatObjectList.end())
		{
			return it->second.pObject->MessageProc(pMessage);
		}
		
		return false;
	}

	result CoreFactory::DestroyObject(const guid& ObjectId)
	{
		object_map_type::iterator it = flatObjectList.find(ObjectId);
		if(it != flatObjectList.end())
		{
			// only master objects can explicity 
			if( it->second.bMaster)
			{
				if(succeeded(it->second.pObject->Destroy()))
				{
					if(m_pout)
						m_pout->PostMessage(MessageFactoryObjectDestroyed(it->first));
					flatObjectList.erase(it);
					return S_OK;
				}
			}
		}
		return E_FAIL;
	}

	void CoreFactory::Shutdown()
	{
		for(auto it = flatObjectList.begin();it!=flatObjectList.end();it++)
		{
			// assert( it->second.bMaster );
			it->second.pObject->Destroy();
			if(m_pout)
				m_pout->PostMessage(MessageFactoryObjectDestroyed(it->first));
		}
		flatObjectList.clear();
	}
}