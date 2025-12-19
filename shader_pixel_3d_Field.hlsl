/*==============================================================================

   Mesh Field Pixel Shader [shader_pixel_field.hlsl]

==============================================================================*/

struct PS_IN
{
    float4 posH : SV_POSITION;
    float4 color : COLOR0;
    float4 directional : COLOR1;
    float4 ambient : COLOR2;
    float2 uv : TEXCOORD0;
    float height : TEXCOORD1;
    float slope : TEXCOORD2;
};

Texture2D texGrass : register(t0);
Texture2D texRock : register(t1);
SamplerState samp : register(s0);

static const float TERRAIN_BASE_Y = 0.0f;

float4 main(PS_IN pi) : SV_TARGET
{
    float relativeHeight = pi.height - TERRAIN_BASE_Y;

    float4 grass = texGrass.Sample(samp, pi.uv * 0.8f);

    float angle = 3.14159265f * 45.0f / 180.0f;
    float2 rockUV;
    rockUV.x = pi.uv.x * cos(angle) + pi.uv.y * sin(angle);
    rockUV.y = -pi.uv.x * sin(angle) + pi.uv.y * cos(angle);

    float4 rock = texRock.Sample(samp, rockUV * 0.1f);

    float rockStart = 0.1f;
    float rockEnd = 3.5f;

    float heightMask = saturate(
        (relativeHeight - rockStart) / (rockEnd - rockStart)
    );

    float slopeMask = saturate(
        (pi.slope - 0.35f) / 0.4f
    );

    float rockMask = max(heightMask, slopeMask * heightMask);
    
    float4 texColor = lerp(grass, rock, rockMask);

    float3 light = saturate(pi.directional.rgb + pi.ambient.rgb);

    return float4(texColor.rgb * light * 0.85f, texColor.a);
}
