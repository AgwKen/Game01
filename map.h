/*===============================================================================

  Map header[Map.h]
														 Author : PYAE SONE THANT
														 Date   : 2025/11/11
---------------------------------------------------------------------------------

=================================================================================*/
#ifndef MAP_H
#define MAP_H

#include <DirectXMath.h>

void Map_Initialize();
void Map_Finalize();

//void Map_Update(double elapsed_time);
void Map_Draw();

int Map_GetObjectCount();

struct MapObject
{
	int KindId;
	DirectX::XMFLOAT3 Position;
};
const MapObject* Map_GetObject(int index);

#endif //MAP_H
