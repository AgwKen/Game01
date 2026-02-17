    /*==============================================================================
       Mesh Field Vertex Shader WITH SHADOW
    ==============================================================================*/

    cbuffer VS_CONSTANT_BUFFER : register(b0)
    {
        float4x4 world;
    };

    cbuffer VS_CONSTANT_BUFFER : register(b1)
    {
        float4x4 view;
    };

    cbuffer VS_CONSTANT_BUFFER : register(b2)
    {
        float4x4 projection;
    };

    cbuffer VS_CONSTANT_BUFFER : register(b3)
    {
        float4 ambient_color;
    };

    cbuffer VS_CONSTANT_BUFFER : register(b4)
    {
        float4 directional_world_vector;
        float4 directional_color;
    };

    // ?? NEW: Light matrices
    cbuffer SHADOW_BUFFER : register(b5)
    {
        float4x4 lightView;
        float4x4 lightProjection;
    };

    struct VS_IN
    {
        float4 posL : POSITION0;
        float4 normalL : NORMAL0;
        float4 color : COLOR0;
        float2 uv : TEXCOORD0;
    };

    struct VS_OUT
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

    VS_OUT main(VS_IN vi)
    {
        VS_OUT vo;

        // World position
        float4 posW = mul(vi.posL, world);

        // Standard transform
        float4x4 mtxWVP = mul(mul(world, view), projection);
        vo.posH = mul(vi.posL, mtxWVP);

        // Light space position
        float4 lightPos = mul(posW, lightView);
        lightPos = mul(lightPos, lightProjection);
        vo.lightViewPos = lightPos;

        // World normal
        float3 normalW = normalize(
            mul(float4(vi.normalL.xyz, 0.0f), world).xyz
        );

        vo.slope = 1.0f - normalW.y;

        float dl = max(0.0f, dot(-directional_world_vector.xyz, normalW));
        vo.directional = float4(directional_color.rgb * dl, 1.0f);
        vo.ambient = float4(ambient_color.rgb, 1.0f);

        vo.color = vi.color;
        vo.uv = vi.uv;
        vo.height = posW.y;

        return vo;
    }
