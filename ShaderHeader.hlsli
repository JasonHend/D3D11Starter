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

// Special vertex to pixel for skybox
struct VertexToPixel_Sky
{
    float4 position : SV_Position;
    float3 sampleDir : DIRECTION;
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

// -----PBR CONSTANTS-----

// Fresnel value for non metals
static const float F0_NON_METAL = 0.0f;

static const float MIN_ROUGHNESS = 0.0000001f;

static const float PI = 3.14159265359f;

// -----PBR FUNCTIONS-----
// Conserve Energy
float3 DiffuseEnergyConserve(float3 diffuse, float3 F, float metalness)
{
    return diffuse * (1 - F) * (1 - metalness);
}

// Normal Distribution (Trowbridge-Reitz)
// D(h, n, a) = a^2 / pi * ((n dot h)^2 * (a^2 - 1) + 1)^2
float D_GGX(float3 n, float3 h, float roughness)
{
    // Pre-Calculation variables
    float NdotH = saturate(dot(n, h));
    float NdotH2 = NdotH * NdotH;
    float a = roughness * roughness;
    float a2 = max(a * a, MIN_ROUGHNESS);
    
    // ((n dot h)^2 * (a^2 - 1) + 1)
    float denomToSquare = NdotH2 * (a2 - 1) + 1;
    
    // Final value
    return a2 / (PI * denomToSquare * denomToSquare);
}

// Fresnel term - Schlick approximation
// F(v,h,f0) = f0 + (1-f0)(1 - (v dot h))^5
float3 F_Schlick(float3 v, float3 h, float3 f0)
{
    // Pre-Calculation
    float VdotH = saturate(dot(v, h));
    
    // Final value
    return f0 + (1 - f0) * pow(1 - VdotH, 5);
}

// Geometric Shadowing - Schlick-GGX
// G_Schlick(n,v,a) = (n dot v) / ((n dot v) * (1 - k) * k)
float G_SchlickGGX(float3 n, float3 v, float roughness)
{
    // End result of remapping:
    float k = pow(roughness + 1, 2) / 8.0f;
    float NdotV = saturate(dot(n, v));
    
    // Final value
    return 1 / (NdotV * (1 - k) + k);
}

// Cook-Torrence Microfacet BRDF (Specular)
// f(l,v) = D(h)F(v,h)G(l,v,h) / 4(n dot l)(n dot v)
float3 MicrofacetBRDF(float3 n, float3 l, float3 v, float roughness, float3 f0, out float3 F_out)
{
    // Other vectors
    float3 h = normalize(v + 1);
    
    // Run numerator functions
    float D = D_GGX(n, h, roughness);
    float3 F = F_Schlick(v, h, f0);
    float G = G_SchlickGGX(n, v, roughness) * G_SchlickGGX(n, 1, roughness);
    
    // Pass F out of the function for diffuse balance
    F_out = F;
    
    // Final specular formula
    float3 specularResult = (D * F * G) / 4;
    
    // Specular must have same dot product applied as diffuse
    return specularResult * max(dot(n, l), 0);
}

// Calculations for directional lights
float3 DirectionalLight(Light currentLight, float3 inputNormal, float3 surfaceColor, float3 cameraPosition, float3 worldPosition, float roughness, float metalness)
{
    // Calculate directions needed
    float3 lightDirection = normalize(-currentLight.direction);
    float3 cameraDirection = normalize(cameraPosition - worldPosition);
    
    // Diffuse
    float3 diffuse = CalculateDiffusionTerm(inputNormal, lightDirection);
    
    // Specular
    float3 F;
    float3 specular = MicrofacetBRDF(inputNormal, lightDirection, cameraDirection, roughness, surfaceColor, F);
    float3 balanceDiff = DiffuseEnergyConserve(diffuse, specular, metalness);
    
    // Apply lighting
    return (balanceDiff * surfaceColor + specular) * currentLight.intensity * currentLight.color;
}

// Calculations for point lights
float3 PointLight(Light currentLight, float3 inputNormal, float3 surfaceColor, float3 cameraPosition, float3 worldPosition, float roughness, float metalness)
{
    // Calculate directions needed
    float3 lightDirection = normalize(currentLight.position - worldPosition);
    float3 cameraDirection = normalize(cameraPosition - worldPosition);
    
    // Attenuation
    float atten = Attenuate(currentLight, worldPosition);
    
    // Diffuse
    float3 diffuse = CalculateDiffusionTerm(inputNormal, lightDirection);
    
    // Specular
    float3 F;
    float3 specular = MicrofacetBRDF(inputNormal, lightDirection, cameraDirection, roughness, surfaceColor, F);
    float3 balanceDiff = DiffuseEnergyConserve(diffuse, specular, metalness);
    
    // Apply lighting
    return (balanceDiff * surfaceColor + specular) * atten * currentLight.intensity * currentLight.color;
    
}

// Calculations for spotlights
float3 SpotLight(Light currentLight, float3 inputNormal, float3 surfaceColor, float3 cameraPosition, float3 worldPosition, float roughness, float metalness)
{
    float3 lightDirection = normalize(currentLight.position - worldPosition);
    
    // Code for spotlight implementation taken from slides
    float pixelAngle = saturate(dot(worldPosition, lightDirection));
    
    // Cosine angles for range
    float cosOuter = cos(currentLight.spotOuterAngle);
    float cosInner = cos(currentLight.spotInnerAngle);
    float falloffRange = cosOuter - cosInner;
    float spotTerm = saturate((cosOuter - pixelAngle) / falloffRange);
    
    return PointLight(currentLight, inputNormal, surfaceColor, cameraPosition, worldPosition, roughness, metalness) * spotTerm;
}
#endif