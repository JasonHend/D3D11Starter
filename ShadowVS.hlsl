#include "ShaderHeader.hlsli"

// External Data
cbuffer externalData : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
};

// Simplified VS for shadows
float4 main(VertexShaderInput input) : SV_POSITION
{
    matrix wvp = mul(projection, mul(view, world));
    return mul(wvp, float4(input.localPosition, 1.0f));

}