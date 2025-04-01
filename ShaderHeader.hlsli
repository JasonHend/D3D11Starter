#ifndef __SHADER_HEADER__
#define __SHADER_HEADER__
#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2
#define MAX_SPECULAR_EXPONENT 256.0f

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
    float3 worldPosition : POSITION;
    float3 tangent : TANGENT;
};

// Light struct
struct Light
{
    int type;
    float3 direction;
    float range;
    float3 position;
    float intensity;
    float3 color;
    float spotInnerAngle;
    float spotOuterAngle;
    float2 padding;
};

// Struct representing a single vertex worth of data
// - This should match the vertex definition in our C++ code
// - By "match", I mean the size, order and number of members
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexShaderInput
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float3 localPosition : POSITION; // XYZ position
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};

// Lighting functions
float3 CalculateDiffusionTerm(float3 inputNormal, float3 lightDirection)
{
    return saturate(dot(inputNormal, lightDirection));
}

float CalculateSpecularTerm(float3 inputNormal, float3 lightDirection, float3 cameraDirection, float roughness)
{
    // Calculate specular exponent based on roughness
    float specExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
    
    // Branch if roughness is 0
    [branch]
    if (roughness == 1)
    {
        specExponent = 0.95f * MAX_SPECULAR_EXPONENT;
    }
    
    // Calculate view and reflection
    float3 R = reflect(-lightDirection, inputNormal);
    
    // Final spec
    return pow(max(dot(cameraDirection, R), 0.0f), specExponent);
}

float Attenuate(Light light, float3 worldPos)
{
    float dist = distance(light.position, worldPos);
    float att = saturate(1.0f - (dist * dist / (light.range * light.range)));
    return att * att;
}

// Calculations for directional lights
float3 DirectionalLight(Light currentLight, float3 inputNormal, float3 surfaceColor, float3 cameraPosition, float3 worldPosition, float roughness)
{
    // Calculate directions needed
    float3 lightDirection = normalize(-currentLight.direction);
    float3 cameraDirection = normalize(cameraPosition - worldPosition);
    
    // Diffuse
    float3 diffuse = CalculateDiffusionTerm(inputNormal, lightDirection);
    
    // Specular
    float specular = CalculateSpecularTerm(inputNormal, lightDirection, cameraDirection, roughness);
    specular *= any(diffuse);
    
    // Apply lighting
    return (surfaceColor * (diffuse + specular)) * currentLight.intensity * currentLight.color;
}

// Calculations for point lights
float3 PointLight(Light currentLight, float3 inputNormal, float3 surfaceColor, float3 cameraPosition, float3 worldPosition, float roughness)
{
    // Calculate directions needed
    float3 lightDirection = normalize(currentLight.position - worldPosition);
    float3 cameraDirection = normalize(cameraPosition - worldPosition);
    
    // Diffuse
    float3 diffuse = CalculateDiffusionTerm(inputNormal, lightDirection);
    
    // Specular
    float specular = CalculateSpecularTerm(inputNormal, lightDirection, cameraDirection, roughness);
    specular *= any(diffuse);
    
    // Attenuation
    float atten = Attenuate(currentLight, worldPosition);
    
    // Apply lighting
    return (surfaceColor * (diffuse + specular)) * atten * currentLight.intensity * currentLight.color;
    
}

// Calculations for spotlights
float3 SpotLight(Light currentLight, float3 inputNormal, float3 surfaceColor, float3 cameraPosition, float3 worldPosition, float roughness)
{
    float3 lightDirection = normalize(currentLight.position - worldPosition);
    
    // Code for spotlight implementation taken from slides
    float pixelAngle = saturate(dot(worldPosition, lightDirection));
    
    // Cosine angles for range
    float cosOuter = cos(currentLight.spotOuterAngle);
    float cosInner = cos(currentLight.spotInnerAngle);
    float falloffRange = cosOuter - cosInner;
    float spotTerm = saturate((cosOuter - pixelAngle) / falloffRange);
    
    return PointLight(currentLight, inputNormal, surfaceColor, cameraPosition, worldPosition, roughness) * spotTerm;
}
#endif