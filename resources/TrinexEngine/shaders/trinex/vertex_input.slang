interface IVertexInput
{
	property float3 position { get; }
	property float3 normal { get; }
	property float4 tangent { get; }
	property float2 uv0 { get; }
	property float2 uv1 { get; }
	property float4 color { get; }
}

struct DummyVertexInput : IVertexInput
{
	property float3 position { [ForceInline] get { return 0; } }
	property float3 normal { [ForceInline] get { return 0; } }
	property float4 tangent { [ForceInline] get { return 0; } }
	property float2 uv0 { [ForceInline] get { return 0; } }
	property float2 uv1 { [ForceInline] get { return 0; } }
	property float4 color { [ForceInline] get { return 0; } }
}