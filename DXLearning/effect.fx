string desc = "some effects";
// https://docs.microsoft.com/en-us/windows/win32/direct3d9/effect-states

matrix world_transform;
matrix view_transform;
matrix proj_transform;
matrix obj_to_view_transform;
matrix obj_to_proj_transform;

float3 light_dir = { 0.0f,-0.99f,-0.14f };
float4 diffuse = { 1,1,1,1 };

float4 white = { 1,1,1,1 };
float4 black = { 0,0,0,1 };
float outline = 0.05f;
float time;

texture2D tex0 < string name = "tex_cartoon.png"; > ; // annotation
sampler2D samp0 = sampler_state
{
    Texture = tex0;
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};



struct VSInput
{
    float4 pos : POSITION;
    float4 color : COLOR;
    float4 normal : NORMAL;
    float2 tex : TEXCOORD;
};
struct VSOutput
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float2 tex : TEXCOORD;
};
VSOutput vs0_main(VSInput input)
{
    VSOutput output;
    float4 pos = mul(input.pos, obj_to_proj_transform);
    output.pos = pos;

    float4 N = { input.normal.xyz, 0 };
    N = normalize(mul(N, obj_to_view_transform)); // normal
    float4 V = { -mul(input.pos, obj_to_view_transform).xyz, 0 };
    V = normalize(V); // to eye
    float4 ld = { light_dir, 0 };
    float4 L = normalize(-mul(ld, view_transform)); // to light

    float s = min(0.99f, max(0.01f, 0.9f * dot(L, N)));
    output.tex.x = s;
    output.tex.y = 0.5f;
    output.color = diffuse;

    return output;
}
VSOutput vs1_main(VSInput input)
{
    VSOutput output;

    input.normal.w = 0;
    input.pos += outline * input.normal;
    output.pos = mul(input.pos, obj_to_proj_transform);
    output.color = black;
    output.tex = input.tex;

    return output;
}

vertexshader vs0 = compile vs_3_0 vs0_main();
vertexshader vs1 = compile vs_3_0 vs1_main();



struct PSInput
{
    float2 uv : TEXCOORD;
};
struct PSOutput
{
    float4 color : SV_TARGET;
};
PSOutput ps0_main(PSInput input)
{
    PSOutput output;
    output.color = tex2D(samp0, input.uv);
    return output;
}

pixelshader ps0 = compile ps_3_0 ps0_main();



technique Default
{
    pass
    {
        VertexShader = NULL;
        PixelShader = NULL;

        CullMode = CCW;
        NormalizeNormals = true;
        Lighting = true;
        SpecularEnable = true;

        FillMode = SOLID;
        ShadeMode = GOURAUD;

        LightType[0] = DIRECTIONAL;
        LightAmbient[0] = { 0.3f,0.3f,0.3f,1.0f };
        LightDiffuse[0] = { 1.0f,1.0f,1.0f,1.0f };
        LightSpecular[0] = { 0.6f,0.6f,0.6f,1.0f };
        LightDirection[0] = light_dir;
        LightEnable[0] = true;

        WorldTransform[0] = world_transform;
        ViewTransform = view_transform;
        ProjectionTransform = proj_transform;

        MaterialAmbient = { 1,1,1,1 };
        MaterialDiffuse = diffuse;
        MaterialSpecular = { 1,1,1,1 };
        MaterialPower = 8;
        MaterialEmissive = { 0,0,0,1 };

        Texture[0] = NULL;
    }
}

technique Fog
{
    pass
    {
        VertexShader = NULL;
        PixelShader = NULL;

        CullMode = CCW;
        NormalizeNormals = true;
        Lighting = true;
        SpecularEnable = false;

        FillMode = SOLID;
        ShadeMode = GOURAUD;

        LightType[0] = DIRECTIONAL;
        LightAmbient[0] = { 0.3f,0.3f,0.3f,1.0f };
        LightDiffuse[0] = { 1.0f,1.0f,1.0f,1.0f };
        LightSpecular[0] = { 0.6f,0.6f,0.6f,1.0f };
        LightDirection[0] = light_dir;
        LightEnable[0] = true;

        FogVertexMode = LINEAR;
        FogStart = 5.0f;
        FogEnd = 15.0f;
        FogColor = 0x00cccccc;
        FogEnable = true;

        WorldTransform[0] = world_transform;
        ViewTransform = view_transform;
        ProjectionTransform = proj_transform;

        MaterialAmbient = { 1,1,1,1 };
        MaterialDiffuse = diffuse;
        MaterialSpecular = { 1,1,1,1 };
        MaterialPower = 8;
        MaterialEmissive = { 0,0,0,1 };

        Texture[0] = NULL;
    }
}

technique Cartoon
{
    pass
    {
        VertexShader = vs0;
        PixelShader = ps0;
        NormalizeNormals = true;
    }
    pass
    {
        VertexShader = vs1;
        PixelShader = NULL;
        CullMode = CW;
        NormalizeNormals = true;
    }
}
