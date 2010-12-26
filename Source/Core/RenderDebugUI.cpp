#include "HAGE.h"
#include "RenderDebugUI.h"
#include "Windows.h"
/*
	MESSAGE_UI_UNKNOWN			= 0x00110000,
	MESSAGE_UI_CURSOR_UPDATE	= 0x00110001,
	MESSAGE_UI_ADJUST_CAMERA	= 0x00110002,
	MESSAGE_UI_MOVE_PLAYER		= 0x00110003,
	*/
namespace HAGE {
	static const char* __Effect2D =
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
	"void vertex(in Effect2DV_Input VS_IN, out Effect2DV_Output VS_OUT)\n"
	"{	\n"
	"\n"
	"  VS_OUT.vposition = mul( PositionMatrix,	float4( VS_IN.cposition, 0.0f, 1.0f ) );\n "
	"  VS_OUT.vtexture = mul( TextureMatrix,	float4( VS_IN.ctexcoord, 0.0f, 1.0f ) ).xy;\n"
	"  VS_OUT.vcolor = ConstantColor;\n"
	"}"
	"\n"
	"void fragment(in Effect2DV_Output VS_OUT, out Effect2DF_Output PS_OUT)\n"
	"{\n"
	"  PS_OUT.fcolor = VS_OUT.vcolor;\n"
	"}\n";

	struct __Effect2DConstants
	{
		Vector4<>	color;
		Matrix4<>	position_matrix;
		Matrix4<>	texture_matrix;
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

		m_pEffect2D = pWrapper->CreateEffect(__Effect2D,nullptr,&rast,&blend);
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
				printf("recieved %i\n",GetTickCount());
				m_bVisible = true;
				break;
			case MESSAGE_UI_HIDE:
				printf("recieved %i\n",GetTickCount());
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
