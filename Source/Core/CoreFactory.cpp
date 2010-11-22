#include <HAGE.h>

namespace HAGE {

	CoreFactory::CoreFactory(PinBase*& pOut,TaskManager* pTask) : m_pout(pOut),m_pTask(pTask)
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
			*ppInterface=it2->second.function(ObjectId);
		
			if(*ppInterface)
			{
				ObjectRefContainer container = {*ppInterface, *it2, 1, bMaster };

				flatObjectList.insert(std::pair<guid,ObjectRefContainer>(ObjectId,container));
				object_map_type::iterator inserted = flatObjectList.find(ObjectId);
				assert(inserted != flatObjectList.end());
				
				for(auto caps = it2->second.capabilities.begin(); caps != it2->second.capabilities.end(); ++caps)
				{
					inserted->second.vCapabilitiesIndices.push_back((u32)caps->pEntry->second.size());
					ObjectEntry o = {*ppInterface,&(*inserted),inserted->second.vCapabilitiesIndices.size()-1};
					caps->pEntry->second.push_back( o );
				}

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

	std::pair<const void*,u32> CoreFactory::_ForEach(boost::function<void (void*,IObject*)> f,size_t size,const guid& capability,bool bSync)
	{
		auto found = capabilitiesObjectLists.find(capability);
		assert(found !=capabilitiesObjectLists.end());

		u32 nItems = (u32)found->second.size();
		
		m_ForEachReturnBuffer.resize(nItems * size);

		if(!bSync)
		{

			m_ForEachFunction = f;
			m_ForEachList = &*found;
			m_ForEachReturnSize = (u32)size;

			m_ForEachGroupSize = nItems/32;
			m_ForEachTotalSize = nItems;

			if(m_ForEachGroupSize == 0)
				m_ForEachGroupSize = 1;

			u32 nTasks = (m_ForEachTotalSize-1) / m_ForEachGroupSize + 1;

			u32 oldSize = (u32)m_ForEachTasks.size();

			if(oldSize > nTasks)
				oldSize = nTasks;
		
			for(u32 i = 0;i<oldSize;++i)
				m_pTask->QueueTask(&m_ForEachTasks[i]);

			if(oldSize < nTasks)
			{
				m_ForEachTasks.resize(nTasks);
				for(int i =oldSize;i<m_ForEachTasks.size();++i)
				{
					m_ForEachTasks[i].m_pFactory = this;
					m_ForEachTasks[i].m_nMyIndex = i;
					m_pTask->QueueTask(&m_ForEachTasks[i]);
				}
			}

			m_pTask->Execute();
		}
		else
		{
			for(u32 i=0;i<nItems;++i)
				f(&m_ForEachReturnBuffer[i*size],found->second[i].pObject);
		}

		std::pair<const void*,u32> res(&m_ForEachReturnBuffer[0],nItems);

		return res;
	}
	
	void CoreFactory::DestroyObjectInternal(object_map_type::iterator& item)
	{
		result res=item->second.pObject->Destroy();
		assert(succeeded(res));
		if(m_pout)
			m_pout->PostMessage(MessageFactoryObjectDestroyed(item->first));

		for(u32 i = 0;i< item->second.registration_entry.second.capabilities.size();++i)
		{
			auto found = item->second.registration_entry.second.capabilities[i].pEntry;

			u32 removed = item->second.vCapabilitiesIndices[i];
			u32 swapped = (u32)found->second.size() -1;

			if(removed != swapped)
			{
				found->second[removed] = found->second[swapped];
				found->second[removed].object_entry->second.vCapabilitiesIndices[found->second[removed].index]=(u32)removed;
				found->second.pop_back();	
			}
			else
				found->second.pop_back();
		}
					
		flatObjectList.erase(item);
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
					DestroyObjectInternal(it);
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
				DestroyObjectInternal(it);
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