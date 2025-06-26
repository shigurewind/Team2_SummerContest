#include "debugUI.h"
#include "imgui.h"
#include "camera.h"
#include "player.h"
#include "FBXmodel.h"





extern BOOL g_bPause;


void ShowDebugUI()
{
    ImGui::Begin("Debug Menu");

    ImGui::Checkbox(u8"�Q�[�����~", (bool*)&g_bPause);

	//�v���C���[�̐���
    if (ImGui::CollapsingHeader(u8"�v���C���[����"))
    {
        PLAYER* player = GetPlayer();
        ImGui::DragFloat3(u8"�v���C���[�ʒu", (float*)&player->pos, 0.5f);
        
        ImGui::SliderFloat3(u8"�v���C���[��]", (float*)&player->rot, -XM_PI, XM_PI);
        
		ImGui::SliderFloat(u8"�ړ����x", &player->spd, 0.0f, 20.0f);
        ImGui::InputFloat(u8"���x����", &player->spd, 0.1f, 1.0f, "%.2f");

	}

	//�J�����̐���
    if (ImGui::CollapsingHeader(u8"�J��������"))
    {
        CAMERA* cam = GetCamera();

        ImGui::Checkbox(u8"���l�̎��_ (Tab�L�[�ł��ؑ։�)", &isFirstPersonMode);

        ImGui::SliderFloat(u8"�}�E�X���x", &sensitivity, 0.0001f, 0.005f, "%.4f");

        ImGui::DragFloat3(u8"�J�������W", (float*)&cam->pos, 0.5f);
        
        ImGui::SliderFloat3(u8"�J������]", (float*)&cam->rot, -XM_PI, XM_PI);

        if (ImGui::SliderFloat(u8"�J��������(Player����)", &cam->len, 10.0f, 500.0f))
        {
            PLAYER* player = GetPlayer();
            cam->at = player->pos;

            cam->pos.x = cam->at.x - sinf(cam->rot.y) * cosf(cam->rot.x) * cam->len;
            cam->pos.y = cam->at.y - sinf(cam->rot.x) * cam->len;
            cam->pos.z = cam->at.z - cosf(cam->rot.y) * cosf(cam->rot.x) * cam->len;
        }

        if (ImGui::Button("Reset Camera"))
        {
            UninitCamera();
            InitCamera();
        }
    }

    //���f���G�f�B�^�[
    if (ImGui::CollapsingHeader(u8"���f���G�f�B�^�["))
    {
		FBXTESTMODEL* FBXModel = GetFBXTestModel();

        ImGui::DragFloat3(u8"�ʒu", (float*)&FBXModel->pos,  0.5f);
        ImGui::SliderFloat3(u8"��]", (float*)&FBXModel->rot, -XM_PI, XM_PI);
        ImGui::SliderFloat3(u8"�T�C�Y", (float*)&FBXModel->scl, 0, 100.0f);
        ImGui::InputFloat3(u8"�T�C�Y����", (float*)&FBXModel->scl, "%.2f");

    }



    ImGui::End();
}