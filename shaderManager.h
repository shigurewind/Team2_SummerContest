#pragma once
#include "main.h"
#include "renderer.h"

struct SHADER
{
    ID3D11VertexShader* vertexShader;
    ID3D11PixelShader* pixelShader;
    ID3D11InputLayout* inputLayout;
};

bool LoadShaderFromFile(const char* fileName,const char* vsEntry,const char* psEntry,SHADER* outShader);

