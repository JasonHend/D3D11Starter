#include "ShaderHeader.hlsli"

//Struct that will be passed in by the constant buffer
cbuffer ExternalData : register(b0)
{
    float4x4 m4World;
    float4x4 m4View;
    float4x4 m4Projection;
    float4x4 m4WorldInvTranspose;
	
    matrix lightView;
    matrix lightProjection;
}

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixel main(VertexShaderInput input )
{
	// Set up output struct
	VertexToPixel output;

	// Here we're essentially passing the input position directly through to the next
	// stage (rasterizer), though it needs to be a 4-component vector now.  
	// - To be considered within the bounds of the screen, the X and Y components 
	//   must be between -1 and 1.  
	// - The Z component must be between 0 and 1.  
	// - Each of these components is then automatically divided by the W component, 
	//   which we're leaving at 1.0 for now (this is more useful when dealing with 
	//   a perspective projection matrix, which we'll get to in the future).
    matrix wvp = mul(m4Projection, mul(m4View, m4World));
    output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));

	// Pass uv normal and tangent data through
    output.uv = input.uv;
    output.normal = mul((float3x3)m4WorldInvTranspose, input.normal);
    output.tangent = mul((float3x3) m4World, input.tangent);
	
	// Update world position of output
    output.worldPosition = mul(m4World, float4(input.localPosition, 1)).xyz;
	
	// Include any shadowing position
    matrix shadowWVP = mul(lightProjection, mul(lightView, m4World));
    output.shadowMapPos = mul(shadowWVP, float4(input.localPosition, 1.0f));
	
	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
	return output;
}