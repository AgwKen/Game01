// depth_vertex.hlsl
cbuffer MatrixBuffer : register(b0)
{
    float4x4 worldMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
};

struct VS_IN
{
    float3 posL : POSITION;
};

struct VS_OUT
{
    float4 posH : SV_POSITION;
};

VS_OUT main(VS_IN vi)
{
    VS_OUT vo;

    float4 posW = mul(float4(vi.posL, 1.0f), worldMatrix);
    float4 posV = mul(posW, viewMatrix);
    vo.posH = mul(posV, projectionMatrix);

    return vo;
}
