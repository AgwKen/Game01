/*==============================================================================
   Mesh Field Pixel Shader WITH SHADOW
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
    float4 lightViewPos : TEXCOORD3; //  NEW
};

Texture2D texGrass : register(t0);
Texture2D texRock : register(t1);
Texture2D shadowMap : register(t2); // NEW

SamplerState samp : register(s0);
SamplerState shadowSampler : register(s1); //  NEW

static const float TERRAIN_BASE_Y = 0.0f;

float4 main(PS_IN pi) : SV_TARGET
{
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
    // ?? SHADOW CALCULATION
    // =========================================

    float shadow = 1.0f;

    float2 shadowUV;
    shadowUV.x = pi.lightViewPos.x / pi.lightViewPos.w * 0.5f + 0.5f;
    shadowUV.y = -pi.lightViewPos.y / pi.lightViewPos.w * 0.5f + 0.5f;

    float lightDepth = pi.lightViewPos.z / pi.lightViewPos.w;

    if (shadowUV.x >= 0 && shadowUV.x <= 1 &&
        shadowUV.y >= 0 && shadowUV.y <= 1)
    {
        float shadowDepth = shadowMap.Sample(shadowSampler, shadowUV).r;

        float bias = 0.002f;

        if (lightDepth > shadowDepth + bias)
            shadow = 0.4f; // darken in shadow
    }

    // =========================================

    float3 light = saturate(pi.directional.rgb + pi.ambient.rgb);

    return float4(texColor.rgb * light * shadow, texColor.a);
}
