/*==============================================================================
   Mesh Field Pixel Shader WITH SHADOW (Stable Version)
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
    float4 lightViewPos : TEXCOORD3;
};

Texture2D texGrass : register(t0);
Texture2D texRock : register(t1);
Texture2D shadowMap : register(t2);

SamplerState samp : register(s0);
SamplerComparisonState shadowSampler : register(s1);

static const float TERRAIN_BASE_Y = 0.0f;

float4 main(PS_IN pi) : SV_TARGET
{
    // =========================================
    // TERRAIN TEXTURE BLENDING
    // =========================================

    float relativeHeight = pi.height - TERRAIN_BASE_Y;

    float4 grass = texGrass.Sample(samp, pi.uv * 8.0f);

    float angle = 3.14159265f * 45.0f / 180.0f;
    float2 rockUV;
    rockUV.x = pi.uv.x * cos(angle) + pi.uv.y * sin(angle);
    rockUV.y = -pi.uv.x * sin(angle) + pi.uv.y * cos(angle);

    float4 rock = texRock.Sample(samp, rockUV * 5.0f);

    float rockStart = 0.1f;
    float rockEnd = 3.5f;

    float heightMask = saturate((relativeHeight - rockStart) / (rockEnd - rockStart));
    float slopeMask = saturate((pi.slope - 0.35f) / 0.4f);

    float rockMask = max(heightMask, slopeMask * heightMask);

    float flatThreshold = 0.05f;
    float flatMask = saturate(pi.slope / flatThreshold);
    rockMask *= flatMask;

    float4 texColor = lerp(grass, rock, rockMask);

    // =========================================
    // SHADOW CALCULATION
    // =========================================

    float shadow = 1.0f;

    // Perspective divide
    float3 projCoords = pi.lightViewPos.xyz / pi.lightViewPos.w;

    // Convert from NDC (-1..1) to texture space (0..1)
    projCoords.xy = projCoords.xy * 0.5f + 0.5f;
    projCoords.y = 1.0f - projCoords.y;

    // Convert depth to 0..1
    float lightDepth = projCoords.z * 0.5f + 0.5f;

    if (projCoords.x >= 0.0f && projCoords.x <= 1.0f &&
    projCoords.y >= 0.0f && projCoords.y <= 1.0f &&
    lightDepth >= 0.0f && lightDepth <= 1.0f)
    {
        float bias = 0.002f;

        shadow = shadowMap.SampleCmpLevelZero(
        shadowSampler,
        projCoords.xy,
        lightDepth - bias
    );
    }
    // =========================================
    // LIGHTING
    // =========================================

    float3 lighting = saturate(pi.directional.rgb + pi.ambient.rgb);

    float3 lit = pi.ambient.rgb + pi.directional.rgb * shadow;
    return float4(texColor.rgb * lit, texColor.a);

}
