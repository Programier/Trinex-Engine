#include <Core/config_manager.hpp>
#include <Core/default_resources.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/engine_resource.hpp>
#include <Core/etl/templates.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/library.hpp>
#include <Core/logger.hpp>
#include <Core/memory.hpp>
#include <Core/package.hpp>
#include <Core/reflection/class.hpp>
#include <Engine/settings.hpp>
#include <Graphics/gpu_buffers.hpp>
#include <Platform/platform.hpp>

namespace Engine
{
	static void skip_destroy_assets(Object* object)
	{
		if (object->class_instance()->is_asset())
		{
			object->flags(Object::Flag::IsUnreachable, false);
		}
	}

	namespace EditorResources
	{
		Texture2D* default_icon      = nullptr;
		Texture2D* light_sprite      = nullptr;
		Texture2D* blueprint_texture = nullptr;
	}// namespace EditorResources

	static void preinit()
	{
		auto fs = rootfs();

		fs->mount("[content]:/TrinexEditor", "[exec]:/resources/TrinexEditor");
		fs->mount("[assets]:/TrinexEditor", "[exec]:/resources/TrinexEditor/assets");
		fs->mount("[configs]:/editor", "[exec]:/resources/TrinexEditor/configs");
		fs->mount("[shaders]:/TrinexEditor", "[exec]:/resources/TrinexEditor/shaders");
		fs->mount("[scripts]:/TrinexEditor", "[exec]:/resources/TrinexEditor/scripts");

		Settings::engine_class                        = "Engine::EditorEngine";
		Settings::Rendering::force_keep_cpu_resources = true;
	}

	template<typename T>
	static T* load_object(const char* name)
	{
		Object* obj = Object::load_object(name);
		return reinterpret_cast<T*>(obj);
	}

	static void initialialize_editor()
	{
#define load_resource(var, name, type, group_name)                                                                               \
	EditorResources::var =                                                                                                       \
	        reinterpret_cast<type*>(load_object_from_memory(name##_data, name##_len, "Editor::" #group_name "::" #name));        \
	reinterpret_cast<Object*>(EditorResources::var)->add_reference()

		EditorResources::default_icon      = load_object<Texture2D>("TrinexEditor::Textures::DefaultIcon");
		EditorResources::blueprint_texture = load_object<Texture2D>("TrinexEditor::Textures::BlueprintBackground");
		EditorResources::light_sprite      = load_object<Texture2D>("TrinexEditor::Textures::PointLightSprite");

		GarbageCollector::on_unreachable_check.push(skip_destroy_assets);
	}

	static void load_configs()
	{
		Engine::Settings::Splash::font = "[content]:/TrinexEditor/fonts/Source Code Pro/SourceCodePro-Bold.ttf";
	}

	static StartupResourcesInitializeController on_init(initialialize_editor, "Load Editor Package");
	static Engine::ConfigsInitializeController configs_initializer(load_configs, "EditorConfig");
	static Engine::PreInitializeController on_preinit(preinit, "EditorPreinit");
}// namespace Engine
