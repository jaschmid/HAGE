/********************************************************/
/* FILE: PinBase.h                                      */
/* DESCRIPTION: Defines PinBase and MessageQueue classes*/
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/ 

#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef PINBASE_H_INCLUDED
#define PINBASE_H_INCLUDED

#include "HAGE.h"
#include "global_allocator.h"
#include <boost/function.hpp>
#include <vector>
#include <array>

namespace HAGE {

const u32 FRAME_BUFFER_COUNT = 3;
#define FRAMESKIP_ENABLED

class WrapMessage : public MessageHelper<WrapMessage>
{
public:
	WrapMessage() : MessageHelper<WrapMessage>(MESSAGE_RESERVED_WRAP_QUEUE) {}

	static const WrapMessage _wrap;
};

template<u32 size> class StaticMessageQueue
{
public:
	StaticMessageQueue() : nReadIndex(0),nWriteIndex(0)
	{
	}
	virtual ~StaticMessageQueue() {}

	// called by producer
	result			PostMessage(const Message& m)
	{
		u32 nSize = m.GetSize();

		if( nWriteIndex < size - nSize)
		{
			assert(nReadIndex > nWriteIndex + nSize || nReadIndex <= nWriteIndex);
			// no wrap
			m.CopyTo(&Mem[nWriteIndex]);
			nWriteIndex += nSize;
			return S_OK;
		}
		else if( nWriteIndex >= size - sizeof(Message))
		{
			assert(nReadIndex <= nWriteIndex);
			// auto wrap
			nWriteIndex = 0;
			return PostMessage(m);
		}
		else
		{
			// manual wrap
			assert(nReadIndex <= nWriteIndex);
			assert(succeeded(PostMessage(WrapMessage::_wrap)));
			nWriteIndex = 0;
			return PostMessage(m);
		}
	}

	// called by consumer
	void			PopMessage()
	{
		if(nReadIndex == nWriteIndex)
			return;

		nReadIndex += GetTopMessage()->GetSize();

		if(nReadIndex + sizeof(Message) >= size)
			nReadIndex = 0;
		else if(nReadIndex != nWriteIndex && GetTopMessage()->GetMessageCode() == MESSAGE_RESERVED_WRAP_QUEUE)
			nReadIndex = 0;

		return;
	}
	const Message*	GetTopMessage() const
	{
		if(nReadIndex == nWriteIndex)
			return nullptr;

		return (const Message*)(&Mem[nReadIndex]);
	}

private:

	u32		nReadIndex;
	u32		nWriteIndex;

	typedef std::array<u8,size> mm_vector;

	mm_vector		Mem;

};

class LockedMessageQueue
{
public:
	LockedMessageQueue();
	virtual ~LockedMessageQueue();

	const Message*	GetNextMessage(const Message* prev) const;

	template<class _C> result PostMessage(const _C& m)
	{
		if(std::has_trivial_destructor<_C>::value)
			return _PostMessage(m);
		else
			return _PostPackage((const Package&)m);
	}

	template<class _C> result ForwardMessage(const _C& m)
	{
		if(std::has_trivial_destructor<_C>::value)
			return _ForwardMessage(m);
		else
			return _ForwardPackage((const Package&)m);
	}

	result CloseReadPin();
	result CloseWritePin();
	result InitializeInputPin(boost::function<void()> f);
	result InitializeOutputPin(boost::function<void()> f,const guid& sourceId);

	void	Shutdown();

protected:

	i32		nReadIndex;
	i32		nWriteIndex;
	volatile i32 nAvailableSlots;

private:

	u32  GetRead(){return nReadIndex;}
	u32  GetWrite(){return nWriteIndex;}

	result			_PostMessage(const Message& m);
	result			_PostPackage(const Package& m);
	result			_ForwardMessage(const Message& m);
	result			_ForwardPackage(const Package& m);

	typedef std::vector<boost::function<void()>,global_allocator<boost::function<void()>>> callback_list;
	typedef std::vector<u8,global_allocator<u8>> mm_vector;
	typedef std::vector<u32,global_allocator<u32>> pd_vector;

	// proper private
	volatile i32	nClosedAccesses;
	volatile bool	bInit;
	volatile i32	nShutdown;
	volatile i32	nFreeSlots;
	callback_list fReadReadyCallback;
	boost::function<void()> fWriteReadyCallback;
	guid			sourceStamp;
	mm_vector*		pMem;
	pd_vector*		m_pvpDestructors;
	void*			pVoid;

	template<class _T> friend class OutputPin;
	template<class _T,class _T2> friend class InputPin;
	friend class CoreFactory;

};

class PinBase : public LockedMessageQueue
{
public:
	PinBase();
	virtual ~PinBase();

	void* GetWriteMem(MemHandle itemHandle,u32 size) const
	{
		MemoryEntry* entry = (MemoryEntry*)itemHandle._p;

		assert(entry->size == (i32)size);

		return ((u8*)entry)+sizeof(MemoryEntry)+size*nWriteIndex;
	}
	const void* GetReadMem(MemHandle itemHandle,u32 size) const
	{
		MemoryEntry* entry = (MemoryEntry*)itemHandle._p;

		assert(entry->size == (i32)size);

		return ((u8*)entry)+sizeof(MemoryEntry)+size*nReadIndex;
	}
	MemHandle AllocateMemBlock(u32 size);
	static void ReferenceMemBlock(MemHandle Handle);
	static void FreeMemBlock(MemHandle Handle);

private:

	u32 GetAll(MemHandle h,const void* (&values)[FRAME_BUFFER_COUNT]);

	class MemoryEntry
	{
	public:
		i32 size;
		i32 references;
	};

	friend class CoreFactory;
};

}

#endif
