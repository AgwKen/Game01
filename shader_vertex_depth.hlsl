/*==============================================================================

[shader_pixel_depth.hlsl]                   		 Author : PYAE SONE THANT
												     Date   : 2025/14/11
--------------------------------------------------------------------------------

==============================================================================*/
/*==============================================================================

  NO Light Shader Vertex [shader_vertex_3d_unlit.hlsl]
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


struct VS_IN
{
    float4 posL : POSITION0;
};

struct VS_OUT
{
    float4 posH : SV_POSITION;
    float4 posW : POSITION0;
};

VS_OUT main(VS_IN vi)
{
    VS_OUT vo;

    vo.posW = mul(vi.posL, world);
    float4x4 mtxVP = mul(view, projection);
    vo.posH = mul(vo.posW, mtxVP);

    return vo;
}