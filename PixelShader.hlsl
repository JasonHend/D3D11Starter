#include "ShaderHeader.hlsli"

// Create sampler and surface texture values
Texture2D SurfaceTexture : register(t0);
SamplerState BasicSampler : register(s0);

cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float2 scale;
    float2 offset;
    float roughness;
    float3 cameraPosition;
    float3 ambientLight;
    Light lights[5];
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
    int lightCount = 5;

    // Adjust normals
    input.normal = normalize(input.normal);
    
	// Scale and offset UVs
    input.uv = input.uv * scale + offset;
	
	// Adjust texture variables
    float4 surfaceColor = SurfaceTexture.Sample(BasicSampler, input.uv);
    surfaceColor *= colorTint;
    
    // Ambient
    float3 totalLight = ambientLight * surfaceColor;
    
    for (int i = 0; i < lightCount; i++)
    {
        Light light = lights[i];
        light.direction = normalize(light.direction);
        
        switch (light.type)
        {
            case LIGHT_TYPE_DIRECTIONAL:
                totalLight += DirectionalLight(light, input.normal, surfaceColor, cameraPosition, input.worldPosition, roughness);
                break;
            
            case LIGHT_TYPE_POINT:
                totalLight += PointLight(light, input.normal, surfaceColor, cameraPosition, input.worldPosition, roughness);
                break;
            
            case LIGHT_TYPE_SPOT:
                totalLight += SpotLight(light, input.normal, surfaceColor, cameraPosition, input.worldPosition, roughness);
                break;
        }
    }
    
	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering
    return float4(totalLight, 1);
}