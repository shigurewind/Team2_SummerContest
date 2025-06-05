#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <codecvt> 
#include <cstdio>
#include <sstream>
#include <string>
#include <stdio.h>
#include <iomanip>
#include "DirectGraphics.h"
#include "Utility.h"
#include "FbxMeshFile.h"

bool FbxMeshFile::Load(const char* file_name, ID3D11Device *device, VertexShader* vertex_shader)
{
	char file_path[256];
	std::vector<std::string> out_material_list;
	int len = (int)strlen(file_name);
	int path_tail_point = 0;

	for (int i = len - 1; i >= 0; i--)
	{
		if (file_name[i] == '/')
		{
			path_tail_point = i;
			break;
		}
	}

	strncpy_s(file_path, file_name, path_tail_point + 1);

	if (LoadFbxFile(out_material_list, file_path, file_name) == false)
	{
		return false;
	}

	if (CreateVertexBuffer(device) == false)
	{
		return false;
	}

	if (CreateIndexBuffer(device) == false)
	{
		return false;
	}

	if (CreateInputLayout(device, vertex_shader) == false)
	{
		return false;
	}

	return true;
}

void FbxMeshFile::Render(DirectGraphics* graphics, Vector3 pos, Vector3 scale, Vector3 degree)
{
	graphics->SetUpContext();
	UINT strides = sizeof(CustomVertex);
	UINT offsets = 0;

	for (auto& mesh : m_MeshList)
	{
		// �C���f�b�N�X�o�b�t�@�̐� = �}�e���A���̐��������b�V����`�悷��
		// IA(InputAssemblerStage)�ɓ��̓��C�A�E�g��ݒ肷��
		graphics->GetContext()->IASetInputLayout(m_InputLayout);
		// IA�ɐݒ肷�钸�_�o�b�t�@�̎w��
		graphics->GetContext()->IASetVertexBuffers(
				0,								// �o�b�t�@���M�̃X���b�g�ԍ�
				1,								// �o�b�t�@�̐�
				&mesh.m_VertexBuffer,	// ���_�o�b�t�@
				&strides,						// �o�b�t�@�Ɏg�p���Ă���\���̂̃T�C�Y
				&offsets);						// �J�n�I�t�Z�b�g

		graphics->GetContext()->IASetIndexBuffer(
				mesh.m_IndexBuffer,
				DXGI_FORMAT_R32_UINT,
				0);

		// ���[���h�}�g���N�X�ݒ�
		DirectX::XMMATRIX world_matrix;
		DirectX::XMMATRIX translate = DirectX::XMMatrixTranslation(pos.X, pos.Y, pos.Z);
		DirectX::XMMATRIX rotate_x = DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(degree.X));
		DirectX::XMMATRIX rotate_y = DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(degree.Y));
		DirectX::XMMATRIX rotate_z = DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(degree.Z));
		DirectX::XMMATRIX scale_mat = DirectX::XMMatrixScaling(scale.X, scale.Y, scale.Z);
		world_matrix = scale_mat * rotate_x * rotate_y * rotate_z * translate;

		// ���[���h�}�g���N�X���R���X�^���g�o�b�t�@�ɐݒ�
		XMStoreFloat4x4(&graphics->GetConstantBufferData()->World, XMMatrixTranspose(world_matrix));

		SetMaterialColor(graphics, m_Materials[mesh.m_MaterialName]);

		// �R���X�^���g�o�b�t�@�X�V
		graphics->GetContext()->UpdateSubresource(graphics->GetConstantBuffer(), 0, NULL, graphics->GetConstantBufferData(), 0, 0);

		ID3D11Buffer* constant_buffer = graphics->GetConstantBuffer();
		// �R���e�L�X�g�ɃR���X�^���g�o�b�t�@��ݒ�
		graphics->GetContext()->VSSetConstantBuffers(0, 1, &constant_buffer);
		graphics->GetContext()->PSSetConstantBuffers(0, 1, &constant_buffer);

		// �`��
		graphics->GetContext()->DrawIndexed(
				(UINT)mesh.m_Indices.size(),	// ���_��
				0,								// �I�t�Z�b�g
				0);								// �J�n���_�̃C���f�b�N�X
	}
}

void FbxMeshFile::LoadVertices(MeshData& mesh_data, FbxMesh* mesh)
{
	// ���_�o�b�t�@�̎擾
	FbxVector4* vertices = mesh->GetControlPoints();
	// �C���f�b�N�X�o�b�t�@�̎擾
	int* indices = mesh->GetPolygonVertices();
	// ���_���W�̐��̎擾
	int polygon_vertex_count = mesh->GetPolygonVertexCount();
	// GetPolygonVertexCount => ���_��
	for (int i = 0; i < polygon_vertex_count; i++)
	{
		CustomVertex vertex;
		// �C���f�b�N�X�o�b�t�@���璸�_�ԍ����擾
		int index = indices[i];

		// ���_���W���X�g������W���擾����
		vertex.Position.X = (float)-vertices[index][0];
		vertex.Position.Y = (float)vertices[index][1];
		vertex.Position.Z = (float)vertices[index][2];

		// �ǉ�
		mesh_data.m_Vertices.push_back(vertex);
	}
}

void FbxMeshFile::LoadIndices(MeshData& mesh_data, FbxMesh* mesh)
{
	// �|���S�����̎擾
	int polygon_count = mesh->GetPolygonCount();

	// �|���S���̐������A�ԂƂ��ĕۑ�����
	for (int i = 0; i < polygon_count; i++)
	{
		mesh_data.m_Indices.push_back(i * 3 + 2);
		mesh_data.m_Indices.push_back(i * 3 + 1);
		mesh_data.m_Indices.push_back(i * 3);
	}
}

void FbxMeshFile::LoadNormals(MeshData& mesh_data, FbxMesh* mesh)
{
	FbxArray<FbxVector4> normals;
	// �@�����X�g�̎擾
	mesh->GetPolygonVertexNormals(normals);

	// �@���ݒ�
	for (int i = 0; i < normals.Size(); i++)
	{
		mesh_data.m_Vertices[i].Normal.X = (float)-normals[i][0];
		mesh_data.m_Vertices[i].Normal.Y = (float)normals[i][1];
		mesh_data.m_Vertices[i].Normal.Z = (float)normals[i][2];
	}
}

void FbxMeshFile::LoadColors(MeshData& mesh_data, FbxMesh* mesh)
{
	// ���_�J���[�f�[�^�̐����m�F
	int color_count = mesh->GetElementVertexColorCount();
	if (color_count == 0)
	{
		return;
	}
	
	// ���_�J���[�f�[�^�̎擾
	FbxGeometryElementVertexColor* vertex_colors = mesh->GetElementVertexColor(0);

	if (vertex_colors == nullptr)
	{
		return;
	}

	FbxLayerElement::EMappingMode mapping_mode = vertex_colors->GetMappingMode();
	FbxLayerElement::EReferenceMode reference_mode = vertex_colors->GetReferenceMode();

	if (mapping_mode == FbxLayerElement::eByPolygonVertex)
	{
		if (reference_mode == FbxLayerElement::eIndexToDirect)
		{
			// ���_�J���[�o�b�t�@�擾
			FbxLayerElementArrayTemplate<FbxColor>& colors = vertex_colors->GetDirectArray();
			// ���_�J���[�C���f�b�N�X�o�b�t�@�擾
			FbxLayerElementArrayTemplate<int>& indeces = vertex_colors->GetIndexArray();
			for (int i = 0; i < indeces.GetCount(); i++)
			{
				int id = indeces.GetAt(i);
				FbxColor color = colors.GetAt(id);
				mesh_data.m_Vertices[i].Color.Alpha = (float)color.mAlpha;
				mesh_data.m_Vertices[i].Color.Red = (float)color.mRed;
				mesh_data.m_Vertices[i].Color.Green = (float)color.mGreen;
				mesh_data.m_Vertices[i].Color.Blue = (float)color.mBlue;
			}
		}
	}
}

void FbxMeshFile::SetMaterialName(MeshData& mesh_data, FbxMesh* mesh)
{
	// �}�e���A����������ΏI���
	if (mesh->GetElementMaterialCount() == 0)
	{
		mesh_data.m_MaterialName = "";
		return;
	}

	// Mesh���̃}�e���A�������擾
	FbxLayerElementMaterial* material = mesh->GetElementMaterial(0);
	int index = material->GetIndexArray().GetAt(0);
	FbxSurfaceMaterial* surface_material = mesh->GetNode()->GetSrcObject<FbxSurfaceMaterial>(index);

	if (surface_material != nullptr)
	{
		mesh_data.m_MaterialName = surface_material->GetName();
	}
	else
	{
		mesh_data.m_MaterialName = "";
	}
}

void FbxMeshFile::LoadMaterial(FbxSurfaceMaterial* material)
{
	ObjMaterial entry_material;
	enum class MaterialOrder
	{
		Ambient,
		Diffuse,
		Specular,
		MaxOrder,
	};

	FbxDouble3 colors[(int)MaterialOrder::MaxOrder];
	FbxDouble factors[(int)MaterialOrder::MaxOrder];
	FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sAmbient);

	if (material->GetClassId().Is(FbxSurfaceLambert::ClassId))
	{
		const char* element_check_list[] =
		{
			FbxSurfaceMaterial::sAmbient,
			FbxSurfaceMaterial::sDiffuse,
		};

		const char* factor_check_list[] =
		{
			FbxSurfaceMaterial::sAmbientFactor,
			FbxSurfaceMaterial::sDiffuseFactor,
		};

		for (int i = 0; i < 2; i++)
		{
			prop = material->FindProperty(element_check_list[i]);
			if (prop.IsValid())
			{
				colors[i] = prop.Get<FbxDouble3>();

			}
			else
			{
				colors[i] = FbxDouble3(1.0, 1.0, 1.0);
			}

			prop = material->FindProperty(factor_check_list[i]);
			if (prop.IsValid())
			{
				factors[i] = prop.Get<FbxDouble>();
			}
			else
			{
				factors[i] = 1.0;
			}
		}
	}

	FbxDouble3 color = colors[(int)MaterialOrder::Ambient];
	FbxDouble factor = factors[(int)MaterialOrder::Ambient];
	entry_material.SetAmbient((float)color[0], (float)color[1], (float)color[2], (float)factor);

	color = colors[(int)MaterialOrder::Diffuse];
	factor = factors[(int)MaterialOrder::Diffuse];
	entry_material.SetDiffuse((float)color[0], (float)color[1], (float)color[2], (float)factor);

	m_Materials[material->GetName()] = entry_material;
}

void FbxMeshFile::CreateMesh(FbxMesh* mesh)
{
	MeshData mesh_data;
	LoadIndices(mesh_data, mesh);
	LoadVertices(mesh_data, mesh);
	LoadNormals(mesh_data, mesh);
	LoadColors(mesh_data, mesh);
	SetMaterialName(mesh_data, mesh);

	m_MeshList.push_back(mesh_data);
}

bool FbxMeshFile::LoadFbxFile(std::vector<std::string>& out_material_list, const char* file_path, const char* file_name)
{
	// FbxManager�쐬
	FbxManager* fbx_manager = fbxsdk::FbxManager::Create();
	if (fbx_manager == nullptr)
	{
		return false;
	}
	
	// FbxImporter�쐬
	FbxImporter* fbx_importer = FbxImporter::Create(fbx_manager, "");
	if (fbx_importer == nullptr)
	{
		fbx_manager->Destroy();
		return false;
	}
	
	// FbxScene�쐬
	FbxScene* fbx_scene = FbxScene::Create(fbx_manager, "");
	if (fbx_scene == nullptr)
	{
		fbx_importer->Destroy();
		fbx_manager->Destroy();
		return false;
	}

	// File��������
	fbx_importer->Initialize(file_name);
	// scene�ɃC���|�[�g
	fbx_importer->Import(fbx_scene);

	FbxGeometryConverter converter(fbx_manager);
	// ���b�V���Ɏg���Ă���}�e���A���P�ʂŃ��b�V���𕪊�����
	converter.SplitMeshesPerMaterial(fbx_scene, true);
	// �|���S�����O�p�`�ɂ���
	converter.Triangulate(fbx_scene, true);

	int material_num = fbx_scene->GetSrcObjectCount<FbxSurfaceMaterial>();

	for (int i = 0; i < material_num; i++)
	{
		LoadMaterial(fbx_scene->GetSrcObject<FbxSurfaceMaterial>(i));
	}

	// FbxMesh�̐����擾
	int mesh_num = fbx_scene->GetSrcObjectCount<FbxMesh>();

	for (int i = 0; i < mesh_num; i++)
	{
		// Mesh�쐬
		CreateMesh(fbx_scene->GetSrcObject<FbxMesh>(i));
	}

	fbx_importer->Destroy();
	fbx_scene->Destroy();
	fbx_manager->Destroy();

	return true;
}

bool FbxMeshFile::CreateVertexBuffer(ID3D11Device* device)
{
	for (auto& mesh : m_MeshList)
	{
		//���_�o�b�t�@�쐬
		D3D11_BUFFER_DESC buffer_desc;
		buffer_desc.ByteWidth = sizeof(CustomVertex) * (UINT)mesh.m_Vertices.size();	// �o�b�t�@�̃T�C�Y
		buffer_desc.Usage = D3D11_USAGE_DEFAULT;			// �g�p���@
		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;	// BIND�ݒ�
		buffer_desc.CPUAccessFlags = 0;						// ���\�[�X�ւ�CPU�̃A�N�Z�X�����ɂ��Ă̐ݒ�
		buffer_desc.MiscFlags = 0;							// ���\�[�X�I�v�V�����̃t���O
		buffer_desc.StructureByteStride = 0;				// �\���̂̃T�C�Y

		D3D11_SUBRESOURCE_DATA sub_resource;
		sub_resource.pSysMem = &mesh.m_Vertices[0];	// �o�b�t�@�̒��g�̐ݒ�
		sub_resource.SysMemPitch = 0;			// texture�f�[�^���g�p����ۂɎg�p���郁���o
		sub_resource.SysMemSlicePitch = 0;		// texture�f�[�^���g�p����ۂɎg�p���郁���o

		// �o�b�t�@�쐬
		if (FAILED(device->CreateBuffer(
			&buffer_desc,								// �o�b�t�@���
			&sub_resource,								// ���\�[�X���
			&mesh.m_VertexBuffer)))	// �쐬���ꂽ�o�b�t�@�̊i�[��
		{
			return false;
		}
	}

	return true;
}

bool FbxMeshFile::CreateIndexBuffer(ID3D11Device* device)
{
	for (auto& mesh : m_MeshList)
	{
		//���_�o�b�t�@�쐬
		D3D11_BUFFER_DESC buffer_desc;
		buffer_desc.ByteWidth = sizeof(UINT) * (UINT)mesh.m_Indices.size();	// �o�b�t�@�̃T�C�Y
		buffer_desc.Usage = D3D11_USAGE_DEFAULT;							// �g�p���@
		buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;					// BIND�ݒ�
		buffer_desc.CPUAccessFlags = 0;										// ���\�[�X�ւ�CPU�̃A�N�Z�X�����ɂ��Ă̐ݒ�
		buffer_desc.MiscFlags = 0;											// ���\�[�X�I�v�V�����̃t���O
		buffer_desc.StructureByteStride = 0;								// �\���̂̃T�C�Y

		D3D11_SUBRESOURCE_DATA sub_resource;
		sub_resource.pSysMem = &mesh.m_Indices[0];							// �o�b�t�@�̒��g�̐ݒ�
		sub_resource.SysMemPitch = 0;										// texture�f�[�^���g�p����ۂɎg�p���郁���o
		sub_resource.SysMemSlicePitch = 0;									// texture�f�[�^���g�p����ۂɎg�p���郁���o

		// �o�b�t�@�쐬
		if (FAILED(device->CreateBuffer(
			&buffer_desc,						// �o�b�t�@���
			&sub_resource,						// ���\�[�X���
			&mesh.m_IndexBuffer)))		// �쐬���ꂽ�o�b�t�@�̊i�[��
		{
			return false;
		}
	}

	return true;
}

bool FbxMeshFile::CreateInputLayout(ID3D11Device* device, VertexShader* vertex_shader)
{
	D3D11_INPUT_ELEMENT_DESC vertex_desc[]{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0,	0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0,	D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0,	D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXTURE",	0, DXGI_FORMAT_R32G32_FLOAT,		0,	D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	//���_���C�A�E�g�쐬
	if (FAILED(device->CreateInputLayout(
		vertex_desc,				// ���C�A�E�g�ݒ�
		ARRAYSIZE(vertex_desc),		// �z��T�C�Y
		vertex_shader->GetData(),	// ���C�A�E�g�Ɗ֘A�t���钸�_�V�F�[�_�̃f�[�^
		vertex_shader->GetSize(),	// ���C�A�E�g�Ɗ֘A�t���钸�_�V�F�[�_�̃T�C�Y
		&m_InputLayout)))			// �쐬���ꂽ���_���C�A�E�g�̊i�[��
	{
		return false;
	}

	return true;
}

void FbxMeshFile::SetMaterialColor(DirectGraphics* graphics, ObjMaterial& material)
{
	ConstantBuffer* constant = graphics->GetConstantBufferData();
	constant->MaterialDiffuse = DirectX::XMFLOAT4(material.Diffuse[0], material.Diffuse[1], material.Diffuse[2], material.Diffuse[3]);
	constant->MaterialAmbient = DirectX::XMFLOAT4(material.Ambient[0], material.Ambient[1], material.Ambient[2], material.Ambient[3]);
}