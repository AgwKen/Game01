/*==============================================================================

  Skybox Pixel Shader [shader_pixel_skybox.hlsl]
														 Author : PYAE SONE THANT
														 Date   : 2025/16/12
--------------------------------------------------------------------------------

==============================================================================*/

TextureCube skyTex : register(t0);
SamplerState samp : register(s0);

struct PS_IN
{
    float4 posH : SV_POSITION;
    float3 dir : TEXCOORD0;
};

float4 main(PS_IN pi) : SV_Target
{
    // Sample cubemap using the direction vector
    float4 color = skyTex.Sample(samp, pi.dir);

    return color; // fully unlit
}
