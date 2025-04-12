#include <Core/archive.hpp>
#include <Core/base_engine.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/default_resources.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/memory.hpp>
#include <Core/package.hpp>
#include <Graphics/gpu_buffers.hpp>

namespace Engine
{
	namespace DefaultResources
	{
		namespace Samplers
		{
			ENGINE_EXPORT Sampler* default_sampler = nullptr;
		}

		namespace Textures
		{
			ENGINE_EXPORT Texture2D* default_texture = nullptr;
		}

		namespace Buffers
		{
			ENGINE_EXPORT PositionVertexBuffer* screen_quad = nullptr;
		}

		namespace Materials
		{
			ENGINE_EXPORT Material* sprite    = nullptr;
			ENGINE_EXPORT Material* screen    = nullptr;
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

	void load_default_resources()
	{
		using namespace DefaultResources;
		Samplers::default_sampler = load_object<Sampler>("TrinexEngine::Samplers::DefaultSampler");
		Textures::default_texture = load_object<Texture2D>("TrinexEngine::Textures::DefaultTexture");
		Materials::sprite         = load_object<Material>("TrinexEngine::Materials::SpriteMaterial");
		Materials::screen         = load_object<Material>("TrinexEngine::Materials::ScreenMaterial");
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
		release(DefaultResources::Buffers::screen_quad);
	}

	static byte destroy_id = DestroyController(on_destroy).id();
}// namespace Engine
