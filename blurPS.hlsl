cbuffer externalData : register(b0)
{
    int blurRadius;
    float pixelWidth;
    float pixelHeight;
}

struct VertexToPixel
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

Texture2D Pixels : register(t0);
SamplerState ClampSampler : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
    // Total amount of color and number of samples
    float4 total = 0;
    int sampleCount = 0;

    // Loop through the previous render
    for (int x = -blurRadius; x <= blurRadius; x++)
    {
        for (int y = -blurRadius; y <= blurRadius; y++)
        {
            // Calculate UV
            float2 uv = input.uv;
            uv += float2(x * pixelWidth, y * pixelHeight);
            
            // Add to total color
            total += Pixels.Sample(ClampSampler, uv);
            sampleCount++;
        }
    }
    
    // Process with blur
    return total / sampleCount;
}