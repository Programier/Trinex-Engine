import "trinex/vertex_input.slang";
import "trinex/trinex.slang";
import "trinex/math.slang";

[parameter_type(meta::type::ExcludeMaterialParameter)]
uniform StructuredBuffer<float4x4> trx_skinning_bones;

export struct VertexInput : IVertexInput
{
	float3 m_position : POSITION;
	float3 m_normal : NORMAL;
	float4 m_tangent : TANGENT;
	float2 m_uv0 : TEXCOORD0;
	// float2 m_uv1 : TEXCOORD1;
	// float2 m_uv2 : TEXCOORD2;
	// float2 m_uv3 : TEXCOORD3;
	float4 m_color : COLOR0;
	float4 blend_weights : BLENDWEIGHT0;
	uint4 blend_indices : BLENDINDICES0;

	property float3 position { [ForceInline] get { return m_position; } }
	property float3 normal { [ForceInline] get { return m_normal; } }
	property float4 tangent { [ForceInline] get { return m_tangent; } }
	property float2 uv0 { [ForceInline] get { return m_uv0; } }
	property float2 uv1 { [ForceInline] get { return 0; } }
	property float2 uv2 { [ForceInline] get { return 0; } }
	property float2 uv3 { [ForceInline] get { return 0; } }
	property float4 color { [ForceInline] get { return m_color; } }

	[ForceInline]
	void vertex_input_main(out GeometryVertexInput output)
    {
		float4x4 bone = 0;

		[unroll]
		for (uint i = 0; i < 4; i++)
		{
			uint index   = blend_indices[i];
			float weight = blend_weights[i];

			if (index != 0xFFFF)
				bone += trx_skinning_bones[index] * weight;
		}

		output.m_position = Math::perspective_divide(bone * float4(m_position, 1.f));
		output.m_normal   = ((float3x3) bone) * m_normal;
		output.m_tangent  = float4(((float3x3) bone) * m_tangent.xyz, m_tangent.w);
		
		output.m_uv0   = m_uv0;
		output.m_uv1   = 0;
		output.m_uv2   = 0;
		output.m_uv3   = 0;
		output.m_color = m_color;
	}
}
