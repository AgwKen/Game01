#pragma once

#include <unordered_map>
#include <d3d11.h>
#include <DirectXMath.h>
#include "Assimp/assimp/cimport.h"
#include "Assimp/assimp/scene.h"
#include "Assimp/assimp/postprocess.h"
#include "Assimp/assimp/matrix4x4.h"
#pragma comment (lib, "assimp-vc143-mt.lib")

struct MODEL
{
    const aiScene* AiScene = nullptr;

    ID3D11Buffer** VertexBuffer = nullptr;
    ID3D11Buffer** IndexBuffer = nullptr;

    std::unordered_map<std::string, ID3D11ShaderResourceView*> Texture;
};


MODEL* ModelLoad(const char* FileName,float scale=1.0f);
void ModelRelease(MODEL* model);

void ModelDraw(MODEL* model, const DirectX :: XMMATRIX& mtxWorld);

