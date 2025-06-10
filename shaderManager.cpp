#include "shaderManager.h"




bool LoadShaderFromFile(
    const char* fileName,
    const char* vsEntry,
    const char* psEntry,
    SHADER* outShader)
{
    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    // Compile Vertex Shader
    HRESULT hr = D3DX11CompileFromFile(fileName, nullptr, nullptr, vsEntry, "vs_4_0", flags, 0, nullptr, &vsBlob, &errorBlob, nullptr);
    if (FAILED(hr)) {
        MessageBox(NULL, (char*)errorBlob->GetBufferPointer(), "VS Compile Error", MB_OK);
        return false;
    }

    // Compile Pixel Shader
    hr = D3DX11CompileFromFile(fileName, nullptr, nullptr, psEntry, "ps_4_0", flags, 0, nullptr, &psBlob, &errorBlob, nullptr);
    if (FAILED(hr)) {
        MessageBox(NULL, (char*)errorBlob->GetBufferPointer(), "PS Compile Error", MB_OK);
        vsBlob->Release();
        return false;
    }

	//Shader‚ğì¬
    GetDevice()->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &outShader->vertexShader);
    GetDevice()->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &outShader->pixelShader);

    //Layout‚ğŒˆ‚ß‚é
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    GetDevice()->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &outShader->inputLayout);

    vsBlob->Release();
    psBlob->Release();

    return true;
}

