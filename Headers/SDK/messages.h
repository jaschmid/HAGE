#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef __MESSAGES_H__
#define __MESSAGES_H__

#include "types.h"

namespace HAGE {


class Message
{
public:
	Message(u32 MessageCode,u32 Size) : code(MessageCode),size(Size),source(guidNull){}
	// due to performance reasons we won't call deconstructors anyway, bear this in mind when inheriting
	// from the message class

	u32 GetMessageCode() const
	{
		return code;
	}

	u32 GetSize() const
	{
		return size;
	}

	const guid& GetSource() const
	{
		return source;
	}

	void SetSource(const guid& Source)
	{
		source=Source;
	}

	virtual Message* CopyTo(void* pTarget) const = 0;

private:

	const u32	code;
	const u32	size;
	guid		source;
};


template<class _C,class _C2 = Message> class MessageHelper : public _C2
{
public:
	MessageHelper(const u32 code) : _C2(code,sizeof(_C)) { }

	virtual Message* CopyTo(void* pTarget) const
	{
		return new (pTarget) _C(*(_C*)this);
	}
};

template<class _C> class SimpleMessage : public Message
{
public:
	SimpleMessage(const u32 code,const _C& c) : Message(code,sizeof(SimpleMessage<_C>)),data(c) {}

	virtual SimpleMessage<_C>* CopyTo(void* pTarget) const
	{
		return new (pTarget) SimpleMessage<_C>(*this);
	}

	const _C& GetData() const{return data;}
private:
	const _C data;
};

const u32 MESSAGE_ITEM_CREATED = 0x1000;

const u32 MESSAGE_UPPER_MASK	= 0xffff0000;
const u32 MESSAGE_LOWER_MASK	= 0x0000ffff;

inline bool IsMessageType(u32 code,u32 type)
{
	return ((code & MESSAGE_UPPER_MASK) == type);
}

//message codes
enum {
	MESSAGE_RESERVED_WRAP_QUEUE			= 0xffff0001
};

enum {
	// INPUT MESSAGES 000e 0000
	MESSAGE_INPUT_UNKNOWN			= 0x000e0000,
	MESSAGE_INPUT_KEYDOWN			= 0x000e0001,
	MESSAGE_INPUT_KEYUP				= 0x000e0002,
	MESSAGE_INPUT_AXIS_RELATIVE		= 0x000e0003,
	MESSAGE_INPUT_AXIS_ABSOLUTE		= 0x000e0004,
	MESSAGE_INPUT_RESET				= 0x000e0005
};

DECLARE_GUID(DefMouse,			0x5d0ccc7e,0xcc5e,0x44ad,0x9063,0xd9586b266efd);
DECLARE_GUID(DefKeyboard,		0xc1093448,0xf540,0x4f56,0x974c,0xd4a34f6bd23c);

class MessageInputUnknown : public Message
{
public:
	MessageInputUnknown(u32 MessageCode,u32 Size) : Message(MessageCode,Size) {}
private:
	static const u32 id = MESSAGE_INPUT_UNKNOWN;
};

enum
{
	KEY_CODE_NONE = 0xffffffff
};

enum _MouseKey
{
	MOUSE_BUTTON_1		= 0x0001,
	MOUSE_BUTTON_2		= 0x0002,
	MOUSE_BUTTON_3		= 0x0003,
	MOUSE_BUTTON_4		= 0x0004,
	MOUSE_BUTTON_5		= 0x0005,
	MOUSE_BUTTON_NONE	= KEY_CODE_NONE
};

class MessageInputKey : public MessageHelper<MessageInputKey>
{
public:
	MessageInputKey(const u32 id,const guid& device,const HAGE::u32 key) :
	  MessageHelper<MessageInputKey>(id),m_Device(device),m_Key(key) {}
	const guid& GetDevice() const{return m_Device;}
	const u32 GetKey() const{return m_Key;}
private:
	const guid m_Device;
	const HAGE::u32 m_Key;
};

class MessageInputKeydown : public MessageInputKey
{
public:
	MessageInputKeydown(const guid& device,const HAGE::u32 key) :
	  MessageInputKey(id,device,key) {}
private:
	static const u32 id = MESSAGE_INPUT_KEYDOWN;
};

class MessageInputKeyup : public MessageInputKey
{
public:
	MessageInputKeyup(const guid& device,const HAGE::u32 key) :
	  MessageInputKey(id,device,key) {}
private:
	static const u32 id = MESSAGE_INPUT_KEYUP;
};

enum
{
	AXIS_CODE_NONE = 0xffffffff
};

enum _MouseAxis
{
	MOUSE_AXIS_X	= 0x0001,
	MOUSE_AXIS_Y	= 0x0002,
	MOUSE_AXIS_Z	= 0x0003,
	MOUSE_AXIS_NONE	= AXIS_CODE_NONE
};

template<class _C> class MessageInputAxis : public MessageHelper<_C,MessageInputUnknown>
{
public:
	MessageInputAxis(const u32 id,const guid& device,const u32 axis) :
	  MessageHelper<_C,MessageInputUnknown>(id),m_Device(device),m_Axis(axis) {}
	const guid& GetDevice() const{return m_Device;}
	const u32 GetAxis() const{return m_Axis;}
private:
	const guid m_Device;
	const u32 m_Axis;
};

class MessageInputAxisRelative : public MessageInputAxis<MessageInputAxisRelative>
{
public:
	MessageInputAxisRelative(const guid& device,const u32 axis,const f32 change) :
	  MessageInputAxis<MessageInputAxisRelative>(id,device,axis),m_Change(change) {}
	const f32 GetChange() const{return m_Change;}
private:
	const f32 m_Change;
	static const u32 id = MESSAGE_INPUT_AXIS_RELATIVE;
};

class MessageInputAxisAbsolute : public MessageInputAxis<MessageInputAxisAbsolute>
{
public:
	MessageInputAxisAbsolute(const guid& device,const u32 axis,const f32 value) :
	  MessageInputAxis<MessageInputAxisAbsolute>(id,device,axis),m_Value(value) {}
	const f32 GetValue() const{return m_Value;}
private:
	const f32 m_Value;
	static const u32 id = MESSAGE_INPUT_AXIS_ABSOLUTE;
};

class MessageInputReset : public MessageHelper<MessageInputReset,MessageInputUnknown>
{
public:
	MessageInputReset():
	  MessageHelper<MessageInputReset,MessageInputUnknown>(id) {}
private:
	static const u32 id = MESSAGE_INPUT_RESET;
};

enum {
	// FACTORY MESSAGES 000f 0000
	MESSAGE_FACTORY_UNKNOWN				= 0x000f0000,
	MESSAGE_FACTORY_OBJECT_CREATED		= 0x000f0001,
	MESSAGE_FACTORY_OBJECT_DESTROYED	= 0x000f0002
};

class MessageFactoryUnknown : public Message
{
private:
	static const u32 id = MESSAGE_FACTORY_UNKNOWN;
};

class MessageFactoryObjectCreated : public MessageHelper<MessageFactoryObjectCreated>
{
public:
	MessageFactoryObjectCreated(const guid& ObjectId,const guid& ObjectTypeId) :
		MessageHelper<MessageFactoryObjectCreated>(id),
		objectId(ObjectId),
		objectTypeId(ObjectTypeId) {}
	const guid& GetObjectTypeId() const{return objectTypeId;}
	const guid& GetObjectId() const{return objectId;}
private:
	const guid	objectId;
	const guid& objectTypeId;
	static const u32 id = MESSAGE_FACTORY_OBJECT_CREATED;
};


class MessageFactoryObjectDestroyed : public SimpleMessage<guid>
{
public:
	MessageFactoryObjectDestroyed(const guid& ObjectId) : SimpleMessage<guid>(id,ObjectId) {}
private:
	static const u32 id = MESSAGE_FACTORY_OBJECT_DESTROYED;
};

enum {
	// OBJECT MESSAGE 0010 0000
	MESSAGE_OBJECT_UNKNOWN		= 0x00100000,
	MESSAGE_OBJECT_OUTPUT_INIT	= 0x00100002
};

class MessageObjectUnknown : public Message
{
public:
	MessageObjectUnknown(const u32 code,const u32 size,const guid& Target) : Message(code,size),target(Target) {}

	const guid GetTarget() const{return target;}
private:
	const guid target;
};

template<class _C> class MessageObjectHelper : public MessageObjectUnknown
{
public:
	MessageObjectHelper(const u32 id,const guid& target) : MessageObjectUnknown(id,sizeof(_C),target) { }
	virtual Message* CopyTo(void* pTarget) const
	{
		return new (pTarget) _C(*(_C*)this);
	}
};

class MessageObjectOutputInit : public MessageObjectHelper<MessageObjectOutputInit>
{
public:
	MessageObjectOutputInit(const guid& target,const MemHandle& Handle) : MessageObjectHelper<MessageObjectOutputInit>(id,target),handle(Handle) {}

	const MemHandle& GetHandle() {return handle;}

private:
	static const u32 id = MESSAGE_OBJECT_OUTPUT_INIT;
	const MemHandle handle;
};

enum {
	// UI MESSAGE 0011 0000
	MESSAGE_UI_UNKNOWN			= 0x00110000,
	MESSAGE_UI_CURSOR_UPDATE	= 0x00110001,
	MESSAGE_UI_ADJUST_CAMERA	= 0x00110002,
	MESSAGE_UI_MOVE_PLAYER		= 0x00110003,
	MESSAGE_UI_INIT				= 0x00110004
};

class MessageUIUnknown : public Message
{
public:
	MessageUIUnknown(u32 MessageCode,u32 Size) : Message(MessageCode,Size) {}
private:
	static const u32 id = MESSAGE_UI_UNKNOWN;
};

class MessageUICursorUpdate : public MessageHelper<MessageUICursorUpdate,MessageUIUnknown>
{
public:
	MessageUICursorUpdate(f32 fCursorX,f32 fCursorY,bool bVisible,bool bLocationWorld) :
	  MessageHelper<MessageUICursorUpdate,MessageUIUnknown>(id),m_fCursorX(fCursorX),m_fCursorY(fCursorY),m_bVisible(bVisible),m_bLocationWorld(bLocationWorld)
	{}
	f32 GetCursorX() const{return m_fCursorX;}
	f32 GetCursorY() const{return m_fCursorY;}
	bool IsVisible() const{return m_bVisible;}
	bool IsWorldLocation() const{return m_bLocationWorld;}
private:
	const f32		m_fCursorX,m_fCursorY;
	const bool	m_bVisible,m_bLocationWorld;
	static const u32 id = MESSAGE_UI_CURSOR_UPDATE;
};

class MessageUIAdjustCamera : public MessageHelper<MessageUIAdjustCamera,MessageUIUnknown>
{
public:
	MessageUIAdjustCamera(f32 fRotateY,f32 fRotateX,f32 fZoom) :
	  MessageHelper<MessageUIAdjustCamera,MessageUIUnknown>(id),m_fRotateY(fRotateY),m_fRotateX(fRotateX),m_fZoom(fZoom)
	{}
	f32 GetRotateY() const{return m_fRotateY;}
	f32 GetRotateX() const{return m_fRotateX;}
	f32 GetZoom() const{return m_fZoom;}
private:
	const f32		m_fRotateY,m_fRotateX,m_fZoom;
	static const u32 id = MESSAGE_UI_ADJUST_CAMERA;
};

class MessageUIMovePlayer : public MessageHelper<MessageUIAdjustCamera,MessageUIUnknown>
{
public:
	MessageUIMovePlayer(f32 fForwardMovement,f32 fStraveMovement,f32 fRotate) :
	  MessageHelper<MessageUIAdjustCamera,MessageUIUnknown>(id),m_fForwardMovement(fForwardMovement),m_fStraveMovement(fStraveMovement),m_fRotate(fRotate)
	{}
	f32 GetForward() const{return m_fForwardMovement;}
	f32 GetStrave() const{return m_fStraveMovement;}
	f32 GetRotate() const{return m_fRotate;}
private:
	const f32		m_fForwardMovement,m_fStraveMovement,m_fRotate;
	static const u32 id = MESSAGE_UI_MOVE_PLAYER;
};

}

#endif
