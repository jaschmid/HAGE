#include <HAGE.h>
#include "D3D11APIWrapper.h"

#ifndef NO_D3D

D3D11Effect::D3D11Effect(D3D11APIWrapper* pWrapper,const char* pVertexProgram,const char* pFragmentProgram,ID3D11RasterizerState* pRasterizerState, ID3D11BlendState* pBlendState) :
	m_pWrapper(pWrapper),
	m_pCompiledShader(nullptr),
	m_pRasterizerState(pRasterizerState),
	m_pBlendState(pBlendState)
{

	// Create Shaders

	m_pVertexShader=CompileVertexShader(pVertexProgram);
	m_pPixelShader=CompilePixelShader(pVertexProgram);

}

D3D11Effect::~D3D11Effect()
{
	m_pRasterizerState->Release();
	m_pBlendState->Release();

	for(auto i=m_ArrayLayoutList.begin();i!=m_ArrayLayoutList.end();++i)
		i->second->Release();

	m_ArrayLayoutList.clear();

	cgDestroyProgram(m_CgVertexProgram);
	cgDestroyProgram(m_CgFragmentProgram);

	if(m_pCompiledShader)m_pCompiledShader->Release();
	if(m_pVertexShader)m_pVertexShader->Release();
	if(m_pPixelShader)m_pPixelShader->Release();
}


ID3D11VertexShader* D3D11Effect::CompileVertexShader(const char* shader)
{
	ID3D11VertexShader* pReturn;
	m_CgVertexProgram =	cgCreateProgram(
		m_pWrapper->GetCGC(),              /* Cg runtime context */
		CG_SOURCE,                /* Program in human-readable form */
		shader,				/* Name of file containing program */
		m_pWrapper->GetVertexProfile(),        /* Profile: OpenGL ARB vertex program */
		"vertex",			/* Entry function name */
		NULL);
    m_pWrapper->checkForCgError("creating vertex program from CG source");

	cgCompileProgram(m_CgVertexProgram);
    m_pWrapper->checkForCgError("compiling vertex program");

	const char* hlsl_source = cgGetProgramString( m_CgVertexProgram, CG_COMPILED_PROGRAM);

// Compile and create the vertex shader
    DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;

#ifdef _DEBUG
    dwShaderFlags |= D3D10_SHADER_DEBUG;
#endif

    ID3D10Blob* pBlobError = NULL;
    HRESULT hr = D3DCompile( hlsl_source, lstrlenA( hlsl_source ) + 1, "VS", NULL, NULL, "main",
                     "vs_4_0", dwShaderFlags, 0, &m_pCompiledShader, &pBlobError );
	if(FAILED(hr))
	{
		printf((const char*)pBlobError->GetBufferPointer());
	}
	assert( SUCCEEDED( hr ) );

	hr = m_pWrapper->GetDevice()->CreateVertexShader( m_pCompiledShader->GetBufferPointer(), m_pCompiledShader->GetBufferSize(),
                                        NULL, &pReturn );

	assert( SUCCEEDED( hr ) );


	if(pBlobError)
		pBlobError->Release();

	return pReturn;
}

ID3D11PixelShader* D3D11Effect::CompilePixelShader(const char* shader)
{
	ID3D11PixelShader* pReturn;
	m_CgFragmentProgram =	cgCreateProgram(
		m_pWrapper->GetCGC(),              /* Cg runtime context */
		CG_SOURCE,                /* Program in human-readable form */
		shader,				/* Name of file containing program */
		m_pWrapper->GetFragmentProfile(),        /* Profile: OpenGL ARB vertex program */
		"fragment",			/* Entry function name */
		NULL);
    m_pWrapper->checkForCgError("creating vertex program from CG source");

	cgCompileProgram(m_CgFragmentProgram);
    m_pWrapper->checkForCgError("compiling vertex program");

	const char* hlsl_source = cgGetProgramString( m_CgFragmentProgram, CG_COMPILED_PROGRAM);

// Compile and create the vertex shader
    DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;

#ifdef _DEBUG
    dwShaderFlags |= D3D10_SHADER_DEBUG;
#endif

    ID3D10Blob* pBlobPS = NULL;
    ID3D10Blob* pBlobError = NULL;
    HRESULT hr = D3DCompile( hlsl_source, lstrlenA( hlsl_source ) + 1, "PS", NULL, NULL, "main",
                     "ps_4_0", dwShaderFlags, 0, &pBlobPS, &pBlobError );
	if(FAILED(hr))
	{
		printf((const char*)pBlobError->GetBufferPointer());
	}
    assert( SUCCEEDED( hr ) );

    hr = m_pWrapper->GetDevice()->CreatePixelShader( pBlobPS->GetBufferPointer(), pBlobPS->GetBufferSize(),
                                       NULL, &pReturn );

    assert( SUCCEEDED( hr ) );
	pBlobPS->Release();

	if(pBlobError)
		pBlobError->Release();

	return pReturn;
}


void D3D11Effect::Draw(HAGE::APIWVertexArray* pVertexArray,HAGE::APIWConstantBuffer* const * pConstants,HAGE::u32 nConstants)
{
	for(HAGE::u32 i = 0; i<nConstants; ++i)
	{
		m_pWrapper->GetContext()->VSSetConstantBuffers(i,1,&(((D3D11ConstantBuffer*)pConstants[i])->m_pBuffer));
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

	m_pWrapper->GetContext()->RSSetState(m_pRasterizerState);
	float blend_factor[4] = {1.0f,1.0f,1.0f,1.0f};
	m_pWrapper->GetContext()->OMSetBlendState(m_pBlendState,blend_factor,0xffffffff);

	if(pArray->m_pIndexBuffer)
		m_pWrapper->GetContext()->IASetIndexBuffer(pArray->m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	for(HAGE::u32 i=0;i<pArray->m_nBuffers;++i)
	{
		D3D11VertexBuffer* pBuffer= pArray->m_VertexBuffers[i];
		UINT stride = m_pWrapper->GetVertexSize(pBuffer->m_nCode);
		UINT offset = 0;
		m_pWrapper->GetContext()->IASetVertexBuffers( i, 1, &pBuffer->m_pVertexBuffer, &stride, &offset );
	}

	m_pWrapper->GetContext()->VSSetShader( m_pVertexShader, NULL, 0 );

    m_pWrapper->GetContext()->PSSetShader( m_pPixelShader, NULL, 0 );


    // Render a triangle
	if(pArray->m_pIndexBuffer)
		m_pWrapper->GetContext()->DrawIndexed( nItems, 0, 0 );
	else
		m_pWrapper->GetContext()->Draw( nItems, 0 );
}


void D3D11Effect::CreateInputLayout(std::vector<HAGE::u8> v)
{
	const D3D11APIWrapper::ArrayFormatEntry* pEntry = m_pWrapper->GetArrayFormat(v);

	ID3D11InputLayout* pLayout=nullptr;

	HRESULT hr = m_pWrapper->GetDevice()->CreateInputLayout( pEntry->pD3DDescription, pEntry->nElements, m_pCompiledShader->GetBufferPointer(),
										m_pCompiledShader->GetBufferSize(), &pLayout );

	assert(SUCCEEDED(hr) && pLayout);

	m_ArrayLayoutList.insert(std::pair<std::vector<HAGE::u8>,ID3D11InputLayout*>(v,pLayout));
}

#endif
