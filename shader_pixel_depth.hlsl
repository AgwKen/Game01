/*==============================================================================

[shader_pixel_depth.hlsl]                   		 Author : PYAE SONE THANT
												     Date   : 2025/14/11
--------------------------------------------------------------------------------

==============================================================================*/
struct PS_IN
{
    float4 posH : SV_POSITION;
    float4 posW : POSITION0;
};

float4 main(PS_IN pi) : SV_TARGET
{
    return float4(pi.posW.z, pi.posW.z, pi.posW.z, 1.0f);
}
