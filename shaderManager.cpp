#include "shaderManager.h"
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
    HRESULT hr = D3DX11CompileFromFile(fileName, nullptr, nullptr, vsEntry, "vs_4_0", flags, 0, nullptr, &vsBlob, &errorBlob, nullptr);
    if (FAILED(hr)) {
        MessageBox(NULL, (char*)errorBlob->GetBufferPointer(), "VS Compile Error", MB_OK);
        return false;
    }

    // ピクセルシェーダーコンパイル
    hr = D3DX11CompileFromFile(fileName, nullptr, nullptr, psEntry, "ps_4_0", flags, 0, nullptr, &psBlob, &errorBlob, nullptr);
    if (FAILED(hr)) {
        MessageBox(NULL, (char*)errorBlob->GetBufferPointer(), "PS Compile Error", MB_OK);
        vsBlob->Release();
        return false;
    }

    // シェーダー作成
    GetDevice()->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &outShader->vertexShader);
    GetDevice()->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &outShader->pixelShader);

    // レイアウト作成
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

// ===== ShaderManagerクラスの実装 =====

// 初期化：全てのシェーダーをロード
bool ShaderManager::Initialize()
{
    // 各シェーダータイプの情報を設定
    m_Shaders[SHADER_DEFAULT].name = "Default Shader";
    m_Shaders[SHADER_DEFAULT].fileName = "Shader/shader.hlsl";
    m_Shaders[SHADER_DEFAULT].vsEntry = "VertexShaderPolygon";
    m_Shaders[SHADER_DEFAULT].psEntry = "PixelShaderPolygon";
    
    m_Shaders[SHADER_FBX].name = "FBX Shader";
    m_Shaders[SHADER_FBX].fileName = "Shader/testShader.hlsl";
    m_Shaders[SHADER_FBX].vsEntry = "VertexShaderPolygon";
    m_Shaders[SHADER_FBX].psEntry = "PixelShaderPolygon";

    // 全シェーダーをロード
    bool success = true;
    success &= LoadShader(SHADER_DEFAULT, m_Shaders[SHADER_DEFAULT].fileName.c_str());
    success &= LoadShader(SHADER_FBX, m_Shaders[SHADER_FBX].fileName.c_str());

    // デフォルトシェーダーに設定
    m_CurrentShader = SHADER_DEFAULT;
    
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
}

// シェーダーロード
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

    // 元の関数を使ってロード
    bool result = LoadShaderFromFile(fileName, vsEntry, psEntry, &m_Shaders[type].shader);
    m_Shaders[type].isLoaded = result;

    return result;
}

// シェーダー再ロード（開発時に便利）
bool ShaderManager::ReloadShader(SHADER_TYPE type)
{
    if (m_Shaders.find(type) == m_Shaders.end())
        return false;

    return LoadShader(type, m_Shaders[type].fileName.c_str(), 
                      m_Shaders[type].vsEntry.c_str(), 
                      m_Shaders[type].psEntry.c_str());
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
        // 現在のシェーダー表示
        ImGui::Text("Current Shader: %s", GetShaderName(GetCurrentShader()));
        ImGui::Separator();

        // シェーダー切り替えボタン
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

        // シェーダー再ロードボタン
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

        // デバッグオーバーライド
        ImGui::Text("Debug Override:");
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
            if (ImGui::Combo("Override Type", &overrideType, "Default\0FBX\0"))
            {
                SetDebugOverride(true, (SHADER_TYPE)overrideType);
            }
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

