#include "debugUI.h"
#include "imgui.h"
#include "camera.h"
#include "player.h"
#include "FBXmodel.h"

#include "enemy.h"
#include "item.h"
#include <fstream>
#include <nlohmann/json.hpp>


#include "shaderManager.h"

#include "light.h"
#include "boundingBoxDebug.h"


// item.cppにあるアイテム配列
#define MAX_ITEM (128)


extern BOOL g_bPause;


void ShowDetailedOctreeInfo(OctreeNode* node, int depth, int maxShow);



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

		ImGui::Separator();
		ImGui::Text(u8"=== インベントリ情報 ===");

		Inventory* inventory = GetPlayerInventory();

		// 選択中のアイテム表示
		const std::vector<Item>& consumables = inventory->GetConsumables();
		if (!consumables.empty()) {
			int currentIndex = GetPlayer()->currentConsumableIndex;
			if (currentIndex >= 0 && currentIndex < (int)consumables.size()) {
				const Item& currentItem = consumables[currentIndex];
				ImGui::Text(u8"選択中アイテム: %s (数量: %d)",
					currentItem.GetName().c_str(), currentItem.GetCount());
			}
		}
		else {
			ImGui::Text(u8"選択中アイテム: なし");
		}

		// アイテム一覧表示
		if (ImGui::TreeNode(u8"消耗品")) {
			for (const auto& item : consumables) {
				ImGui::Text(u8"・%s x%d", item.GetName().c_str(), item.GetCount());
			}
			if (consumables.empty()) {
				ImGui::Text(u8"アイテムなし");
			}
			ImGui::TreePop();
		}

		if (ImGui::TreeNode(u8"弾薬パーツ")) {
			const std::vector<Item>& ammoParts = inventory->GetAmmoParts();
			for (const auto& item : ammoParts) {
				ImGui::Text(u8"・%s x%d", item.GetName().c_str(), item.GetCount());
			}
			if (ammoParts.empty()) {
				ImGui::Text(u8"アイテムなし");
			}
			ImGui::TreePop();
		}

		if (ImGui::TreeNode(u8"発射方法パーツ")) {
			const std::vector<Item>& fireTypeParts = inventory->GetFireTypeParts();
			for (const auto& item : fireTypeParts) {
				ImGui::Text(u8"・%s x%d", item.GetName().c_str(), item.GetCount());
			}
			if (fireTypeParts.empty()) {
				ImGui::Text(u8"アイテムなし");
			}
			ImGui::TreePop();
		}

	}

	//カメラ視点の制御
	if (ImGui::CollapsingHeader(u8"カメラ制御"))
	{
		CAMERA* cam = GetCamera();

		ImGui::Checkbox(u8"第一人称視点 (Tabキーでも切替可)", &isFirstPersonMode);

		ImGui::SliderFloat(u8"マウス感度", &sensitivity, 0.0001f, 0.005f, "%.4f");
		ImGui::SliderFloat(u8"コントローラー感度", &controllerSensitivity, 0.01f, 0.3f, "%.3f");

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
		FBXMAPMODEL* FBXModel = GetFBXMapModel();

		if (FBXModel && FBXModel->model) {
			ImGui::DragFloat3(u8"位置", (float*)&FBXModel->pos, 0.5f);
			ImGui::SliderFloat3(u8"回転", (float*)&FBXModel->rot, -XM_PI, XM_PI);
			ImGui::SliderFloat3(u8"サイズ", (float*)&FBXModel->scl, 0, 100.0f);
			ImGui::InputFloat3(u8"サイズ入力", (float*)&FBXModel->scl, "%.2f");
		}

		

	}

	//エネミーエディター
	if (ImGui::CollapsingHeader(u8"エネミー配置エディター"))
	{
		std::vector<BaseEnemy*>& enemies = GetEnemies();
		ImGui::Text(u8"敵の数：%d", (int)enemies.size());
		ImGui::Separator();

		for (int i = 0; i < (int)enemies.size(); ++i)
		{
			BaseEnemy* enemy = enemies[i];
			if (!enemy || !enemy->IsUsed()) continue;

			ImGui::PushID(i);

			// 敵の種類
			const char* enemyTypeName = "Unknown";
			int enemyType = -1;
			if (dynamic_cast<SpiderEnemy*>(enemy)) {
				enemyTypeName = "Spider";
				enemyType = SPIDER;
			}
			else if (dynamic_cast<GhostEnemy*>(enemy)) {
				enemyTypeName = "Ghost";
				enemyType = GHOST;
			}
			else if (dynamic_cast<BugEnemy*>(enemy)) {
				enemyTypeName = "Bug";
				enemyType = BUG;
			}

			ImGui::Text(u8"ID: %d (%s)", i, enemyTypeName);

			XMFLOAT3 pos = enemy->GetPosition();
			if (ImGui::DragFloat3(u8"位置", (float*)&pos, 0.5f)) {
				enemy->SetPosition(pos);
			}

			XMFLOAT3 scl = enemy->GetScale();
			if (ImGui::DragFloat3(u8"サイズ", (float*)&scl, 0.1f)) {
				enemy->SetScale(scl);
			}

			if (SpiderEnemy* spider = dynamic_cast<SpiderEnemy*>(enemy)) {
				ImGui::Text(u8"HP: %d", spider->GetHP());
			}
			else if (GhostEnemy* ghost = dynamic_cast<GhostEnemy*>(enemy)) {
				ImGui::Text(u8"HP: %d", ghost->GetHP());
			}
			else if (BugEnemy* bug = dynamic_cast<BugEnemy*>(enemy)) {
				ImGui::Text(u8"HP: %d", bug->GetHP());
			}

			if (ImGui::Button(u8"削除")) {
				enemy->SetUsed(false);
			}

			ImGui::Separator();
			ImGui::PopID();
		}

	}


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
			SpawnItem(cam->pos, selectedItemID);
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
	/*if (ImGui::CollapsingHeader(u8"シェーダーエディター"))
	{


		

		
	}*/

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

			ImGui::Separator();

			//法線ベクトル表示
			bool normalVector = debugRenderer.GetNormalVectorEnable();
			if (ImGui::Checkbox(u8"法線ベクトル描画", &normalVector)) {
				debugRenderer.SetNormalVectorEnable(normalVector);
			}

			if (normalVector) {
				bool floorNormal = debugRenderer.GetFloorNormalEnable();
				if (ImGui::Checkbox(u8"床の法線ベクトル（緑）", &floorNormal)) {
					debugRenderer.SetFloorNormalEnable(floorNormal);
				}

				bool wallNormal = debugRenderer.GetWallNormalEnable();
				if (ImGui::Checkbox(u8"壁の法線ベクトル（赤）", &wallNormal)) {
					debugRenderer.SetWallNormalEnable(wallNormal);
				}

				float normalLength = debugRenderer.GetNormalLength();
				if (ImGui::SliderFloat(u8"法線の長さ", &normalLength, 1.0f, 20.0f)) {
					debugRenderer.SetNormalLength(normalLength);
				}

				float normalRange = debugRenderer.GetNormalDisplayRange();
				if (ImGui::SliderFloat(u8"描画範囲", &normalRange, 10.0f, 200.0f)) {
					debugRenderer.SetNormalDisplayRange(normalRange);
				}

				ImGui::Text(u8"床の法線は緑、壁のは赤");
				
			}
		}
	}

	//LODシステムと八分木構造の解析
	if (ImGui::CollapsingHeader(u8"LOD八分木システム情報"))
	{
		

		ImGui::Separator();

		
		ImGui::Text(u8"=== 八分木構造解析 ===");

		OctreeNode* wallTree = GetWallTree();
		OctreeNode* floorTree = GetFloorTree();

		if (wallTree) {
			int wallDepth = 0, wallLeafCount = 0, wallNodeCount = 0;
			AnalyzeOctreeStructure(wallTree, 0, wallDepth, wallLeafCount, wallNodeCount);
			ImGui::Text(u8"壁八分木 - 最大深度: %d, リーフ数: %d, 総ノード数: %d",
				wallDepth, wallLeafCount, wallNodeCount);
		}

		if (floorTree) {
			int floorDepth = 0, floorLeafCount = 0, floorNodeCount = 0;
			AnalyzeOctreeStructure(floorTree, 0, floorDepth, floorLeafCount, floorNodeCount);
			ImGui::Text(u8"床八分木 - 最大深度: %d, リーフ数: %d, 総ノード数: %d",
				floorDepth, floorLeafCount, floorNodeCount);
		}

		// 各深度のノード数をカウントして表示
		ImGui::Text(u8"=== 深度別ノード統計 ===");
		if (wallTree) {
			std::vector<int> wallDepthCounts(10, 0);
			CountNodesByDepth(wallTree, 0, wallDepthCounts);
			for (int i = 0; i <= 6; i++) {
				if (wallDepthCounts[i] > 0) {
					ImGui::Text(u8"壁深度 %d: %d ノード", i, wallDepthCounts[i]);
				}
			}
		}

		if (floorTree) {
			std::vector<int> floorDepthCounts(10, 0);
			CountNodesByDepth(floorTree, 0, floorDepthCounts);
			for (int i = 0; i <= 6; i++) {
				if (floorDepthCounts[i] > 0) {
					ImGui::Text(u8"床深度 %d: %d ノード", i, floorDepthCounts[i]);
				}
			}
		}

		ImGui::Text(u8"=== 詳細八分木情報 ===");
		if (wallTree) {
			ImGui::Text(u8"壁八分木:");
			ShowDetailedOctreeInfo(wallTree, 0, 10);
		}
	}



	ImGui::End();

	//ShaderManager::ShowShaderDebugUI();

	ShaderManager::ShowEffectDebugUI();
}


void ShowDetailedOctreeInfo(OctreeNode* node, int depth, int maxShow = 20) {
	static int nodeCount = 0;
	if (depth == 0) nodeCount = 0;

	if (nodeCount >= maxShow) return;

	ImGui::Text(u8"深度%d: %zu個三角形, 細分済み: %s",
		depth, node->triangleIndices.size(),
		node->isSubdivided ? u8"はい" : u8"いいえ");

	nodeCount++;

	if (node->isSubdivided) {
		for (int i = 0; i < 8; i++) {
			if (node->children[i]) {
				ShowDetailedOctreeInfo(node->children[i], depth + 1, maxShow);
			}
		}
	}
}





