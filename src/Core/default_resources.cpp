#include <Core/archive.hpp>
#include <Core/base_engine.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/default_resources.hpp>
#include <Core/math/math.hpp>
#include <Core/memory.hpp>
#include <Core/package.hpp>
#include <Graphics/gpu_buffers.hpp>
#include <Graphics/material.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/texture.hpp>
#include <cmath>
#include <numbers>
#include <random>

namespace Trinex
{
	namespace DefaultResources
	{
		namespace Textures
		{
			ENGINE_EXPORT Texture2D* white                  = nullptr;
			ENGINE_EXPORT Texture2D* black                  = nullptr;
			ENGINE_EXPORT Texture2D* gray                   = nullptr;
			ENGINE_EXPORT Texture2D* normal                 = nullptr;
			ENGINE_EXPORT Texture2D* default_texture        = nullptr;
			ENGINE_EXPORT TextureCube* default_texture_cube = nullptr;
			ENGINE_EXPORT Texture2D* noise4x4               = nullptr;
			ENGINE_EXPORT Texture2D* noise16x16             = nullptr;
			ENGINE_EXPORT Texture2D* noise128x128           = nullptr;
			ENGINE_EXPORT Texture3D* default_lut            = nullptr;
		}// namespace Textures

		namespace Buffers
		{
			ENGINE_EXPORT PositionVertexBuffer* screen_quad = nullptr;
		}

		namespace Materials
		{
			ENGINE_EXPORT Material* sprite    = nullptr;
			ENGINE_EXPORT Material* base_pass = nullptr;
		}// namespace Materials

		namespace Meshes
		{
			ENGINE_EXPORT StaticMesh* cube     = nullptr;
			ENGINE_EXPORT StaticMesh* sphere   = nullptr;
			ENGINE_EXPORT StaticMesh* cylinder = nullptr;
			ENGINE_EXPORT StaticMesh* plane    = nullptr;
			ENGINE_EXPORT StaticMesh* cone     = nullptr;
		}// namespace Meshes

	}// namespace DefaultResources

	ENGINE_EXPORT Object* load_object_from_memory(const u8* data, usize size, const StringView& name)
	{
		if (size > 0)
		{
			Vector<u8> tmp(data, data + size);
			VectorReader reader = &tmp;
			return Object::load_object(name, &reader);
		}

		return nullptr;
	}

	template<typename T>
	static T* load_object(const char* name)
	{
		Object* obj = Object::load_object(name);
		return reinterpret_cast<T*>(obj);
	}

	template<typename T>
	static T* create_texture(Package* package, const char* name)
	{
		T* texture = Object::new_instance<T>(name, package);
		texture->flags |= Object::Flags::StandAlone;
		texture->format = RHIColorFormat::R8G8B8A8;
		return texture;
	}

	static void generate_noise_texture(Package* package, Texture2D*& texture, const char* name, Vector2u size)
	{
		texture = create_texture<Texture2D>(package, name);

		auto& mip = texture->mips.emplace_back();
		mip.size  = size;
		mip.data.resize(mip.size.x * mip.size.y * 4);

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<u8> dist(0, 255);

		for (u8& value : mip.data)
		{
			value = dist(gen);
		}

		texture->init_render_resources();
	}


	static void generate_noise_textures(Package* package)
	{
		generate_noise_texture(package, DefaultResources::Textures::noise4x4, "Noise4x4", {4, 4});
		generate_noise_texture(package, DefaultResources::Textures::noise16x16, "Noise16x16", {16, 16});
		generate_noise_texture(package, DefaultResources::Textures::noise128x128, "Noise128x128", {128, 128});
	}

	static void generate_checker_texture(Vector2u size, u32 cell, Color* texture)
	{
		const Color colors[2] = {{200, 200, 200, 255}, {100, 100, 100, 255}};

		for (u32 y = 0; y < size.y; ++y)
		{
			for (u32 x = 0; x < size.x; ++x)
			{
				u32 index               = ((x / cell) + (y / cell)) % 2;
				texture[y * size.x + x] = colors[index];
			}
		}
	}

	static void generate_lut_texture(Vector3u size, Color* texture)
	{
		for (u32 z = 0; z < size.z; ++z)
		{
			for (u32 y = 0; y < size.y; ++y)
			{
				for (u32 x = 0; x < size.x; ++x)
				{
					u32 offset = x + size.x * (y + size.y * z);

					texture[offset].r = u8(float(x) / float(size.x - 1) * 255.f);
					texture[offset].g = u8(float(y) / float(size.y - 1) * 255.f);
					texture[offset].b = u8(float(z) / float(size.z - 1) * 255.f);
					texture[offset].a = 255;
				}
			}
		}
	}

	static void generate_dummy_texture(Package* package, Texture2D*& texture, const char* name, Color color)
	{
		texture = create_texture<Texture2D>(package, name);

		auto& mip = texture->mips.emplace_back();
		mip.size  = {1, 1};
		mip.data.resize(mip.size.x * mip.size.y * 4);

		(*reinterpret_cast<Color*>(mip.data.data())) = color;
		texture->init_render_resources();
	}

	static void generate_default_textures(Package* package)
	{
		{
			namespace Textures = DefaultResources::Textures;
			generate_dummy_texture(package, Textures::white, "White", {255, 255, 255, 255});
			generate_dummy_texture(package, Textures::black, "Black", {0, 0, 0, 255});
			generate_dummy_texture(package, Textures::gray, "Gray", {127, 127, 127, 255});
			generate_dummy_texture(package, Textures::normal, "Normal", {127, 127, 255, 255});
		}

		{
			Texture2D*& texture = DefaultResources::Textures::default_texture;
			texture             = create_texture<Texture2D>(package, "DefaultTexture");

			auto& mip = texture->mips.emplace_back();
			mip.size  = {128, 128};
			mip.data.resize(mip.size.x * mip.size.y * 4);

			Color* pixels = reinterpret_cast<Color*>(mip.data.data());
			generate_checker_texture({128, 128}, 8, pixels);
			texture->init_render_resources();
		}

		{
			TextureCube*& texture = DefaultResources::Textures::default_texture_cube;
			texture               = create_texture<TextureCube>(package, "DefaultTextureCube");

			auto& mip = texture->mips.emplace_back();
			mip.size  = {128, 128};
			mip.data.resize(mip.size.x * mip.size.y * 4 * 6);

			Color* pixels = reinterpret_cast<Color*>(mip.data.data());

			for (u32 i = 0; i < 6; ++i)
			{
				generate_checker_texture({128, 128}, 8, pixels);
				pixels += mip.size.x * mip.size.y;
			}

			texture->init_render_resources();
		}

		{
			Texture3D*& texture = DefaultResources::Textures::default_lut;
			texture             = create_texture<Texture3D>(package, "DefaultLUT");

			auto& mip = texture->mips.emplace_back();
			mip.size  = {32, 32, 32};
			mip.data.resize(mip.size.x * mip.size.y * mip.size.z * 4);

			Color* pixels = reinterpret_cast<Color*>(mip.data.data());
			generate_lut_texture(mip.size, pixels);
			texture->init_render_resources();
		}
	}

	static void generate_textures()
	{
		Package* package = Package::static_find_package("TrinexEngine::Textures", true);

		generate_default_textures(package);
		generate_noise_textures(package);
	}

	struct ProceduralVertex {
		Vector3f position;
		Vector2f uv0;
		Vector3f normal;
		Vector4f tangent;
	};

	static u32 pack_unorm4x8(const Vector4f& value)
	{
		return Color(Color::float_to_byte(value.x * 0.5f + 0.5f), Color::float_to_byte(value.y * 0.5f + 0.5f),
		             Color::float_to_byte(value.z * 0.5f + 0.5f), Color::float_to_byte(value.w * 0.5f + 0.5f))
		        .rgba;
	}

	static Box3f calculate_bounds(const Vector<ProceduralVertex>& vertices)
	{
		if (vertices.empty())
			return {};

		Vector3f min = vertices[0].position;
		Vector3f max = vertices[0].position;

		for (const ProceduralVertex& vertex : vertices)
		{
			min = Math::min(min, vertex.position);
			max = Math::max(max, vertex.position);
		}

		return {min, max};
	}

	static StaticMesh* create_mesh(Package* package, const char* name, const Vector<ProceduralVertex>& vertices,
	                               const Vector<u32>& indices)
	{
		StaticMesh* mesh = Object::new_instance<StaticMesh>(name, package);
		mesh->flags |= Object::Flags::StandAlone;
		mesh->bounds = calculate_bounds(vertices);
		mesh->materials.push_back(DefaultResources::Materials::base_pass);

		auto& lod              = mesh->lods.emplace_back();
		auto& surface          = lod.surfaces.emplace_back();
		surface.topology       = RHITopology::TriangleList;
		surface.material_index = 0;

		if (!indices.empty())
		{
			surface.first_index    = 0;
			surface.vertices_count = indices.size();
			lod.indices.init(RHIBufferFlags::IndexBuffer, indices.size(), indices.data());
		}
		else
		{
			surface.first_index    = ~0u;
			surface.vertices_count = vertices.size();
		}

		Vector<MeshVertexStream> positions;
		Vector<MeshSurfaceStream> surface_stream;
		positions.reserve(vertices.size());
		surface_stream.reserve(vertices.size());

		for (const ProceduralVertex& vertex : vertices)
		{
			positions.push_back(vertex.position);

			auto& stream   = surface_stream.emplace_back();
			stream.uv0     = vertex.uv0;
			stream.uv1     = Vector2f16(0.f, 0.f);
			stream.normal  = pack_unorm4x8(Vector4f(vertex.normal, 0.f));
			stream.tangent = pack_unorm4x8(vertex.tangent);
		}

		lod.vertex_stream.init(RHIBufferFlags::VertexBuffer, positions.size(), positions.data());
		lod.surface_stream.init(RHIBufferFlags::VertexBuffer, surface_stream.size(), surface_stream.data());

		mesh->init_render_resources();
		return mesh;
	}

	static void add_triangle(Vector<u32>& indices, u32 a, u32 b, u32 c)
	{
		indices.push_back(a);
		indices.push_back(b);
		indices.push_back(c);
	}

	static void add_quad_indices(Vector<u32>& indices, u32 a, u32 b, u32 c, u32 d)
	{
		add_triangle(indices, a, b, c);
		add_triangle(indices, a, c, d);
	}

	static void append_quad(Vector<ProceduralVertex>& vertices, Vector<u32>& indices, const Vector3f& origin,
	                        const Vector3f& axis_x, const Vector3f& axis_y, const Vector3f& normal)
	{
		const u32 base = vertices.size();
		const Vector4f tangent(Math::normalize(axis_x), 1.f);

		vertices.push_back({origin, {0.f, 0.f}, normal, tangent});
		vertices.push_back({origin + axis_y, {0.f, 1.f}, normal, tangent});
		vertices.push_back({origin + axis_x + axis_y, {1.f, 1.f}, normal, tangent});
		vertices.push_back({origin + axis_x, {1.f, 0.f}, normal, tangent});

		add_quad_indices(indices, base + 0, base + 1, base + 2, base + 3);
	}

	static StaticMesh* generate_plane(Package* package)
	{
		Vector<ProceduralVertex> vertices;
		Vector<u32> indices;

		append_quad(vertices, indices, {-0.5f, 0.f, -0.5f}, {1.f, 0.f, 0.f}, {0.f, 0.f, 1.f}, {0.f, 1.f, 0.f});
		return create_mesh(package, "Plane", vertices, indices);
	}

	static StaticMesh* generate_cube(Package* package)
	{
		Vector<ProceduralVertex> vertices;
		Vector<u32> indices;

		append_quad(vertices, indices, {-0.5f, -0.5f, 0.5f}, {1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f});
		append_quad(vertices, indices, {0.5f, -0.5f, -0.5f}, {-1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, -1.f});
		append_quad(vertices, indices, {-0.5f, -0.5f, -0.5f}, {0.f, 0.f, 1.f}, {0.f, 1.f, 0.f}, {-1.f, 0.f, 0.f});
		append_quad(vertices, indices, {0.5f, -0.5f, 0.5f}, {0.f, 0.f, -1.f}, {0.f, 1.f, 0.f}, {1.f, 0.f, 0.f});
		append_quad(vertices, indices, {-0.5f, 0.5f, 0.5f}, {1.f, 0.f, 0.f}, {0.f, 0.f, -1.f}, {0.f, 1.f, 0.f});
		append_quad(vertices, indices, {-0.5f, -0.5f, -0.5f}, {1.f, 0.f, 0.f}, {0.f, 0.f, 1.f}, {0.f, -1.f, 0.f});

		return create_mesh(package, "Cube", vertices, indices);
	}

	static StaticMesh* generate_uv_sphere(Package* package, u32 segments = 32, u32 rings = 16)
	{
		Vector<ProceduralVertex> vertices;
		Vector<u32> indices;
		const float pi = std::numbers::pi_v<float>;

		for (u32 ring = 0; ring <= rings; ++ring)
		{
			const float v   = static_cast<float>(ring) / static_cast<float>(rings);
			const float phi = v * pi;
			const float y   = std::cos(phi);
			const float r   = std::sin(phi);

			for (u32 segment = 0; segment <= segments; ++segment)
			{
				const float u     = static_cast<float>(segment) / static_cast<float>(segments);
				const float theta = u * 2.f * pi;
				Vector3f normal(r * std::cos(theta), y, r * std::sin(theta));
				Vector3f position = normal * 0.5f;
				Vector3f tangent_dir(-std::sin(theta), 0.f, std::cos(theta));

				vertices.push_back({position, {u, v}, Math::normalize(normal), Vector4f(Math::normalize(tangent_dir), 1.f)});
			}
		}

		const u32 stride = segments + 1;
		for (u32 ring = 0; ring < rings; ++ring)
		{
			for (u32 segment = 0; segment < segments; ++segment)
			{
				const u32 a = ring * stride + segment;
				const u32 b = a + stride;
				const u32 c = b + 1;
				const u32 d = a + 1;

				add_quad_indices(indices, a, b, c, d);
			}
		}

		return create_mesh(package, "Sphere", vertices, indices);
	}

	static StaticMesh* generate_cylinder(Package* package, u32 segments = 32)
	{
		Vector<ProceduralVertex> vertices;
		Vector<u32> indices;
		const float pi = std::numbers::pi_v<float>;

		for (u32 segment = 0; segment <= segments; ++segment)
		{
			const float u     = static_cast<float>(segment) / static_cast<float>(segments);
			const float theta = u * 2.f * pi;
			const float x     = std::cos(theta) * 0.5f;
			const float z     = std::sin(theta) * 0.5f;
			Vector3f normal   = Math::normalize(Vector3f(x, 0.f, z));
			Vector4f tangent(-std::sin(theta), 0.f, std::cos(theta), 1.f);

			vertices.push_back({{x, -0.5f, z}, {u, 0.f}, normal, tangent});
			vertices.push_back({{x, 0.5f, z}, {u, 1.f}, normal, tangent});
		}

		for (u32 segment = 0; segment < segments; ++segment)
		{
			const u32 base = segment * 2;
			add_quad_indices(indices, base + 0, base + 1, base + 3, base + 2);
		}

		const u32 top_center = vertices.size();
		vertices.push_back({{0.f, 0.5f, 0.f}, {0.5f, 0.5f}, {0.f, 1.f, 0.f}, {1.f, 0.f, 0.f, 1.f}});
		const u32 bottom_center = vertices.size();
		vertices.push_back({{0.f, -0.5f, 0.f}, {0.5f, 0.5f}, {0.f, -1.f, 0.f}, {1.f, 0.f, 0.f, 1.f}});

		for (u32 segment = 0; segment <= segments; ++segment)
		{
			const float u     = static_cast<float>(segment) / static_cast<float>(segments);
			const float theta = u * 2.f * pi;
			const float x     = std::cos(theta) * 0.5f;
			const float z     = std::sin(theta) * 0.5f;
			const Vector2f uv(x + 0.5f, z + 0.5f);

			vertices.push_back({{x, 0.5f, z}, uv, {0.f, 1.f, 0.f}, {1.f, 0.f, 0.f, 1.f}});
			vertices.push_back({{x, -0.5f, z}, uv, {0.f, -1.f, 0.f}, {1.f, 0.f, 0.f, 1.f}});
		}

		const u32 cap_start = top_center + 2;
		for (u32 segment = 0; segment < segments; ++segment)
		{
			const u32 top0    = cap_start + segment * 2;
			const u32 top1    = top0 + 2;
			const u32 bottom0 = top0 + 1;
			const u32 bottom1 = top1 + 1;

			add_triangle(indices, top_center, top0, top1);
			add_triangle(indices, bottom_center, bottom1, bottom0);
		}

		return create_mesh(package, "Cylinder", vertices, indices);
	}

	static StaticMesh* generate_cone(Package* package, u32 segments = 32)
	{
		Vector<ProceduralVertex> vertices;
		Vector<u32> indices;
		const float pi = std::numbers::pi_v<float>;
		const Vector3f apex(0.f, 0.5f, 0.f);

		for (u32 segment = 0; segment <= segments; ++segment)
		{
			const float u         = static_cast<float>(segment) / static_cast<float>(segments);
			const float theta     = u * 2.f * pi;
			const float x         = std::cos(theta) * 0.5f;
			const float z         = std::sin(theta) * 0.5f;
			const Vector3f radial = Math::normalize(Vector3f(x, 0.f, z));
			const Vector3f tangent_dir(-std::sin(theta), 0.f, std::cos(theta));
			const Vector3f normal = Math::normalize(Vector3f(radial.x, 0.5f, radial.z));

			vertices.push_back({{x, -0.5f, z}, {u, 0.f}, normal, Vector4f(Math::normalize(tangent_dir), 1.f)});
			vertices.push_back({apex, {u, 1.f}, normal, Vector4f(Math::normalize(tangent_dir), 1.f)});
		}

		for (u32 segment = 0; segment < segments; ++segment)
		{
			const u32 base = segment * 2;
			add_triangle(indices, base + 0, base + 1, base + 3);
		}

		const u32 bottom_center = vertices.size();
		vertices.push_back({{0.f, -0.5f, 0.f}, {0.5f, 0.5f}, {0.f, -1.f, 0.f}, {1.f, 0.f, 0.f, 1.f}});

		for (u32 segment = 0; segment <= segments; ++segment)
		{
			const float u     = static_cast<float>(segment) / static_cast<float>(segments);
			const float theta = u * 2.f * pi;
			const float x     = std::cos(theta) * 0.5f;
			const float z     = std::sin(theta) * 0.5f;
			vertices.push_back({{x, -0.5f, z}, {x + 0.5f, z + 0.5f}, {0.f, -1.f, 0.f}, {1.f, 0.f, 0.f, 1.f}});
		}

		const u32 bottom_ring = bottom_center + 1;
		for (u32 segment = 0; segment < segments; ++segment)
		{
			const u32 a = bottom_ring + segment;
			const u32 b = a + 1;
			add_triangle(indices, bottom_center, b, a);
		}

		return create_mesh(package, "Cone", vertices, indices);
	}

	static void generate_default_meshes()
	{
		Package* package = Package::static_find_package("TrinexEngine::Meshes", true);
		using namespace DefaultResources::Meshes;

		cube     = generate_cube(package);
		sphere   = generate_uv_sphere(package);
		cylinder = generate_cylinder(package);
		plane    = generate_plane(package);
		cone     = generate_cone(package);
	}

	void load_default_resources()
	{
		using namespace DefaultResources;

		generate_textures();

		Materials::sprite    = load_object<Material>("TrinexEngine::Materials::SpriteMaterial");
		Materials::base_pass = load_object<Material>("TrinexEngine::Materials::BasePassMaterial");
		generate_default_meshes();

		Buffers::screen_quad = trx_new PositionVertexBuffer({
		        Vector3f{-1.f, -1.f, 0.0f},
		        Vector3f{-1.f, 1.f, 0.0f},
		        Vector3f{1.f, 1.f, 0.0f},
		        Vector3f{-1.f, -1.f, 0.0f},
		        Vector3f{1.f, 1.f, 0.0f},
		        Vector3f{1.f, -1.f, 0.0f},
		});
	}

	trinex_on_shutdown()
	{
		if (DefaultResources::Buffers::screen_quad)
			trx_delete DefaultResources::Buffers::screen_quad;
	}
}// namespace Trinex
