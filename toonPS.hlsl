#include "ShaderHeader.hlsli"

// Create sampler and surface texture values
Texture2D Albedo : register(t0);
Texture2D NormalMap : register(t1);
Texture2D RoughnessMap : register(t2);
Texture2D RampTexture : register(t4);
SamplerState BasicSampler : register(s0);
SamplerState ClampSampler : register(s1);
SamplerComparisonState ShadowSampler : register(s2);

// Texture for shadows
Texture2D ShadowMap : register(t4);

cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float2 scale;
    float2 offset;
    float roughness;
    float3 cameraPosition;
    Light lights[5];
    float3 ambientLight;
};

float4 main(VertexToPixel input) : SV_TARGET
{
    // Check shadow map
    // Prespective divide
    input.shadowMapPos /= input.shadowMapPos.w;
    
    // Convert to UVs
    float2 shadowUV = input.shadowMapPos.xy * 0.5f + 0.5f;
    shadowUV.y = 1 - shadowUV.y;
    
    // Grab distances
    float distToLight = input.shadowMapPos.z;
    float shadowAmount = ShadowMap.SampleCmpLevelZero(
        ShadowSampler,
        shadowUV,
        distToLight).r;
    
    int lightCount = 5;

    // Adjust normals
    input.normal = normalize(input.normal);
    input.tangent = normalize(input.tangent);
    
	// Scale and offset UVs
    input.uv = input.uv * scale + offset;
    
    // Unpack normal map
    float3 unpackedNormal = NormalMap.Sample(BasicSampler, input.uv).rgb * 2 - 1;
    unpackedNormal = normalize(unpackedNormal);
    
    // Create the TBN matrix and transform the normal map
    float3 N = normalize(input.normal);
    float3 T = input.tangent;
    T = normalize(T - N * dot(T, N));
    float3 B = cross(T, N);
    float3x3 TBN = float3x3(T, B, N);
    
    input.normal = normalize(mul(unpackedNormal, TBN));
	
	// Adjust albedo
    float3 surfaceColor = pow(Albedo.Sample(BasicSampler, input.uv).rgb, 2.2f);
    
    // Sample roughness and metalness
    float roughnessValue = RoughnessMap.Sample(BasicSampler, input.uv).r;
    
    // Total light
    float3 totalLight = ambientLight * surfaceColor.rgb;
    
    // Since toonshading, just handle first light
    float3 toLight = lights[0].position - input.worldPosition;
    float3 toCam = normalize(cameraPosition - input.worldPosition);
    
    float diffuse = CalculateDiffusionTerm(
        input.normal, normalize(lights[0].position - input.worldPosition)).r;
    float specular = CalculateSpecularTerm(
        input.normal, toLight, toCam, roughness);
    
    // Handle toon shading
    diffuse = RampTexture.Sample(BasicSampler, float2(diffuse, 0)).r;
    specular = RampTexture.Sample(BasicSampler, float2(diffuse, 0)).r;
    
    totalLight += (diffuse * surfaceColor.rgb + specular) * lights[0].intensity * lights[0].color;
    return float4(totalLight.r, totalLight.g, totalLight.g, 0);
}