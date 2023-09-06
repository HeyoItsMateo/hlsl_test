struct VSInput
{
    [[vk::location(0)]] float4 Position : POSITION0;
    [[vk::location(1)]] float4 Color : COLOR0;
    [[vk::location(2)]] float2 UVcoord : TEXCOORD0;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    [[vk::location(0)]] float4 Color : COLOR0;
    [[vk::location(1)]] float2 UVcoord : TEXCOORD0;
};

VSOutput main(VSInput input, uint VertexIndex : SV_VertexID)
{
    VSOutput output = (VSOutput) 0;

    output.Position = input.Position;
    
    output.Color = input.Color;
    output.UVcoord = input.UVcoord;
    
    return output;
}