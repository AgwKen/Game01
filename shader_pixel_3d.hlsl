//============================================================
// Shader Pixel 3D (NO SHADOW VERSION)
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
};

Texture2D tex : register(t0);
SamplerState samp : register(s0);

float4 main(PS_IN pi) : SV_Target
{
    float4 texSample = tex.Sample(samp, pi.uv);

    float3 material_color =
        texSample.rgb *
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

    float alpha =
        texSample.a *
        pi.color.a *
        diffuse_color.a;

    float3 finalColor = ambient + diffuse + specular;

    return float4(finalColor, alpha);
}