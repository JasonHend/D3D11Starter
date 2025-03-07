
// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float4 screenPosition : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

cbuffer ExternalData : register(b0)
{
    float4 colorTint;
};

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
    if ((input.uv.x * 10) % 2 >= 1 && (input.uv.y * 10) % 2 >= 1)
    {
        return float4(0.8f, 0.25f, 0.63f, 1.0f);
    }
    
    if ((input.uv.x * 10) % 2 <= 1 && (input.uv.y * 10) % 2 <= 1)
    {
        return float4(0.17f, 0.34f, 0.66f, 1.0f);
    }
    
    return float4(0.0f, 0.0f, 0.0f, 1.0f);
}