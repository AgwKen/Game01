/*==============================================================================

  Fog Header [fog.h]
                                                         Author : PYAE SONE THANT
                                                         Date   : 2025/12/12
--------------------------------------------------------------------------------

==============================================================================*/

#ifndef FOG_H
#define FOG_H

#include <DirectXMath.h>

// A single fog puff
struct FogPuff
{
    DirectX::XMFLOAT3 position;     // world position
    float size;                      // size (billboard scale)
    float lifetime;                  // remaining time
    float totalLifetime;             // fade out calculation
    float speed;                     // drifting speed
};

// Initialize fog system (load texture, allocate puffs)
void Fog_Initialize();

// Remove everything
void Fog_Finalize();

// Update fog motion + UV scrolling
void Fog_Update(double elapsed_time);

// Draw all fog puffs (uses billboard shader)
void Fog_Draw();

// Spawn fog puff at position
void Fog_Spawn(const DirectX::XMFLOAT3& pos,
    float size = 6.0f,
    float lifetime = 8.0f);

#endif //FOG_H

