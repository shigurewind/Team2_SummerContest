#include "shaderManager.h"
#include "renderer.h"
#include "imgui.h"

// ===== ShaderManagerクラスの静的メンバー変数を定義 =====
std::map<SHADER_TYPE, ShaderInfo> ShaderManager::m_Shaders;
SHADER_TYPE ShaderManager::m_CurrentShader = SHADER_DEFAULT;
bool ShaderManager::m_UseDebugOverride = false;
SHADER_TYPE ShaderManager::m_DebugOverrideShader = SHADER_DEFAULT;

// ===== 元の関数は変更なし（後方互換性を保つ） =====
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

    // 頂点シェーダーコンパイル
    HRESULT hr = D3DX11CompileFromFile(fileName, nullptr, nullptr, vsEntry, "vs_4_0", flags, 0, nullptr, &vsBlob,
        &errorBlob, nullptr);
    if (FAILED(hr)) {
        if (errorBlob) {
            MessageBox(NULL, (char*)errorBlob->GetBufferPointer(), "VS Compile Error", MB_OK);
            errorBlob->Release();
        }
        return false;
    }

    // ピクセルシェーダーコンパイル
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

    // シェーダー作成
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

    // レイアウト作成
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



// ===== ShaderManagerクラスの実装 =====

// 初期化：全てのシェーダーをロード
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

    // 効果管理初期化
    if (!EffectManager::Initialize()) {
        MessageBox(NULL, "EffectManager::Initialize() failed!", "ShaderManager Error", MB_OK);
        return false;
    }
    //MessageBox(NULL, "EffectManager initialized successfully", "ShaderManager Debug", MB_OK);

    // 各シェーダータイプの情報を設定
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

    // 全シェーダーのコンビをロード
    // デフォルトシェーダーの読み込み
    bool success = true;
    if (!LoadCombinedShader(SHADER_DEFAULT,
        m_Shaders[SHADER_DEFAULT].vsFile.c_str(),
        m_Shaders[SHADER_DEFAULT].psFile.c_str())) {
        MessageBox(NULL, "Failed to load SHADER_DEFAULT", "ShaderManager Error", MB_OK);
        success = false;
    }

    // FBXシェーダーの読み込み
    if (!LoadCombinedShader(SHADER_TERRAIN,
        m_Shaders[SHADER_TERRAIN].vsFile.c_str(),
        m_Shaders[SHADER_TERRAIN].psFile.c_str())) {
        MessageBox(NULL, "Failed to load SHADER_TERRAIN", "ShaderManager Error", MB_OK);
        success = false;
    }

	

    // デフォルトシェーダーに設定
    m_CurrentShader = SHADER_DEFAULT;

    if (success) {
        //MessageBox(NULL, "ShaderManager initialized successfully!", "ShaderManager Debug", MB_OK);
    }
    else {
        MessageBox(NULL, "ShaderManager initialization FAILED!", "ShaderManager Error", MB_OK);
    }
    
    return success;
}


// クリーンアップ：全リソースを解放
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
    // 既にロード済みならアンロード
    if (m_Shaders[type].isLoaded)
    {
        UnloadShader(type);
    }

    // シェーダー情報を設定
    m_Shaders[type].fileName = fileName;
    m_Shaders[type].vsEntry = vsEntry;
    m_Shaders[type].psEntry = psEntry;
    m_Shaders[type].isCombined = false;

    // 元の関数を使ってロード
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

    // シェーダー情報設定
    m_Shaders[type].vsFile = vsFile;
    m_Shaders[type].psFile = psFile;
    m_Shaders[type].vsEntry = vsEntry;
    m_Shaders[type].psEntry = psEntry;
    m_Shaders[type].isCombined = true;

    // 分別ロード
    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	// 引数チェック
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

	// ファイル存在チェック
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

    // 頂点シェーダーコンパイル
    HRESULT hr = D3DX11CompileFromFile(vsFile, nullptr, nullptr,
        vsEntry, "vs_4_0", flags, 0, nullptr, &vsBlob, &errorBlob, nullptr);
    if (FAILED(hr)) {
        if (errorBlob) {
            MessageBox(NULL, (char*)errorBlob->GetBufferPointer(), "VS Compile Error", MB_OK);
            errorBlob->Release();
        }
        return false;
    }

    // ピクセルシェーダーコンパイル
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

    // シェーダーオブジェクト作成
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

    // 入力レイアウト作成
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




// シェーダー再ロード（開発時に便利）
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

// シェーダーアンロード
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

// シェーダー切り替え
void ShaderManager::SetShader(SHADER_TYPE type)
{
    // デバッグオーバーライドが有効な場合は、そちらを優先
    SHADER_TYPE actualType = m_UseDebugOverride ? m_DebugOverrideShader : type;
    
    if (!IsShaderLoaded(actualType))
        return;

    m_CurrentShader = actualType;
    
    ID3D11DeviceContext* context = GetDeviceContext();
    context->IASetInputLayout(m_Shaders[actualType].shader.inputLayout);
    context->VSSetShader(m_Shaders[actualType].shader.vertexShader, nullptr, 0);
    context->PSSetShader(m_Shaders[actualType].shader.pixelShader, nullptr, 0);
}

// デバッグオーバーライド設定
void ShaderManager::SetDebugOverride(bool enable, SHADER_TYPE type)
{
    m_UseDebugOverride = enable;
    m_DebugOverrideShader = type;
    
    // 現在のシェーダーを再設定（オーバーライドを反映）
    SetShader(m_CurrentShader);
}

// ImGuiデバッグ界面
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

        // 溶解効果
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

        // 血痕効果
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
                ImGui::SliderFloat("Blood Intensity", &params->bloodIntensity, 0.0f, 4.0f);
                ImGui::Text("Blood Count: %d", params->bloodCount);

                ImGui::Separator();
                ImGui::Text("Blood Splatter Parameters:");
                ImGui::SliderFloat("Main Blood Radius", &params->customParam1.x, 10.0f, 100.0f);
                ImGui::SliderFloat("Splatter Radius", &params->customParam1.y, 5.0f, 50.0f);
                ImGui::SliderFloat("Splatter Distance", &params->customParam1.z, 5.0f, 80.0f);
                ImGui::SliderFloat("Splatter Intensity", &params->customParam1.w, 0.1f, 1.0f);
                
                ImGui::Separator();

                if (ImGui::Button("Clear Blood Stains"))
                {
                    EffectManager::ClearBloodStains();
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

// シェーダー名取得
const char* ShaderManager::GetShaderName(SHADER_TYPE type)
{
    if (m_Shaders.find(type) != m_Shaders.end())
    {
        return m_Shaders[type].name.c_str();
    }
    return "Unknown";
}

// シェーダーがロード済みかチェック
bool ShaderManager::IsShaderLoaded(SHADER_TYPE type)
{
    if (m_Shaders.find(type) != m_Shaders.end())
    {
        return m_Shaders[type].isLoaded;
    }
    return false;
}




// ===== EffectManagerクラスの静的メンバー変数を定義 =====
EffectParams EffectManager::s_EffectParams = {};
ID3D11Buffer* EffectManager::s_EffectBuffer = nullptr;
bool EffectManager::s_IsInitialized = false;


// ===== EffectManagerクラスの実装 =====

bool EffectManager::Initialize()
{
    //MessageBox(NULL, "EffectManager::Initialize() called", "EffectManager Debug", MB_OK);

    if (s_IsInitialized) {
        MessageBox(NULL, "EffectManager already initialized", "EffectManager Debug", MB_OK);
        return true;
    }

    // GetDevice チェック
    if (GetDevice() == nullptr) {
        MessageBox(NULL, "GetDevice() is nullptr in EffectManager", "EffectManager Error", MB_OK);
        return false;
    }

    // 効果パラメーター初期化
    ZeroMemory(&s_EffectParams, sizeof(EffectParams));
    //MessageBox(NULL, "EffectParams initialized", "EffectManager Debug", MB_OK);

	s_EffectParams.bloodIntensity = 1.0f;// 血痕のデフォルト強度（掛け算なので）

    // 効果パラメーター用定数バッファ作成
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

	//血痕パラメーター初期化
    s_EffectParams.customParam1.x = 50.0f;  // 半径
    s_EffectParams.customParam1.y = 30.0f;  // 拡散半径
    s_EffectParams.customParam1.z = 30.0f;  // 拡散距離
    s_EffectParams.customParam1.w = 0.5f;   // 強度係数


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
    XMFLOAT3 defaultProjection = { 0.0f, 1.0f, 0.0f }; // 上
    AddBloodProjection(position, radius, defaultProjection, 1.0f);
}

void EffectManager::ClearBloodStains()
{
    DisableEffect(EFFECT_BLOOD_STAIN);
    s_EffectParams.bloodCount = 0;

    for (int i = 0; i < 8; i++) {
        s_EffectParams.bloodPositions[i] = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
        s_EffectParams.bloodProjections[i] = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    }

    s_EffectParams.bloodRadii[0] = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    s_EffectParams.bloodRadii[1] = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

    ApplyEffects();

}

void EffectManager::SetBloodIntensity(float intensity)
{
    s_EffectParams.bloodIntensity = intensity;
}

//血痕投影の追加
void EffectManager::AddBloodProjection(XMFLOAT3 position, float radius,
    XMFLOAT3 projectionDirection, float intensity)
{
    
    EnableEffect(EFFECT_BLOOD_STAIN);
    int index = s_EffectParams.bloodCount;

	// 位置と半径を保存
    if (s_EffectParams.bloodCount < 8) {
        index = s_EffectParams.bloodCount;
        s_EffectParams.bloodCount++;
    }
    else {
		// 8個を超えたら古いものから上書き
        static int replaceIndex = 0;
        index = replaceIndex;
        replaceIndex = (replaceIndex + 1) % 8;  // loop
    }

	// 投影方向と強度を保存
    s_EffectParams.bloodPositions[index] = XMFLOAT4(position.x, position.y, position.z, 1.0f);
    s_EffectParams.bloodProjections[index] = XMFLOAT4(
        projectionDirection.x, projectionDirection.y, projectionDirection.z, intensity
    );

	// 半径を保存
    if (index < 4) {
        if (index == 0) s_EffectParams.bloodRadii[0].x = radius;
        else if (index == 1) s_EffectParams.bloodRadii[0].y = radius;
        else if (index == 2) s_EffectParams.bloodRadii[0].z = radius;
        else if (index == 3) s_EffectParams.bloodRadii[0].w = radius;
    }
    else {
        if (index == 4) s_EffectParams.bloodRadii[1].x = radius;
        else if (index == 5) s_EffectParams.bloodRadii[1].y = radius;
        else if (index == 6) s_EffectParams.bloodRadii[1].z = radius;
        else if (index == 7) s_EffectParams.bloodRadii[1].w = radius;
    }
}


void EffectManager::CreateBloodSplatter(XMFLOAT3 hitPos, XMFLOAT3 hitNormal,
    XMFLOAT3 bulletDirection, float intensity)
{
    float mainRadius = s_EffectParams.customParam1.x;        
    float splatterRadius = s_EffectParams.customParam1.y;    
    float splatterDistance = s_EffectParams.customParam1.z;  
    float splatterIntensity = s_EffectParams.customParam1.w;

	// ヒット位置に血痕投影を追加
    AddBloodProjection(hitPos, mainRadius, hitNormal, intensity);

    for (int i = 0; i < 2; i++) {
		// ランダムな方向を生成
        XMFLOAT3 splatterDir = {
            bulletDirection.x + ((rand() % 200 - 100) / 500.0f), // ±0.2
            bulletDirection.y + ((rand() % 200 - 100) / 500.0f),
            bulletDirection.z + ((rand() % 200 - 100) / 500.0f)
        };

        // 正規化
        XMVECTOR splatterVec = XMVector3Normalize(XMLoadFloat3(&splatterDir));
        XMStoreFloat3(&splatterDir, splatterVec);

		// ヒットポイントからランダムな距離に飛ばす
        float randomDistance = splatterDistance * (0.3f + (rand() % 70) / 100.0f); // 30%-100%
        XMFLOAT3 splatterPos = {
            hitPos.x + splatterDir.x * randomDistance,
            hitPos.y + splatterDir.y * randomDistance,
            hitPos.z + splatterDir.z * randomDistance
        };

        float randomRadius = splatterRadius * (0.5f + (rand() % 50) / 100.0f); // 50%-100%
        float randomIntensity = intensity * splatterIntensity * (0.5f + (rand() % 50) / 100.0f);

        AddBloodProjection(splatterPos, randomRadius, splatterDir, randomIntensity);
    }

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
