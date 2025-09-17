#include <Core/archive.hpp>
#include <Core/base_engine.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/default_resources.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/memory.hpp>
#include <Core/package.hpp>
#include <Graphics/gpu_buffers.hpp>
#include <Graphics/texture.hpp>
#include <random>

namespace Engine
{
	namespace DefaultResources
	{
		namespace Textures
		{
			ENGINE_EXPORT Texture2D* default_texture        = nullptr;
			ENGINE_EXPORT TextureCube* default_texture_cube = nullptr;
			ENGINE_EXPORT Texture2D* noise4x4               = nullptr;
			ENGINE_EXPORT Texture2D* noise16x16             = nullptr;
			ENGINE_EXPORT Texture2D* noise128x128           = nullptr;
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

	ENGINE_EXPORT Object* load_object_from_memory(const byte* data, size_t size, const StringView& name)
	{
		if (size > 0)
		{
			Vector<byte> tmp(data, data + size);
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
		texture->flags |= Object::StandAlone;
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
		std::uniform_int_distribution<uint8_t> dist(0, 255);

		for (byte& value : mip.data)
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

	static void generate_checker_texture(Vector2u size, uint_t cell, Color* texture)
	{
		const Color colors[2] = {{200, 200, 200, 255}, {100, 100, 100, 255}};

		for (uint32_t y = 0; y < size.y; ++y)
		{
			for (uint32_t x = 0; x < size.x; ++x)
			{
				uint_t index            = ((x / cell) + (y / cell)) % 2;
				texture[y * size.x + x] = colors[index];
			}
		}
	}

	static void generate_default_textures(Package* package)
	{
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

			for (uint_t i = 0; i < 6; ++i)
			{
				generate_checker_texture({128, 128}, 8, pixels);
				pixels += mip.size.x * mip.size.y;
			}

			texture->init_render_resources();
		}
	}

	static void generate_textures()
	{
		Package* package = Package::static_find_package("TrinexEngine::Textures", true);

		generate_default_textures(package);
		generate_noise_textures(package);
	}


	void load_default_resources()
	{
		using namespace DefaultResources;

		generate_textures();

		Materials::sprite    = load_object<Material>("TrinexEngine::Materials::SpriteMaterial");
		Materials::base_pass = load_object<Material>("TrinexEngine::Materials::BasePassMaterial");
		Meshes::cube         = load_object<StaticMesh>("TrinexEngine::Meshes::Cube");
		Meshes::sphere       = load_object<StaticMesh>("TrinexEngine::Meshes::Sphere");
		Meshes::cylinder     = load_object<StaticMesh>("TrinexEngine::Meshes::Cylinder");
		Meshes::plane        = load_object<StaticMesh>("TrinexEngine::Meshes::Plane");
		Meshes::cone         = load_object<StaticMesh>("TrinexEngine::Meshes::Cone");

		Buffers::screen_quad = trx_new PositionVertexBuffer({
		        Vector3f{-1.f, -1.f, 0.0f},
		        Vector3f{-1.f, 1.f, 0.0f},
		        Vector3f{1.f, 1.f, 0.0f},
		        Vector3f{-1.f, -1.f, 0.0f},
		        Vector3f{1.f, 1.f, 0.0f},
		        Vector3f{1.f, -1.f, 0.0f},
		});
	}

	static void on_destroy()
	{
		if (DefaultResources::Buffers::screen_quad)
			trx_delete DefaultResources::Buffers::screen_quad;
	}

	static byte destroy_id = DestroyController(on_destroy).id();
}// namespace Engine
