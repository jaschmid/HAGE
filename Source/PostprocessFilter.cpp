#include "header.h"
#include "PostprocessFilter.h"

namespace HAGE {

const char* PostprocessFilterEffect =
"H_CONSTANT_BUFFER_BEGIN(PostprocessConstants)\n"
"	 float4 const_color;\n"
"    float4x4 position_matrix;\n"
"    float4x4 texture_matrix;\n"
"H_CONSTANT_BUFFER_END\n"
"H_TEXTURE_2D(InputSurface);\n"
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
"  VS_OUT(tex) = VS_IN(texcoord);\n"
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
"  FS_OUT_COLOR = H_SAMPLE_2D(InputSurface,FS_IN(tex));\n"
"}\n";

struct PreprocEffectConstants
{
	Vector4<>	color;
	Matrix4<>	position_matrix;
	Matrix4<>	texture_matrix;
};

extern const char szPreprocFormat[] = "PreprocVertexFormat";
struct PreprocVertexFormat
{
public:
	Vector2<>	position;
	Vector2<>	texture;

	static const char* name;
};
const char* PreprocVertexFormat::name = szPreprocFormat;
VertexDescriptionEntry PreprocVertexFormatDescriptor[] = {
	{"position",	1,	R32G32_FLOAT	},
	{"texcoord",	1,	R32G32_FLOAT	}
};


PostprocessFilter::PostprocessFilter(RenderingAPIWrapper* pWrapper) : _pWrapper(pWrapper)
{
	PreprocVertexFormat vertices[] =
	{
		{Vector2<>(-1.0f, 1.0f ),Vector2<>(0.0f, 0.0f)},
		{Vector2<>(1.0f,  1.0f ),Vector2<>(1.0f, 0.0f)},
		{Vector2<>(-1.0f, -1.0f),Vector2<>(0.0f, 1.0f)},
		{Vector2<>(1.0f,  -1.0f),Vector2<>(1.0f, 1.0f)},
	};

	u32	indices[] =
	{
		0,1,2,
		1,3,2
	};

	pWrapper->RegisterVertexFormat(PreprocVertexFormat::name,PreprocVertexFormatDescriptor,2);
	_pPostprocessQuadBuffer = pWrapper->CreateVertexBuffer(PreprocVertexFormat::name,vertices,4);
	_pPostprocessQuadArray = pWrapper->CreateVertexArray(2,PRIMITIVE_TRIANGLELIST,&_pPostprocessQuadBuffer,1,indices);
	_pPostprocessEffect = pWrapper->CreateEffect(PostprocessFilterEffect);
	_pPostprocessConstants = pWrapper->CreateConstantBuffer(sizeof(PreprocEffectConstants));

	APIWViewport vp;
	pWrapper->GetCurrentViewport(vp);
	
	_pPostprocessBuffer[0] = pWrapper->CreateTexture(vp.XSize,vp.YSize,1,HAGE::R32G32B32A32_FLOAT,HAGE::TEXTURE_GPU_WRITE,nullptr);
	_pPostprocessBuffer[1] = pWrapper->CreateTexture(vp.XSize,vp.YSize,1,HAGE::R32G32B32A32_FLOAT,HAGE::TEXTURE_GPU_WRITE,nullptr);
	_pPostprocessDepthBuffer = pWrapper->CreateTexture(vp.XSize,vp.YSize,1,HAGE::R16_UNORM,HAGE::TEXTURE_GPU_DEPTH_STENCIL,nullptr);
}

PostprocessFilter::~PostprocessFilter()
{
	if(_pPostprocessBuffer[0])
		delete _pPostprocessBuffer[0];
	if(_pPostprocessBuffer[1])
		delete _pPostprocessBuffer[1];
	if(_pPostprocessDepthBuffer)
		delete _pPostprocessDepthBuffer;
	if(_pPostprocessConstants)
		delete _pPostprocessConstants;
	if(_pPostprocessQuadBuffer)
		delete _pPostprocessQuadBuffer;
	if(_pPostprocessQuadArray)
		delete _pPostprocessQuadArray;
	if(_pPostprocessEffect)
		delete _pPostprocessEffect;
}

void PostprocessFilter::BeginSceneRendering()
{
	_pPostprocessBuffer[0]->Clear(Vector4<>(0.0f,1.0f,0.0f,0.0f));
	_pPostprocessDepthBuffer->Clear(true,1.0f);
	_pWrapper->SetRenderTarget(_pPostprocessBuffer[0],_pPostprocessDepthBuffer);
}

void PostprocessFilter::EndSceneRendering()
{

	_pWrapper->SetRenderTarget(RENDER_TARGET_DEFAULT,RENDER_TARGET_DEFAULT);
	
	PreprocEffectConstants constants;
	constants.texture_matrix = Matrix4<>::One();
	constants.position_matrix = _pWrapper->GenerateProjectionMatrixBase()*Matrix4<>::Scale(Vector3<>(1.0f,1.0f,1.0f));

	_pPostprocessConstants->UpdateContent(&constants);
	_pPostprocessEffect->SetConstant("PostprocessConstants",_pPostprocessConstants);
	_pPostprocessEffect->SetTexture("InputSurface", _pPostprocessBuffer[0]);
	_pPostprocessEffect->Draw(_pPostprocessQuadArray);
}

};