uniform writeonly RWTexture2D<float4> output;
uniform Sampler2D<float4> input;

uniform float sigma;
uniform int kernel_size;

float gaussian_weight(int x) 
{
    return exp(-0.5 * (x * x) / (sigma * sigma)) / (sqrt(2.0 * 3.14159265359) * sigma);
}

[numthreads (8, 8, 1)]
void compute_main(uint3 id : SV_DispatchThreadID)
{
    int x, y;

    output.GetDimensions(x, y);
    float2 output_size = float2(x, y);
    float2 input_size = output_size;

    float2 input_texel = float2(1.0, 1.0) / input_size;
    float2 uv = float2(id.xy) * (float2(1.0, 1.0) / output_size);

    float4 color = float4(0, 0, 0, 0);
    float weight_sum = 0.0;
    
    for (int i = -kernel_size; i <= kernel_size; i++) 
    {
        float w = gaussian_weight(i);
        color += input.SampleLevel(uv + float2(i * input_texel.x, 0), 0) * w;
        weight_sum += w;
    }
    
    for (int i = -kernel_size; i <= kernel_size; i++) 
    {
        float w = gaussian_weight(i);
        color += input.SampleLevel(uv + float2(0, i * input_texel.y), 0) * w;
        weight_sum += w;
    }
    
    output[id.xy] = color / weight_sum;
}
