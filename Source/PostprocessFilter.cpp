#include "header.h"
#include "PostprocessFilter.h"

namespace HAGE {

const char* PostprocessFilterEffect =
"H_CONSTANT_BUFFER_BEGIN(PostprocessConstants)\n"
"	 float4		const_color;\n"
"    float4x4	position_matrix;\n"
"    float4x4	texture_matrix;\n"
"    float4		tex_offsets_x;\n"
"    float4		tex_offsets_y;\n"
"    float4		kernel[4];\n"
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
"	float4 accum = float4(0.0f,0.0f,0.0f,0.0f);\n"
"	for(int x=0;x<4;++x)\n"
"		for(int y=0;y<4;++y)\n"
"		{\n"
"			int index = y*4+x;\n"
"			accum += H_SAMPLE_2D(InputSurface,float2(FS_IN(tex).x + tex_offsets_x[x],FS_IN(tex).y + tex_offsets_y[y]))*kernel[index/4][index%4];\n"
"		}\n"
"	FS_OUT_COLOR = accum;\n"
"}\n";

struct PreprocEffectConstants
{
	Vector4<>	color;
	Matrix4<>	position_matrix;
	Matrix4<>	texture_matrix;

	f32			tex_offsets_x[4];
	f32			tex_offsets_y[4];

	f32			kernel[16];
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


PostprocessFilter::PostprocessFilter(RenderingAPIWrapper* pWrapper) : _pWrapper(pWrapper),_nPasses(0),_currentFilter(0)
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
	if(_nPasses)
	{
		_pPostprocessBuffer[0]->Clear(Vector4<>(0.0f,1.0f,0.0f,0.0f));
		_pPostprocessDepthBuffer->Clear(true,1.0f);
		_pWrapper->SetRenderTarget(_pPostprocessBuffer[0],_pPostprocessDepthBuffer);
	}
}

const f32 kernel[3][16] = {
{
	1.0f/16.0f,		2.0f/16.0f,		1.0f/16.0f,		0.0f,
	2.0f/16.0f,		4.0f/16.0f,		2.0f/16.0f,		0.0f,
	1.0f/16.0f,		2.0f/16.0f,		1.0f/16.0f,		0.0f,
	0.0f,			0.0f,			0.0f,			0.0f
},
{
	0.0f,		1.0f,		0.0f,		0.0f,
	1.0f,		-4.0f,		1.0f,		0.0f,
	0.0f,		1.0f,		0.0f,		0.0f,
	0.0f,		0.0f,		0.0f,		0.0f
},
{
	1.0f/64.0f,		3.0f/64.0f,		3.0f/64.0f,		1.0f/64.0f,
	3.0f/64.0f,		9.0f/64.0f,		9.0f/64.0f,		3.0f/64.0f,
	3.0f/64.0f,		9.0f/64.0f,		9.0f/64.0f,		3.0f/64.0f,
	1.0f/64.0f,		3.0f/64.0f,		3.0f/64.0f,		1.0f/64.0f
},
};

const f32 xOffsets[3][4] = {
	{ -1.0f,	0.0f,	1.0f,	0.0f },
	{ -1.0f,	0.0f,	1.0f,	0.0f },
	{ -1.5f,	-0.5f,	0.5f,	1.5f }
};
const f32 yOffsets[3][4] = {
	{ -1.0f,	0.0f,	1.0f,	0.0f },
	{ -1.0f,	0.0f,	1.0f,	0.0f },
	{ -1.5f,	-0.5f,	0.5f,	1.5f }
};

void PostprocessFilter::EndSceneRendering()
{
	
	if(_nPasses)
	{

		APIWViewport vp;
		_pWrapper->GetCurrentViewport(vp);

		PreprocEffectConstants constants;
		constants.texture_matrix = Matrix4<>::One();
		constants.position_matrix = _pWrapper->GenerateRenderTargetProjectionBase()*Matrix4<>::Scale(Vector3<>(1.0f,1.0f,1.0f));
		memcpy(constants.tex_offsets_x,xOffsets[_currentFilter],4*sizeof(f32));
		memcpy(constants.tex_offsets_y,yOffsets[_currentFilter],4*sizeof(f32));
		for(int i = 0; i < 4;++i)
		{
			constants.tex_offsets_x[i]/=vp.XSize;
			constants.tex_offsets_y[i]/=vp.YSize;
		}
		memcpy(constants.kernel,kernel[_currentFilter],16*sizeof(f32));



		_pPostprocessConstants->UpdateContent(&constants);
		_pPostprocessEffect->SetConstant("PostprocessConstants",_pPostprocessConstants);
		
		u32 currentSource = 0;
		for(u32 pass = 0; pass < _nPasses-1; ++pass)
		{
			_pWrapper->SetRenderTarget(_pPostprocessBuffer[(currentSource+1)%2],RENDER_TARGET_NONE);
	
			_pPostprocessEffect->SetTexture("InputSurface", _pPostprocessBuffer[currentSource]);
			_pPostprocessEffect->Draw(_pPostprocessQuadArray);

			currentSource= (currentSource+1)%2;
		}

		_pWrapper->SetRenderTarget(RENDER_TARGET_DEFAULT,RENDER_TARGET_DEFAULT);
	
		constants.texture_matrix = Matrix4<>::One();
		constants.position_matrix = _pWrapper->GenerateProjectionMatrixBase()*Matrix4<>::Scale(Vector3<>(1.0f,1.0f,1.0f));
		_pPostprocessConstants->UpdateContent(&constants);

		_pPostprocessEffect->SetTexture("InputSurface", _pPostprocessBuffer[currentSource]);
		_pPostprocessEffect->Draw(_pPostprocessQuadArray);
	}
}

};