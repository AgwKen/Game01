/*==============================================================================

   shader pixel billboard[shader_pixel_billboard]		 Author : PYAE SONE THANT
														 Date   : 2025/14/11
--------------------------------------------------------------------------------

==============================================================================*/
struct PS_IN
{
    float4 posH : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

Texture2D tex; // テクスチャ
SamplerState samp; //

float4 main(PS_IN pi) : SV_TARGET
{
   // float4 color = tex.Sample(samp, pi.uv) * pi.color;
   // if (color.a < 0.5f)
   // {
   //     discard;
   // }
   // return color;
    
    return tex.Sample(samp, pi.uv) * pi.color;
}
