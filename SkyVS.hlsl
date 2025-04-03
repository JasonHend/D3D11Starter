#include "ShaderHeader.hlsli"

// Constant buffer
cbuffer ExternalData : register(b0)
{
    float4x4 m4View;
    float4x4 m4Projection;
}

VertexToPixel_Sky main(VertexShaderInput input)
{
    // Set up output
    VertexToPixel_Sky output;
    
    // Copy a translation with no position
    float4x4 m4ViewOrigin = m4View;
    m4ViewOrigin._14 = 0;
    m4ViewOrigin._24 = 0;
    m4ViewOrigin._34 = 0;
    
    // Apply projection and new view to input position
    float4x4 m4ViewProjection = mul(m4Projection, m4ViewOrigin);
    output.position = mul(m4ViewProjection, float4(input.localPosition, 1.0f));
    
    // Change output depth to be 1.0
    output.position.z = output.position.w;
    
    // Update sample direction
    output.sampleDir = input.localPosition;
    
    return output;
}