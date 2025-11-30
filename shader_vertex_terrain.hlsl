/*==============================================================================

  Shader Vertex Terrain  [shader_vertex_terrain.hlsl]
														 Author : PYAE SONE THANT
														 Date   : 2025/24/11
--------------------------------------------------------------------------------

==============================================================================*/
cbuffer TerrainCB : register(b0)
{
    float4x4 gWorld;
    float4x4 gWVP;
    float3 gLightDir;
    float pad;
};

struct VS_IN
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

VS_OUT main(VS_IN input)
{
    VS_OUT output;

    float4 worldPos = mul(float4(input.pos, 1.0f), gWorld);
    output.pos = mul(worldPos, gWVP);

    output.normal = normalize(mul(float4(input.normal, 0.0f), gWorld).xyz);
    output.uv = input.uv;

    return output;
}
