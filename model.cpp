#include <assert.h>
#include "direct3d.h"
#include "texture.h"
#include "model.h"
#include <DirectXMath.h>
#include "WICTextureLoader11.h"
#include "shader3d.h"
using namespace DirectX;
#include "shader3d_unlit.h"

// 3D頂点構造体
struct Vertex3d
{
	XMFLOAT3 position;	// 頂点座標
	XMFLOAT3 normal;	// 法線
	XMFLOAT4 color;		// 色
	XMFLOAT2 texcoord;	// UV
};

static int g_TextureWhite = -1;

MODEL* ModelLoad(const char* FileName, float scale, bool blender)
{
	MODEL* model = new MODEL;

	model->AiScene = aiImportFile(FileName, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded);


	if (!model->AiScene)
	{
		delete model;
		return nullptr;
	}

	model->VertexBuffer = new ID3D11Buffer * [model->AiScene->mNumMeshes];
	model->IndexBuffer = new ID3D11Buffer * [model->AiScene->mNumMeshes];

	for (unsigned int m = 0; m < model->AiScene->mNumMeshes; m++)
	{
		aiMesh* mesh = model->AiScene->mMeshes[m];
		assert(mesh);
		// 頂点バッファ生成
		{
			Vertex3d* vertex = new Vertex3d[mesh->mNumVertices];

			for (unsigned int v = 0; v < mesh->mNumVertices; v++)
			{
				vertex[v].position = XMFLOAT3(
					mesh->mVertices[v].x * scale,
					mesh->mVertices[v].y * scale,
					mesh->mVertices[v].z * scale);

				if (mesh->mTextureCoords && mesh->mTextureCoords[0])
					vertex[v].texcoord = XMFLOAT2(mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y);
				else
					vertex[v].texcoord = XMFLOAT2(0.0f, 0.0f);

				// ---- Safety: some models have no normals ----
				if (mesh->mNormals)
					vertex[v].normal = XMFLOAT3(mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z);
				else
					vertex[v].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);

				vertex[v].color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
				

				//aabbw
				if (v == 0 && m == 0) {
					model->local_aabb.min = vertex[v].position;
					model->local_aabb.max = vertex[v].position;
				}
				else {
					model->local_aabb.min.x = std::min(model->local_aabb.min.x, vertex[v].position.x);
					model->local_aabb.min.y = std::min(model->local_aabb.min.y, vertex[v].position.y);
					model->local_aabb.min.z = std::min(model->local_aabb.min.z, vertex[v].position.z);
					model->local_aabb.max.x = std::max(model->local_aabb.max.x, vertex[v].position.x);
					model->local_aabb.max.y = std::max(model->local_aabb.max.y, vertex[v].position.y);
					model->local_aabb.max.z = std::max(model->local_aabb.max.z, vertex[v].position.z);
				}
			}

			D3D11_BUFFER_DESC bd{};
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(Vertex3d) * mesh->mNumVertices;
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

			D3D11_SUBRESOURCE_DATA sd{};
			sd.pSysMem = vertex;

			HRESULT hr = Direct3D_GetDevice()->CreateBuffer(&bd, &sd, &model->VertexBuffer[m]);
			assert(SUCCEEDED(hr));

			delete[] vertex;
		}

		// インデックスバッファ生成
		{
			unsigned int indexCount = 0;
			for (unsigned int f = 0; f < mesh->mNumFaces; f++)
				indexCount += mesh->mFaces[f].mNumIndices;

			unsigned int* index = new unsigned int[indexCount];
			unsigned int idx = 0;

			for (unsigned int f = 0; f < mesh->mNumFaces; f++)
			{
				const aiFace* face = &mesh->mFaces[f];
				for (unsigned int i = 0; i < face->mNumIndices; i++)
					index[idx++] = face->mIndices[i];
			}

			D3D11_BUFFER_DESC bd{};
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(unsigned int) * indexCount;
			bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

			D3D11_SUBRESOURCE_DATA sd{};
			sd.pSysMem = index;

			HRESULT hr = Direct3D_GetDevice()->CreateBuffer(&bd, &sd, &model->IndexBuffer[m]);
			assert(SUCCEEDED(hr));

			delete[] index;
		}
	}

	// 白テクスチャ（フォールバック）
	g_TextureWhite = Texture_Load(L"white.png");

	// テクスチャ読み込み（埋め込み）
	for (unsigned int i = 0; i < model->AiScene->mNumTextures; i++)
	{
		aiTexture* aitexture = model->AiScene->mTextures[i];
		if (!aitexture) continue;

		ID3D11ShaderResourceView* texture = nullptr;
		ID3D11Resource* resource = nullptr;

		if (aitexture->mHeight == 0)
		{
			CreateWICTextureFromMemory(
				Direct3D_GetDevice(),
				Direct3D_GetDeviceContext(),
				(const uint8_t*)aitexture->pcData,
				(size_t)aitexture->mWidth,
				&resource,
				&texture);
		}
		else
		{
			D3D11_TEXTURE2D_DESC desc{};
			desc.Width = aitexture->mWidth;
			desc.Height = aitexture->mHeight;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc.SampleDesc.Count = 1;
			desc.Usage = D3D11_USAGE_IMMUTABLE;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

			D3D11_SUBRESOURCE_DATA init{};
			init.pSysMem = aitexture->pcData;
			init.SysMemPitch = aitexture->mWidth * 4;

			ID3D11Texture2D* tex = nullptr;
			HRESULT hr = Direct3D_GetDevice()->CreateTexture2D(&desc, &init, &tex);
			if (SUCCEEDED(hr))
				Direct3D_GetDevice()->CreateShaderResourceView(tex, nullptr, &texture);
			if (tex) tex->Release();
		}

		if (texture)
		{
			if (aitexture->mFilename.length > 0)
				model->Texture[aitexture->mFilename.C_Str()] = texture;

			char keyBuf[32];
			sprintf_s(keyBuf, sizeof(keyBuf), "*%u", i);
			model->Texture[keyBuf] = texture;

			if (resource) resource->Release();
		}
	}

	// テクスチャ読み込み（外部ファイル）
	const std::string modelPath(FileName);
	size_t pos = modelPath.find_last_of("/\\");
	std::string directory = (pos != std::string::npos) ? modelPath.substr(0, pos) : "";

	for (unsigned int m = 0; m < model->AiScene->mNumMeshes; m++)
	{
		aiString filename;
		aiMaterial* aimaterial = model->AiScene->mMaterials[model->AiScene->mMeshes[m]->mMaterialIndex];
		if (!aimaterial) continue;

		if (AI_SUCCESS != aimaterial->GetTexture(aiTextureType_DIFFUSE, 0, &filename))
			continue;
		if (filename.length == 0)
			continue;

		if (model->Texture.count(filename.C_Str()))
			continue;

		std::string texfilename = directory + "/" + filename.C_Str();

		int len = MultiByteToWideChar(CP_UTF8, 0, texfilename.c_str(), -1, nullptr, 0);
		wchar_t* pWideFilename = new wchar_t[len];
		MultiByteToWideChar(CP_UTF8, 0, texfilename.c_str(), -1, pWideFilename, len);

		ID3D11ShaderResourceView* texture = nullptr;
		ID3D11Resource* resource = nullptr;
		HRESULT hr = CreateWICTextureFromFile(
			Direct3D_GetDevice(),
			Direct3D_GetDeviceContext(),
			pWideFilename,
			&resource,
			&texture);

		delete[] pWideFilename;

		if (SUCCEEDED(hr) && texture)
		{
			model->Texture[filename.C_Str()] = texture;
			if (resource) resource->Release();
		}
	}

	return model;
}

void ModelRelease(MODEL* model)
{
	if (!model) return;
	if (model->AiScene)
	{
		for (unsigned int m = 0; m < model->AiScene->mNumMeshes; m++)
		{
			if (model->VertexBuffer && model->VertexBuffer[m]) model->VertexBuffer[m]->Release();
			if (model->IndexBuffer && model->IndexBuffer[m])  model->IndexBuffer[m]->Release();
		}
	}


	delete[] model->VertexBuffer;
	delete[] model->IndexBuffer;

	for (auto& pair : model->Texture)
	{
		if (pair.second) pair.second->Release();
	}
	if (model->AiScene)
		aiReleaseImport(model->AiScene);

	delete model;
}

void ModelDraw(MODEL* model, const DirectX::XMMATRIX& mtxWorld)
{
	if (!model || !model->AiScene) return;

	Shader3d_Begin();
	Direct3D_GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Shader3d_SetWorldMatrix(mtxWorld);

	static ID3D11ShaderResourceView* nullSRV[1] = { nullptr };

	for (unsigned int m = 0; m < model->AiScene->mNumMeshes; m++)
	{
		aiMaterial* aimaterial = model->AiScene->mMaterials[model->AiScene->mMeshes[m]->mMaterialIndex];
		if (!aimaterial) continue;

		aiString texPath;
		bool foundTexture = false;
		ID3D11ShaderResourceView* texSRV = nullptr;

		// 1) Try classic diffuse
		if (AI_SUCCESS == aimaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) && texPath.length != 0)
		{
			std::string key = texPath.C_Str();
			if (model->Texture.count(key))
			{
				texSRV = model->Texture[key];
				foundTexture = true;
			}
			else
			{
				// Try basename
				const std::string pathStr = key;
				size_t pos = pathStr.find_last_of("/\\");
				if (pos != std::string::npos)
				{
					std::string base = pathStr.substr(pos + 1);
					if (model->Texture.count(base))
					{
						texSRV = model->Texture[base];
						foundTexture = true;
					}
				}
			}
		}

		// 2) Try PBR base color
		if (!foundTexture)
		{
			if (AI_SUCCESS == aimaterial->GetTexture(aiTextureType_BASE_COLOR, 0, &texPath) && texPath.length != 0)
			{
				std::string key = texPath.C_Str();
				if (model->Texture.count(key))
				{
					texSRV = model->Texture[key];
					foundTexture = true;
				}
			}
		}

		// 3) Apply
		if (foundTexture && texSRV)
		{
			Direct3D_GetDeviceContext()->PSSetShaderResources(0, 1, &texSRV);
			Shader3d_SetColor({ 1,1,1,1 });
		}
		else
		{
			Direct3D_GetDeviceContext()->PSSetShaderResources(0, 1, nullSRV);
			Texture_SetTexture(g_TextureWhite);
			aiColor3D diffuse(1, 1, 1);
			aimaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
			Shader3d_SetColor({ diffuse.r, diffuse.g, diffuse.b, 1 });
		}

		UINT stride = sizeof(Vertex3d);
		UINT offset = 0;
		Direct3D_GetDeviceContext()->IASetVertexBuffers(0, 1, &model->VertexBuffer[m], &stride, &offset);
		Direct3D_GetDeviceContext()->IASetIndexBuffer(model->IndexBuffer[m], DXGI_FORMAT_R32_UINT, 0);
		Direct3D_GetDeviceContext()->DrawIndexed(model->AiScene->mMeshes[m]->mNumFaces * 3, 0, 0);
	}
}
void ModelUnlitDraw(MODEL* model, const DirectX::XMMATRIX& mtxWorld)
{
	if (!model || !model->AiScene) return;

	Shader3dUnlit_Begin();
	Direct3D_GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Shader3dUnlit_SetWorldMatrix(mtxWorld);

	static ID3D11ShaderResourceView* nullSRV[1] = { nullptr };

	for (unsigned int m = 0; m < model->AiScene->mNumMeshes; m++)
	{
		aiMaterial* aimaterial = model->AiScene->mMaterials[model->AiScene->mMeshes[m]->mMaterialIndex];
		if (!aimaterial) continue;

		aiString texPath;
		bool foundTexture = false;
		ID3D11ShaderResourceView* texSRV = nullptr;

		// 1) Try classic diffuse
		if (AI_SUCCESS == aimaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) && texPath.length != 0)
		{
			std::string key = texPath.C_Str();
			if (model->Texture.count(key))
			{
				texSRV = model->Texture[key];
				foundTexture = true;
			}
			else
			{
				// Try basename
				const std::string pathStr = key;
				size_t pos = pathStr.find_last_of("/\\");
				if (pos != std::string::npos)
				{
					std::string base = pathStr.substr(pos + 1);
					if (model->Texture.count(base))
					{
						texSRV = model->Texture[base];
						foundTexture = true;
					}
				}
			}
		}

		// 2) Try PBR base color
		if (!foundTexture)
		{
			if (AI_SUCCESS == aimaterial->GetTexture(aiTextureType_BASE_COLOR, 0, &texPath) && texPath.length != 0)
			{
				std::string key = texPath.C_Str();
				if (model->Texture.count(key))
				{
					texSRV = model->Texture[key];
					foundTexture = true;
				}
			}
		}

		// 3) Apply
		if (foundTexture && texSRV)
		{
			Direct3D_GetDeviceContext()->PSSetShaderResources(0, 1, &texSRV);
			Shader3dUnlit_SetColor({ 1,1,1,1 });
		}
		else
		{
			Direct3D_GetDeviceContext()->PSSetShaderResources(0, 1, nullSRV);
			Texture_SetTexture(g_TextureWhite);
			aiColor3D diffuse(1, 1, 1);
			aimaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
			Shader3dUnlit_SetColor({ diffuse.r, diffuse.g, diffuse.b, 1 });
		}

		UINT stride = sizeof(Vertex3d);
		UINT offset = 0;
		Direct3D_GetDeviceContext()->IASetVertexBuffers(0, 1, &model->VertexBuffer[m], &stride, &offset);
		Direct3D_GetDeviceContext()->IASetIndexBuffer(model->IndexBuffer[m], DXGI_FORMAT_R32_UINT, 0);
		Direct3D_GetDeviceContext()->DrawIndexed(model->AiScene->mMeshes[m]->mNumFaces * 3, 0, 0);
	}
}


AABB Model_GetAABB(MODEL* model, const DirectX::XMFLOAT3& position)
{
	return {
	   { position.x + model->local_aabb.min.x, position.y + model->local_aabb.min.y, position.z + model->local_aabb.min.z },
	   { position.x + model->local_aabb.max.x, position.y + model->local_aabb.max.y, position.z + model->local_aabb.max.z}
	};
}