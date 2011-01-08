#include "header.h"
#include "UserInterfaceRendering.h"
/*
	MESSAGE_UI_UNKNOWN			= 0x00110000,
	MESSAGE_UI_CURSOR_UPDATE	= 0x00110001,
	MESSAGE_UI_ADJUST_CAMERA	= 0x00110002,
	MESSAGE_UI_MOVE_PLAYER		= 0x00110003,
	*/
namespace HAGE {
	const char* Effect2D =
	"uniform struct\n"
	"{\n"
	"	 float4 color;\n"
	"    float4x4 position_matrix;\n"
	"    float4x4 texture_matrix;\n"
	"} cbuffer0_Effect2DConstants: BUFFER[0];\n"
	"\n"
	"// cbuffer Effect2DConstants : register(b0)\n"
	"#define PositionMatrix       cbuffer0_Effect2DConstants.position_matrix\n"
	"#define TextureMatrix        cbuffer0_Effect2DConstants.texture_matrix\n"
	"#define ConstantColor        cbuffer0_Effect2DConstants.color\n"
	"\n"
	"struct Effect2DV_Input {\n"
	"  float2 cposition : POSITION;\n"
	"  float2 ctexcoord : TEXCOORD0;\n"
	"};\n"
	"struct Effect2DV_Output {\n"
	"  float4 vcolor    : TEXCOORD0;\n"
	"  float2 vtexture  : TEXCOORD1;\n"
	"  float4 vposition : POSITION;\n"
	"};\n"
	"struct Effect2DF_Output {\n"
	"  float4 fcolor : COLOR;\n"
	"};\n"
	"\n"
	"\n"
	"void vertex(in Effect2DV_Input IN, out Effect2DV_Output OUT)\n"
	"{	\n"
	"\n"
	"  OUT.vposition = mul( PositionMatrix,	float4( IN.cposition, 0.0f, 1.0f ) );\n "
	"  OUT.vtexture = mul( TextureMatrix,	float4( IN.ctexcoord, 0.0f, 1.0f ) ).xy;\n"
	"  OUT.vcolor = ConstantColor;\n"
	"}"
	"\n"
	"void fragment(in Effect2DV_Output IN, out Effect2DF_Output OUT)\n"
	"{\n"
	"  OUT.fcolor = IN.vcolor;\n"
	"}\n";

	struct Effect2DConstants
	{
		Vector4<>	color;
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

	UserInterfaceRendering::UserInterfaceRendering(RenderingAPIWrapper* pWrapper) : m_pWrapper(pWrapper),m_bMouseVisible(false)
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
		m_pEffect2D = pWrapper->CreateEffect(Effect2D);
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
				m_bMouseVisible = m->IsVisible();
				return true;
			}
			break;
		}
		return true;
	}

	void UserInterfaceRendering::Draw()
	{
		if(m_bMouseVisible)
		{
			Effect2DConstants constants;
			constants.texture_matrix = Matrix4<>::One();
			constants.position_matrix = Matrix4<>::Translate(Vector3<>(m_vMousePosition,0.0f))*Matrix4<>::Scale(Vector3<>(0.1f,0.1f,0.0f));

			m_pConstants->UpdateContent(&constants);
			m_pEffect2D->Draw(m_pVASquare,&m_pConstants);
		}
	}
}