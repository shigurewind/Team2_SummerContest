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


// item.cppにあるアイテム配列
#define MAX_ITEM (128)


extern BOOL g_bPause;


void ShowDebugUI()
{
	ImGui::Begin("Debug Menu");

	ImGui::Checkbox(u8"ゲームを停止", (bool*)&g_bPause);

	//プレイヤーの制御
	if (ImGui::CollapsingHeader(u8"プレイヤー制御"))
	{

		ImGui::DragFloat3(u8"プレイヤー位置", (float*)&GetPlayer()->GetPosition(), 0.5f);

		ImGui::SliderFloat3(u8"プレイヤー回転", (float*)&GetPlayer()->rot, -XM_PI, XM_PI);

		ImGui::SliderFloat(u8"移動速度", &GetPlayer()->speed, 0.0f, 20.0f);
		ImGui::InputFloat(u8"速度入力", &GetPlayer()->speed, 0.1f, 1.0f, "%.2f");

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
			//PLAYER* player = GetPlayer();
			cam->at = GetPlayer()->GetPosition();

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

		ImGui::DragFloat3(u8"位置", (float*)&FBXModel->pos, 0.5f);
		ImGui::SliderFloat3(u8"回転", (float*)&FBXModel->rot, -XM_PI, XM_PI);
		ImGui::SliderFloat3(u8"サイズ", (float*)&FBXModel->scl, 0, 100.0f);
		ImGui::InputFloat3(u8"サイズ入力", (float*)&FBXModel->scl, "%.2f");

	}

	//エネミーエディター
	/*if (ImGui::CollapsingHeader(u8"エネミー配置エディター"))
	{
		auto& enemies = GetEnemies();

		static int selectedEnemy = -1;

		for (int i = 0; i < enemies.size(); ++i)
		{
			BaseEnemy* e = enemies[i];
			if (!e->IsUsed()) continue;

			XMFLOAT3 pos = e->GetPosition();

			ImGui::PushID(i);
			if (ImGui::DragFloat3(u8"位置", (float*)&pos, 0.5f)) {
				e->SetPosition(pos);
			}
			if (ImGui::Button(u8"削除")) {
				e->SetUsed(false);
			}
			ImGui::Separator();
			ImGui::PopID();
		}

		if (ImGui::Button(u8"エネミー追加")) {
			ScarecrowEnemy* newEnemy = new ScarecrowEnemy();
			newEnemy->Init();
			newEnemy->SetUsed(true);
			CAMERA* cam = GetCamera();
			newEnemy->SetPosition(cam->pos);
			GetEnemies().push_back(newEnemy);
		}

		if (ImGui::Button(u8"保存")) {
			SaveEnemyData("enemy_data.json");
		}
		ImGui::SameLine();
		if (ImGui::Button(u8"読込")) {
			LoadEnemyData("enemy_data.json");
		}
	}*/


	//アイテムエディター
	if (ImGui::CollapsingHeader(u8"アイテム配置エディター"))
	{
		ITEM_OBJ* itemObj = GetItemOBJ();

		for (int i = 0; i < MAX_ITEM; ++i)
		{
			if (!itemObj[i].use) continue;

			ImGui::PushID(i);
			ImGui::Text("ID: %d (%s)", itemObj[i].item.id, itemObj[i].item.name.c_str());
			ImGui::DragFloat3(u8"位置", (float*)&itemObj[i].pos, 0.5f);
			ImGui::DragFloat3(u8"サイズ", (float*)&itemObj[i].scl, 0.1f);
			if (ImGui::Button(u8"削除")) {
				itemObj[i].use = false;
			}
			ImGui::Separator();
			ImGui::PopID();
		}

		static int selectedItemID = 0;
		ImGui::InputInt(u8"追加アイテムID", &selectedItemID);
		if (ImGui::Button(u8"アイテム追加"))
		{
			CAMERA* cam = GetCamera();
			SetItem(cam->pos, selectedItemID);
		}

		if (ImGui::Button(u8"保存")) {
			SaveItemData("item_data.json");
		}
		ImGui::SameLine();
		if (ImGui::Button(u8"読込")) {
			LoadItemData("item_data.json");
		}
	}

	//Shaderエディター
	if (ImGui::CollapsingHeader(u8"シェーダーエディター"))
	{
		DissolveTest* dissolveTest = GetDissolveTest();
		ImGui::Checkbox(u8"ディゾルブ有効", &dissolveTest->isDissolving);
		ImGui::DragFloat(u8"ディゾルブ値", &dissolveTest->dissolve, 0.01f, 0.0f, 1.0f, "%.2f");

	}



	ImGui::End();
}