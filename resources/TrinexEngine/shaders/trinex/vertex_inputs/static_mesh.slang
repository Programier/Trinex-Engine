import "trinex/vertex_input.slang";

export struct VertexInput : IVertexInput
{
	float4 m_color : COLOR;
	float3 m_position : POSITION;
	float3 m_normal : NORMAL;
	float3 m_tangent : TANGENT;
	float2 m_uv0 : TEXCOORD0;
	float2 m_uv1 : TEXCOORD1;

	property float3 position { [ForceInline] get { return m_position; } }
	property float3 normal { [ForceInline] get { return m_normal; } }
	property float4 tangent { [ForceInline] get { return float4(m_tangent, 1.0); } }
	property float2 uv0 { [ForceInline] get { return m_uv0; } }
	property float2 uv1 { [ForceInline] get { return m_uv1; } }
	property float4 color { [ForceInline] get { return m_color; } }
	property float4 blend_weight { [ForceInline] get { return 0; } }
	property uint4 blend_indices { [ForceInline] get { return 0; } }
}