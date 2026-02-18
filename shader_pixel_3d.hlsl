//============================================================
// Shader Pixel 3D WITH SHADOW (Stable Version)
//============================================================

cbuffer PS_CONSTANT_BUFFER : register(b0)
{
    float4 diffuse_color;
};

cbuffer PS_CONSTANT_BUFFER : register(b1)
{
    float4 ambient_color;
};

cbuffer PS_CONSTANT_BUFFER : register(b2)
{
    float4 directional_world_vector;
    float4 directional_color;
};

cbuffer PS_CONSTANT_BUFFER : register(b3)
{
    float3 eye_posW;
    float specular_power;
    float4 specular_color;
};

struct PS_IN
{
    float4 posH : SV_POSITION;
    float4 posW : POSITION0;
    float4 normalW : NORMAL0;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    float4 lightViewPos : TEXCOORD1;
};

Texture2D tex : register(t0);
Texture2D shadowMap : register(t2);

SamplerState samp : register(s0);
SamplerComparisonState shadowSampler : register(s1);

float4 main(PS_IN pi) : SV_Target
{
    float3 material_color =
        tex.Sample(samp, pi.uv).rgb *
        pi.color.rgb *
        diffuse_color.rgb;

    float3 normalW = normalize(pi.normalW.xyz);

    // Directional lighting
    float dl = max(0.0f, dot(-directional_world_vector.xyz, normalW));
    float3 diffuse = material_color * directional_color.rgb * dl;

    float3 ambient = material_color * ambient_color.rgb;

    // Specular
    float3 toEye = normalize(eye_posW - pi.posW.xyz);
    float3 r = reflect(directional_world_vector.xyz, normalW);
    float spec = pow(max(dot(r, toEye), 0.0f), specular_power);
    float3 specular = specular_color.rgb * spec;

    float3 lighting = ambient + diffuse + specular;

    // =========================
    // SHADOW (CORRECT VERSION)
    // =========================

    float shadow = 1.0f;

    float3 projCoords = pi.lightViewPos.xyz / pi.lightViewPos.w;

    // NDC (-1..1) ¨ texture (0..1)
    projCoords.xy = projCoords.xy * 0.5f + 0.5f;
    projCoords.y = 1.0f - projCoords.y;

    // Depth to 0..1
    float lightDepth = projCoords.z * 0.5f + 0.5f;

    if (projCoords.x >= 0 && projCoords.x <= 1 &&
        projCoords.y >= 0 && projCoords.y <= 1 &&
        lightDepth >= 0 && lightDepth <= 1)
    {
        float bias = 0.002f;

        shadow = shadowMap.SampleCmpLevelZero(
            shadowSampler,
            projCoords.xy,
            lightDepth - bias
        );
    }

    float alpha =
        tex.Sample(samp, pi.uv).a *
        pi.color.a *
        diffuse_color.a;

    float3 finalColor = ambient + diffuse * shadow + specular * shadow;
    return float4(finalColor, alpha);

}
