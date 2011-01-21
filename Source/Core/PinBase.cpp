#include <HAGE.h>

namespace HAGE {
	
	const WrapMessage WrapMessage::_wrap = WrapMessage();

	LockedMessageQueue::LockedMessageQueue() :
		nClosedAccesses(0),bInit(false),nShutdown(0),
		nWriteIndex(0),nReadIndex(0),
		pMem((mm_vector*)DomainMemory::GlobalAllocate(sizeof(mm_vector)*FRAME_BUFFER_COUNT)),
		m_pvpDestructors((pd_vector*)DomainMemory::GlobalAllocate(sizeof(pd_vector)*FRAME_BUFFER_COUNT)),
		bReadWaiting(true),bWriteWaiting(false),nAvailableSlots(0)

	{
		for(int i=0;i<FRAME_BUFFER_COUNT;++i)
		{
			new (&pMem[i]) mm_vector;
			new (&m_pvpDestructors[i]) pd_vector;
		}
	}
		
	u32 PinBase::GetAll(MemHandle h,const void* (&values)[FRAME_BUFFER_COUNT])
	{
		MemoryEntry* entry = (MemoryEntry*)h._p;

		for(int i =0;i<FRAME_BUFFER_COUNT;++i)
			values[i]=((u8*)entry)+sizeof(MemoryEntry)+entry->size*i;

		return entry->size;
	}

	LockedMessageQueue::~LockedMessageQueue()
	{
		for(int i=0;i<FRAME_BUFFER_COUNT;++i)
		{
			for(auto j=m_pvpDestructors[i].begin();j!=m_pvpDestructors[i].end();++j)
				((Package*)&pMem[i][*j])->~Package();
			m_pvpDestructors[i].~vector();
			pMem[i].~vector();
		}
		DomainMemory::GlobalFree(pMem);
	}

	result LockedMessageQueue::CloseReadPin()
	{
		//printf("Pin %08x closed\n",this);
		i32 count = _InterlockedIncrement(&nClosedAccesses);
		if(count == fReadReadyCallback.size())
		{
			//printf("Pin %08x ready\n",this);
			nClosedAccesses=nShutdown;
			for(auto i = m_pvpDestructors[nReadIndex].begin();i!=m_pvpDestructors[nReadIndex].end();++i)
				((Package*)&pMem[nReadIndex][*i])->~Package();

			m_pvpDestructors[nReadIndex].clear();
			pMem[nReadIndex].clear();

			nReadIndex = (nReadIndex+1) % FRAME_BUFFER_COUNT;

			if(_InterlockedDecrement(&nAvailableSlots)==0)
				bReadWaiting = true;
			else
			{
				if(bWriteWaiting)
				{
#ifdef FRAMESKIP_ENABLED
					if(FRAME_BUFFER_COUNT >= 3 && nAvailableSlots==FRAME_BUFFER_COUNT-1)
					{
						//skip frames
						i32 nNewReadIndex = (nWriteIndex+FRAME_BUFFER_COUNT-1) % FRAME_BUFFER_COUNT;
						i32 oldReadIndex = nReadIndex;
						//assert(nNewReadIndex != nWriteIndex);
						//get total size of older messages
						u32 newSize = 0;
						u32 newDestructors = 0;
						while(nReadIndex != nNewReadIndex)
						{
							newSize+=pMem[nReadIndex].size();
							newDestructors += m_pvpDestructors[nReadIndex].size();
							nReadIndex = (nReadIndex+1) % FRAME_BUFFER_COUNT;
						}
						
						nReadIndex=oldReadIndex;

						//backup newest messages
						u32 oldSize = pMem[nNewReadIndex].size();
						u32 oldDestructors = m_pvpDestructors[nNewReadIndex].size();
						pMem[nNewReadIndex].resize( oldSize + newSize);
						m_pvpDestructors[nNewReadIndex].resize( oldDestructors + newDestructors);
						if(oldSize)
						{
							memmove( &pMem[nNewReadIndex][newSize],&pMem[nNewReadIndex][0],oldSize);
							for(int i =oldDestructors-1;i>=0;--i)
								m_pvpDestructors[nNewReadIndex][newDestructors +i]=m_pvpDestructors[nNewReadIndex][i]+newSize;
						}


						//copy older messages
						u32 nCopied=0;
						u32 nCopiedDestructors = 0;
						while(nReadIndex != nNewReadIndex)
						{
							if(pMem[nReadIndex].size())
							{
								memcpy(&pMem[nNewReadIndex][nCopied],&pMem[nReadIndex][0],pMem[nReadIndex].size());
								for(int i =0;i<m_pvpDestructors[nReadIndex].size();++i)
								{
									m_pvpDestructors[nNewReadIndex][nCopiedDestructors +i]=m_pvpDestructors[nReadIndex][i]+nCopied;
								}
								
								nCopiedDestructors+=m_pvpDestructors[nReadIndex].size();
								nCopied+=pMem[nReadIndex].size();

								m_pvpDestructors[nReadIndex].clear();
								pMem[nReadIndex].clear();
							}
							nReadIndex = (nReadIndex+1) % FRAME_BUFFER_COUNT;
						}
						assert(nCopiedDestructors == newDestructors);
						assert(nCopied == newSize);
						nAvailableSlots=1;
					}
#endif
					bWriteWaiting = false;
					fWriteReadyCallback();
				}
				for(u32 i=0;i<fReadReadyCallback.size();++i)
					fReadReadyCallback[i]();
			}
		}
		return S_OK;
	}
	result LockedMessageQueue::CloseWritePin()
	{
		//printf("Pin %08x closed\n",this);
			//printf("Pin %08x ready\n",this);
		bInit=true;
		nWriteIndex = (nWriteIndex+1) % FRAME_BUFFER_COUNT;

		if(_InterlockedIncrement(&nAvailableSlots)==FRAME_BUFFER_COUNT && fReadReadyCallback.size() >= 1)
			bWriteWaiting = true;
		else
		{			
			if(bReadWaiting)
			{
				bReadWaiting = false;
				for(u32 i=0;i<fReadReadyCallback.size();++i)
						fReadReadyCallback[i]();
			}
			fWriteReadyCallback();
		}
		return S_OK;
	}
	result LockedMessageQueue::InitializeOutputPin(boost::function<void()> f,const guid& guid)
	{
		fWriteReadyCallback = f;
		sourceStamp = guid;
		fWriteReadyCallback();

		return S_OK;
	}

	result LockedMessageQueue::InitializeInputPin(boost::function<void()> f)
	{
		fReadReadyCallback.push_back( f );
		//printf("Pin %08x has %i Domains that use it as input now\n",this,fReadReadyCallback.size());
		return S_OK;
	}


	const Message* LockedMessageQueue::GetNextMessage(const Message* prev) const
	{
		if(prev == (void*)nullptr)
		{
			if( pMem[nReadIndex].size() == 0)
				return nullptr;
			else
				return (const Message*) &pMem[nReadIndex][0];
		}
		else if(prev->GetSize() + (u64)prev >= (u64)&pMem[nReadIndex][0] + pMem[nReadIndex].size())
		{
			return nullptr;
		}
		else
		{
			return (const Message*)((u64)prev + (prev->GetSize()));
		}
	}

	result LockedMessageQueue::_PostMessage(const Message& m)
	{
		if(m.GetMessageCode()&MESSAGE_IS_PACKAGE)
			return _PostPackage((const Package&)m);
		pMem[nWriteIndex].resize( pMem[nWriteIndex].size() + m.GetSize());
		Message* target = (Message*)(&pMem[nWriteIndex][0] + pMem[nWriteIndex].size() - m.GetSize());
		m.CopyTo(target);
		target->SetSource(sourceStamp);
		return S_OK;
	}

	result LockedMessageQueue::_ForwardMessage(const Message& m)
	{
		if(m.GetMessageCode()&MESSAGE_IS_PACKAGE)
			return _ForwardPackage((const Package&)m);
		assert( m.GetSource() != guidNull);
		pMem[nWriteIndex].resize( pMem[nWriteIndex].size() + m.GetSize());
		Message* target = (Message*)(&pMem[nWriteIndex][0] + pMem[nWriteIndex].size() - m.GetSize());
		m.CopyTo(target);
		return S_OK;
	}
	result LockedMessageQueue::_PostPackage(const Package& m)
	{
		assert(m.GetMessageCode()&MESSAGE_IS_PACKAGE);
		pMem[nWriteIndex].resize( pMem[nWriteIndex].size() + m.GetSize());
		u32 index =  (u32)(pMem[nWriteIndex].size() - m.GetSize());
		Package* target = (Package*)(&pMem[nWriteIndex][index]);
		m_pvpDestructors[nWriteIndex].push_back(index);
		m.CopyTo(target);
		target->SetSource(sourceStamp);
		return S_OK;
	}

	result LockedMessageQueue::_ForwardPackage(const Package& m)
	{
		assert( !(bInit && nWriteIndex == nReadIndex));
		assert(m.GetMessageCode()&MESSAGE_IS_PACKAGE);
		assert( m.GetSource() != guidNull);
		pMem[nWriteIndex].resize( pMem[nWriteIndex].size() + m.GetSize());
		u32 index =  (u32)(pMem[nWriteIndex].size() - m.GetSize());
		Package* target = (Package*)(&pMem[nWriteIndex][index]);
		m_pvpDestructors[nWriteIndex].push_back(index);
		m.CopyTo(target);
		return S_OK;
	}
	void LockedMessageQueue::Shutdown()
	{
		_InterlockedIncrement((i32*)&nShutdown);
	}


	PinBase::PinBase()
	{
	}

	PinBase::~PinBase()
	{
	}

	MemHandle PinBase::AllocateMemBlock(u32 size)
	{
		MemoryEntry* pEntry = (MemoryEntry*)DomainMemory::GlobalAllocate(sizeof(MemoryEntry)+FRAME_BUFFER_COUNT*size);
		pEntry->size = size;
		pEntry->references = 1;
		return MemHandle(pEntry);
	}

	void PinBase::ReferenceMemBlock(MemHandle handle)
	{
		MemoryEntry* pEntry = (MemoryEntry*)handle._p;
		i32 val =_InterlockedIncrement(&pEntry->references);
		assert(val > 1);
	}

	void PinBase::FreeMemBlock(MemHandle handle)
	{
		MemoryEntry* pEntry = (MemoryEntry*)handle._p;
		i32 val =_InterlockedDecrement(&pEntry->references);
		if(val == 0)
			DomainMemory::GlobalFree(pEntry);
	}
}
