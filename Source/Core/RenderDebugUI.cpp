#include "HAGE.h"
#include "RenderDebugUI.h"
/*
	MESSAGE_UI_UNKNOWN			= 0x00110000,
	MESSAGE_UI_CURSOR_UPDATE	= 0x00110001,
	MESSAGE_UI_ADJUST_CAMERA	= 0x00110002,
	MESSAGE_UI_MOVE_PLAYER		= 0x00110003,
	*/
namespace HAGE {
	static const char* __Effect2D_VP =
	"struct Effect2DConstants\n"
	"{\n"
	"    float4x4 position_matrix;\n"
	"    float4x4 texture_matrix;\n"
	"	 float4 color;\n"
	"} cbuffer0_Effect2DConstants: BUFFER[0];\n"
	"\n"
	"// cbuffer Effect2DConstants : register(b0)\n"
	"#define PositionMatrix       cbuffer0_Effect2DConstants.position_matrix\n"
	"#define TextureMatrix        cbuffer0_Effect2DConstants.texture_matrix\n"
	"#define ConstantColor        cbuffer0_Effect2DConstants.color\n"
	"\n"
	"struct Effect2DV_Output {\n"
	"  float4 color    : COLOR;\n"
	"  float2 texture  : TEXCOORD0;\n"
	"  float4 position : POSITION;\n"
	"};\n"
	"\n"
	"Effect2DV_Output vertex(float2 position : POSITION, float2 texcoord : TEXCOORD0)\n"
	"{	\n"
	"  Effect2DV_Output OUT;\n"
	"\n"
	"  OUT.position = mul( PositionMatrix,	float4( position, 0.0f, 1.0f ) );\n "
	"  OUT.texture = mul( TextureMatrix,	float4( texcoord, 0.0f, 1.0f ) ).xy;\n"
	"  OUT.color = ConstantColor;\n"
	"\n"
	"  return OUT;	\n"
	"}";

	static const char* __Effect2D_FP =
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

	struct __Effect2DConstants
	{
		Matrix4<>	position_matrix;
		Matrix4<>	texture_matrix;
		Vector4<>	color;
	};

	static const char __sz2DFormat[] = "__DebugVertex2DFormat";
	struct __Vertex2DFormat
	{
	public:
		Vector2<>	position;
		Vector2<>	texture;

		static const char* name;
	};
	const char* __Vertex2DFormat::name = __sz2DFormat;
	VertexDescriptionEntry __Vertex2DFormatDescriptor[] = {
		{"Position",	1,	R32G32_FLOAT	},
		{"Texcoord",	1,	R32G32_FLOAT	}
	};

	RenderDebugUI::RenderDebugUI(RenderingAPIWrapper* pWrapper) : m_pWrapper(pWrapper),m_bVisible(false)
	{
		__Vertex2DFormat vertices[] =
		{
			{Vector2<>(-1.0f, 1.0f ),	Vector2<>(0.0f, 1.0f)},
			{Vector2<>(1.0f,  1.0f ),	Vector2<>(1.0f, 1.0f)},
			{Vector2<>(-1.0f, -1.0f),	Vector2<>(0.0f, 0.0f)},
			{Vector2<>(1.0f,  -1.0f),	Vector2<>(1.0f, 0.0f)},
		};

		u32	indices[] =
		{
			0,1,2,
			1,3,2
		};

		pWrapper->RegisterVertexFormat(__Vertex2DFormat::name,__Vertex2DFormatDescriptor,2);
		m_pVBSquare = pWrapper->CreateVertexBuffer(__Vertex2DFormat::name,vertices,4);
		m_pVASquare = pWrapper->CreateVertexArray(2,PRIMITIVE_TRIANGLELIST,&m_pVBSquare,1,indices);

		APIWRasterizerState rast = DefaultRasterizerState;
		rast.bDepthClipEnable = false;

		APIWBlendState blend = DefaultBlendState;
		blend.bBlendEnable = true;
		blend.SrcBlend = BLEND_SRC_ALPHA;
		blend.DestBlend = BLEND_INV_SRC_ALPHA;

		m_pEffect2D = pWrapper->CreateEffect(__Effect2D_VP,__Effect2D_FP,&rast,&blend);
		m_pConstantsBackdrop = pWrapper->CreateConstantBuffer(sizeof(__Effect2DConstants));
		m_pConstantsPointer = pWrapper->CreateConstantBuffer(sizeof(__Effect2DConstants));

		__Effect2DConstants constants;

		constants.texture_matrix = Matrix4<>::One();
		constants.position_matrix = Matrix4<>::One();
		constants.color =  Vector4<>(0.0f,0.0f,0.0f,0.3f);
		m_pConstantsBackdrop->UpdateContent(&constants);

		constants.texture_matrix = Matrix4<>::One();
		constants.position_matrix = Matrix4<>::Translate(Vector3<>(m_vMousePosition,0.0f))*Matrix4<>::Scale(Vector3<>(0.1f,0.1f,0.0f));
		constants.color =  Vector4<>(1.0f,1.0f,1.0f,1.0f);
		m_pConstantsPointer->UpdateContent(&m_pConstantsPointer);
	}
	RenderDebugUI::~RenderDebugUI()
	{
		delete m_pConstantsBackdrop;
		delete m_pConstantsPointer;
		delete m_pEffect2D;
		delete m_pVASquare;
		delete m_pVBSquare;
	}

	void RenderDebugUI::Draw()
	{
		while(const Message* m=DebugUIControl.GetTopMessage())
		{
			switch(m->GetMessageCode())
			{
			case MESSAGE_UI_SHOW:
				m_bVisible = true;
				break;
			case MESSAGE_UI_HIDE:
				m_bVisible = false;
				break;
			case MESSAGE_UI_CURSOR_UPDATE:
				{
					const MessageUICursorUpdate* pMessage = (const MessageUICursorUpdate*)m;
					m_vMousePosition.x=pMessage->GetCursorX();
					m_vMousePosition.y=pMessage->GetCursorY();

					// update rendering constants
					__Effect2DConstants constants;
					constants.texture_matrix = Matrix4<>::One();
					constants.position_matrix = Matrix4<>::Translate(Vector3<>(m_vMousePosition,0.0f))*Matrix4<>::Scale(Vector3<>(0.1f,0.1f,0.0f));
					constants.color =  Vector4<>(1.0f,1.0f,1.0f,1.0f);

					m_pConstantsPointer->UpdateContent(&constants);
				}
				break;
			}

			DebugUIControl.PopMessage();
		}

		if(m_bVisible)
		{
			//draw darkening square first
			m_pEffect2D->Draw(m_pVASquare,&m_pConstantsBackdrop);

			//draw mouse cursor
			m_pEffect2D->Draw(m_pVASquare,&m_pConstantsPointer);
		}
	}


	StaticMessageQueue<1024*128>	RenderDebugUI::DebugUIControl;
}
