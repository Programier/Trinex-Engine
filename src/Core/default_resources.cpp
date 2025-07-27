#include <Core/archive.hpp>
#include <Core/base_engine.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/default_resources.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/memory.hpp>
#include <Core/package.hpp>
#include <Graphics/gpu_buffers.hpp>
#include <Graphics/texture_2D.hpp>
#include <random>

namespace Engine
{
	namespace DefaultResources
	{
		namespace Textures
		{
			ENGINE_EXPORT Texture2D* default_texture = nullptr;
			ENGINE_EXPORT Texture2D* noise4x4;
			ENGINE_EXPORT Texture2D* noise16x16;
			ENGINE_EXPORT Texture2D* noise128x128;
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

	static void generate_noise_texture(Package* package, Texture2D*& texture, const char* name, Vector2u size)
	{
		texture = Object::new_instance<Texture2D>(name, package);
		texture->flags |= Object::StandAlone;
		texture->format = RHIColorFormat::R8G8B8A8;

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


	static void generate_noise_textures()
	{
		Package* package = Package::static_find_package("TrinexEngine::Textures", true);

		generate_noise_texture(package, DefaultResources::Textures::noise4x4, "Noise4x4", {4, 4});
		generate_noise_texture(package, DefaultResources::Textures::noise16x16, "Noise16x16", {16, 16});
		generate_noise_texture(package, DefaultResources::Textures::noise128x128, "Noise128x128", {128, 128});
	}

	void load_default_resources()
	{
		using namespace DefaultResources;

		generate_noise_textures();

		Textures::default_texture = load_object<Texture2D>("TrinexEngine::Textures::DefaultTexture");
		Materials::sprite         = load_object<Material>("TrinexEngine::Materials::SpriteMaterial");
		Materials::base_pass      = load_object<Material>("TrinexEngine::Materials::BasePassMaterial");
		Meshes::cube              = load_object<StaticMesh>("TrinexEngine::Meshes::Cube");
		Meshes::sphere            = load_object<StaticMesh>("TrinexEngine::Meshes::Sphere");
		Meshes::cylinder          = load_object<StaticMesh>("TrinexEngine::Meshes::Cylinder");

		Buffers::screen_quad = allocate<PositionVertexBuffer>(std::initializer_list<Vector3f>{
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
			release(DefaultResources::Buffers::screen_quad);
	}

	static byte destroy_id = DestroyController(on_destroy).id();
}// namespace Engine
