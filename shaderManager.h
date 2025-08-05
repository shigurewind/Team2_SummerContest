#pragma once
#include "main.h"
#include "renderer.h"
#include <map>
#include <string>

// �V�F�[�_�[�^�C�v�񋓌^ - �v���W�F�N�g�Ŏg�p����S�ẴV�F�[�_�[�^�C�v���`
enum SHADER_TYPE
{
    SHADER_DEFAULT = 0,    // �f�t�H���g�V�F�[�_�[ (shader.hlsl)
    SHADER_FBX,           // FBX���f����p (testShader.hlsl)  
    SHADER_TYPE_MAX       // �ő�l�A���[�v�p
};

// ����SHADER�\���͕̂ύX�Ȃ��i����݊����j
struct SHADER
{
    ID3D11VertexShader* vertexShader;
    ID3D11PixelShader* pixelShader;
    ID3D11InputLayout* inputLayout;
};

// �V�K�F�ڍ׏����܂ރV�F�[�_�[���\����
struct ShaderInfo
{
    SHADER shader;              // ���ۂ̃V�F�[�_�[�I�u�W�F�N�g
    std::string name;           // �\�����i�f�o�b�O�E�ʗp�j
    std::string fileName;       // �t�@�C���p�X
    std::string vsEntry;        // ���_�V�F�[�_�[�G���g���֐���
    std::string psEntry;        // �s�N�Z���V�F�[�_�[�G���g���֐���
    bool isLoaded;             // ���[�h�ς݂��ǂ���
};

// ===== ���̊֐��͕s�ρi����݊����j =====
bool LoadShaderFromFile(const char* fileName,const char* vsEntry,const char* psEntry,SHADER* outShader);

// ===== �V�K�ǉ��̃V�F�[�_�[�}�l�[�W���[�N���X =====
class ShaderManager
{
private:
    static std::map<SHADER_TYPE, ShaderInfo> m_Shaders;  // �S�V�F�[�_�[�����i�[
    static SHADER_TYPE m_CurrentShader;                   // ���ݎg�p���̃V�F�[�_�[
    static bool m_UseDebugOverride;                       // �f�o�b�O�I�[�o�[���C�h���g�p���邩
    static SHADER_TYPE m_DebugOverrideShader;            // �f�o�b�O�I�[�o�[���C�h�̃V�F�[�_�[�^�C�v

public:
    // ===== �������ƏI������ =====
    static bool Initialize();     // �}�l�[�W���[�������A�S�V�F�[�_�[�����[�h
    static void Cleanup();        // �S���\�[�X���N���[���A�b�v
    
    // ===== �V�F�[�_�[���[�h�Ǘ� =====
    static bool LoadShader(SHADER_TYPE type, const char* fileName, 
                          const char* vsEntry = "VertexShaderPolygon", 
                          const char* psEntry = "PixelShaderPolygon");
    static bool ReloadShader(SHADER_TYPE type);  // �V�F�[�_�[�ă��[�h�i�J�����֗��j
    static void UnloadShader(SHADER_TYPE type);  // �V�F�[�_�[�A�����[�h
    
    // ===== �V�F�[�_�[�g�p =====
    static void SetShader(SHADER_TYPE type);             // �w��V�F�[�_�[�ɐ؂�ւ�
    static void SetDefaultShader() { SetShader(SHADER_DEFAULT); }  // �f�t�H���g�V�F�[�_�[�ɐ؂�ւ�
    static SHADER_TYPE GetCurrentShader() { return m_CurrentShader; }  // ���݂̃V�F�[�_�[���擾
    
    // ===== �f�o�b�O�@�\ =====
    static void SetDebugOverride(bool enable, SHADER_TYPE type = SHADER_DEFAULT);  // ����V�F�[�_�[�������g�p
    static bool IsDebugOverrideEnabled() { return m_UseDebugOverride; }
    
    // ===== ImGui�f�o�b�O�E�� =====
    static void ShowShaderDebugUI();  // �V�F�[�_�[�f�o�b�O�E�ʂ�\��
    
    // ===== ���[�e�B���e�B�֐� =====
    static const char* GetShaderName(SHADER_TYPE type);    // �V�F�[�_�[�\�������擾
    static bool IsShaderLoaded(SHADER_TYPE type);          // �V�F�[�_�[�����[�h�ς݂��`�F�b�N
    
    // ===== ���������@�\ =====
    // ���̃N���X�̓V�F�[�_�[�̎��������Ɏg�p�ARAII�p�^�[��
    // �I�u�W�F�N�g�쐬���ɃV�F�[�_�[�؂�ւ��A�j�����ɑO�̃V�F�[�_�[�Ɏ�������
    class ShaderScope
    {
    private:
        SHADER_TYPE m_PrevShader;  // �O�̃V�F�[�_�[��ۑ�
    public:
        // �R���X�g���N�^�F�V�����V�F�[�_�[�ɐ؂�ւ��A�O�̃V�F�[�_�[���L��
        ShaderScope(SHADER_TYPE newShader) : m_PrevShader(GetCurrentShader()) 
        { 
            SetShader(newShader); 
        }
        
        // �f�X�g���N�^�F�O�̃V�F�[�_�[�Ɏ�������
        ~ShaderScope() 
        { 
            SetShader(m_PrevShader); 
        }
    };
};

// �֗��}�N���F�V�F�[�_�[�؂�ւ��������Ǘ�
// �g�p���@�F�ꎞ�I�ɃV�F�[�_�[��؂�ւ������R�[�h�u���b�N�̐擪�� SHADER_SCOPE(SHADER_FBX); �Ə���
// �R�[�h�u���b�N�I�����ɑO�̃V�F�[�_�[�Ɏ������������
#define SHADER_SCOPE(type) ShaderManager::ShaderScope _scope(type)

