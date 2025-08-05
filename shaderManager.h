#pragma once
#include "main.h"
#include "renderer.h"
#include <map>
#include <string>

// シェーダータイプ列挙型 - プロジェクトで使用する全てのシェーダータイプを定義
enum SHADER_TYPE
{
    SHADER_DEFAULT = 0,    // デフォルトシェーダー (shader.hlsl)
    SHADER_FBX,           // FBXモデル専用 (testShader.hlsl)  
    SHADER_TYPE_MAX       // 最大値、ループ用
};

// 元のSHADER構造体は変更なし（後方互換性）
struct SHADER
{
    ID3D11VertexShader* vertexShader;
    ID3D11PixelShader* pixelShader;
    ID3D11InputLayout* inputLayout;
};

// 新規：詳細情報を含むシェーダー情報構造体
struct ShaderInfo
{
    SHADER shader;              // 実際のシェーダーオブジェクト
    std::string name;           // 表示名（デバッグ界面用）
    std::string fileName;       // ファイルパス
    std::string vsEntry;        // 頂点シェーダーエントリ関数名
    std::string psEntry;        // ピクセルシェーダーエントリ関数名
    bool isLoaded;             // ロード済みかどうか
};

// ===== 元の関数は不変（後方互換性） =====
bool LoadShaderFromFile(const char* fileName,const char* vsEntry,const char* psEntry,SHADER* outShader);

// ===== 新規追加のシェーダーマネージャークラス =====
class ShaderManager
{
private:
    static std::map<SHADER_TYPE, ShaderInfo> m_Shaders;  // 全シェーダー情報を格納
    static SHADER_TYPE m_CurrentShader;                   // 現在使用中のシェーダー
    static bool m_UseDebugOverride;                       // デバッグオーバーライドを使用するか
    static SHADER_TYPE m_DebugOverrideShader;            // デバッグオーバーライドのシェーダータイプ

public:
    // ===== 初期化と終了処理 =====
    static bool Initialize();     // マネージャー初期化、全シェーダーをロード
    static void Cleanup();        // 全リソースをクリーンアップ
    
    // ===== シェーダーロード管理 =====
    static bool LoadShader(SHADER_TYPE type, const char* fileName, 
                          const char* vsEntry = "VertexShaderPolygon", 
                          const char* psEntry = "PixelShaderPolygon");
    static bool ReloadShader(SHADER_TYPE type);  // シェーダー再ロード（開発時便利）
    static void UnloadShader(SHADER_TYPE type);  // シェーダーアンロード
    
    // ===== シェーダー使用 =====
    static void SetShader(SHADER_TYPE type);             // 指定シェーダーに切り替え
    static void SetDefaultShader() { SetShader(SHADER_DEFAULT); }  // デフォルトシェーダーに切り替え
    static SHADER_TYPE GetCurrentShader() { return m_CurrentShader; }  // 現在のシェーダーを取得
    
    // ===== デバッグ機能 =====
    static void SetDebugOverride(bool enable, SHADER_TYPE type = SHADER_DEFAULT);  // 特定シェーダーを強制使用
    static bool IsDebugOverrideEnabled() { return m_UseDebugOverride; }
    
    // ===== ImGuiデバッグ界面 =====
    static void ShowShaderDebugUI();  // シェーダーデバッグ界面を表示
    
    // ===== ユーティリティ関数 =====
    static const char* GetShaderName(SHADER_TYPE type);    // シェーダー表示名を取得
    static bool IsShaderLoaded(SHADER_TYPE type);          // シェーダーがロード済みかチェック
    
    // ===== 自動復元機能 =====
    // このクラスはシェーダーの自動復元に使用、RAIIパターン
    // オブジェクト作成時にシェーダー切り替え、破棄時に前のシェーダーに自動復元
    class ShaderScope
    {
    private:
        SHADER_TYPE m_PrevShader;  // 前のシェーダーを保存
    public:
        // コンストラクタ：新しいシェーダーに切り替え、前のシェーダーを記憶
        ShaderScope(SHADER_TYPE newShader) : m_PrevShader(GetCurrentShader()) 
        { 
            SetShader(newShader); 
        }
        
        // デストラクタ：前のシェーダーに自動復元
        ~ShaderScope() 
        { 
            SetShader(m_PrevShader); 
        }
    };
};

// 便利マクロ：シェーダー切り替えを自動管理
// 使用方法：一時的にシェーダーを切り替えたいコードブロックの先頭で SHADER_SCOPE(SHADER_FBX); と書く
// コードブロック終了時に前のシェーダーに自動復元される
#define SHADER_SCOPE(type) ShaderManager::ShaderScope _scope(type)

