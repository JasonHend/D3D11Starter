struct VertexToPixel
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VertexToPixel main(uint id : SV_VertexID)
{
    VertexToPixel output;
    
    // Doing branchless fullscreen VS logic
    output.uv = float2(
        (id << 1) & 2,
        id & 2);
    
    // Alter positions based on uv value
    output.position = float4(output.uv, 0, 1);
    output.position.x = output.position.x * 2 - 1;
    output.position.y = output.position.y * -2 + 1;
    
    return output;
}