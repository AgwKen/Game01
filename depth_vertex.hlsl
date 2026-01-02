// depth_vertex.hlsl
cbuffer MatrixBuffer : register(b0)
{
    float4x4 worldMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
};

struct VS_IN
{
    float4 posL : POSITION;
};

struct VS_OUT
{
    float4 posH : SV_POSITION;
    float4 depthPos : TEXCOORD0;
};

VS_OUT main(VS_IN vi)
{
    VS_OUT vo;
    float4 posW = mul(vi.posL, worldMatrix);
    float4 posV = mul(posW, viewMatrix);
    vo.posH = mul(posV, projectionMatrix);
    vo.depthPos = vo.posH; // Store for pixel shader
    return vo;
}