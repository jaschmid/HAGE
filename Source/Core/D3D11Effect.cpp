#include <HAGE.h>
#include "D3D11APIWrapper.h"

#ifndef NO_D3D

static const D3D10_SHADER_MACRO vsDef[] = {
	{"_H_HLSL",""},
	{"_V_SHADER",""},
	{nullptr,nullptr}
};
static const D3D10_SHADER_MACRO fsDef[] = {
	{"_H_HLSL",""},
	{"_F_SHADER",""},
	{nullptr,nullptr}
};
static const D3D10_SHADER_MACRO gsDef[] = {
	{"_H_HLSL",""},
	{"_G_SHADER",""},
	{nullptr,nullptr}
};
static const char* preproc = 
	"#ifdef _H_HLSL\n"
	"	#define H_CONSTANT_BUFFER_BEGIN(x) cbuffer x {\n"
	"	#define H_CONSTANT_BUFFER_END } ;\n"
	"	#define VERTEX_SHADER void _vertex(in _VS_IN_STRUCT _VS_IN,out _VS_OUT_STRUCT _VS_OUT) \n"
	"	#define GEOMETRY_SHADER(x,y,z) [maxvertexcount(z)] void _geometry(x, inout y<_GS_OUT_STRUCT> _GS_OUT)\n"
	"	#define FRAGMENT_SHADER void _fragment(in _FS_IN_STRUCT _FS_IN,out _FS_OUT_STRUCT _FS_OUT) \n"
	"	#define VS_IN_BEGIN struct _VS_IN_STRUCT {\n"
	"	#define VS_IN_END };\n"
	"	#define VS_OUT_BEGIN struct _VS_OUT_STRUCT {\n"
	"	#define VS_OUT_END };\n"
	"	#define VS_OUT_POSITION (_VS_OUT._position)\n"
	"	#define FS_IN_BEGIN struct _FS_IN_STRUCT {\n"
	"	#define FS_IN_END };\n"
	"	#define FS_OUT_BEGIN struct _FS_OUT_STRUCT {\n"
	"	#define FS_OUT_END };\n"
	"	#define FS_OUT_COLOR (_FS_OUT._target)\n"
	"	#define GS_IN_BEGIN struct _GS_IN_STRUCT {\n"
	"	#define GS_IN_END };\n"
	"	#define GS_OUT_BEGIN struct _GS_OUT_STRUCT {\n"
	"	#define GS_OUT_END };\n"
	"	#define DECL_VS_IN(x,y) x y : y\n"
	"	#define DECL_VS_OUT(x,y) x y : y\n"
	"	#define DECL_FS_IN(x,y) x y : y\n"
	"	#define DECL_FS_OUT(x,y) x y : y\n"
	"	#define DECL_GS_IN(x,y) x y : y\n"
	"	#define DECL_GS_OUT(x,y) x y : y\n"
	"	#define DECL_VS_POSITION float4 _position : SV_Position\n"
	"	#define DECL_FS_COLOR float4 _target : SV_Target\n"
	"	#define VS_OUT(x) _VS_OUT.x\n"
	"	#define VS_IN(x) _VS_IN.x\n"
	"	#define FS_OUT(x) _FS_OUT.x\n"
	"	#define FS_IN(x) _FS_IN.x\n"
	"	#define GS_IN_POSITION(i) _GS_IN[i]._position\n"
	"	#define GS_IN(x,i) _GS_IN[i].x\n"
	"	#define GS_OUT(x) _GS_OUT_BUFFER.x\n"
	"	#define GS_OUT_POSITION _GS_OUT_BUFFER._position\n"
	"	#define GS_OUT_LAYER _GS_OUT_BUFFER._layer\n"
	"	#define DECL_GS_IN_POSITION float4 _position : SV_Position\n"
	"	#define DECL_GS_OUT_POSITION float4 _position : SV_Position\n"
	"	#define DECL_GS_OUT_LAYER uint _layer : SV_RenderTargetArrayIndex\n"
	"	#define GS_INIT_OUT _GS_OUT_STRUCT _GS_OUT_BUFFER\n"
	"	#define GS_END_VERTEX _GS_OUT.Append(_GS_OUT_BUFFER)\n"
	"	#define GS_END_PRIMITIVE _GS_OUT.RestartStrip()\n"
	"	#define GS_STREAM_OUT(x) inout x<_GS_OUT_STRUCT> _GS_OUT\n"
	"	#define GS_PASS_STREAM_OUT _GS_OUT\n"
	"	#define TRIANGLE_IN triangle _GS_IN_STRUCT _GS_IN[3]\n"
	"	#define TRIANGLE_ADJ_IN triangleadj _GS_IN_STRUCT _GS_IN[6]\n"
	"	#define LINE_IN line _GS_IN_STRUCT _GS_IN[2]\n"
	"	#define LINE_ADJ_IN lineadj _GS_IN_STRUCT _GS_IN[4]\n"
	"	#define POINT_IN point _GS_IN_STRUCT _GS_IN[1]\n"
	"	#define TRIANGLE_OUT TriangleStream\n"
	"	#define LINE_OUT LineStream\n"
	"	#define POINT_OUT PointStream\n"
	"	#define H_TEXTURE_2D(x) Texture2D x;SamplerState _Sampler##x\n"
	"	#define H_TEXTURE_2D_INT(x) Texture2D<int4> x;SamplerState _Sampler##x\n"
	"	#define H_TEXTURE_2D_UINT(x) Texture2D<uint4> x;SamplerState _Sampler##x\n"
	"	#define H_TEXTURE_CUBE(x) TextureCube x;SamplerState _Sampler##x\n"
	"	#define H_TEXTURE_2D_CMP(x) Texture2D x;SamplerComparisonState _Sampler##x\n"
	"	#define H_TEXTURE_CUBE_CMP(x) TextureCube x;SamplerComparisonState _Sampler##x\n"
	"	#define H_SAMPLE_2D(x,y) (x.Sample( _Sampler##x,(y)))\n"
	"	#define H_SAMPLE_2D_LOD(x,y,z) (x.SampleLevel( _Sampler##x,(y),(z)))\n"
	"	#define H_SAMPLE_2D_GRAD(x,y,z,w) (x.SampleGrad( _Sampler##x,(y),(z),(w)))\n"
	"	#define H_SAMPLE_CUBE(x,y) (x.Sample( _Sampler##x,(y)))\n"
	"	#define H_SAMPLE_CUBE_LOD(x,y,z) (x.SampleLevel( _Sampler##x,(y),(z)))\n"
	"	#define H_SAMPLE_2D_CMP(x,y,z) (x.SampleCmp( _Sampler##x,(y),(z)))\n"
	"	#define H_SAMPLE_CUBE_CMP(x,y,z) (x.SampleCmp( _Sampler##x,(y),(z)))\n"
	"	#define H_SAMPLE_2D_CMP0(x,y,z) (x.SampleCmpLevelZero( _Sampler##x,(y),(z)))\n"
	"	#define H_SAMPLE_CUBE_CMP0(x,y,z) (x.SampleCmpLevelZero( _Sampler##x,(y),(z)))\n"
	"#endif\n";
static const int preproc_size = strlen(preproc);

D3D11Effect::D3D11Effect(D3D11APIWrapper* pWrapper,const char* pProgram,ID3D11RasterizerState* pRasterizerState, ID3D11BlendState* pBlendState, ID3D11DepthStencilState* pDepthState,D3D11APIWrapper::D3DSampler* pSamplers,HAGE::u32 nSamplers) :
	m_pWrapper(pWrapper),
	m_pCompiledShader(nullptr),
	m_pRasterizerState(pRasterizerState),
	m_pBlendState(pBlendState),
	m_pDepthState(pDepthState)
{
	char* tBuffer = new char[strlen(pProgram)+preproc_size+1];
	memcpy(tBuffer,preproc,preproc_size);
	strcpy(&tBuffer[preproc_size],pProgram);
	// Create Shaders

	if(strstr(pProgram,"VERTEX_SHADER"))
		m_pVertexShader=CompileVertexShader(tBuffer);
	else
		m_pVertexShader=nullptr;
	if(strstr(pProgram,"FRAGMENT_SHADER"))
		m_pPixelShader=CompilePixelShader(tBuffer);
	else
		m_pPixelShader=nullptr;
	if(strstr(pProgram,"GEOMETRY_SHADER"))
		m_pGeometryShader=CompileGeometryShader(tBuffer);
	else
		m_pGeometryShader=nullptr;

	for(int b =0;b<3;++b)
	{
		for(int i = 0;i<_boundConstants[b].size();++i)
			_boundConstants[b][i] = nullptr;
		for(int i = 0;i<_boundTextures[b].size();++i)
			_boundTextures[b][i] = nullptr;
		for(int i = 0;i<_boundTextures[b].size();++i)
			_boundSamplers[b][i] = m_pWrapper->GetDefaultSampler();
	}
	
	for(int isampler = 0;isampler<nSamplers;++isampler)
	{
		auto bindpoint=_samplerBinds.find(global_string("_Sampler") + global_string(pSamplers[isampler].Name));
		if(bindpoint != _samplerBinds.end())
			for(int istage = 0; istage< 3;++istage)
				if(bindpoint->second._ShaderStages[istage] != -1)
					_boundSamplers[istage][bindpoint->second._ShaderStages[istage]] = pSamplers[isampler].pSampler;
	}

	delete tBuffer;
}

D3D11Effect::~D3D11Effect()
{
	m_pRasterizerState->Release();
	m_pBlendState->Release();
	m_pDepthState->Release();

	for(auto i=m_ArrayLayoutList.begin();i!=m_ArrayLayoutList.end();++i)
		i->second->Release();

	m_ArrayLayoutList.clear();

	for(int i = 0;i<3;++i)
		for(auto sampler = _boundSamplers[i].begin();sampler!=_boundSamplers[i].end();++sampler)
			(*sampler)->Release();

	if(m_pCompiledShader)m_pCompiledShader->Release();
	if(m_pVertexShader)m_pVertexShader->Release();
	if(m_pPixelShader)m_pPixelShader->Release();
	if(m_pGeometryShader)m_pGeometryShader->Release();
}


ID3D11VertexShader* D3D11Effect::CompileVertexShader(const char* shader)
{
	ID3D11VertexShader* pReturn;

// Compile and create the vertex shader
    DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;

#ifdef _DEBUG
    dwShaderFlags |= D3D10_SHADER_DEBUG;
#endif

    ID3D10Blob* pBlobError = NULL;
    HRESULT hr = D3DCompile( shader, lstrlenA( shader ) + 1, nullptr, vsDef, NULL, "_vertex",
                     "vs_4_0", dwShaderFlags, 0, &m_pCompiledShader, &pBlobError );
	if(FAILED(hr))
	{
		printf((const char*)pBlobError->GetBufferPointer());
	}
	assert( SUCCEEDED( hr ) );
	
	m_pWrapper->EnterDeviceCritical();
	hr = m_pWrapper->GetDevice()->CreateVertexShader( m_pCompiledShader->GetBufferPointer(), m_pCompiledShader->GetBufferSize(),
                                        NULL, &pReturn );
	m_pWrapper->LeaveDeviceCritical();

	assert( SUCCEEDED( hr ) );


	if(pBlobError)
		pBlobError->Release();

	ID3D11ShaderReflection* pReflector = nullptr; 
	D3DReflect( m_pCompiledShader->GetBufferPointer(),m_pCompiledShader->GetBufferSize(), IID_ID3D11ShaderReflection,(void**)&pReflector);

	D3D11_SHADER_DESC sDesc;
	pReflector->GetDesc(&sDesc);

	D3D11_SHADER_INPUT_BIND_DESC tDesc;
	for(int i = 0; i < sDesc.BoundResources; ++i)
	{
		pReflector->GetResourceBindingDesc(i,&tDesc);
		ParseBindPoints(tDesc,0);
	}

	pReflector->Release();

	return pReturn;
}

ID3D11GeometryShader* D3D11Effect::CompileGeometryShader(const char* shader)
{
	ID3D11GeometryShader* pReturn;

    ID3D10Blob* pBlobGS = NULL;
	
// Compile and create the vertex shader
    DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;

#ifdef _DEBUG
    dwShaderFlags |= D3D10_SHADER_DEBUG;
#endif

    ID3D10Blob* pBlobError = NULL;
    HRESULT hr = D3DCompile( shader, lstrlenA( shader ) + 1, nullptr, gsDef, NULL, "_geometry",
                     "gs_4_0", dwShaderFlags, 0, &pBlobGS, &pBlobError );
	if(FAILED(hr))
	{
		printf((const char*)pBlobError->GetBufferPointer());
	}
	assert( SUCCEEDED( hr ) );
	
	m_pWrapper->EnterDeviceCritical();
	hr = m_pWrapper->GetDevice()->CreateGeometryShader( pBlobGS->GetBufferPointer(), pBlobGS->GetBufferSize(),
                                        NULL, &pReturn );
	m_pWrapper->LeaveDeviceCritical();

	assert( SUCCEEDED( hr ) );
	
	ID3D11ShaderReflection* pReflector = nullptr; 
	D3DReflect( pBlobGS->GetBufferPointer(),pBlobGS->GetBufferSize(), IID_ID3D11ShaderReflection,(void**)&pReflector);

	D3D11_SHADER_DESC sDesc;
	pReflector->GetDesc(&sDesc);

	D3D11_SHADER_INPUT_BIND_DESC tDesc;
	for(int i = 0; i < sDesc.BoundResources; ++i)
	{
		pReflector->GetResourceBindingDesc(i,&tDesc);
		ParseBindPoints(tDesc,1);
	}

	pReflector->Release();

	if(pBlobError)
		pBlobError->Release();
	if(pBlobGS)
		pBlobGS->Release();

	return pReturn;
}

ID3D11PixelShader* D3D11Effect::CompilePixelShader(const char* shader)
{
	ID3D11PixelShader* pReturn;

// Compile and create the vertex shader
    DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;

#ifdef _DEBUG
    dwShaderFlags |= D3D10_SHADER_DEBUG;
#endif

    ID3D10Blob* pBlobPS = NULL;
    ID3D10Blob* pBlobError = NULL;
    HRESULT hr = D3DCompile( shader, lstrlenA( shader ) + 1, nullptr, fsDef, NULL, "_fragment",
                     "ps_4_0", dwShaderFlags, 0, &pBlobPS, &pBlobError );
	if(FAILED(hr))
	{
		printf((const char*)pBlobError->GetBufferPointer());
	}
    assert( SUCCEEDED( hr ) );

	m_pWrapper->EnterDeviceCritical();
    hr = m_pWrapper->GetDevice()->CreatePixelShader( pBlobPS->GetBufferPointer(), pBlobPS->GetBufferSize(),
                                       NULL, &pReturn );
	m_pWrapper->LeaveDeviceCritical();
	
	ID3D11ShaderReflection* pReflector = nullptr; 
	D3DReflect( pBlobPS->GetBufferPointer(),pBlobPS->GetBufferSize(), IID_ID3D11ShaderReflection,(void**)&pReflector);

	D3D11_SHADER_DESC sDesc;
	pReflector->GetDesc(&sDesc);

	D3D11_SHADER_INPUT_BIND_DESC tDesc;
	for(int i = 0; i < sDesc.BoundResources; ++i)
	{
		pReflector->GetResourceBindingDesc(i,&tDesc);
		ParseBindPoints(tDesc,2);
		/*
		pReflector->GetResourceBindingDesc(i,&tDesc);
		printf("Resource %i:\n",i);
		printf("\tName: %s\n",tDesc.Name);
		printf("\tType: %i\n",tDesc.Type);
		printf("\tBindPoint: %i\n",tDesc.BindPoint);*/
	}

	pReflector->Release();

    assert( SUCCEEDED( hr ) );
	pBlobPS->Release();

	if(pBlobError)
		pBlobError->Release();

	return pReturn;
}

void D3D11Effect::ParseBindPoints(const D3D11_SHADER_INPUT_BIND_DESC& desc,int bindPoint)
{
	global_string name(desc.Name);
	if(desc.Type == D3D10_SIT_CBUFFER)
	{
		auto found = _constantBinds.find(name);
		if(found!=_constantBinds.end())
			found->second._ShaderStages[bindPoint] = desc.BindPoint;
		else
		{
			BindPoints p;
			for(int i = 0;i<3;++i)
				p._ShaderStages[i] = -1;
			p._ShaderStages[bindPoint] = desc.BindPoint;
			_constantBinds.insert(std::pair<global_string,BindPoints>(name,p));
		}
		if(_boundConstants[bindPoint].size() < desc.BindPoint+1)
			_boundConstants[bindPoint].resize(desc.BindPoint+1);
	}
	else if(desc.Type == D3D10_SIT_TEXTURE)
	{
		auto found = _textureBinds.find(name);
		if(found!=_textureBinds.end())
			found->second._ShaderStages[bindPoint] = desc.BindPoint;
		else
		{
			BindPoints p;
			for(int i = 0;i<3;++i)
				p._ShaderStages[i] = -1;
			p._ShaderStages[bindPoint] = desc.BindPoint;
			_textureBinds.insert(std::pair<global_string,BindPoints>(name,p));
		}
		if(_boundTextures[bindPoint].size() < desc.BindPoint+1)
			_boundTextures[bindPoint].resize(desc.BindPoint+1);
	}
	else if(desc.Type == D3D10_SIT_SAMPLER)
	{	
		auto found = _samplerBinds.find(name);
		if(found!=_samplerBinds.end())
			found->second._ShaderStages[bindPoint] = desc.BindPoint;
		else
		{
			BindPoints p;
			for(int i = 0;i<3;++i)
				p._ShaderStages[i] = -1;
			p._ShaderStages[bindPoint] = desc.BindPoint;
			_samplerBinds.insert(std::pair<global_string,BindPoints>(name,p));
		}
		if(_boundSamplers[bindPoint].size() < desc.BindPoint+1)
			_boundSamplers[bindPoint].resize(desc.BindPoint+1);
	}
}

void D3D11Effect::Draw(HAGE::APIWVertexArray* pVertexArray)
{

	if(m_pWrapper->GetCurrentEffect() != this)
	{
		m_pWrapper->SetCurrentEffect(this);
		// set constants
		for(int i= 0; i<_boundConstants[0].size();++i)
		{
			if(_boundConstants[0][i])
				m_pWrapper->GetContext()->VSSetConstantBuffers(i,1,&(_boundConstants[0][i]->m_pBuffer));
		}
		for(int i= 0; i<_boundConstants[1].size();++i)
		{
			if(_boundConstants[1][i])
				m_pWrapper->GetContext()->GSSetConstantBuffers(i,1,&(_boundConstants[1][i]->m_pBuffer));
		}
		for(int i= 0; i<_boundConstants[2].size();++i)
		{
			if(_boundConstants[2][i])
				m_pWrapper->GetContext()->PSSetConstantBuffers(i,1,&(_boundConstants[2][i]->m_pBuffer));
		}

		// set textures
		for(int i= 0; i<_boundTextures[0].size();++i)
		{
			if(_boundTextures[0][i])
				m_pWrapper->GetContext()->VSSetShaderResources(i,1,&(_boundTextures[0][i]->_shaderResourceView));
		}
		for(int i= 0; i<_boundTextures[1].size();++i)
		{
			if(_boundTextures[1][i])
				m_pWrapper->GetContext()->GSSetShaderResources(i,1,&(_boundTextures[1][i]->_shaderResourceView));
		}
		for(int i= 0; i<_boundTextures[2].size();++i)
		{
			if(_boundTextures[2][i])
				m_pWrapper->GetContext()->PSSetShaderResources(i,1,&(_boundTextures[2][i]->_shaderResourceView));
		}
	
		// set samplers
		for(int i= 0; i<_boundSamplers[0].size();++i)
		{
			if(_boundSamplers[0][i])
				m_pWrapper->GetContext()->VSSetSamplers(i,1,&_boundSamplers[0][i]);
		}
		for(int i= 0; i<_boundSamplers[1].size();++i)
		{
			if(_boundSamplers[1][i])
				m_pWrapper->GetContext()->GSSetSamplers(i,1,&_boundSamplers[1][i]);
		}
		for(int i= 0; i<_boundSamplers[2].size();++i)
		{
			if(_boundSamplers[2][i])
				m_pWrapper->GetContext()->PSSetSamplers(i,1,&_boundSamplers[2][i]);
		}
		
		m_pWrapper->GetContext()->RSSetState(m_pRasterizerState);
		float blend_factor[4] = {1.0f,1.0f,1.0f,1.0f};
		m_pWrapper->GetContext()->OMSetBlendState(m_pBlendState,blend_factor,0xffffffff);
		m_pWrapper->GetContext()->OMSetDepthStencilState(m_pDepthState,0);

		m_pWrapper->GetContext()->VSSetShader( m_pVertexShader, NULL, 0 );

		m_pWrapper->GetContext()->PSSetShader( m_pPixelShader, NULL, 0 );

		m_pWrapper->GetContext()->GSSetShader( m_pGeometryShader, NULL, 0 );
	}


	D3D11VertexArray* pArray = (D3D11VertexArray*)pVertexArray;

	auto layout = m_ArrayLayoutList.find(pArray->m_ArrayCode);

	if( layout == m_ArrayLayoutList.end() )
	{
		CreateInputLayout(pArray->m_ArrayCode);
		layout = m_ArrayLayoutList.find(pArray->m_ArrayCode);

		assert (layout != m_ArrayLayoutList.end());
	}

	ID3D11InputLayout* pLayout = layout->second;

	m_pWrapper->GetContext()->IASetInputLayout( pLayout );

	HAGE::u32 nItems;

	switch(pArray->m_PrimitiveType)
	{
	case HAGE::PRIMITIVE_POINTLIST:
		m_pWrapper->GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );
		nItems = pArray->m_nPrimitives;
		break;
	case HAGE::PRIMITIVE_LINELIST:
		m_pWrapper->GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );
		nItems = pArray->m_nPrimitives*2;
		break;
	case HAGE::PRIMITIVE_LINESTRIP:
		m_pWrapper->GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP );
		nItems = pArray->m_nPrimitives-1;
		break;
	case HAGE::PRIMITIVE_TRIANGLELIST:
		m_pWrapper->GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		nItems = pArray->m_nPrimitives*3;
		break;
	case HAGE::PRIMITIVE_TRIANGLESTRIP:
		m_pWrapper->GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
		nItems = pArray->m_nPrimitives-2;
		break;
	}


	if(pArray->m_pIndexBuffer)
		m_pWrapper->GetContext()->IASetIndexBuffer(pArray->m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	for(HAGE::u32 i=0;i<pArray->m_nBuffers;++i)
	{
		D3D11VertexBuffer* pBuffer= pArray->m_VertexBuffers[i];
		UINT stride = m_pWrapper->GetVertexSize(pBuffer->m_nCode);
		UINT offset = 0;
		m_pWrapper->GetContext()->IASetVertexBuffers( i, 1, &pBuffer->m_pVertexBuffer, &stride, &offset );
	}

    // Render a triangle
	if(pArray->m_pIndexBuffer)
		m_pWrapper->GetContext()->DrawIndexed( nItems, 0, 0 );
	else
		m_pWrapper->GetContext()->Draw( nItems, 0 );

	/*
	for(HAGE::u32 i = 0; i<nTextures; ++i)
	{
		ID3D11ShaderResourceView* pView = nullptr;
		m_pWrapper->GetContext()->PSSetShaderResources(i,1,&pView);
	}*/
}


void D3D11Effect::CreateInputLayout(const D3D11APIWrapper::VertexFormatKey& v)
{
	const D3D11APIWrapper::ArrayFormatEntry* pEntry = m_pWrapper->GetArrayFormat(v);

	ID3D11InputLayout* pLayout=nullptr;

	m_pWrapper->EnterDeviceCritical();
	HRESULT hr = m_pWrapper->GetDevice()->CreateInputLayout( pEntry->pD3DDescription, pEntry->nElements, m_pCompiledShader->GetBufferPointer(),
										m_pCompiledShader->GetBufferSize(), &pLayout );
	m_pWrapper->LeaveDeviceCritical();

	assert(SUCCEEDED(hr) && pLayout);

	m_ArrayLayoutList.insert(std::pair<D3D11APIWrapper::VertexFormatKey,ID3D11InputLayout*>(v,pLayout));
}

void D3D11Effect::SetConstant(const char* pName,const HAGE::APIWConstantBuffer* constant)
{
	auto found = _constantBinds.find(global_string(pName));
	if(found != _constantBinds.end())
	{
		if(found->second._ShaderStages[0] >= 0)
		{
			_boundConstants[0][found->second._ShaderStages[0]] = (const D3D11ConstantBuffer*)constant;
			if(m_pWrapper->GetCurrentEffect() == this)
				m_pWrapper->GetContext()->VSSetConstantBuffers(found->second._ShaderStages[0],1,&((const D3D11ConstantBuffer*)constant)->m_pBuffer);
		}
		if(found->second._ShaderStages[1] >= 0)
		{
			_boundConstants[1][found->second._ShaderStages[1]] = (const D3D11ConstantBuffer*)constant;
			if(m_pWrapper->GetCurrentEffect() == this)
				m_pWrapper->GetContext()->GSSetConstantBuffers(found->second._ShaderStages[1],1,&((const D3D11ConstantBuffer*)constant)->m_pBuffer);
		}
		if(found->second._ShaderStages[2] >= 0)
		{
			_boundConstants[2][found->second._ShaderStages[2]] = (const D3D11ConstantBuffer*)constant;
			if(m_pWrapper->GetCurrentEffect() == this)
				m_pWrapper->GetContext()->PSSetConstantBuffers(found->second._ShaderStages[2],1,&((const D3D11ConstantBuffer*)constant)->m_pBuffer);
		}
	}
	/*
	int loc = glGetUniformBlockIndex(_glProgram,temp);
	if(loc >= 0 && loc < _constantBuffers.size())
		_constantBuffers[loc] = (const OGL3ConstantBuffer*)constant;*/
}

void D3D11Effect::SetTexture(const char* pName,const HAGE::APIWTexture* texture)
{/*
	int loc = glGetUniformLocation(_glProgram,pName);
	if(loc >= 0 && loc < _textureSlots.size())
		_textures[_textureSlots[loc]] = (const OGL3Texture*)texture;*/
	auto found = _textureBinds.find(global_string(pName));
	if(found != _textureBinds.end())
	{
		if(found->second._ShaderStages[0] >= 0)
		{
			_boundTextures[0][found->second._ShaderStages[0]] = (const D3D11Texture*)texture;
			if(m_pWrapper->GetCurrentEffect() == this)
				m_pWrapper->GetContext()->VSSetShaderResources(found->second._ShaderStages[0],1,&((const D3D11Texture*)texture)->_shaderResourceView);
		}
		if(found->second._ShaderStages[1] >= 0)
		{
			_boundTextures[1][found->second._ShaderStages[1]] = (const D3D11Texture*)texture;
			if(m_pWrapper->GetCurrentEffect() == this)
				m_pWrapper->GetContext()->GSSetShaderResources(found->second._ShaderStages[1],1,&((const D3D11Texture*)texture)->_shaderResourceView);
		}
		if(found->second._ShaderStages[2] >= 0)
		{
			_boundTextures[2][found->second._ShaderStages[2]] = (const D3D11Texture*)texture;
			if(m_pWrapper->GetCurrentEffect() == this)
				m_pWrapper->GetContext()->PSSetShaderResources(found->second._ShaderStages[2],1,&((const D3D11Texture*)texture)->_shaderResourceView);
		}
	}
}
#endif
