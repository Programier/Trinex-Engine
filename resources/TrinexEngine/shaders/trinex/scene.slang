import "trinex/trinex.slang";
import "trinex/math.slang";

namespace Scene
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
		Buffer surface_stream;
		Buffer animation_stream;
		Buffer index_stream;

		Math::Box aabb;
	}

	struct Primitive {
		enum Flags
		{
			IsLast = 0x1,
		}

		uint2 material;
		uint first_vertex;
		uint first_index;
		uint vertices_count;
		uint transform;
		uint data;
		uint chunk;
		uint geometry;
		Flags flags;
	};
}
