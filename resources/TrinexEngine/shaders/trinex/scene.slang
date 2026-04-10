import "trinex/trinex.slang";

struct Scene
{
	struct Geometry
    {
		struct Buffer {
			RWByteAddressBuffer::Handle handle;
			uint offset;
			uint stride;

			[ForceInline]
			T load<T>(uint element)
			{
				return handle.Load<T>(offset + stride * element);
			}
		};

		Buffer vertex_stream;
		Buffer animation_stream;
		Buffer index_stream;

		uint vertices;
		float3 aabb_min;
		float3 aabb_max;
    }

	struct Primitive {
		[UnscopedEnum]
		[Flags]
		enum Flags
		{
			IsActive = 0x1,
		}

		uint transform;
		uint material;
		uint geometry;
		Flags flags;
	};

	RWByteAddressBuffer::Handle heap;
	RWStructuredBuffer<Geometry>::Handle geometries;
	RWStructuredBuffer<Primitive>::Handle primitives;
}