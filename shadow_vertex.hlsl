/*==============================================================================
   Shadow Vertex Shader [shadow.vs]
==============================================================================*/

cbuffer MatrixBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    matrix lightView;
    matrix lightProjection;
};

cbuffer LightPositionBuffer : register(b1)
{
    float3 lightPosition;
    float padding;
};

struct VS_IN
{
    float4 posL : POSITION0;
    float2 uv : TEXCOORD0;
    float3 normalL : NORMAL0;
};

struct VS_OUT
{
    float4 posH : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normalW : NORMAL0;
    float4 lightViewPos : TEXCOORD1;
    float3 lightVecW : TEXCOORD2;
};

VS_OUT main(VS_IN vi)
{
    VS_OUT vo;
    float4 posW;

    // Ensure W is 1.0 for transformation
    vi.posL.w = 1.0f;

    // Standard camera projection
    posW = mul(vi.posL, world);
    vo.posH = mul(posW, view);
    vo.posH = mul(vo.posH, projection);
    
    // Projection from the light's perspective
    vo.lightViewPos = mul(vi.posL, world);
    vo.lightViewPos = mul(vo.lightViewPos, lightView);
    vo.lightViewPos = mul(vo.lightViewPos, lightProjection);

    vo.uv = vi.uv;
    
    // Transform normal to world space
    vo.normalW = mul(vi.normalL, (float3x3) world);
    vo.normalW = normalize(vo.normalW);

    // Vector from vertex to light source
    vo.lightVecW = lightPosition.xyz - posW.xyz;

    return vo;
}