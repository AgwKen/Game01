/*==============================================================================

   shader pixel billboard[shader_pixel_billboard]		 Author : PYAE SONE THANT
														 Date   : 2025/14/11
--------------------------------------------------------------------------------

==============================================================================*/
struct PS_IN
{
    float4 posH : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

// ===== ADD THIS BLOCK (THIS IS THE MISSING PART) =====
cbuffer PS_COLOR : register(b0)
{
    float4 g_TintColor;
};
// =====================================================

Texture2D tex;
SamplerState samp;

float4 main(PS_IN pi) : SV_TARGET
{
    float4 texColor = tex.Sample(samp, pi.uv);

    // MULTIPLY BY:
    //  - texture color
    //  - vertex color (usually white)
    //  - C++ particle tint color
    float4 finalColor = texColor * pi.color * g_TintColor;

    // Alpha cutout
    if (finalColor.a < 0.5f)
    {
        discard;
    }

    return finalColor;
}

