import "trinex/trinex.slang";

struct Scene
{
	typealias Transform = float4x4;

	struct Geometry
    {
		struct Buffer {
			ByteAddressBuffer::Handle address;
			uint offset;
			uint stride;
		};

		Buffer vertex_stream;
		Buffer animation_stream;
		Buffer index_stream;

		uint vertices;
		float3 aabb_min;
		float3 aabb_max;
    }

	struct Primitive {
		uint transform;
		uint material;
		uint geometry;
		uint flags;
	};

	RWStructuredBuffer<Transform>::Handle transforms;
	RWStructuredBuffer<Geometry>::Handle geometries;
	RWStructuredBuffer<Primitive>::Handle primitives;
}