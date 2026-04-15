import "trinex/trinex.slang";
import "trinex/math.slang";

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
		Math::Box aabb;
	}

	struct Primitive {
		enum Flags
		{
			IsAlive = 0x1,
		}

		uint transform;
		uint material;
		uint geometry;
		Flags flags;

		inline bool is_alive() { return flags & Flags::IsAlive; }
	};

	RWByteAddressBuffer::Handle heap;
	RWByteAddressBuffer::Handle geometries;
	RWByteAddressBuffer::Handle primitives;

	inline Geometry geometry(uint id = 0)
	{
		return geometries.Load<Geometry>(id * sizeof(Geometry));
	}

	inline Primitive primitive(uint id = 0)
	{
		return primitives.Load<Primitive>(id * sizeof(Primitive));
	}
}
