#include "header.h"
#include "UserInterfaceRendering.h"
/*
	MESSAGE_UI_UNKNOWN			= 0x00110000,
	MESSAGE_UI_CURSOR_UPDATE	= 0x00110001,
	MESSAGE_UI_ADJUST_CAMERA	= 0x00110002,
	MESSAGE_UI_MOVE_PLAYER		= 0x00110003,
	*/
namespace HAGE {
	const char* Effect2D_VP =
	"struct Effect2DConstants\n"
	"{\n"
	"    float4x4 position_matrix;\n"
	"    float4x4 texture_matrix;\n"
	"} cbuffer0_Effect2DConstants: BUFFER[0];\n"
	"\n"
	"// cbuffer Effect2DConstants : register(b0)\n"
	"#define PositionMatrix       cbuffer0_Effect2DConstants.position_matrix\n"
	"#define TextureMatrix        cbuffer0_Effect2DConstants.texture_matrix\n"
	"\n"
	"struct Effect2DV_Output {\n"
	"  float4 color    : COLOR;\n"
	"  float2 texture  : TEXCOORD0;\n"
	"  float4 position : POSITION;\n"
	"};\n"
	"\n"
	"Effect2DV_Output vertex(float2 position : POSITION,float3 color : COLOR, float2 texcoord : TEXCOORD0)\n"
	"{	\n"
	"  Effect2DV_Output OUT;\n"
	"\n"
	"  OUT.position = mul( PositionMatrix,	float4( position, 0.0f, 1.0f ) );\n "
	"  OUT.texture = mul( TextureMatrix,	float4( texcoord, 0.0f, 1.0f ) ).xy;\n"
	"  OUT.color = float4(color,1.0f);\n"
	"\n"
	"  return OUT;	\n"
	"}";

	const char* Effect2D_FP =
	"\n"
	"struct Effect2DF_Output {\n"
	"  float4 color : COLOR;\n"
	"};\n"
	"\n"
	"Effect2DF_Output fragment(float4 color : COLOR, float4 position : POSITION, float2 texture : TEXCOORD0)\n"
	"{\n"
	"  Effect2DF_Output OUT;\n"
	"  OUT.color = color;\n"
	"  return OUT;\n"
	"}\n";

	struct Effect2DConstants
	{
		Matrix4<>	position_matrix;
		Matrix4<>	texture_matrix;
	};

	extern const char sz2DFormat[] = "Vertex2DFormat";
	struct Vertex2DFormat
	{
	public:
		Vector2<>	position;
		Vector3<>	color;
		Vector2<>	texture;

		static const char* name;
	};
	const char* Vertex2DFormat::name = sz2DFormat;
	VertexDescriptionEntry Vertex2DFormatDescriptor[] = {
		{"Position",	1,	R32G32_FLOAT	},
		{"Color",		1,	R32G32B32_FLOAT	},
		{"Texcoord",	1,	R32G32_FLOAT	}
	};

	UserInterfaceRendering::UserInterfaceRendering(RenderingAPIWrapper* pWrapper) : m_pWrapper(pWrapper)
	{
		Vertex2DFormat vertices[] =
		{
			{Vector2<>(-1.0f, 1.0f ),	Vector3<>(1.0f, 0.0f, 0.0f), Vector2<>(0.0f, 1.0f)},
			{Vector2<>(1.0f,  1.0f ),	Vector3<>(0.0f, 1.0f, 0.0f), Vector2<>(1.0f, 1.0f)},
			{Vector2<>(-1.0f, -1.0f),	Vector3<>(0.0f, 0.0f, 1.0f), Vector2<>(0.0f, 0.0f)},
			{Vector2<>(1.0f,  -1.0f),	Vector3<>(0.1f, 0.1f, 0.1f), Vector2<>(1.0f, 0.0f)},
		};


		u32	indices[] =
		{
			0,1,2,
			1,3,2
		};

		pWrapper->RegisterVertexFormat(Vertex2DFormat::name,Vertex2DFormatDescriptor,3);
		m_pVBSquare = pWrapper->CreateVertexBuffer(Vertex2DFormat::name,vertices,4);
		m_pVASquare = pWrapper->CreateVertexArray(2,PRIMITIVE_TRIANGLELIST,&m_pVBSquare,1,indices);
		m_pEffect2D = pWrapper->CreateEffect(Effect2D_VP,Effect2D_FP);
		m_pConstants = pWrapper->CreateConstantBuffer(sizeof(Effect2DConstants));
	}
	UserInterfaceRendering::~UserInterfaceRendering()
	{
		delete m_pConstants;
		delete m_pEffect2D;
		delete m_pVASquare;
		delete m_pVBSquare;
	}
	bool UserInterfaceRendering::ProcessUserInterfaceMessage(const Message* pMessage)
	{
		switch(pMessage->GetMessageCode())
		{
		case MESSAGE_UI_CURSOR_UPDATE:
			{
				const MessageUICursorUpdate* m = (const MessageUICursorUpdate*)pMessage;
				m_vMousePosition.x=m->GetCursorX();
				m_vMousePosition.y=m->GetCursorY();
				return true;
			}
			break;
		}
		return true;
	}

	void UserInterfaceRendering::Draw()
	{
		Effect2DConstants constants;
		constants.texture_matrix = Matrix4<>::One();
		constants.position_matrix = Matrix4<>::Translate(Vector3<>(m_vMousePosition,0.0f))*Matrix4<>::Scale(Vector3<>(0.1f,0.1f,0.0f));

		m_pConstants->UpdateContent(&constants);
		m_pConstants->Set(0);
		m_pEffect2D->Draw(m_pVASquare);
	}
}