/*==============================================================================

   Mesh Fieldシェーダーバーテクステ [shader_vertex_field.hlsl]
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

cbuffer VS_CONSTANT_BUFFER : register(b3)
{
    float4 ambient_color;
};

cbuffer VS_CONSTANT_BUFFER : register(b4)
{
    float4 directional_world_vector;
    float4 directional_color;
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
    float4 color : COLOR0; // its zero not O
    float4 directional: COLOR1; 
    float4 ambient : COLOR2;
    float2 uv : TEXCOORD0;
};

//=============================================================================
//頂点シェーダー
//=============================================================================
VS_OUT main(VS_IN vi)
{
    VS_OUT vo;
    
    //// coordinate transformation
   // float4 posW = mul(vi.posL, world);
  //  float4 posWV = mul(posW, view);
  //  vo.posH = mul(posWV, projection);

  //  vo.color = vi.color;
    
    float4x4 mtxWV = mul(world, view);
    float4x4 mtxWVP = mul(mtxWV, projection);
    vo.posH = mul(vi.posL, mtxWVP);
    
    //caculate lighting
    //Normal world transformation matrix x transform normal vector from local space to world space
    //The transposed inverse of the world transformation matrix
    
    float4 normalW = mul(float4(vi.normalL.xyz, 0.0f), world);
    normalW = normalize(normalW);
    float dl = max(0.0f, dot(-directional_world_vector, normalW));
    
    vo.color = vi.color;
    
    vo.directional = float4(directional_color.rgb * dl, 1.0f);
    vo.ambient = float4(ambient_color.rgb, 1.0f);
    vo.uv = vi.uv;

    return vo;
}