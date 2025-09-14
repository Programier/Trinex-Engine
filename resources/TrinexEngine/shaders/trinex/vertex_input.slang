interface IVertexInput
{
	property float3 position { get; }
	property float3 normal { get; }
	property float4 tangent { get; }
	property float2 uv0 { get; }
	property float2 uv1 { get; }
	property float2 uv2 { get; }
	property float2 uv3 { get; }
	property float4 color { get; }

	void vertex_input_main(out GeometryVertexInput output);
}

struct DummyVertexInput : IVertexInput
{
	property float3 position { [ForceInline] get { return 0; } }
	property float3 normal { [ForceInline] get { return 0; } }
	property float4 tangent { [ForceInline] get { return 0; } }
	property float2 uv0 { [ForceInline] get { return 0; } }
	property float2 uv1 { [ForceInline] get { return 0; } }
	property float2 uv2 { [ForceInline] get { return 0; } }
	property float2 uv3 { [ForceInline] get { return 0; } }
	property float4 color { [ForceInline] get { return 0; } }

	[ForceInline]
	void vertex_input_main(out GeometryVertexInput output)
	{
		output.m_position = position;
		output.m_normal   = normal;
		output.m_tangent  = tangent;
		output.m_uv0      = uv0;
		output.m_uv1      = uv1;
		output.m_uv2      = uv2;
		output.m_uv3      = uv3;
		output.m_color    = color;
	}
}

struct GeometryVertexInput : IVertexInput
{
	float3 m_position : POSITION0;
	float3 m_normal : NORMAL0;
	float4 m_tangent : TANGENT0;
	float2 m_uv0 : TEXCOORD0;
	float2 m_uv1 : TEXCOORD1;
	float2 m_uv2 : TEXCOORD2;
	float2 m_uv3 : TEXCOORD3;
	float4 m_color : COLOR0;

	property float3 position { [ForceInline] get { return m_position; } }
	property float3 normal { [ForceInline] get { return m_normal; } }
	property float4 tangent { [ForceInline] get { return m_tangent; } }
	property float2 uv0 { [ForceInline] get { return m_uv0; } }
	property float2 uv1 { [ForceInline] get { return m_uv1; } }
	property float2 uv2 { [ForceInline] get { return m_uv2; } }
	property float2 uv3 { [ForceInline] get { return m_uv3; } }
	property float4 color { [ForceInline] get { return m_color; } }

	[ForceInline]
	void vertex_input_main(out GeometryVertexInput output)
	{
		output.m_position = m_position;
		output.m_normal   = m_normal;
		output.m_tangent  = m_tangent;
		output.m_uv0      = m_uv0;
		output.m_uv1      = m_uv1;
		output.m_uv2      = m_uv2;
		output.m_uv3      = m_uv3;
		output.m_color    = m_color;
	}
}