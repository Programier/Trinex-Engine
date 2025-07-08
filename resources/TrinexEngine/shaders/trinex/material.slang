import "trinex/math.slang";
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

struct Material : IMaterial
{
	property float3 base_color { [ForceInline] get { return 0; } [ForceInline] set {} }
	property float3 emissive { [ForceInline] get { return 0; } [ForceInline] set {} }
	property float3 normal { [ForceInline] get { return 0; } [ForceInline] set {} }
	property float3 position_offset { [ForceInline] get { return 0; } [ForceInline] set {} }
	property float specular { [ForceInline] get { return 0; } [ForceInline] set {} }
	property float metalness { [ForceInline] get { return 0; } [ForceInline] set {} }
	property float roughness { [ForceInline] get { return 0; } [ForceInline] set {} }
	property float opacity { [ForceInline] get { return 0; } [ForceInline] set {} }
	property float ao { [ForceInline] get { return 0; } [ForceInline] set {} }

	[ForceInline] static void discard() {}
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