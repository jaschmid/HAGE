#include <HAGE.h>

namespace HAGE {

	CoreFactory::CoreFactory(PinBase*& pOut,TaskManager* pTask) : m_pout(pOut),m_pTask(pTask),m_nCurrentStep(0)
	{
	}
	CoreFactory::~CoreFactory()
	{
	}

	CoreFactory::iterator	CoreFactory::begin(const guid& capability)
	{
		auto found = capabilitiesObjectLists.find(capability);
		assert(found !=capabilitiesObjectLists.end());	
		return iterator(found->second.begin(),this);
	}

	CoreFactory::iterator	CoreFactory::end(const guid& capability)
	{
		auto found = capabilitiesObjectLists.find(capability);
		assert(found !=capabilitiesObjectLists.end());
		return iterator(found->second.end(),this);
	}
	

	u32 CoreFactory::size(const guid& capability)
	{
		auto found = capabilitiesObjectLists.find(capability);
		assert(found !=capabilitiesObjectLists.end());

		return (u32)found->second.size();
	}
	

	void	CoreFactory::RegisterObjectOut( const guid& guid, const MemHandle& handle,u32 size)
	{
		object_map_type::iterator new_object = flatObjectList.find(guid);
		assert(new_object != flatObjectList.end());
		new_object->second.nResultSize=m_pout->GetAll(handle,new_object->second.pResultMem);	
		assert(new_object->second.nResultSize==size);
		if(m_pout)
			m_pout->PostMessage(MessageFactoryObjectCreated(guid,new_object->second.registration_entry.first,handle));
	}

	bool CoreFactory::TryCreateObjectWithGuid(const guid& ObjectId, const guid& ObjectTypeId,void* init_data, bool bMaster)
	{
		// check if already exists
		object_map_type::iterator it = flatObjectList.find(ObjectId);
		if(it != flatObjectList.end())
		{
			if(bMaster || it->second.bMaster)
			{
				return false;
			}
			else
			{
				std::pair<const MemHandle&,const guid&>* pPair=(std::pair<const MemHandle&,const guid&>*)init_data;
				MessageObjectOutputInit Init(ObjectId,pPair->first);
				Init.SetSource(pPair->second);
				it->second.pObject->MessageProc(&Init);
				it->second.uRefCounter++;
				return true;
			}
		}

		function_map_type::iterator it2 =registeredFunctionCreation.find(ObjectTypeId);
		if(it2 != registeredFunctionCreation.end())
		{
			ObjectRefContainer container = {nullptr, *it2, 1, bMaster , m_nCurrentStep};	
			container.nResultSize = 0xffffffff;
			flatObjectList.insert(std::pair<guid,ObjectRefContainer>(ObjectId,container));
			object_map_type::iterator new_object = flatObjectList.find(ObjectId);
			assert(new_object != flatObjectList.end());
			IObject* pObject;
			if(bMaster)
				pObject=it2->second.init_function(ObjectId,init_data);
			else
				pObject=it2->second.sub_init(ObjectId,init_data);
			assert(pObject);//currently don't support object creation failing

			new_object->second.pObject=pObject;	
			if(new_object->second.nResultSize == 0xffffffff)
			{
				//output mem has not been set assume no output
				new_object->second.nResultSize=0;	
				
				// generate the output message with empty memhandle
				if(m_pout)
					m_pout->PostMessage(MessageFactoryObjectCreated(ObjectId,ObjectTypeId,MemHandle()));
			}		
			
			
			for(auto caps = it2->second.capabilities.begin(); caps != it2->second.capabilities.end(); ++caps)
			{
				new_object->second.vCapabilitiesIndices.push_back((u32)caps->pEntry->second.size());
				std::vector<u32>* vp = &(new_object->second.vCapabilitiesIndices);
				u32 index = (u32)new_object->second.vCapabilitiesIndices.size()-1;
				ObjectEntry o = {&new_object->first,&new_object->second,index};
				caps->pEntry->second.push_back( o );
			}

			return true;
		}
		return false;
	}

	guid CoreFactory::_CreateObject(const guid& ObjectTypeId, void* InitValue)
	{
		guid ObjectId = *(guid*)number_generator().data;
		function_map_type::iterator it2 =registeredFunctionCreation.find(ObjectTypeId);
		if(it2 != registeredFunctionCreation.end())
		{
			assert(!it2->second.bNoInit);
			if(TryCreateObjectWithGuid(ObjectId,ObjectTypeId,InitValue,true))
				return ObjectId;
			else
				return guidNull;
		}
	}

	IObject* CoreFactory::QueryObject(const guid& ObjectId)
	{
		auto res = flatObjectList.find(ObjectId);
		if(res == flatObjectList.end())
			return nullptr;
		else
			return res->second.pObject;
	}

	std::pair<const void*,u32> CoreFactory::_ForEach(boost::function<bool (void*,IObject*)> f,size_t size,const guid& capability,bool bSync,void* pData,u32 nData,bool bGetSome)
	{
		auto found = capabilitiesObjectLists.find(capability);
		assert(found !=capabilitiesObjectLists.end());

		u32 nItems = (u32)found->second.size();

		if(nItems == 0)
		{
			return std::pair<const void*,u32>(nullptr,0);
		}

		assert( (!bGetSome) || pData);
		assert( (!pData) || (nData >= nItems) );

		if(!pData || bGetSome)
		{
			//gonna use the return buffer
			m_ForEachReturnBuffer.resize(nItems * size);
		}

		if(!pData)
		{
			m_ForEachOut = &m_ForEachReturnBuffer[0];
			m_ForEachGetSomeOut = nullptr;
			m_ForEachGetSomeCounter = 0;
		}
		else if(bGetSome)
		{
			m_ForEachOut = &m_ForEachReturnBuffer[0];
			m_ForEachGetSomeOut =  &((u8*)pData)[0];
			m_ForEachGetSomeCounter = 0;
		}
		else
		{
			m_ForEachOut = &((u8*)pData)[0];
			m_ForEachGetSomeOut = nullptr;
			m_ForEachGetSomeCounter = 0;
		}

		m_ForEachFunction = f;
		m_ForEachList = &*found;
		m_ForEachReturnSize = (u32)size;

		if(!bSync)
		{
			m_ForEachGroupSize = nItems/32;
			m_ForEachTotalSize = nItems;

			if(m_ForEachGroupSize == 0)
				m_ForEachGroupSize = 1;

			u32 nTasks = (m_ForEachTotalSize-1) / m_ForEachGroupSize + 1;
			u32 oldSize = (u32)m_ForEachTasks.size();
			if(oldSize > nTasks)
				oldSize = nTasks;
			else
				m_ForEachTasks.resize(nTasks);

			for(u32 i = 0;i<oldSize;++i)
				m_pTask->QueueTask(&m_ForEachTasks[i]);
			if(oldSize < nTasks)
			{
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
			m_ForEachGroupSize = nItems;
			m_ForEachTotalSize = nItems;

			if(m_ForEachTasks.size() == 0)
			{
				m_ForEachTasks.resize(1);
				m_ForEachTasks[0].m_pFactory = this;
				m_ForEachTasks[0].m_nMyIndex = 0;
			}

			m_ForEachTasks[0]();
		}

		u32 nItemsOut;

		if(bGetSome)
			nItemsOut = m_ForEachGetSomeCounter;
		else
			nItemsOut = nItems;

		std::pair<const void*,u32> res(&m_ForEachOut[0],nItemsOut);

		return res;
	}

	void CoreFactory::_RegisterObjectType(
			std::function<IObject* (const guid&,void*)>* fInit,
			std::function<IObject* (const guid&,void*)>* fSub,
			const guid& classType,const guid& guidInput1,const guid& guidInput2,const guid* guidCapabilities,u32 nCapabilities,u32 nMemOutOffset)
	{
		RegistrationContainer reg = {(fInit?*fInit:(std::function<IObject* (const guid&,void*)>())),(fInit?false:true),(fSub?*fSub:(std::function<IObject* (const guid&,void*)>())),nMemOutOffset};
		registeredFunctionCreation.insert(
			function_map_type::value_type(classType,reg) 
		);

		function_map_type::iterator inserted = registeredFunctionCreation.find(classType);
		assert(inserted != registeredFunctionCreation.end());
		inserted->second.capabilities.resize(nCapabilities);
		for(int i =0;i<nCapabilities;++i)
		{
			const guid& icapability = guidCapabilities[i];
			auto found = capabilitiesObjectLists.find(icapability);
			if(found ==capabilitiesObjectLists.end())
			{
				capabilitiesObjectLists.insert(object_capabilties_map_type::value_type(icapability,std::vector<ObjectEntry>()));
				found = capabilitiesObjectLists.find(icapability);
			}
			inserted->second.capabilities[i].cap_guid=icapability;
			inserted->second.capabilities[i].pEntry = &(*found);
		}

		if(guidInput1 != guidNull)
		{
			assert(objectDependancyMap.find(guidInput1) == objectDependancyMap.end());
			objectDependancyMap.insert(object_dependancy_map_type::value_type(guidInput1,std::pair<guid,u32>(classType,1)));

			if(guidInput2 != guidNull)
			{
				assert(objectDependancyMap.find(guidInput2) == objectDependancyMap.end());
				objectDependancyMap.insert(object_dependancy_map_type::value_type(guidInput2,std::pair<guid,u32>(classType,2)));
			}
		}
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
				found->second[removed].pRef->vCapabilitiesIndices[found->second[removed].index]=(u32)removed;
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

			auto found = objectDependancyMap.find(pDetailed->GetObjectTypeId());
			
			std::pair<const MemHandle&,const guid&> pair(pDetailed->GetInitHandle(),pDetailed->GetSource());

			if(found != objectDependancyMap.end())
			{
				assert(found->second.second < 3);
				TryCreateObjectWithGuid(pDetailed->GetObjectId(),found->second.first,(void*)&pair,false);
				return true;
			}
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
			{
				DestroyObjectInternal(it);
				return S_OK;
			}
			else
				return E_FAIL;
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
