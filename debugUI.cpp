#include "debugUI.h"
#include "imgui.h"
#include "camera.h"
#include "player.h"
#include "FBXmodel.h"

#include "enemy.h"
#include "item.h"
#include <fstream>
#include <nlohmann/json.hpp>

#include "dissolveTest.h"


 // item.cpp�ɂ���A�C�e���z��
#define MAX_ITEM (128)


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

    //�G�l�~�[�G�f�B�^�[
    if (ImGui::CollapsingHeader(u8"�G�l�~�[�z�u�G�f�B�^�["))
    {
        auto& enemies = GetEnemies();
        static int selectedEnemyID = 0;

        ImGui::InputInt(u8"�ǉ��G�l�~�[ID", &selectedEnemyID);
        for (int i = 0; i < enemies.size(); ++i)
        {
            BaseEnemy* e = enemies[i];
            if (!e->IsUsed()) continue;

            XMFLOAT3 pos = e->GetPosition();

            ImGui::PushID(i);
            if (ImGui::DragFloat3(u8"�ʒu", (float*)&pos, 0.5f)) {
                e->SetPosition(pos);
            }
            if (ImGui::Button(u8"�폜")) {
                e->SetUsed(false);
            }
            ImGui::Separator();
            ImGui::PopID();
        }

        const char* enemyTypes[] = { "SPIDER", "GHOST" };
        ImGui::Combo(u8"�ǉ��G�l�~�[�^�C�v", &selectedEnemyID, enemyTypes, IM_ARRAYSIZE(enemyTypes));
        static XMFLOAT3 newEnemyPos = { 0.0f, 0.0f, 0.0f };
        ImGui::DragFloat3(u8"�ǉ��G�l�~�[�ʒu", (float*)&newEnemyPos, 0.1f);
        if (ImGui::Button(u8"�G�l�~�[�ǉ�")) {
            EnemySpawner(newEnemyPos, selectedEnemyID);
        }

        if (ImGui::Button(u8"�ۑ�")) {
            SaveEnemyData("enemy_data.json");
        }
        ImGui::SameLine();
        if (ImGui::Button(u8"�Ǎ�")) {
            LoadEnemyData("enemy_data.json");
        }
    }


    //�A�C�e���G�f�B�^�[
    if (ImGui::CollapsingHeader(u8"�A�C�e���z�u�G�f�B�^�["))
    {
        ITEM_OBJ* itemObj = GetItemOBJ();

        for (int i = 0; i < MAX_ITEM; ++i)
        {
            if (!itemObj[i].use) continue;

            ImGui::PushID(i);
            ImGui::Text("ID: %d (%s)", itemObj[i].item.id, itemObj[i].item.name.c_str());
            ImGui::DragFloat3(u8"�ʒu", (float*)&itemObj[i].pos, 0.5f);
            ImGui::DragFloat3(u8"�T�C�Y", (float*)&itemObj[i].scl, 0.1f);
            if (ImGui::Button(u8"�폜")) {
                itemObj[i].use = false;
            }
            ImGui::Separator();
            ImGui::PopID();
        }

        static int selectedItemID = 0;
        ImGui::InputInt(u8"�ǉ��A�C�e��ID", &selectedItemID);
        if (ImGui::Button(u8"�A�C�e���ǉ�"))
        {
            CAMERA* cam = GetCamera();
            SetItem(cam->pos, selectedItemID);
        }

        if (ImGui::Button(u8"�ۑ�")) {
            SaveItemData("item_data.json");
        }
        ImGui::SameLine();
        if (ImGui::Button(u8"�Ǎ�")) {
            LoadItemData("item_data.json");
        }
    }

	//Shader�G�f�B�^�[
    if (ImGui::CollapsingHeader(u8"�V�F�[�_�[�G�f�B�^�["))
    {
		DissolveTest* dissolveTest = GetDissolveTest();
		ImGui::Checkbox(u8"�f�B�]���u�L��", &dissolveTest->isDissolving);
        ImGui::DragFloat(u8"�f�B�]���u�l", &dissolveTest->dissolve, 0.01f, 0.0f, 1.0f, "%.2f");
        
    }



    ImGui::End();
}