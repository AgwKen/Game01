Texture2D tex : register(t0);
SamplerState samp : register(s0);

cbuffer PS_CONSTANT_BUFFER : register(b0)
{
    float4 color;
};

struct PS_IN
{
    float4 posH : SV_POSITION;
    float4 posW : POSITION0;
    float4 normalW : NORMAL0;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

float4 main(PS_IN input) : SV_Target
{
    float4 texColor = tex.Sample(samp, input.uv);
    return texColor * color;
}