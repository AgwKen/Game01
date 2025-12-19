/*==============================================================================

  Skybox Vertex Shader [shader_vertex_skybox.hlsl]
														 Author : PYAE SONE THANT
														 Date   : 2025/16/12
--------------------------------------------------------------------------------

==============================================================================*/

// constant buffers
cbuffer VS_CONSTANT_BUFFER : register(b0)
{
    float4x4 view; // Camera view matrix (translation removed!)
};

cbuffer VS_CONSTANT_BUFFER : register(b1)
{
    float4x4 projection; // Projection matrix
}

struct VS_IN
{
    float4 posL : POSITION0;
};

struct VS_OUT
{
    float4 posH : SV_POSITION;
    float3 dir : TEXCOORD0; // Direction for cubemap sampling
};

VS_OUT main(VS_IN vi)
{
    VS_OUT vo;

    // Transform vertex by view (rotation only) and projection
    float4 pos = mul(vi.posL, view);
    vo.posH = mul(pos, projection);

    // Pass direction for cubemap (use world-space position without translation)
    vo.dir = vi.posL.xyz;

    return vo;
}
