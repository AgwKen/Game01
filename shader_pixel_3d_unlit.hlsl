/*==============================================================================

   shader pixel billboard [shader_pixel_billboard.hlsl]
                                                         Author : PYAE SONE THANT
                                                         Date   : 2025/14/11
--------------------------------------------------------------------------------

==============================================================================*/

Texture2D tex : register(t0);
SamplerState samp : register(s0);

// PS cb for tint color / global color (bound from C++)
cbuffer PS_CONSTANT_BUFFER : register(b0)
{
    float4 tintColor;
};

struct PS_IN
{
    float4 posH : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    float alpha : TEXCOORD1;
};

float4 main(PS_IN pi) : SV_Target
{
    float4 texc = tex.Sample(samp, pi.uv);

    // Multiply sampled color by vertex color then by the PS tint color
    texc *= pi.color;
    texc *= tintColor;

    // Apply alpha control (so C++ can change global alpha easily)
    texc.a *= pi.alpha;

    // Optional: discard very transparent pixels to improve blend precision
    if (texc.a < 0.01f)
        discard;

    return texc;
}
