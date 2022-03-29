string desc = "some effects";

matrix world_transform;
matrix view_transform;
matrix proj_transform;

texture2D tex0 < string name = "tex_cartoon.png"; > ; // annotation
sampler2D samp0 = sampler_state
{
    Texture = (tex0);
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};
vertexshader vs0 < string name = "vs_cartoon.hlsl"; > ;
vertexshader vs1 < string name = "vs_outline.hlsl"; > ;


technique Default
{
    pass
    {
        VertexShader = NULL;
        PixelShader = NULL;

        CullMode = CCW;
        NormalizeNormals = true;

        FillMode = SOLID;
        ShadeMode = GOURAUD;
        Lighting = true;
        SpecularEnable = true;

        WorldTransform[0] = (world_transform);
        ViewTransform = (view_transform);
        ProjectionTransform = (proj_transform);

        LightType[0] = DIRECTIONAL;
        LightAmbient[0] = { 0.3f,0.3f,0.3f,1.0f };
        LightDiffuse[0] = { 1.0f,1.0f,1.0f,1.0f };
        LightSpecular[0] = { 0.6f,0.6f,0.6f,1.0f };
        LightDirection[0] = { 0.0f,-0.99f,-0.14f };
        LightEnable[0] = true;
    }
}

technique Cartoon
{
    pass P0
    {
        VertexShader = (vs0);
        CullMode = CCW;
    }
    pass P1
    {
        VertexShader = (vs1);
        CullMode = CW;
    }
}
