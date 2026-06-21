import "trinex/trinex.slang";
import "trinex/math.slang";
import "trinex/scene_view.slang";
import "trinex/vertex_input.slang";


interface IMaterial
{
	property float3 base_color { get; set; }
	property float3 emissive { get; set; }
	property float3 normal { get; set; }
	property float3 position_offset { get; set; }
	property float specular { get; set; }
	property float metalness { get; set; }
	property float roughness { get; set; }
	property float opacity { get; set; }
	property float ao { get; set; }

	static void discard();
}

interface IMaterialInput
{
	property IVertexInput vertex { get; }
	property float3 world_position { get; }
	property float4x4 local_to_world { get; }
	property float3x3 TBN { get; }

	// [ForceInline] float3 world_tangent() { return TBN[0]; }
	// [ForceInline] float3 world_bitangent() { return TBN[1]; }
	// [ForceInline] float3 world_normal() { return TBN[2]; }
}

struct VertexMaterial : IMaterial
{
	float3 position_offset;

	property float3 base_color { [ForceInline] get { return 0; } [ForceInline] set {} }
	property float3 emissive { [ForceInline] get { return 0; } [ForceInline] set {} }
	property float3 normal { [ForceInline] get { return 0; } [ForceInline] set {} }
	property float specular { [ForceInline] get { return 0; } [ForceInline] set {} }
	property float metalness { [ForceInline] get { return 0; } [ForceInline] set {} }
	property float roughness { [ForceInline] get { return 0; } [ForceInline] set {} }
	property float opacity { [ForceInline] get { return 0; } [ForceInline] set {} }
	property float ao { [ForceInline] get { return 0; } [ForceInline] set {} }

	[ForceInline] static void discard() { }
}

struct FragmentMaterial : IMaterial
{
	float3 base_color;
	float3 emissive;
	float3 normal;
	float specular;
	float metalness;
	float roughness;
	float opacity;
	float ao;

	property float3 position_offset { [ForceInline] get { return 0; } [ForceInline] set {} }

	[ForceInline] static void discard() { discard; }
}


//////////////////////// MATERIAL IMPLEMENTATION ////////////////////////

extern Material main<Material : IMaterial>(in IMaterialInput input);

struct MaterialVertexStage
{
	struct Output
	{
		float4 screen : SV_Position;
		float3 bary : BARYCENTRICS;
	}

	uint vertex_index : SV_VertexID;
	uint instance_index : SV_InstanceID;

	uint start_vertex_index: SV_StartVertexLocation;
	uint start_instance_index : SV_StartInstanceLocation;

	float3 load_vertex()
	{
		let primitive = scene_view.primitive(start_instance_index);

		float4x4 local_to_world = scene_view.heap.Load<float4x4>(primitive.transform);

		let geometry = scene_view.geometry(primitive.geometry);
		let index    = geometry.index_stream.load<uint>(vertex_index);

		float4 vertex = local_to_world * float4(geometry.vertex_stream.load<float3>(index), 1.f);
		return vertex.xyz / vertex.w;
	}

	Output execute()
	{
		static const float3 barycentrics[3] = {
			float3(1, 0, 0),
			float3(0, 1, 0),
			float3(0, 0, 1),
		};

		Output output;
		output.screen = scene_view.camera.world_to_clip(load_vertex());
		output.bary   = barycentrics[vertex_index % 3];
		return output;
	}
}

struct MaterialFragmentStage
{
	struct Output
	{
		float3 base_color;
		float3 emissive;
		float3 normal;
		float specular;
		float metalness;
		float roughness;
		float opacity;
		float ao;
	}

	float4 screen : SV_Position;
	float3 bary : BARYCENTRICS;

	Output execute()
	{
		Output output = (Output) 0;

		output.base_color = bary;
		output.normal     = float3(0, 0, 1);
		output.specular   = 0.f;
		output.metalness  = 0.f;
		output.roughness  = 1.f;
		output.opacity    = 1.f;
		output.ao         = 1.f;
		return output;
	}
}