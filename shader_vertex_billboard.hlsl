/*==============================================================================

  Shader Vertex Billboard  [shader_vertex_billboard.hlsl]
                                                         Author : PYAE SONE THANT
                                                         Date   : 2025/14/11
--------------------------------------------------------------------------------

==============================================================================*/

// world, view, projection are separate cbuffers mapped to b0, b1, b2
cbuffer VS_WORLD : register(b0)
{
    float4x4 world;
};
cbuffer VS_VIEW : register(b1)
{
    float4x4 view;
};
cbuffer VS_PROJ : register(b2)
{
    float4x4 projection;
};

// parameters for UV, scale, translation and fog alpha are in b3
cbuffer VS_PARAMS : register(b3)
{
    float2 scale; // how to scale the vertex UVs (sheet cut size)
    float2 translation; // UV translation (sheet cut offset)

    float2 uvOffset; // UV offset for scrolling the texture (fog movement)
    float alpha; // global alpha multiplier (e.g. fog alpha)
    float pad; // pad to 16-byte boundary
};

struct VS_IN
{
    float4 posL : POSITION0;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

struct VS_OUT
{
    float4 posH : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    float alpha : TEXCOORD1; // pass alpha to pixel shader
};

VS_OUT main(VS_IN vi)
{
    VS_OUT vo;

    // build world-view-projection
    float4x4 mtxWV = mul(world, view);
    float4x4 mtxWVP = mul(mtxWV, projection);

    vo.posH = mul(vi.posL, mtxWVP);

    vo.color = vi.color;
    vo.uv = vi.uv * scale + translation + uvOffset;
    vo.alpha = alpha;

    return vo;
}
