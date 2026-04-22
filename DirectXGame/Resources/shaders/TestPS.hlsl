struct PixxelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixxelShaderOutput main()
{
    PixxelShaderOutput output;
    output.color = float4(1.0, 1.0, 1.0, 1.0); // White color
    return output;
}