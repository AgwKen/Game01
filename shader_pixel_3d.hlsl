/*==============================================================================

   3D shaderpixel[shader_pixel_3d.hlsl]S				 Author : PYAE SONE THANT
														 Date   : 2025/09/10
--------------------------------------------------------------------------------

==============================================================================*/
// constant buffer
cbuffer VS_CONSTANT_BUFFER : register(b0)
{
    float4 color;
};

struct PS_IN
{
    float4 posH : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};
Texture2D tex; // テクスチャ
SamplerState samp; //

float4 main(PS_IN pi) : SV_TARGET
{
    return tex.Sample(samp, pi.uv) * pi.color *color;
}
