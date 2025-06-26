#include "debugUI.h"
#include "imgui.h"
#include "camera.h"
#include "player.h"
#include "FBXmodel.h"





extern BOOL g_bPause;


void ShowDebugUI()
{
    ImGui::Begin("Debug Menu");

    ImGui::Checkbox(u8"ゲームを停止", (bool*)&g_bPause);

	//プレイヤーの制御
    if (ImGui::CollapsingHeader(u8"プレイヤー制御"))
    {
        PLAYER* player = GetPlayer();
        ImGui::DragFloat3(u8"プレイヤー位置", (float*)&player->pos, 0.5f);
        
        ImGui::SliderFloat3(u8"プレイヤー回転", (float*)&player->rot, -XM_PI, XM_PI);
        
		ImGui::SliderFloat(u8"移動速度", &player->spd, 0.0f, 20.0f);
        ImGui::InputFloat(u8"速度入力", &player->spd, 0.1f, 1.0f, "%.2f");

	}

	//カメラの制御
    if (ImGui::CollapsingHeader(u8"カメラ制御"))
    {
        CAMERA* cam = GetCamera();

        ImGui::Checkbox(u8"第一人称視点 (Tabキーでも切替可)", &isFirstPersonMode);

        ImGui::SliderFloat(u8"マウス感度", &sensitivity, 0.0001f, 0.005f, "%.4f");

        ImGui::DragFloat3(u8"カメラ座標", (float*)&cam->pos, 0.5f);
        
        ImGui::SliderFloat3(u8"カメラ回転", (float*)&cam->rot, -XM_PI, XM_PI);

        if (ImGui::SliderFloat(u8"カメラ距離(Playerから)", &cam->len, 10.0f, 500.0f))
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

    //モデルエディター
    if (ImGui::CollapsingHeader(u8"モデルエディター"))
    {
		FBXTESTMODEL* FBXModel = GetFBXTestModel();

        ImGui::DragFloat3(u8"位置", (float*)&FBXModel->pos,  0.5f);
        ImGui::SliderFloat3(u8"回転", (float*)&FBXModel->rot, -XM_PI, XM_PI);
        ImGui::SliderFloat3(u8"サイズ", (float*)&FBXModel->scl, 0, 100.0f);
        ImGui::InputFloat3(u8"サイズ入力", (float*)&FBXModel->scl, "%.2f");

    }



    ImGui::End();
}