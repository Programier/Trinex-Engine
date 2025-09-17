import "trinex/trinex.slang";
import "trinex/material.slang";
import "trinex/math.slang";
import "trinex/scene_view.slang";
import "trinex/vertex_input.slang";

extern struct VertexInput : IVertexInput;

extern Material main<Material : IMaterial>(in IMaterialInput input);

struct DepthUniforms
{
	[parameter_type(meta::type::LocalToWorld)]
	uniform float4x4 local_to_world;
}

uniform ConstantBuffer<DepthUniforms> uniforms;

[shader("vertex")]
float4 vertex_main(in VertexInput vs_input) : SV_Position
{
	struct MaterialInput : IMaterialInput
	{
		GeometryVertexInput m_geometry;
		float3 m_world_position;
		float3x3 m_tbn;

		property IVertexInput vertex { [ForceInline] get { return m_geometry; } }
		property float3 world_position { [ForceInline] get { return m_world_position; } }
		property float4x4 local_to_world { [ForceInline] get { return uniforms.local_to_world; } }
		property float3x3 TBN { [ForceInline] get { return m_tbn; } }
	}

	MaterialInput material_input;
	
	vs_input.vertex_input_main(material_input.m_geometry);

	material_input.m_world_position = (uniforms.local_to_world * float4(vs_input.position, 1.0f)).xyz;
	float3x3 rotation     = transpose(Math::inverse((float3x3) uniforms.local_to_world));

	float3 world_normal  = normalize(rotation * vs_input.normal);
	float3 world_tangent = normalize(rotation * vs_input.tangent.xyz);

	float3 world_bitangent = cross(world_normal, world_tangent);
	material_input.m_tbn   = Math::tangent_basis(world_normal, world_tangent, world_bitangent);

	float4 ndc = scene_view.camera.projview * float4(material_input.world_position, 1.0f);

	VertexMaterial material = main<VertexMaterial>(material_input);
	return scene_view.camera.projview * float4(material_input.world_position + material.position_offset, 1.0f);
}

[shader("fragment")]
float4 fragment_main() : SV_Target { return 0; }