/*==============================================================================

   Mesh Field Pixel Shader [shader_pixel_field.hlsl]
   Flat areas always use grass
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

    // Grass texture
    float4 grass = texGrass.Sample(samp, pi.uv * 8.0f);

    // Rock texture with rotated UV
    float angle = 3.14159265f * 45.0f / 180.0f;
    float2 rockUV;
    rockUV.x = pi.uv.x * cos(angle) + pi.uv.y * sin(angle);
    rockUV.y = -pi.uv.x * sin(angle) + pi.uv.y * cos(angle);
    float4 rock = texRock.Sample(samp, rockUV * 5.0f);

    // Height-based mask for rock
    float rockStart = 0.1f;
    float rockEnd = 3.5f;
    float heightMask = saturate((relativeHeight - rockStart) / (rockEnd - rockStart));

    // Slope-based mask for rock
    float slopeMask = saturate((pi.slope - 0.35f) / 0.4f);

    // Combine height and slope masks
    float rockMask = max(heightMask, slopeMask * heightMask);

    // Flat surface override
    float flatThreshold = 0.05f; // slopes below this are considered flat
    float flatMask = saturate(pi.slope / flatThreshold); // 0 = flat, 1 = non-flat
    rockMask *= flatMask; // flat areas force rockMask to 0 Å® grass

    // Final texture blending
    float4 texColor = lerp(grass, rock, rockMask);

    // Lighting
    float3 light = saturate(pi.directional.rgb + pi.ambient.rgb);

    return float4(texColor.rgb * light * 0.85f, texColor.a);
}
