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
	KEY_CODE_ERROR		= 0x00000000,
	KEY_CODE_ESC		= 0x00000001,
	KEY_CODE_ONE		= 0x00000002,
	KEY_CODE_TWO		= 0x00000003,
	KEY_CODE_THREE		= 0x00000004,
	KEY_CODE_FOUR		= 0x00000005,
	KEY_CODE_FIVE		= 0x00000006,
	KEY_CODE_SIX		= 0x00000007,
	KEY_CODE_SEVEN		= 0x00000008,
	KEY_CODE_EIGHT		= 0x00000009,
	KEY_CODE_NINE		= 0x0000000a,
	KEY_CODE_ZERO		= 0x0000000b,
	KEY_CODE_MINUS		= 0x0000000c,
	KEY_CODE_HYPHEN		= 0x0000000d,
	KEY_CODE_BACKSPACE	= 0x0000000e,
	KEY_CODE_TAB		= 0x0000000f,
	KEY_CODE_Q			= 0x00000010,
	KEY_CODE_W			= 0x00000011,
	KEY_CODE_E			= 0x00000012,
	KEY_CODE_R			= 0x00000013,
	KEY_CODE_T			= 0x00000014,
	KEY_CODE_Y			= 0x00000015,
	KEY_CODE_U			= 0x00000016,
	KEY_CODE_I			= 0x00000017,
	KEY_CODE_O			= 0x00000018,
	KEY_CODE_P			= 0x00000019,
	KEY_CODE_OPEN_BRACKET= 0x0000001a,
	KEY_CODE_CLOSE_BRACKET= 0x0000001b,
	KEY_CODE_ENTER		= 0x0000001c,
	KEY_CODE_LEFT_CTRL	= 0x0000001d,
	KEY_CODE_A			= 0x0000001e,
	KEY_CODE_S			= 0x0000001f,
	KEY_CODE_D			= 0x00000020,
	KEY_CODE_F			= 0x00000021,
	KEY_CODE_G			= 0x00000022,
	KEY_CODE_H			= 0x00000023,
	KEY_CODE_J			= 0x00000024,
	KEY_CODE_K			= 0x00000025,
	KEY_CODE_L			= 0x00000026,
	KEY_CODE_SEMICOLON	= 0x00000027,
	KEY_CODE_APOSTROPHE	= 0x00000028,
	KEY_CODE_ACCENT_GRAVE= 0x00000029,
	KEY_CODE_LEFT_SHIFT	= 0x0000002a,
	KEY_CODE_BACK_SLASH	= 0x0000002b,
	KEY_CODE_Z			= 0x0000002c,
	KEY_CODE_X			= 0x0000002d,
	KEY_CODE_C			= 0x0000002e,
	KEY_CODE_V			= 0x0000002f,
	KEY_CODE_B			= 0x00000030,
	KEY_CODE_N			= 0x00000031,
	KEY_CODE_M			= 0x00000032,
	KEY_CODE_COMMA		= 0x00000033,
	KEY_CODE_FULL_STOP	= 0x00000034,
	KEY_CODE_SLASH		= 0x00000035,
	KEY_CODE_RIGHT_SHIFT= 0x00000036,
	KEY_CODE_KEYPAD_ASTERISK= 0x00000037,
	KEY_CODE_LEFT_ALT	= 0x00000038,
	KEY_CODE_SPACE		= 0x00000039,
	KEY_CODE_CAPS_LOCK	= 0x0000003a,
	KEY_CODE_F1			= 0x0000003b,
	KEY_CODE_F2			= 0x0000003c,
	KEY_CODE_F3			= 0x0000003d,
	KEY_CODE_F4			= 0x0000003e,
	KEY_CODE_F5			= 0x0000003f,
	KEY_CODE_F6			= 0x00000040,
	KEY_CODE_F7			= 0x00000041,
	KEY_CODE_F8			= 0x00000042,
	KEY_CODE_F9			= 0x00000043,
	KEY_CODE_F10		= 0x00000044,
	KEY_CODE_NUM_LOCK	= 0x00000045,
	KEY_CODE_SCROLL_LOCK= 0x00000046,
	KEY_CODE_KEYPAD_7	= 0x00000047,
	KEY_CODE_KEYPAD_8	= 0x00000048,
	KEY_CODE_KEYPAD_9	= 0x00000049,
	KEY_CODE_KEYPAD_MINUS= 0x0000004a,
	KEY_CODE_KEYPAD_4	= 0x0000004b,
	KEY_CODE_KEYPAD_5	= 0x0000004c,
	KEY_CODE_KEYPAD_6	= 0x0000004d,
	KEY_CODE_KEYPAD_PLUS= 0x0000004e,
	KEY_CODE_KEYPAD_1	= 0x0000004f,
	KEY_CODE_KEYPAD_2	= 0x00000050,
	KEY_CODE_KEYPAD_3	= 0x00000051,
	KEY_CODE_KEYPAD_0	= 0x00000052,
	KEY_CODE_KEYPAD_FULL_STOP= 0x00000053,
	KEY_CODE_ALT_SYS_RQ	= 0x00000054,
	KEY_CODE_KEYPAD_ENTER=0x0000011c,//0x68
	KEY_CODE_RIGHT_CTRL	= 0x0000011d,//0x69
	//KEY_CODE_FAKE_LEFT_SHIFT=0x0000012a,//
	KEY_CODE_KEYPAD_SLASH=0x00000135,//0x61
	//KEY_CODE_FAKE_RIGHT_SHIFT=0x0000010036,
	KEY_CODE_PRINT_SCREEN=0x00000137,//not on linux!
	KEY_CODE_RIGHT_ALT	= 0x00000138,//0x6c
	//KEY_CODE_NUMPAD_SCROLL_LOCK= 0x0000010046,
	KEY_CODE_GREY_HOME	= 0x00000147,//0x6e
	KEY_CODE_GREY_UP	= 0x00000148,//0x6f
	KEY_CODE_GREY_PG_UP	= 0x00000149,//0x70
	KEY_CODE_GREY_LEFT	= 0x0000014b,//0x71
	KEY_CODE_GREY_RIGHT	= 0x0000014d,//0x72
	KEY_CODE_GREY_END	= 0x0000014f,//0x73
	KEY_CODE_GREY_DOWN	= 0x00000150,//0x74
	KEY_CODE_GREY_PG_DOWN=0x00000151,//0x75
	KEY_CODE_GREY_INSERT= 0x00000152,//0x76
	KEY_CODE_GREY_DELETE= 0x00000153,//0x77
	KEY_CODE_LEFT_WINDOW= 0x0000015b,//0x85
	KEY_CODE_RIGHT_WINDOW=0x0000015c,//0x86
	KEY_CODE_MENU		= 0x0000015d,//0x87
	KEY_CODE_MAX		= 0x000003ff,
	KEY_CODE_NONE		= 0xffffffff
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
	MessageInputKey(const u32 id,const guid& device,const HAGE::u32 key,const HAGE::u16 _char) :
	  MessageHelper<MessageInputKey>(id),m_Device(device),m_Key(key),m_Char(_char) {}
	const guid& GetDevice() const{return m_Device;}
	const u32 GetKey() const{return m_Key;}
	const u16 GetCharacter() const{return m_Char;}
private:
	const guid m_Device;
	const HAGE::u32 m_Key;
	const HAGE::u16 m_Char;
};

class MessageInputKeydown : public MessageInputKey
{
public:
	MessageInputKeydown(const guid& device,const HAGE::u32 key,const HAGE::u16 _char) :
	  MessageInputKey(id,device,key,_char) {}
private:
	static const u32 id = MESSAGE_INPUT_KEYDOWN;
};

class MessageInputKeyup : public MessageInputKey
{
public:
	MessageInputKeyup(const guid& device,const HAGE::u32 key,const HAGE::u16 _char) :
	  MessageInputKey(id,device,key,_char) {}
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
	MESSAGE_UI_SHOW				= 0x00110004,
	MESSAGE_UI_HIDE				= 0x00110005
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

class MessageUIShow : public MessageHelper<MessageUIShow,MessageUIUnknown>
{
public:
	MessageUIShow() : MessageHelper<MessageUIShow,MessageUIUnknown>(id) {}
private:
	static const u32 id = MESSAGE_UI_SHOW;
};

class MessageUIHide : public MessageHelper<MessageUIHide,MessageUIUnknown>
{
public:
	MessageUIHide() : MessageHelper<MessageUIHide,MessageUIUnknown>(id) {}
private:
	static const u32 id = MESSAGE_UI_HIDE;
};

}

#endif
