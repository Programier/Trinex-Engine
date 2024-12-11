#ifndef SURFACE_TEMPLATE
#define SURFACE_TEMPLATE

#include "common.slang"
#include "vertex_factory/static_mesh.slang"
#include "material_common.slang"
#include "platform.slang"


#ifndef USE_VERTEX_COLOR
#define USE_VERTEX_COLOR 0
#endif 

// Global parameters
@0

struct GeometryData
{
#if USE_VERTEX_COLOR
	float4 vertex_color : COLOR;
#endif

	float3 	world_position	: POSITION1;
	float3	world_normal	: NORMAL1;
	float3	world_tangent	: TANGENT1;
	float3	world_bitangent	: BITANGENT1;

	float2 	uv[8]			: TEXCOORD0;
};

struct VertexOutput
{
	float4 position : SV_Position;
	GeometryData geometry;
};

struct FragmentInput
{
	float4 position : SV_Position;
	GeometryData geometry;
	bool is_front_face : SV_IsFrontFace;
};

struct MaterialInput
{
	float3x3 TBN;

#if USE_VERTEX_COLOR
	float4 vertex_color;
#endif

	float3 	world_position;
	float3	world_normal;
	float3	world_tangent;
	float3	world_bitangent;

	float2 	uv[8];
};

struct VertexMaterialInput : MaterialInput
{
	[mutating]
	void init(in VertexFactory vertex_factory)
	{
		world_position 	= vertex_factory.get_world_position();
		world_normal 	= vertex_factory.get_world_normal();
		world_tangent 	= vertex_factory.get_world_tangent();
		world_bitangent 	= vertex_factory.get_world_bitangent();

		TBN = calc_tangent_basis(world_normal, world_tangent);

		for(uint i = 0, count = vertex_factory.get_uv_count(); i < count; ++i)
		{
			uv[i] = platform_validate_uv(vertex_factory.get_uv(i));
		}

#if USE_VERTEX_COLOR
		vertex_color = vertex_factory.get_color();
#endif
	}

	[mutating]
	VertexOutput apply_material(in Material material)
	{
		VertexOutput output = (VertexOutput)0;
		output.geometry.world_position = world_position + material.position_offset;

		output.geometry.world_normal = mul(TBN, material.normal);
		output.geometry.world_tangent = world_tangent;
		output.geometry.world_bitangent = world_bitangent;

		for(uint i = 0; i < 8; ++i)
		{
			output.geometry.uv[i] = uv[i];
		}

		output.position = mul(globals.projview, float4(output.geometry.world_position, 1.0));
		return output;
	}
};

struct FragmentMaterialInput : MaterialInput
{
	float4 	sv_position;
	float front_face;

	[mutating]
	void copy_geometry(in GeometryData data)
	{
		TBN = calc_tangent_basis(data.world_normal, data.world_tangent);
#if USE_VERTEX_COLOR
		vertex_color 	= data.vertex_color;
#endif
		world_position 	= data.world_position;
		world_normal 	= data.world_normal;
		world_tangent 	= data.world_tangent;
		world_bitangent 	= data.world_bitangent;

		for(uint i = 0; i < 8; ++i)
		{
			uv[i] = data.uv[i];
		}
	}

	[mutating]
	void init(in FragmentInput data)
	{
		copy_geometry(data.geometry);
		front_face = data.is_front_face  ? 1.f : -1.f;
		sv_position = data.position;
	}
};

Material get_material_vs(in VertexMaterialInput input)
{
	@1
}

Material get_material_fs(in FragmentMaterialInput input)
{
	@2
}

[shader("vertex")]
VertexOutput vs_main(in VertexFactory vertex_factory)
{
	VertexMaterialInput material_input = (VertexMaterialInput)0;
	material_input.init(vertex_factory);

	Material material = get_material_vs(material_input);
	return material_input.apply_material(material);
}

#include "material_templates/gbuffer.slang"

#endif
