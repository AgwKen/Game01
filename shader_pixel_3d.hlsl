/*==============================================================================

   3D shaderpixel[shader_pixel_3d.hlsl]S				 Author : PYAE SONE THANT
														 Date   : 2025/09/10
--------------------------------------------------------------------------------

==============================================================================*/
//============================================================
// Shader Pixel 3D [shader_pixel_3d.hlsl]
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
    float4 directional_color = { 1.0f, 1.0f, 1.0f, 1.0f };
};

cbuffer PS_CONSTANT_BUFFER : register(b3)
{
    float3 eye_posW;
    float specular_power = 30.0f;
    float4 specular_color = { 0.1f, 0.1f, 0.1f, 1.0f };
};

struct PointLight
{
    float3 posW;
    float range;
    float4 color;
};

cbuffer PS_CONSTANT_BUFFER : register(b4)
{
    PointLight point_light[4];
    int point_light_count;
    float3 point_light_dummy;
};

struct PS_IN
{
    float4 posH : SV_POSITION;
    float4 posW : POSITION0;
    float4 normalW : NORMAL0;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

Texture2D tex;
SamplerState samp;

float4 main(PS_IN pi) : SV_Target
{
    float3 material_color = tex.Sample(samp, pi.uv).rgb * pi.color.rgb * diffuse_color.rgb;

    float4 normalW = normalize(pi.normalW);
    float dl = (dot(-directional_world_vector, normalW) + 1.0f) * 0.5f;
    float3 diffuse = material_color * directional_color.rgb * dl;

    float3 ambient = material_color * ambient_color.rgb;

    float3 toEye = normalize(eye_posW - pi.posW.xyz);
    float3 r = reflect(directional_world_vector, normalW).xyz;
    float t = pow(max(dot(r, toEye), 0.0f), specular_power);
    float3 specular = specular_color.rgb * t;

    float alpha = tex.Sample(samp, pi.uv).a * pi.color.a * diffuse_color.a;
    
    
    float3 color = ambient + diffuse + specular;

    float lim = 1.0f - max(dot(normalW.xyz, toEye), 0.0f);
    lim = pow(lim, 3.2f);
   //color += float3(lim, lim, lim);

    for (int i = 0; i < point_light_count; i++)
    {
        float3 lightToPixel = pi.posW.xyz - point_light[i].posW;
        float D = length(lightToPixel);
        float A = pow(max(1.0f - 1.0f / point_light[i].range * D, 0.0f), 2.0f);
        float dl = max(0.0f, dot(-normalize(lightToPixel), normalW.xyz));

        color += material_color * point_light[i].color.rgb * A * dl;

        float3 r = reflect(normalize(lightToPixel), normalW.xyz);
        float t = pow(max(dot(r, toEye), 0.0f), specular_power);

        color += point_light[i].color.rgb * t;
    }

    return float4(color, alpha);
}
