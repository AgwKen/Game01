/*==============================================================================
   Depth Pixel Shader [depth_pixel.hlsl]
==============================================================================*/

Texture2D shaderTexture : register(t0);
SamplerState sampWrap : register(s0);

struct PS_IN
{
    float4 posH : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(PS_IN pi) : SV_TARGET
{
    float alpha = shaderTexture.Sample(sampWrap, pi.uv).a;

    // Alpha clipping for shadow map
    clip(alpha - 0.1f);

    float depth = pi.posH.z / pi.posH.w;
    return float4(depth, depth, depth, 1.0f);
}
