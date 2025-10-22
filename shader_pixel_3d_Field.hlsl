/*==============================================================================

   shaderpixel Field [shader_pixel_field.hlsl]		     Author : PYAE SONE THANT
														 Date   : 2025/09/26
--------------------------------------------------------------------------------

==============================================================================*/
struct PS_IN
{
    float4 posH : SV_POSITION;
    float4 color : COLOR0;
    float4 directional : COLOR1;
    float4 ambient : COLOR2;
    float2 uv : TEXCOORD0;
};

Texture2D tex0 : register(t0);
Texture2D tex1 : register(t1);
SamplerState samp : register(s0);

float4 main(PS_IN pi) : SV_TARGET
{
    float angle = 3.14159265f * 45.0f / 180.0f;
    float2 uv;
    uv.x = pi.uv.x * cos(angle) + pi.uv.y * sin(angle);
    uv.y = -pi.uv.x * sin(angle) + pi.uv.y * cos(angle);

    float4 tex_color = tex0.Sample(samp, uv) * 0.5f + tex1.Sample(samp, pi.uv * 0.1f) * 0.5f;
    return tex_color * pi.directional + tex_color * pi.ambient;
}
