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
#include "shaderManager.h"

#include "light.h"
#include "boundingBoxDebug.h"


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
			if (!itemObj[i].IsUsed()) continue;

			ImGui::PushID(i);
			ImGui::Text("ID: %d (%s)", itemObj[i].GetItem().GetID(), itemObj[i].GetItem().GetName().c_str());
			XMFLOAT3 pos = itemObj[i].GetPosition();
			ImGui::DragFloat3(u8"位置", (float*)&pos, 0.5f);
			XMFLOAT3 scl = itemObj[i].GetScale();
			ImGui::DragFloat3(u8"サイズ", (float*)&scl, 0.1f);
			if (ImGui::Button(u8"削除")) {
				itemObj[i].SetUsed(false);
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

	//ライトエディター
	if (ImGui::CollapsingHeader(u8"ライトエディター"))
	{
		//
		for (int lightIndex = 0; lightIndex < LIGHT_MAX; lightIndex++)
		{
			LIGHT* light = GetLightData(lightIndex);
			bool changed = false;

			char headerName[64];
			sprintf_s(headerName, u8"光源 %d", lightIndex);

			if (ImGui::CollapsingHeader(headerName))
			{
				ImGui::PushID(lightIndex); // 光源ID衝突ないように

				// 光源スイッチ
				bool enabled = (light->Enable == TRUE);
				if (ImGui::Checkbox(u8"起用", &enabled)) {
					light->Enable = enabled ? TRUE : FALSE;
					changed = true;
				}

				// 光源タイプ
				const char* lightTypes[] = { u8"無し", u8"平行光", u8"点光源" };
				int currentType = light->Type;
				if (ImGui::Combo(u8"光源タイプ", &currentType, lightTypes, 3)) {
					light->Type = currentType;
					changed = true;
				}

				// 平行光
				if (light->Type == LIGHT_TYPE_DIRECTIONAL) {
					if (ImGui::DragFloat3(u8"光源方向", (float*)&light->Direction, 0.01f, -1.0f, 1.0f)) {
						changed = true;
					}
				}

				// 点光源
				if (light->Type == LIGHT_TYPE_POINT) {
					if (ImGui::DragFloat3(u8"光源位置", (float*)&light->Position, 0.5f, -100.0f, 100.0f)) {
						changed = true;
					}
					if (ImGui::DragFloat(u8"減衰距離", &light->Attenuation, 1.0f, 1.0f, 1000.0f)) {
						changed = true;
					}
				}

				// 色情報
				if (ImGui::ColorEdit3(u8"光の色", (float*)&light->Diffuse)) {
					changed = true;
				}
				if (ImGui::ColorEdit3(u8"環境光", (float*)&light->Ambient)) {
					changed = true;
				}

				// アップデートする
				if (changed) {
					SetLightData(lightIndex, light);
				}

				ImGui::PopID();
			}
		}


	}


	//バウンディングボックスデバッグ
	if (ImGui::CollapsingHeader(u8"バウンディングボックス"))
	{
		BoundingBoxDebugRenderer& debugRenderer = BoundingBoxDebugRenderer::GetInstance();

		bool globalEnable = debugRenderer.GetGlobalEnable();
		if (ImGui::Checkbox(u8"バウンディングボックス起用", &globalEnable)) {
			debugRenderer.SetGlobalEnable(globalEnable);
		}

		if (globalEnable) {
			bool playerBox = debugRenderer.GetPlayerBoxEnable();
			if (ImGui::Checkbox(u8"プレイヤーボックス", &playerBox)) {
				debugRenderer.SetPlayerBoxEnable(playerBox);
			}

			bool enemyBox = debugRenderer.GetEnemyBoxEnable();
			if (ImGui::Checkbox(u8"エネミーボックス", &enemyBox)) {
				debugRenderer.SetEnemyBoxEnable(enemyBox);
			}

			bool itemBox = debugRenderer.GetItemBoxEnable();
			if (ImGui::Checkbox(u8"アイテムボックス", &itemBox)) {
				debugRenderer.SetItemBoxEnable(itemBox);
			}

			bool terrainBox = debugRenderer.GetTerrainBoxEnable();
			if (ImGui::Checkbox(u8"マップボックス", &terrainBox)) {
				debugRenderer.SetTerrainBoxEnable(terrainBox);
			}
			//説明
			if (terrainBox) {
				int depthLimit = debugRenderer.GetOctreeDepthLimit();
				if (ImGui::SliderInt(u8"八分木描画深度", &depthLimit, 0, 6)) {
					debugRenderer.SetOctreeDepthLimit(depthLimit);
				}
				ImGui::Text(u8"深度高いほど描画が詳しいが、ボックスを数が増える");
				ImGui::Text(u8"色情報：赤（0）緑（1）青（2）黄（3）");
			}
		}
	}



	ImGui::End();

	//ShaderManager::ShowShaderDebugUI();

	//ShaderManager::ShowEffectDebugUI();
}