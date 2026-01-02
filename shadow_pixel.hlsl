Texture2D shaderTexture : register(t0);
Texture2D depthMapTexture : register(t1);
SamplerState sampClamp : register(s0);
SamplerState sampWrap : register(s1);

cbuffer LightBuffer : register(b0)
{
    float4 ambientColor;
    float4 diffuseColor;
    float bias; // This can now be a smaller base value (e.g., 0.001)
    float3 lightPadding;
};

struct PS_IN
{
    float4 posH : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normalW : NORMAL0;
    float4 lightViewPos : TEXCOORD1;
    float3 lightVecW : TEXCOORD2;
};

float4 main(PS_IN pi) : SV_TARGET
{
    float4 textureColor;
    float2 projectTexCoord;
    float lightDepthValue;
    float shadow = 0.0;

    // 1. Calculate projected coordinates for shadow map [cite: 7, 8]
    projectTexCoord.x = pi.lightViewPos.x / pi.lightViewPos.w / 2.0f + 0.5f;
    projectTexCoord.y = -pi.lightViewPos.y / pi.lightViewPos.w / 2.0f + 0.5f;

    // 2. Slope-Scaled Bias [cite: 10]
    // Adjust bias based on the angle of the surface to the light to prevent acne
    float3 L = normalize(pi.lightVecW);
    float3 N = normalize(pi.normalW);
    float cosTheta = saturate(dot(N, L));
    float currentBias = max(0.005 * (1.0 - cosTheta), bias);

    lightDepthValue = (pi.lightViewPos.z / pi.lightViewPos.w) - currentBias;

    // 3. PCF Filtering (3x3 Kernel)
    // Smoothing the edges by sampling neighboring pixels 
    if (saturate(projectTexCoord.x) == projectTexCoord.x && saturate(projectTexCoord.y) == projectTexCoord.y)
    {
        float width, height;
        depthMapTexture.GetDimensions(width, height);
        float2 texelSize = 1.0 / float2(width, height);

        for (int x = -1; x <= 1; ++x)
        {
            for (int y = -1; y <= 1; ++y)
            {
                float depthValue = depthMapTexture.Sample(sampClamp, projectTexCoord + float2(x, y) * texelSize).r;
                shadow += (lightDepthValue < depthValue) ? 1.0f : 0.0f;
            }
        }
        shadow /= 9.0f; // Average the 9 samples
    }
    else
    {
        shadow = 1.0f; // Outside shadow map is lit
    }

    // 4. Lighting Calculation [cite: 6, 11, 12]
    float lightIntensity = saturate(dot(N, L));
    float4 lighting = ambientColor;
    if (lightIntensity > 0.0f)
    {
        lighting += (diffuseColor * lightIntensity * shadow);
    }

    // 5. Final Color [cite: 13]
    textureColor = shaderTexture.Sample(sampWrap, pi.uv);
    return saturate(lighting) * textureColor;
}