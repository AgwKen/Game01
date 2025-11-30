/*==============================================================================

  Shader pixel terrain  [shader_pixel_terrain.hlsl]
														 Author : PYAE SONE THANT
														 Date   : 2025/24/11
--------------------------------------------------------------------------------

==============================================================================*/
Texture2D terrainTex : register(t0);
SamplerState samp : register(s0);

cbuffer TerrainCB : register(b0)
{
    float4x4 gWorld;
    float4x4 gWVP;
    float3 gLightDir;
    float pad;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

float4 main(PS_IN input) : SV_TARGET
{
    float4 baseColor = terrainTex.Sample(samp, input.uv);

    float light = saturate(dot(normalize(input.normal), -normalize(gLightDir)));
    light = lerp(0.6f, 1.0f, light);

    return baseColor * light;
}

