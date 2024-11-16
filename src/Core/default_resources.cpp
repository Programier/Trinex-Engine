#include <Core/archive.hpp>
#include <Core/base_engine.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/default_resources.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/package.hpp>

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
			ENGINE_EXPORT PositionVertexBuffer* screen_position = nullptr;
		}

		namespace Materials
		{
			ENGINE_EXPORT Material* sprite            = nullptr;
			ENGINE_EXPORT Material* screen            = nullptr;
			ENGINE_EXPORT Material* base_pass         = nullptr;
			ENGINE_EXPORT Material* batched_lines     = nullptr;
			ENGINE_EXPORT Material* batched_triangles = nullptr;
			ENGINE_EXPORT Material* point_light       = nullptr;
			ENGINE_EXPORT Material* spot_light        = nullptr;
			ENGINE_EXPORT Material* directional_light = nullptr;
			ENGINE_EXPORT Material* ambient_light     = nullptr;
			ENGINE_EXPORT Material* imgui             = nullptr;
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
		Samplers::default_sampler    = load_object<Sampler>("DefaultPackage::Samplers::DefaultSampler");
		Textures::default_texture    = load_object<Texture2D>("DefaultPackage::Textures::DefaultTexture");
		Buffers::screen_position     = load_object<PositionVertexBuffer>("DefaultPackage::Buffers::ScreenPositionBuffer");
		Materials::sprite            = load_object<Material>("DefaultPackage::Materials::SpriteMaterial");
		Materials::screen            = load_object<Material>("DefaultPackage::Materials::ScreenMaterial");
		Materials::base_pass         = load_object<Material>("DefaultPackage::Materials::BasePassMaterial");
		Materials::batched_lines     = load_object<Material>("DefaultPackage::Materials::BatchedLinesMaterial");
		Materials::batched_triangles = load_object<Material>("DefaultPackage::Materials::BatchedTrianglesMaterial");
		Materials::point_light       = load_object<Material>("DefaultPackage::Materials::PointLightMaterial");
		Materials::spot_light        = load_object<Material>("DefaultPackage::Materials::SpotLightMaterial");
		Materials::directional_light = load_object<Material>("DefaultPackage::Materials::DirectionalLightMaterial");
		Materials::ambient_light     = load_object<Material>("DefaultPackage::Materials::AmbientLightMaterial");
		Materials::imgui             = load_object<Material>("DefaultPackage::Materials::ImGuiMaterial");
		Meshes::cube                 = load_object<StaticMesh>("DefaultPackage::Meshes::Cube");
		Meshes::sphere               = load_object<StaticMesh>("DefaultPackage::Meshes::Sphere");
		Meshes::cylinder             = load_object<StaticMesh>("DefaultPackage::Meshes::Cylinder");
	}
}// namespace Engine
