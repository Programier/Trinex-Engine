#include <Core/config_manager.hpp>
#include <Core/default_resources.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/engine_resource.hpp>
#include <Core/etl/templates.hpp>
#include <Core/filesystem/native_file_system.hpp>
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

	namespace Icons
	{
		extern void on_editor_package_loaded();
	}

	namespace EditorResources
	{
		Texture2D* default_icon      = nullptr;
		Texture2D* add_icon          = nullptr;
		Texture2D* move_icon         = nullptr;
		Texture2D* remove_icon       = nullptr;
		Texture2D* rotate_icon       = nullptr;
		Texture2D* scale_icon        = nullptr;
		Texture2D* select_icon       = nullptr;
		Texture2D* more_icon         = nullptr;
		Texture2D* light_sprite      = nullptr;
		Texture2D* blueprint_texture = nullptr;
	}// namespace EditorResources

	static void preinit()
	{
		auto fs       = rootfs();
		auto exec_dir = Platform::find_exec_directory();
		auto callback = delete_value<VFS::FileSystem>;

		using FS = VFS::NativeFileSystem;

		fs->mount("[assets_dir]:/TrinexEditor", "Editor Assets", new FS(exec_dir / "resources/TrinexEditor/assets"), callback);
		fs->mount("[configs_dir]:/editor", "Editor Configs", new FS(exec_dir / "resources/TrinexEditor/configs"), callback);
		fs->mount("[shaders_dir]:/TrinexEditor", "Editor Shaders", new FS(exec_dir / "resources/TrinexEditor/shaders"), callback);
		fs->mount("[scripts_dir]:/TrinexEditor", "Editor Scripts", new FS(exec_dir / "resources/TrinexEditor/scripts"), callback);

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
		EditorResources::add_icon          = load_object<Texture2D>("TrinexEditor::Textures::AddIcon");
		EditorResources::move_icon         = load_object<Texture2D>("TrinexEditor::Textures::MoveIcon");
		EditorResources::remove_icon       = load_object<Texture2D>("TrinexEditor::Textures::RemoveIcon");
		EditorResources::rotate_icon       = load_object<Texture2D>("TrinexEditor::Textures::RotateIcon");
		EditorResources::scale_icon        = load_object<Texture2D>("TrinexEditor::Textures::ScaleIcon");
		EditorResources::select_icon       = load_object<Texture2D>("TrinexEditor::Textures::SelectIcon");
		EditorResources::more_icon         = load_object<Texture2D>("TrinexEditor::Textures::MoreIcon");
		EditorResources::blueprint_texture = load_object<Texture2D>("TrinexEditor::Textures::BlueprintBackground");
		EditorResources::light_sprite      = load_object<Texture2D>("TrinexEditor::Textures::PointLightSprite");

		Icons::on_editor_package_loaded();
		GarbageCollector::on_unreachable_check.push(skip_destroy_assets);
	}

	static void load_configs()
	{
		Engine::Settings::Splash::font = "resources/fonts/Source Code Pro/SourceCodePro-Bold.ttf";
	}

	static StartupResourcesInitializeController on_init(initialialize_editor, "Load Editor Package");
	static Engine::ConfigsInitializeController configs_initializer(load_configs, "EditorConfig");
	static Engine::PreInitializeController on_preinit(preinit, "EditorPreinit");
}// namespace Engine
