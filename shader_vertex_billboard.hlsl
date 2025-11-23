/*==============================================================================

  Shader Vertex Billboard  [shader_vertex_billboard.hlsl]
														 Author : PYAE SONE THANT
														 Date   : 2025/14/11
--------------------------------------------------------------------------------

==============================================================================*/

// constant buffer
cbuffer VS_CONSTANT_BUFFER : register(b0)
{
    float4x4 world;
};

cbuffer VS_CONSTANT_BUFFER : register(b1)
{
    float4x4 view;
};
   
cbuffer VS_CONSTANT_BUFFER : register(b2)
{
    float4x4 projection;
};

cbuffer VS_CONSTANT_BUFFER : register(b3)
{
    float2 scale;
    float2 translation;
};

struct VS_IN
{
    float4 posL : POSITION0;
    float4 color : COLOR0; // its zero not O
    float2 uv : TEXCOORD0;
};

struct VS_OUT
{
    float4 posH : SV_POSITION;
    float4 color : COLOR0; // its zero not O
    float2 uv : TEXCOORD0;
};

VS_OUT main(VS_IN vi)
{
    VS_OUT vo;
   
    float4x4 mtxWV = mul(world, view);
    float4x4 mtxWVP = mul(mtxWV, projection);
    vo.posH = mul(vi.posL, mtxWVP);

    vo.color = vi.color;
    vo.uv = vi.uv * scale + translation;

    return vo;
}