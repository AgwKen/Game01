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
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct VS_OUT
{
    float4 posH : SV_POSITION;
    float4 posW : POSITION0;
    float4 normalW : NORMAL0;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

VS_OUT main(VS_IN input)
{
    VS_OUT output;

    float4 worldPos = mul(float4(input.pos, 1.0f), world);

    float4 viewPos = mul(worldPos, view);
    output.posH = mul(viewPos, projection);

    output.posW = worldPos;
    output.normalW = float4(input.normal, 0.0f);
    output.color = input.color;
    output.uv = input.uv;

    return output;
}