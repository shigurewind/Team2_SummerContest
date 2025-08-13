#include "shaderManager.h"
#include "renderer.h"
#include "imgui.h"

// ===== ShaderManager�N���X�̐ÓI�����o�[�ϐ����` =====
std::map<SHADER_TYPE, ShaderInfo> ShaderManager::m_Shaders;
SHADER_TYPE ShaderManager::m_CurrentShader = SHADER_DEFAULT;
bool ShaderManager::m_UseDebugOverride = false;
SHADER_TYPE ShaderManager::m_DebugOverrideShader = SHADER_DEFAULT;

// ===== ���̊֐��͕ύX�Ȃ��i����݊�����ۂj =====
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

    // ���_�V�F�[�_�[�R���p�C��
    HRESULT hr = D3DX11CompileFromFile(fileName, nullptr, nullptr, vsEntry, "vs_4_0", flags, 0, nullptr, &vsBlob,
        &errorBlob, nullptr);
    if (FAILED(hr)) {
        if (errorBlob) {
            MessageBox(NULL, (char*)errorBlob->GetBufferPointer(), "VS Compile Error", MB_OK);
            errorBlob->Release();
        }
        return false;
    }

    // �s�N�Z���V�F�[�_�[�R���p�C��
    hr = D3DX11CompileFromFile(fileName, nullptr, nullptr, psEntry, "ps_4_0", flags, 0, nullptr, &psBlob, &errorBlob,
        nullptr);
    if (FAILED(hr)) {
        if (errorBlob) {
            MessageBox(NULL, (char*)errorBlob->GetBufferPointer(), "PS Compile Error", MB_OK);
            errorBlob->Release();
        }
        vsBlob->Release();
        return false;
    }

    // �V�F�[�_�[�쐬
    hr = GetDevice()->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr,
        &outShader->vertexShader);
    if (FAILED(hr)) {
        vsBlob->Release();
        psBlob->Release();
        return false;
    }

    hr = GetDevice()->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr,
        &outShader->pixelShader);
    if (FAILED(hr)) {
        outShader->vertexShader->Release();
        outShader->vertexShader = nullptr;
        vsBlob->Release();
        psBlob->Release();
        return false;
    }

    // ���C�A�E�g�쐬
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    hr = GetDevice()->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(), &outShader->inputLayout);
    if (FAILED(hr)) {
        outShader->vertexShader->Release();
        outShader->pixelShader->Release();
        outShader->vertexShader = nullptr;
        outShader->pixelShader = nullptr;
        vsBlob->Release();
        psBlob->Release();
        return false;
    }

    vsBlob->Release();
    psBlob->Release();

    return true;
}



// ===== ShaderManager�N���X�̎��� =====

// �������F�S�ẴV�F�[�_�[�����[�h
bool ShaderManager::Initialize()
{
    if (GetDevice() == nullptr) {
        MessageBox(NULL, "GetDevice() returned nullptr!", "ShaderManager Error", MB_OK);
        return false;
    }

    if (GetDeviceContext() == nullptr) {
        MessageBox(NULL, "GetDeviceContext() returned nullptr!", "ShaderManager Error", MB_OK);
        return false;
    }

    //MessageBox(NULL, "Device and DeviceContext are OK", "ShaderManager Debug", MB_OK);

    // ���ʊǗ�������
    if (!EffectManager::Initialize()) {
        MessageBox(NULL, "EffectManager::Initialize() failed!", "ShaderManager Error", MB_OK);
        return false;
    }
    //MessageBox(NULL, "EffectManager initialized successfully", "ShaderManager Debug", MB_OK);

    // �e�V�F�[�_�[�^�C�v�̏���ݒ�
    m_Shaders[SHADER_DEFAULT].name = "Default Shader";
    m_Shaders[SHADER_DEFAULT].isCombined = true;
    m_Shaders[SHADER_DEFAULT].vsFile = "Shader/vertexShader.hlsl";
    m_Shaders[SHADER_DEFAULT].psFile = "Shader/pixelShader_default.hlsl";
    m_Shaders[SHADER_DEFAULT].vsEntry = "VertexShaderPolygon";
    m_Shaders[SHADER_DEFAULT].psEntry = "PixelShaderPolygon";

    m_Shaders[SHADER_TERRAIN].name = "FBX Shader";
    m_Shaders[SHADER_TERRAIN].isCombined = true;
    m_Shaders[SHADER_TERRAIN].vsFile = "Shader/vertexShader.hlsl";
    m_Shaders[SHADER_TERRAIN].psFile = "Shader/pixelShader_FBX.hlsl";
    m_Shaders[SHADER_TERRAIN].vsEntry = "VertexShaderPolygon";
    m_Shaders[SHADER_TERRAIN].psEntry = "PixelShaderPolygon";


    //MessageBox(NULL, "Starting shader loading...", "ShaderManager Debug", MB_OK);

    // �S�V�F�[�_�[�̃R���r�����[�h
    // �f�t�H���g�V�F�[�_�[�̓ǂݍ���
    bool success = true;
    if (!LoadCombinedShader(SHADER_DEFAULT,
        m_Shaders[SHADER_DEFAULT].vsFile.c_str(),
        m_Shaders[SHADER_DEFAULT].psFile.c_str())) {
        MessageBox(NULL, "Failed to load SHADER_DEFAULT", "ShaderManager Error", MB_OK);
        success = false;
    }

    // FBX�V�F�[�_�[�̓ǂݍ���
    if (!LoadCombinedShader(SHADER_TERRAIN,
        m_Shaders[SHADER_TERRAIN].vsFile.c_str(),
        m_Shaders[SHADER_TERRAIN].psFile.c_str())) {
        MessageBox(NULL, "Failed to load SHADER_TERRAIN", "ShaderManager Error", MB_OK);
        success = false;
    }

	

    // �f�t�H���g�V�F�[�_�[�ɐݒ�
    m_CurrentShader = SHADER_DEFAULT;

    if (success) {
        //MessageBox(NULL, "ShaderManager initialized successfully!", "ShaderManager Debug", MB_OK);
    }
    else {
        MessageBox(NULL, "ShaderManager initialization FAILED!", "ShaderManager Error", MB_OK);
    }
    
    return success;
}


// �N���[���A�b�v�F�S���\�[�X�����
void ShaderManager::Cleanup()
{
    for (auto& pair : m_Shaders)
    {
        UnloadShader(pair.first);
    }
    m_Shaders.clear();

    EffectManager::Cleanup();
}

bool ShaderManager::LoadShader(SHADER_TYPE type, const char* fileName,
    const char* vsEntry, const char* psEntry)
{
    // ���Ƀ��[�h�ς݂Ȃ�A�����[�h
    if (m_Shaders[type].isLoaded)
    {
        UnloadShader(type);
    }

    // �V�F�[�_�[����ݒ�
    m_Shaders[type].fileName = fileName;
    m_Shaders[type].vsEntry = vsEntry;
    m_Shaders[type].psEntry = psEntry;
    m_Shaders[type].isCombined = false;

    // ���̊֐����g���ă��[�h
    bool result = LoadShaderFromFile(fileName, vsEntry, psEntry, &m_Shaders[type].shader);
    m_Shaders[type].isLoaded = result;

    return result;
}


bool ShaderManager::LoadCombinedShader(SHADER_TYPE type,
    const char* vsFile,
    const char* psFile,
    const char* vsEntry,
    const char* psEntry)
{
    if (m_Shaders[type].isLoaded)
    {
        UnloadShader(type);
    }

    // �V�F�[�_�[���ݒ�
    m_Shaders[type].vsFile = vsFile;
    m_Shaders[type].psFile = psFile;
    m_Shaders[type].vsEntry = vsEntry;
    m_Shaders[type].psEntry = psEntry;
    m_Shaders[type].isCombined = true;

    // ���ʃ��[�h
    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	// �����`�F�b�N
    if (vsFile == nullptr) {
        MessageBox(NULL, "vsFile is nullptr!", "LoadCombinedShader Error", MB_OK);
        return false;
    }
    if (vsEntry == nullptr) {
        MessageBox(NULL, "vsEntry is nullptr!", "LoadCombinedShader Error", MB_OK);
        return false;
    }
    if (psFile == nullptr) {
        MessageBox(NULL, "psFile is nullptr!", "LoadCombinedShader Error", MB_OK);
        return false;
    }
    if (psEntry == nullptr) {
        MessageBox(NULL, "psEntry is nullptr!", "LoadCombinedShader Error", MB_OK);
        return false;
    }

    char debugMsg[512];
    sprintf_s(debugMsg, "Loading shaders:\nVS: %s (%s)\nPS: %s (%s)",
        vsFile, vsEntry, psFile, psEntry);
    //MessageBox(NULL, debugMsg, "LoadCombinedShader Debug", MB_OK);

	// �t�@�C�����݃`�F�b�N
    FILE* testFile = nullptr;
    if (fopen_s(&testFile, vsFile, "r") != 0 || testFile == nullptr) {
        char errorMsg[512];
        sprintf_s(errorMsg, "Cannot open vertex shader file: %s\nCurrent working directory check needed.", vsFile);
        MessageBox(NULL, errorMsg, "File Access Error", MB_OK);
        return false;
    }
    fclose(testFile);

    if (fopen_s(&testFile, psFile, "r") != 0 || testFile == nullptr) {
        char errorMsg[512];
        sprintf_s(errorMsg, "Cannot open pixel shader file: %s\nCurrent working directory check needed.", psFile);
        MessageBox(NULL, errorMsg, "File Access Error", MB_OK);
        return false;
    }
    fclose(testFile);

    //MessageBox(NULL, "Both shader files exist and are accessible", "File Check OK", MB_OK);

    // ���_�V�F�[�_�[�R���p�C��
    HRESULT hr = D3DX11CompileFromFile(vsFile, nullptr, nullptr,
        vsEntry, "vs_4_0", flags, 0, nullptr, &vsBlob, &errorBlob, nullptr);
    if (FAILED(hr)) {
        if (errorBlob) {
            MessageBox(NULL, (char*)errorBlob->GetBufferPointer(), "VS Compile Error", MB_OK);
            errorBlob->Release();
        }
        return false;
    }

    // �s�N�Z���V�F�[�_�[�R���p�C��
    hr = D3DX11CompileFromFile(psFile, nullptr, nullptr,
        psEntry, "ps_4_0", flags, 0, nullptr, &psBlob, &errorBlob, nullptr);
    if (FAILED(hr)) {
        if (errorBlob) {
            MessageBox(NULL, (char*)errorBlob->GetBufferPointer(), "PS Compile Error", MB_OK);
            errorBlob->Release();
        }
        vsBlob->Release();
        return false;
    }

    // �V�F�[�_�[�I�u�W�F�N�g�쐬
    hr = GetDevice()->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
        nullptr, &m_Shaders[type].shader.vertexShader);
    if (FAILED(hr)) {
        vsBlob->Release();
        psBlob->Release();
        return false;
    }

    hr = GetDevice()->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(),
        nullptr, &m_Shaders[type].shader.pixelShader);
    if (FAILED(hr)) {
        m_Shaders[type].shader.vertexShader->Release();
        m_Shaders[type].shader.vertexShader = nullptr;
        vsBlob->Release();
        psBlob->Release();
        return false;
    }

    // ���̓��C�A�E�g�쐬
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    hr = GetDevice()->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(), &m_Shaders[type].shader.inputLayout);
    if (FAILED(hr)) {
        m_Shaders[type].shader.vertexShader->Release();
        m_Shaders[type].shader.pixelShader->Release();
        m_Shaders[type].shader.vertexShader = nullptr;
        m_Shaders[type].shader.pixelShader = nullptr;
        vsBlob->Release();
        psBlob->Release();
        return false;
    }

    vsBlob->Release();
    psBlob->Release();

    m_Shaders[type].isLoaded = true;
    return true;
}




// �V�F�[�_�[�ă��[�h�i�J�����ɕ֗��j
bool ShaderManager::ReloadShader(SHADER_TYPE type)
{
    if (m_Shaders.find(type) == m_Shaders.end())
        return false;

    if (m_Shaders[type].isCombined)
    {
        return LoadCombinedShader(type,
            m_Shaders[type].vsFile.c_str(),
            m_Shaders[type].psFile.c_str(),
            m_Shaders[type].vsEntry.c_str(),
            m_Shaders[type].psEntry.c_str());
    }
    else
    {
        return LoadShader(type, m_Shaders[type].fileName.c_str(),
            m_Shaders[type].vsEntry.c_str(),
            m_Shaders[type].psEntry.c_str());
    }
}

// �V�F�[�_�[�A�����[�h
void ShaderManager::UnloadShader(SHADER_TYPE type)
{
    if (m_Shaders[type].isLoaded)
    {
        if (m_Shaders[type].shader.vertexShader)
        {
            m_Shaders[type].shader.vertexShader->Release();
            m_Shaders[type].shader.vertexShader = nullptr;
        }
        if (m_Shaders[type].shader.pixelShader)
        {
            m_Shaders[type].shader.pixelShader->Release();
            m_Shaders[type].shader.pixelShader = nullptr;
        }
        if (m_Shaders[type].shader.inputLayout)
        {
            m_Shaders[type].shader.inputLayout->Release();
            m_Shaders[type].shader.inputLayout = nullptr;
        }
        m_Shaders[type].isLoaded = false;
    }
}

// �V�F�[�_�[�؂�ւ�
void ShaderManager::SetShader(SHADER_TYPE type)
{
    // �f�o�b�O�I�[�o�[���C�h���L���ȏꍇ�́A�������D��
    SHADER_TYPE actualType = m_UseDebugOverride ? m_DebugOverrideShader : type;
    
    if (!IsShaderLoaded(actualType))
        return;

    m_CurrentShader = actualType;
    
    ID3D11DeviceContext* context = GetDeviceContext();
    context->IASetInputLayout(m_Shaders[actualType].shader.inputLayout);
    context->VSSetShader(m_Shaders[actualType].shader.vertexShader, nullptr, 0);
    context->PSSetShader(m_Shaders[actualType].shader.pixelShader, nullptr, 0);
}

// �f�o�b�O�I�[�o�[���C�h�ݒ�
void ShaderManager::SetDebugOverride(bool enable, SHADER_TYPE type)
{
    m_UseDebugOverride = enable;
    m_DebugOverrideShader = type;
    
    // ���݂̃V�F�[�_�[���Đݒ�i�I�[�o�[���C�h�𔽉f�j
    SetShader(m_CurrentShader);
}

// ImGui�f�o�b�O�E��
void ShaderManager::ShowShaderDebugUI()
{
    if (ImGui::Begin("Shader Manager"))
    {
        ImGui::Text("Current Shader: %s", GetShaderName(GetCurrentShader()));
        ImGui::Separator();

        ImGui::Text("Shader Selection:");
        for (int i = 0; i < SHADER_TYPE_MAX; i++)
        {
            SHADER_TYPE type = (SHADER_TYPE)i;
            bool isLoaded = IsShaderLoaded(type);
            bool isCurrent = (GetCurrentShader() == type);

            if (!isLoaded) ImGui::BeginDisabled();

            if (ImGui::RadioButton(GetShaderName(type), isCurrent))
            {
                SetShader(type);
            }

            if (!isLoaded)
            {
                ImGui::EndDisabled();
                ImGui::SameLine();
                ImGui::Text("(Not Loaded)");
            }
        }

        ImGui::Separator();

        ImGui::Text("Shader Reload:");
        for (int i = 0; i < SHADER_TYPE_MAX; i++)
        {
            SHADER_TYPE type = (SHADER_TYPE)i;
            char buttonText[64];
            sprintf_s(buttonText, "Reload %s", GetShaderName(type));

            if (ImGui::Button(buttonText))
            {
                ReloadShader(type);
            }
        }

        ImGui::Separator();

        bool useOverride = m_UseDebugOverride;
        if (ImGui::Checkbox("Force Shader Override", &useOverride))
        {
            if (!useOverride)
            {
                SetDebugOverride(false);
            }
        }

        if (useOverride)
        {
            static int overrideType = 0;
            if (ImGui::Combo("Override Type", &overrideType, "Default\0Terrain\0"))
            {
                SetDebugOverride(true, (SHADER_TYPE)overrideType);
            }
        }
    }
    ImGui::End();
}

void ShaderManager::ShowEffectDebugUI()
{
    if (ImGui::Begin("Effect Manager"))
    {
        EffectParams* params = EffectManager::GetEffectParams();

        ImGui::Text("Active Effects: 0x%X", params->effectFlags);
        ImGui::Separator();

        // �n������
        if (ImGui::CollapsingHeader("Dissolve Effect"))
        {
            bool dissolveEnabled = params->effectFlags & EFFECT_DISSOLVE;
            if (ImGui::Checkbox("Enable Dissolve", &dissolveEnabled))
            {
                if (dissolveEnabled)
                    EffectManager::EnableEffect(EFFECT_DISSOLVE);
                else
                    EffectManager::DisableEffect(EFFECT_DISSOLVE);
            }

            if (dissolveEnabled)
            {
                ImGui::SliderFloat("Dissolve Amount", &params->dissolveAmount, 0.0f, 1.0f);
                ImGui::ColorEdit4("Dissolve Color", &params->dissolveColor.x);
            }
        }

        // ��������
        if (ImGui::CollapsingHeader("Blood Stain Effect"))
        {
            bool bloodEnabled = params->effectFlags & EFFECT_BLOOD_STAIN;
            if (ImGui::Checkbox("Enable Blood Stain", &bloodEnabled))
            {
                if (bloodEnabled)
                    EffectManager::EnableEffect(EFFECT_BLOOD_STAIN);
                else
                    EffectManager::DisableEffect(EFFECT_BLOOD_STAIN);
            }

            if (bloodEnabled)
            {
                ImGui::SliderFloat("Blood Intensity", &params->bloodIntensity, 0.0f, 2.0f);
                ImGui::Text("Blood Count: %d", params->bloodCount);

                if (ImGui::Button("Add Blood Stain"))
                {
                    XMFLOAT3 pos = { 0.0f, 0.0f, 0.0f };
                    EffectManager::AddBloodStain(pos, 2.0f);
                }
                ImGui::SameLine();
                if (ImGui::Button("Clear Blood Stains"))
                {
                    EffectManager::ClearBloodStains();
                }
            }
        }

        // ��������
        if (ImGui::CollapsingHeader("Glow Effect"))
        {
            bool glowEnabled = params->effectFlags & EFFECT_GLOW;
            if (ImGui::Checkbox("Enable Glow", &glowEnabled))
            {
                if (glowEnabled)
                    EffectManager::EnableEffect(EFFECT_GLOW);
                else
                    EffectManager::DisableEffect(EFFECT_GLOW);
            }

            if (glowEnabled)
            {
                ImGui::SliderFloat("Glow Intensity", &params->customParam1.x, 0.0f, 2.0f);
                float glowColor[3] = { params->customParam1.y, params->customParam1.z, params->customParam1.w };
                if (ImGui::ColorEdit3("Glow Color", glowColor))
                {
                    params->customParam1.y = glowColor[0];
                    params->customParam1.z = glowColor[1];
                    params->customParam1.w = glowColor[2];
                }
            }
        }

        ImGui::Separator();
        if (ImGui::Button("Apply Effects"))
        {
            EffectManager::ApplyEffects();
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear All Effects"))
        {
            EffectManager::ClearAllEffects();
        }
    }
    ImGui::End();
}

// �V�F�[�_�[���擾
const char* ShaderManager::GetShaderName(SHADER_TYPE type)
{
    if (m_Shaders.find(type) != m_Shaders.end())
    {
        return m_Shaders[type].name.c_str();
    }
    return "Unknown";
}

// �V�F�[�_�[�����[�h�ς݂��`�F�b�N
bool ShaderManager::IsShaderLoaded(SHADER_TYPE type)
{
    if (m_Shaders.find(type) != m_Shaders.end())
    {
        return m_Shaders[type].isLoaded;
    }
    return false;
}




// ===== EffectManager�N���X�̐ÓI�����o�[�ϐ����` =====
EffectParams EffectManager::s_EffectParams = {};
ID3D11Buffer* EffectManager::s_EffectBuffer = nullptr;
bool EffectManager::s_IsInitialized = false;


// ===== EffectManager�N���X�̎��� =====

bool EffectManager::Initialize()
{
    //MessageBox(NULL, "EffectManager::Initialize() called", "EffectManager Debug", MB_OK);

    if (s_IsInitialized) {
        MessageBox(NULL, "EffectManager already initialized", "EffectManager Debug", MB_OK);
        return true;
    }

    // GetDevice �`�F�b�N
    if (GetDevice() == nullptr) {
        MessageBox(NULL, "GetDevice() is nullptr in EffectManager", "EffectManager Error", MB_OK);
        return false;
    }

    // ���ʃp�����[�^�[������
    ZeroMemory(&s_EffectParams, sizeof(EffectParams));
    //MessageBox(NULL, "EffectParams initialized", "EffectManager Debug", MB_OK);

    // ���ʃp�����[�^�[�p�萔�o�b�t�@�쐬
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(bufferDesc));
    bufferDesc.ByteWidth = sizeof(EffectParams);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    //MessageBox(NULL, "About to create buffer", "EffectManager Debug", MB_OK);
    HRESULT hr = GetDevice()->CreateBuffer(&bufferDesc, nullptr, &s_EffectBuffer);
    if (FAILED(hr)) {
        char errorMsg[256];
        sprintf_s(errorMsg, "CreateBuffer failed with HRESULT: 0x%08X", hr);
        MessageBox(NULL, errorMsg, "EffectManager Error", MB_OK);
        return false;
    }

    //MessageBox(NULL, "Buffer created successfully", "EffectManager Debug", MB_OK);
    s_IsInitialized = true;
    return true;
}

void EffectManager::Cleanup()
{
    if (s_EffectBuffer) {
        s_EffectBuffer->Release();
        s_EffectBuffer = nullptr;
    }
    s_IsInitialized = false;
}

void EffectManager::EnableEffect(UINT effectFlag)
{
    s_EffectParams.effectFlags |= effectFlag;
}

void EffectManager::DisableEffect(UINT effectFlag)
{
    s_EffectParams.effectFlags &= ~effectFlag;
}

void EffectManager::ClearAllEffects()
{
    s_EffectParams.effectFlags = 0;
    s_EffectParams.bloodCount = 0;
}

void EffectManager::SetDissolveEffect(float amount, XMFLOAT4 color)
{
    EnableEffect(EFFECT_DISSOLVE);
    s_EffectParams.dissolveAmount = amount;
    s_EffectParams.dissolveColor = color;
}

void EffectManager::ClearDissolveEffect()
{
    DisableEffect(EFFECT_DISSOLVE);
    s_EffectParams.dissolveAmount = 0.0f;
}

void EffectManager::AddBloodStain(XMFLOAT3 position, float radius)
{
    if (s_EffectParams.bloodCount >= 4) return; // �ő�4�܂�

    EnableEffect(EFFECT_BLOOD_STAIN);
    int index = s_EffectParams.bloodCount;

    s_EffectParams.bloodPositions[index] = XMFLOAT4(position.x, position.y, position.z, 1.0f);

    if (index == 0) s_EffectParams.bloodRadii.x = radius;
    else if (index == 1) s_EffectParams.bloodRadii.y = radius;
    else if (index == 2) s_EffectParams.bloodRadii.z = radius;
    else if (index == 3) s_EffectParams.bloodRadii.w = radius;
    s_EffectParams.bloodCount++;
}

void EffectManager::ClearBloodStains()
{
    DisableEffect(EFFECT_BLOOD_STAIN);
    s_EffectParams.bloodCount = 0;
}

void EffectManager::SetBloodIntensity(float intensity)
{
    s_EffectParams.bloodIntensity = intensity;
}

void EffectManager::SetGlowEffect(float intensity, XMFLOAT3 color)
{
    EnableEffect(EFFECT_GLOW);
    s_EffectParams.customParam1.x = intensity;
    s_EffectParams.customParam1.y = color.x;
    s_EffectParams.customParam1.z = color.y;
    s_EffectParams.customParam1.w = color.z;
}

void EffectManager::SetDamageFlash(float intensity)
{
    EnableEffect(EFFECT_DAMAGE);
    s_EffectParams.customParam2.x = intensity;
}

void EffectManager::SetCustomParam1(XMFLOAT4 param)
{
    s_EffectParams.customParam1 = param;
}

void EffectManager::SetCustomParam2(XMFLOAT4 param)
{
    s_EffectParams.customParam2 = param;
}

void EffectManager::ApplyEffects()
{
    if (!s_IsInitialized) return;

    GetDeviceContext()->UpdateSubresource(s_EffectBuffer, 0, nullptr, &s_EffectParams, 0, 0);
    GetDeviceContext()->VSSetConstantBuffers(8, 1, &s_EffectBuffer);
    GetDeviceContext()->PSSetConstantBuffers(8, 1, &s_EffectBuffer);
}
