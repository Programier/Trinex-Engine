#ifndef SURFACE_TEMPLATE
#define SURFACE_TEMPLATE

#include "trinex/attributes.slang"
#include "trinex/common.slang"
#include "trinex/math.slang"
#include "trinex/material_common.slang"
#include "trinex/platform.slang"
#include "trinex/globals.slang"

#ifndef USE_VERTEX_COLOR
#define USE_VERTEX_COLOR 0
#endif 

[parameter_type("LocalToWorld")]
uniform float4x4 local_to_world;

struct VertexFactory
{
	float3 	position        : POSITION0;
	float2 	uv              : TEXCOORD0;
	float3	normal          : NORMAL0;
	float3	tangent         : TANGENT0;

#if USE_VERTEX_COLOR
	float4 vertex_color : COLOR0;
#endif

	// @trinex_vertex_attributes

	float3 get_world_position()
	{
		float4 result = mul(local_to_world, float4(position, 1.0));
		result /= result.w;
		return result.xyz;
	}

	float3 get_world_normal()
	{
		return normalize(rotate_vector(normal, local_to_world));
	}

	float3 get_world_tangent()
	{
		return normalize(rotate_vector(tangent, local_to_world));
	}
};

// Global parameters
// @trinex_global_parameters

struct GeometryData
{
#if USE_VERTEX_COLOR
	float4 vertex_color : COLOR;
#endif

	float3 	world_position	: POSITION1;
	float3	world_normal	: NORMAL1;
	float3	world_tangent	: TANGENT1;
	float3	world_bitangent	: BITANGENT1;
	float2 	uv				: TEXCOORD0;
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
	float2 	uv;
};

struct VertexMaterialInput : MaterialInput
{
	[mutating]
	void init(in VertexFactory vertex_factory)
	{
		world_position 	= vertex_factory.get_world_position();
		world_normal 	= vertex_factory.get_world_normal();
		world_tangent 	= vertex_factory.get_world_tangent();
		world_bitangent = cross(world_normal, world_tangent);
		uv 				= Platform::validate_uv(vertex_factory.uv);

#if USE_VERTEX_COLOR
		vertex_color = vertex_factory.vertex_color;
#endif
		TBN = calc_tangent_basis(world_normal, world_tangent, world_bitangent);
	}

	VertexOutput apply_material(in Material material)
	{
		VertexOutput output;
		output.geometry.world_position = world_position + material.position_offset;

		output.geometry.world_normal = world_normal;
		output.geometry.world_tangent = world_tangent;
		output.geometry.world_bitangent = world_bitangent;
		output.geometry.uv = uv;

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
		world_bitangent = data.world_bitangent;
		uv 				= data.uv;
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
	Material material;
// @trinex_vertex_shader
	return material;
}

Material get_material_fs(in FragmentMaterialInput input)
{
	Material material;
// @trinex_fragment_shader
	return material;
}

[shader("vertex")]
VertexOutput vs_main(in VertexFactory vertex_factory)
{
	VertexMaterialInput material_input;
	material_input.init(vertex_factory);

	Material material = get_material_vs(material_input);
	return material_input.apply_material(material);
}

#include "material_templates/gbuffer.slang"

#endif
