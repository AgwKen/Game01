#include <assert.h>
#include "direct3d.h"
#include "texture.h"
#include "model.h"
#include <DirectXMath.h>
using namespace DirectX;
#include "WICTextureLoader11.h"
#include "shader3d.h"

struct Vertex3d
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT4 color;
	XMFLOAT2 texcoord;
};

static int g_TextureWhite = -1;
MODEL* ModelLoad(const char* FileName, float scale)
{
	MODEL* model = new MODEL;

	const std::string modelPath(FileName);

	model->AiScene = aiImportFile(FileName, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded);
	assert(model->AiScene);

	model->VertexBuffer = new ID3D11Buffer * [model->AiScene->mNumMeshes];
	model->IndexBuffer = new ID3D11Buffer * [model->AiScene->mNumMeshes];

	for (unsigned int m = 0; m < model->AiScene->mNumMeshes; m++)
	{
		aiMesh* mesh = model->AiScene->mMeshes[m];

		// Create vertex buffer
		{
			Vertex3d* vertex = new Vertex3d[mesh->mNumVertices];

			for (unsigned int v = 0; v < mesh->mNumVertices; v++)
			{
				vertex[v].position = XMFLOAT3(mesh->mVertices[v].x * scale, mesh->mVertices[v].y * scale, mesh->mVertices[v].z * scale);
				vertex[v].normal = XMFLOAT3(mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z);
				vertex[v].color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

				if (mesh->HasTextureCoords(0))
					vertex[v].texcoord = XMFLOAT2(mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y);
				else
					vertex[v].texcoord = XMFLOAT2(0, 0);
			}

			D3D11_BUFFER_DESC bd = {};
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.ByteWidth = sizeof(Vertex3d) * mesh->mNumVertices;
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			D3D11_SUBRESOURCE_DATA sd = {};
			sd.pSysMem = vertex;

			Direct3D_GetDevice()->CreateBuffer(&bd, &sd, &model->VertexBuffer[m]);

			delete[] vertex;
		}

		// Create index buffer
		{
			unsigned int* index = new unsigned int[mesh->mNumFaces * 3];

			for (unsigned int f = 0; f < mesh->mNumFaces; f++)
			{
				const aiFace* face = &mesh->mFaces[f];
				assert(face->mNumIndices == 3);

				index[f * 3 + 0] = face->mIndices[0];
				index[f * 3 + 1] = face->mIndices[1];
				index[f * 3 + 2] = face->mIndices[2];
			}

			D3D11_BUFFER_DESC bd = {};
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(unsigned int) * mesh->mNumFaces * 3;
			bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

			D3D11_SUBRESOURCE_DATA sd = {};
			sd.pSysMem = index;

			Direct3D_GetDevice()->CreateBuffer(&bd, &sd, &model->IndexBuffer[m]);

			delete[] index;
		}
	}

	// Extract directory path from FileName (for separate texture loading)
	size_t pos = modelPath.find_last_of("/\\");
	std::string directory = (pos != std::string::npos) ? modelPath.substr(0, pos) : "";

	g_TextureWhite = Texture_Load(L"Texture/white.png");

	//Load embedded textures (inside FBX)
	for (unsigned int i = 0; i < model->AiScene->mNumTextures; i++)
	{
		aiTexture* aitexture = model->AiScene->mTextures[i];

		ID3D11ShaderResourceView* texture;
		ID3D11Resource* resource;

		CreateWICTextureFromMemory(
			Direct3D_GetDevice(),
			Direct3D_GetDeviceContext(),
			(const uint8_t*)aitexture->pcData,
			(size_t)aitexture->mWidth,
			&resource,
			&texture
		);

		assert(texture);
		resource->Release();

		model->Texture[aitexture->mFilename.C_Str()] = texture;
	}

	// --- Load external textures (if not embedded) ---
	for (unsigned int m = 0; m < model->AiScene->mNumMeshes; m++)
	{
		aiMaterial* aimaterial = model->AiScene->mMaterials[model->AiScene->mMeshes[m]->mMaterialIndex];
		aiString filename;
		aimaterial->GetTexture(aiTextureType_DIFFUSE, 0, &filename);

		if (filename.length == 0) {
			continue;
		}
		if (model->Texture.count(filename.C_Str())) {
			continue;
		}// already loaded



		ID3D11ShaderResourceView* texture;
		ID3D11Resource* resource;
		std::string texfilename= directory + "/" + filename.C_Str();

		int len = MultiByteToWideChar(CP_UTF8, 0, texfilename.c_str(), -1, nullptr, 0);
		wchar_t* pWideFilename = new wchar_t[len];
		MultiByteToWideChar(CP_UTF8, 0, texfilename.c_str(), -1, pWideFilename, len);


		CreateWICTextureFromFile(
			Direct3D_GetDevice(),
			Direct3D_GetDeviceContext(),
			pWideFilename,
			&resource,
			&texture);

		delete[] pWideFilename;

		assert(texture);

		resource->Release(); // !!!!!!!!!!
	}

	return model;
}


void ModelRelease(MODEL* model)
{
	for (unsigned int m = 0; m < model->AiScene->mNumMeshes; m++)
	{
		model->VertexBuffer[m]->Release();
		model->IndexBuffer[m]->Release();
	}

	delete[] model->VertexBuffer;
	delete[] model->IndexBuffer;

	for (std::pair<const std::string, ID3D11ShaderResourceView*> pair : model->Texture)
	{
		pair.second->Release();
	}

	aiReleaseImport(model->AiScene);
	delete model;
}


void ModelDraw(MODEL* model, const DirectX::XMMATRIX& mtxWorld)
{
	Shader3d_Begin();

	// Set primitive topology
	Direct3D_GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set world transformation matrix in the vertex shader
	Shader3d_SetWorldMatrix(mtxWorld);

	for (unsigned int m = 0; m < model->AiScene->mNumMeshes; m++)
	{
		// Set texture
		if (model->AiScene->mNumTextures)
		{
			aiString texture;
			aiMaterial* aimaterial = model->AiScene->mMaterials[model->AiScene->mMeshes[m]->mMaterialIndex];
			aimaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texture);

			if (texture != aiString("")) {
				Direct3D_GetDeviceContext()->PSSetShaderResources(0, 1, &model->Texture[texture.data]);
			}
			else {
				Texture_SetTexture(g_TextureWhite);
			}
		}
		aiMaterial* aimaterial = model->AiScene->mMaterials[model->AiScene->mMeshes[m]->mMaterialIndex];
		aiColor3D diffuse;
		aimaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
		Shader3d_SetColor({ diffuse.r, diffuse.g, diffuse.b, 1.0f });

		// Set vertex buffer and index buffer
		UINT stride = sizeof(Vertex3d);
		UINT offset = 0;

		Direct3D_GetDeviceContext()->IASetVertexBuffers(0, 1, &model->VertexBuffer[m], &stride, &offset);
		Direct3D_GetDeviceContext()->IASetIndexBuffer(model->IndexBuffer[m], DXGI_FORMAT_R32_UINT, 0);

		// Issue polygon draw command
		Direct3D_GetDeviceContext()->DrawIndexed(model->AiScene->mMeshes[m]->mNumFaces * 3, 0, 0);
	}
}
