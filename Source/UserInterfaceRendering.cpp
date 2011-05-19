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
	"H_CONSTANT_BUFFER_BEGIN(Effect2DConstants)\n"
	"    float4x4 position_matrix;\n"
	"    float4x4 texture_matrix;\n"
	"	 float4 const_color;\n"
	"H_CONSTANT_BUFFER_END\n"
	"H_TEXTURE_2D(DiffuseTexture);\n"
	"\n"
	"VS_IN_BEGIN\n"
	"  DECL_VS_IN(float2,position);\n"
	"  DECL_VS_IN(float2,texcoord);\n"
	"VS_IN_END\n"
	"VS_OUT_BEGIN\n"
	"  DECL_VS_OUT(float4,color);\n"
	"  DECL_VS_OUT(float2,tex);\n"
	"  DECL_VS_POSITION;\n"
	"VS_OUT_END\n"
	"\n"
	"VERTEX_SHADER\n"
	"{	\n"
	"  VS_OUT_POSITION = mul( position_matrix,	float4( VS_IN(position), 0.0f, 1.0f ) );\n "
	"  VS_OUT(tex) = mul( texture_matrix,	float4( VS_IN(texcoord), 0.0f, 1.0f ) ).xy;\n"
	"  VS_OUT(color) = const_color;\n"
	"}"
	"\n"
	"FS_IN_BEGIN\n"
	"  DECL_FS_IN(float4,color);\n"
	"  DECL_FS_IN(float2,tex);\n"
	"FS_IN_END\n"
	"FS_OUT_BEGIN\n"
	"  DECL_FS_COLOR;\n"
	"FS_OUT_END\n"
	"\n"
	"FRAGMENT_SHADER\n"
	"{\n"
	"  FS_OUT_COLOR = float4(H_SAMPLE_2D_LOD(DiffuseTexture,FS_IN(tex),0).xyz,1.0f);\n"
	"}\n";

	struct Effect2DConstants
	{
		Matrix4<>	position_matrix;
		Matrix4<>	texture_matrix;
		Vector4<>	color;
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
			{Vector2<>(-1.0f, 1.0f ),	Vector3<>(1.0f, 0.0f, 0.0f), Vector2<>(0.0f, 0.0f)},
			{Vector2<>(1.0f,  1.0f ),	Vector3<>(0.0f, 1.0f, 0.0f), Vector2<>(1.0f, 0.0f)},
			{Vector2<>(-1.0f, -1.0f),	Vector3<>(0.0f, 0.0f, 1.0f), Vector2<>(0.0f, 1.0f)},
			{Vector2<>(1.0f,  -1.0f),	Vector3<>(0.1f, 0.1f, 0.1f), Vector2<>(1.0f, 1.0f)},
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
		m_pEffect2D->SetConstant("Effect2DConstants",m_pConstants);

		_VT = GetResource()->OpenResource<IVirtualTexture>("t_landscape.hsvt");
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
			m_pEffect2D->Draw(m_pVASquare);
		}
		/*
		{
			Effect2DConstants constants;
			constants.color = Vector4<>(1.0f,1.0f,1.0f,0.0f);
			constants.texture_matrix = Matrix4<>::One();
			constants.position_matrix = (Matrix4<>::Translate(Vector3<>(0.5f,0.5f,0.0f)) * Matrix4<>::Scale(Vector3<>(0.3f,0.3f,0.0f))).Transpose();
			m_pConstants->UpdateContent(&constants);
			m_pEffect2D->SetTexture("DiffuseTexture",_VT->_Debug_GetFeedbackTexture());
			m_pEffect2D->Draw(m_pVASquare);
		}
		{
			Effect2DConstants constants;
			constants.color = Vector4<>(1.0f,1.0f,1.0f,0.0f);
			constants.texture_matrix = Matrix4<>::One();
			constants.position_matrix = (Matrix4<>::Translate(Vector3<>(-0.5f,0.5f,0.0f)) * Matrix4<>::Scale(Vector3<>(0.3f,0.3f,0.0f))).Transpose();
			m_pConstants->UpdateContent(&constants);
			m_pEffect2D->SetTexture("DiffuseTexture",_VT->GetCurrentVTRedirection());
			m_pEffect2D->Draw(m_pVASquare);
		}
		{
			Effect2DConstants constants;
			constants.color = Vector4<>(1.0f,1.0f,1.0f,0.0f);
			constants.texture_matrix = Matrix4<>::One();
			constants.position_matrix = (Matrix4<>::Translate(Vector3<>(0.5f,-0.5f,0.0f)) * Matrix4<>::Scale(Vector3<>(0.3f,0.3f,0.0f))).Transpose();
			m_pConstants->UpdateContent(&constants);
			m_pEffect2D->SetTexture("DiffuseTexture",_VT->GetCurrentVTCache());
			m_pEffect2D->Draw(m_pVASquare);
		}*/
	}
}