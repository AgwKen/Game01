/*==============================================================================

   3Dシェーダーバーテクステ [shader_vertex_3d.hlsl]
														 Author : PYAE SONE THANT
														 Date   : 2025/09/10
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
    float4 normalL : NORMAL0;
    float4 color : COLOR0; // its zero not O
    float2 uv : TEXCOORD0;
};

struct VS_OUT
{
    float4 posH : SV_POSITION;
    float4 posW : POSITION0;
    float4 normalW : NORMAL0;
    float4 color : COLOR0;// its zero not O
    float2 uv : TEXCOORD0;
};

//=============================================================================
//頂点シェーダー
//=============================================================================
VS_OUT main(VS_IN vi)
{
    VS_OUT vo;
   
    float4x4 mtxWV = mul(world, view);
    float4x4 mtxWVP = mul(mtxWV, projection);   
    vo.posH = mul(vi.posL, mtxWVP);
    
     //Normal world transformation matrix x transform normal vector from local space to world space
    //The transposed inverse of the world transformation matrix
    
    float4 normalW = mul(float4(vi.normalL.xyz, 0.0f),world);
    vo.normalW = normalize(normalW);

    vo.posW = mul(vi.posL, world);
    
    vo.color = vi.color;
    vo.uv = vi.uv;

    return vo;
}