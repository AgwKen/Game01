/*===============================================================================

  Map Cpp[Map.cpp]
														 Author : PYAE SONE THANT
														 Date   : 2025/11/11
---------------------------------------------------------------------------------

=================================================================================*/
#include "map.h"
#include <DirectXMath.h>
using namespace DirectX;
#include "cube.h"
#include "texture.h"

static MapObject g_MapObjects[]{
	{1, { 1.0f, 0.5f,  0.0f}},
	{1, {-1.0f, 0.5f,  0.0f}},
	{1, { 0.0f, 0.5f,  1.0f}},
	{1, { 1.0f, 0.5f,  1.0f}},
	{1, {-1.0f, 0.5f,  1.0f}},
	{1, { 0.0f, 0.5f,  2.0f}},
	{1, { 1.0f, 0.5f,  2.0f}},
	{1, {-1.0f, 0.5f,  2.0f}},
	{2, {-1.0f, 1.5f,  2.0f}},
	{2, { 0.0f, 1.5f, -1.0f}},
};

static int g_CubeTexHoroId{ -1 };
static int g_CubeTexCassId{ -1 };


void Map_Initialize()
{
	g_CubeTexHoroId = Texture_Load(L"Texture/wood.jpg");
	g_CubeTexCassId = Texture_Load(L"Texture/cass.png");

}

void Map_Finalize()
{
}

void Map_Draw()
{
	XMMATRIX mtxWorld;

	for (const MapObject& obj : g_MapObjects)
	{
		switch(obj.KindId)
		{
		case 1: // Cube
			mtxWorld = XMMatrixTranslation(obj.Position.x, obj.Position.y, obj.Position.z);
			CUBE_Draw(g_CubeTexHoroId,mtxWorld);
			break;

		case 2: // Decoration / floating cube
			mtxWorld = XMMatrixTranslation(obj.Position.x, obj.Position.y, obj.Position.z);
			CUBE_Draw(g_CubeTexCassId, mtxWorld);
			break;

		}
	}

}

int Map_GetObjectCount()
{
	return sizeof(g_MapObjects) / sizeof(g_MapObjects[0]);
}

const MapObject* Map_GetObject(int index)
{
	return &g_MapObjects[index];
}
