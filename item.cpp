#include "item.h"
#include "itemDatabase.h"
#include "main.h"
#include "camera.h"
#include "collision.h"
#include "player.h"

#include <cstdlib> // for rand()
#include <ctime>   // for time()

#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;


#define MAX_ITEM (128)
#define ITEM_ID_MAX ITEM_ID_COUNT // �A�C�e��ID�̍ő�l

#define ITEM_FLOAT_OFFSET (1.0f)
#define ITEM_FLOAT_FREQUENCE (3.0f)


#define ITEM_WIDTH (20.0f)		// 
#define ITEM_HEIGHT (20.0f)	// 

#define ITEM_SIZE (20.0f)





static ITEM_OBJ				g_aItem[MAX_ITEM]; // �A�C�e���z��

static ItemDatabase			g_ItemDB;

static BOOL					g_bAlpaTest;		// �A���t�@�e�X�gON/OFF

static ID3D11Buffer* g_VertexBuffer = NULL;	// ���_�o�b�t�@
static ID3D11ShaderResourceView* g_ItemTextures[ITEM_ID_MAX]; // �A�C�e���p�e�N�X�`���iItemID�ő�l�j

static float g_ItemGlobalTime = 0.0f;

static PLAYER* g_player = GetPlayer();



HRESULT MakeVertexItem(void);





int SetItem(XMFLOAT3 pos, int itemID)
{
	for (int i = 0; i < MAX_ITEM; i++)
	{
		if (!g_aItem[i].use)
		{
			g_aItem[i].pos = pos;
			g_aItem[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
			g_aItem[i].item = CreateItemFromID(itemID);
			g_aItem[i].material.Diffuse = XMFLOAT4(1, 1, 1, 1);
			g_aItem[i].use = TRUE;

			g_aItem[i].timeOffset = static_cast<float>((rand() % 1000) / 1000.0f * XM_2PI);
			g_aItem[i].basePosY = pos.y;
			return i;
		}
	}
	return -1;
}

HRESULT InitItem()
{

	srand((unsigned int)time(nullptr));

	g_ItemDB = ItemDatabase();  // TODO:�q�[�v�̈�Ɉړ����邩������Ȃ��̂Œ���
	InitItemTextures();         // �e�N�X�`���ǂݍ���

	MakeVertexItem();

	for (int CntItem = 0; CntItem < MAX_ITEM; CntItem++)
	{
		ZeroMemory(&g_aItem[CntItem].material, sizeof(g_aItem[CntItem].material));
		g_aItem[CntItem].material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		g_aItem[CntItem].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_aItem[CntItem].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);
		g_aItem[CntItem].fWidth = ITEM_WIDTH;
		g_aItem[CntItem].fHeight = ITEM_HEIGHT;
		g_aItem[CntItem].use = FALSE;

	}

	g_bAlpaTest = TRUE;

	SetItem(XMFLOAT3(10.0f, 0.0f, 20.0f), ITEM_APPLE); // �A�C�e�����Z�b�g�i��j
	SetItem(XMFLOAT3(20.0f, 0.0f, 0.0f), ITEM_SAN); // �A�C�e�����Z�b�g�i��j



	return S_OK;

}


void UninitItem()
{
	for (int nCntTex = 0; nCntTex < ITEM_ID_MAX; nCntTex++)
	{
		if (g_ItemTextures[nCntTex] != NULL)
		{// �e�N�X�`���̉��
			g_ItemTextures[nCntTex]->Release();
			g_ItemTextures[nCntTex] = NULL;
		}
	}

	if (g_VertexBuffer != NULL)
	{// ���_�o�b�t�@�̉��
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}
}


void UpdateItem()
{
	g_ItemGlobalTime += ITEM_FLOAT_FREQUENCE / 60.0f;
	// �A�C�e���̍X�V����
	for (int i = 0; i < MAX_ITEM; i++)
	{
		if (g_aItem[i].use)
		{
			//Item�̃A�j���[�V����
			g_aItem[i].pos.y = g_aItem[i].basePosY + sinf(g_ItemGlobalTime + g_aItem[i].timeOffset) * ITEM_FLOAT_OFFSET;


			//Player�Ɠ����蔻��
			if (CollisionBC(g_aItem[i].pos, g_player->pos, ITEM_SIZE, g_player->size))
			{
				switch (g_aItem[i].item.category)
				{
				case ItemCategory::WeaponPart_Ammo:
					//�C���x���g���[�ɓ����

					break;
				case ItemCategory::WeaponPart_FireType:
					//�C���x���g���[�ɓ����

					break;
				case ItemCategory::Consumable:
					//�C���x���g���[�ɓ����

					break;
				case ItemCategory::InstantEffect:
					//�����̌���
					//test
					g_player->HP += 1.0f;
					g_aItem[i].use = false;

					break;

				default:
					break;
				}
			}


		}
	}
}

void DrawItem()
{
	if (g_bAlpaTest == TRUE)
	{
		// ���e�X�g��L����
		SetAlphaTestEnable(TRUE);
	}

	SetLightEnable(FALSE);

	XMMATRIX mtxScl, mtxTranslate, mtxWorld, mtxView;
	CAMERA* cam = GetCamera();

	// ���_�o�b�t�@�ݒ�
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// �v���~�e�B�u�g�|���W�ݒ�
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	for (int i = 0; i < MAX_ITEM; i++)
	{
		if (g_aItem[i].use)
		{
			// ���[���h�}�g���b�N�X�̏�����
			mtxWorld = XMMatrixIdentity();

			// �r���[�}�g���b�N�X���擾
			mtxView = XMLoadFloat4x4(&cam->mtxView);

			// �����s��i�����s��j��]�u�s�񂳂��ċt�s�������Ă��(����)
			mtxWorld.r[0].m128_f32[0] = mtxView.r[0].m128_f32[0];
			mtxWorld.r[0].m128_f32[1] = mtxView.r[1].m128_f32[0];
			mtxWorld.r[0].m128_f32[2] = mtxView.r[2].m128_f32[0];

			mtxWorld.r[1].m128_f32[0] = mtxView.r[0].m128_f32[1];
			mtxWorld.r[1].m128_f32[1] = mtxView.r[1].m128_f32[1];
			mtxWorld.r[1].m128_f32[2] = mtxView.r[2].m128_f32[1];

			mtxWorld.r[2].m128_f32[0] = mtxView.r[0].m128_f32[2];
			mtxWorld.r[2].m128_f32[1] = mtxView.r[1].m128_f32[2];
			mtxWorld.r[2].m128_f32[2] = mtxView.r[2].m128_f32[2];


			// �X�P�[���𔽉f
			mtxScl = XMMatrixScaling(g_aItem[i].scl.x, g_aItem[i].scl.y, g_aItem[i].scl.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

			// �ړ��𔽉f
			mtxTranslate = XMMatrixTranslation(g_aItem[i].pos.x, g_aItem[i].pos.y, g_aItem[i].pos.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

			// ���[���h�}�g���b�N�X�̐ݒ�
			SetWorldMatrix(&mtxWorld);

			SetMaterial(g_aItem[i].material);


			// ItemDatabase���瓾���e�N�X�`���ŕ`��
			int texID = g_aItem[i].item.id;
			if (g_ItemTextures[texID]) {
				GetDeviceContext()->PSSetShaderResources(0, 1, &g_ItemTextures[texID]);
				GetDeviceContext()->Draw(4, 0);
			}
		}
	}
}



//
void InitItemTextures()
{
	for (int id = 0; id < ITEM_ID_MAX; ++id)
	{
		std::string path = g_ItemDB.GetTexturePath(id);
		if (path.empty()) continue;

		ID3D11ShaderResourceView* tex = nullptr;
		D3DX11CreateShaderResourceViewFromFile(GetDevice(), path.c_str(), NULL, NULL, &tex, NULL);
		g_ItemTextures[id] = tex;
	}
}


HRESULT MakeVertexItem(void)
{
	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	// ���_�o�b�t�@�ɒl���Z�b�g����
	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

	float fWidth = ITEM_WIDTH;
	float fHeight = ITEM_HEIGHT;

	// ���_���W�̐ݒ�
	vertex[0].Position = XMFLOAT3(-fWidth / 2.0f, fHeight, 0.0f);
	vertex[1].Position = XMFLOAT3(fWidth / 2.0f, fHeight, 0.0f);
	vertex[2].Position = XMFLOAT3(-fWidth / 2.0f, 0.0f, 0.0f);
	vertex[3].Position = XMFLOAT3(fWidth / 2.0f, 0.0f, 0.0f);

	// �@���̐ݒ�
	vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[3].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

	// �g�U���̐ݒ�
	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// �e�N�X�`�����W�̐ݒ�
	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	GetDeviceContext()->Unmap(g_VertexBuffer, 0);

	return S_OK;
}

Item CreateItemFromID(int id) {
	switch (id) {
	case ITEM_APPLE:
		return Item(id, "Apple", 1, ItemCategory::Consumable);
	case ITEM_SAN:
		return Item(id, "San", 1, ItemCategory::InstantEffect);
	case ITEM_BULLET:
		return Item(id, "Bullet", 10, ItemCategory::InstantEffect);
	default:
		return Item(id, "Unknown", 1, ItemCategory::Consumable);
	}
}


ITEM_OBJ* GetItemOBJ()
{
	return g_aItem;
}


void SaveItemData(const std::string& filename)
{
	json j = json::array();
	for (int i = 0; i < MAX_ITEM; ++i)
	{
		if (g_aItem[i].use)
		{
			json itemObj;
			itemObj["id"] = g_aItem[i].item.id;
			itemObj["pos"] = { g_aItem[i].pos.x, g_aItem[i].pos.y, g_aItem[i].pos.z };
			itemObj["scl"] = { g_aItem[i].scl.x, g_aItem[i].scl.y, g_aItem[i].scl.z };
			j.push_back(itemObj);
		}
	}

	std::ofstream file(filename);
	file << j.dump(4);
}

void LoadItemData(const std::string& filename)
{
	std::ifstream file(filename);
	if (!file) return;

	json j;
	file >> j;

	for (int i = 0; i < MAX_ITEM; ++i)
		g_aItem[i].use = false;

	for (const auto& itemObj : j)
	{
		int id = itemObj["id"];
		XMFLOAT3 pos = XMFLOAT3(itemObj["pos"][0], itemObj["pos"][1], itemObj["pos"][2]);
		int index = SetItem(pos, id);
		if (index >= 0)
		{
			g_aItem[index].scl = XMFLOAT3(itemObj["scl"][0], itemObj["scl"][1], itemObj["scl"][2]);
		}
	}
}