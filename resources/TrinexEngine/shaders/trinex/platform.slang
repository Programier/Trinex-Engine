namespace Platform
{
    static const float2 ndc_depth_range = float2(0.f, 1.f);

    [ForceInline] float linearize_depth(float depth, float near, float far)
    {
        return (near * far) / (far + depth * (near - far));
    }
}
