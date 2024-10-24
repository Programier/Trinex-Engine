#include <Core/config_manager.hpp>
#include <Core/default_resources.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/engine_resource.hpp>
#include <Core/filesystem/native_file_system.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/library.hpp>
#include <Core/logger.hpp>
#include <Core/package.hpp>
#include <Core/reflection/class.hpp>
#include <Engine/settings.hpp>
#include <Graphics/pipeline_buffers.hpp>
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
		Texture2D* default_icon                             = nullptr;
		Texture2D* add_icon                                 = nullptr;
		Texture2D* move_icon                                = nullptr;
		Texture2D* remove_icon                              = nullptr;
		Texture2D* rotate_icon                              = nullptr;
		Texture2D* scale_icon                               = nullptr;
		Texture2D* select_icon                              = nullptr;
		Texture2D* more_icon                                = nullptr;
		Texture2D* light_sprite                             = nullptr;
		Texture2D* blueprint_texture                        = nullptr;
		Sampler* default_sampler                            = nullptr;
		Material* grid_material                             = nullptr;
		Material* point_light_overlay_material              = nullptr;
		Material* spot_light_overlay_material               = nullptr;
		Material* texture_editor_material                   = nullptr;
		PositionVertexBuffer* spot_light_overlay_positions  = nullptr;
		PositionVertexBuffer* point_light_overlay_positions = nullptr;
	}// namespace EditorResources


	static void create_circle(const Function<void(float, float)>& callback)
	{
		for (int i = 1; i <= 360; ++i)
		{
			for (int j = -1; j <= 1; j++)
			{
				float angle = glm::two_pi<float>() * static_cast<float>(i + j) / 360.f;
				float x     = glm::cos(angle);
				float z     = glm::sin(angle);
				callback(x, z);
			}
		}
	}

	static void create_spot_light_overlay_positions()
	{
		EditorResources::spot_light_overlay_positions = Object::new_instance<EngineResource<PositionVertexBuffer>>();
		auto buffer                                   = EditorResources::spot_light_overlay_positions;

		static constexpr float circle_y = -1.f;

		// Create circle
		create_circle([buffer](float x, float z) { buffer->buffer.push_back(Vector3D(x, circle_y, z)); });

		buffer->buffer.push_back({0, 0, 0});
		buffer->buffer.push_back({1, circle_y, 0});

		buffer->buffer.push_back({0, 0, 0});
		buffer->buffer.push_back({-1, circle_y, 0});

		buffer->buffer.push_back({0, 0, 0});
		buffer->buffer.push_back({0, circle_y, 1});

		buffer->buffer.push_back({0, 0, 0});
		buffer->buffer.push_back({0, circle_y, -1});
		buffer->init_resource();
	}

	static void create_point_light_overlay_positions()
	{
		EditorResources::point_light_overlay_positions = Object::new_instance<EngineResource<PositionVertexBuffer>>();
		auto buffer                                    = EditorResources::point_light_overlay_positions;

		create_circle([buffer](float y, float z) { buffer->buffer.push_back(Vector3D(0, y, z)); });
		create_circle([buffer](float x, float z) { buffer->buffer.push_back(Vector3D(x, 0, z)); });
		create_circle([buffer](float x, float y) { buffer->buffer.push_back(Vector3D(x, y, 0)); });
		buffer->init_resource();
	}

	static void preinit()
	{
		auto fs       = rootfs();
		auto exec_dir = Platform::find_exec_directory();

		using FS = VFS::NativeFileSystem;

		fs->mount("[assets_dir]:/Editor", "Editor Assets", new FS(exec_dir / "resources/editor/assets"));
		fs->mount("[configs_dir]:/editor", "Editor Configs", new FS(exec_dir / "resources/editor/configs"));
		fs->mount("[shaders_dir]:/editor", "Editor Shaders", new FS(exec_dir / "resources/editor/shaders"));
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

		EditorResources::default_icon      = load_object<Texture2D>("Editor::Textures::DefaultIcon");
		EditorResources::add_icon          = load_object<Texture2D>("Editor::Textures::AddIcon");
		EditorResources::move_icon         = load_object<Texture2D>("Editor::Textures::MoveIcon");
		EditorResources::remove_icon       = load_object<Texture2D>("Editor::Textures::RemoveIcon");
		EditorResources::rotate_icon       = load_object<Texture2D>("Editor::Textures::RotateIcon");
		EditorResources::scale_icon        = load_object<Texture2D>("Editor::Textures::ScaleIcon");
		EditorResources::select_icon       = load_object<Texture2D>("Editor::Textures::SelectIcon");
		EditorResources::more_icon         = load_object<Texture2D>("Editor::Textures::MoreIcon");
		EditorResources::blueprint_texture = load_object<Texture2D>("Editor::Textures::BlueprintBackground");
		EditorResources::light_sprite      = load_object<Texture2D>("Editor::Textures::PointLightSprite");

		EditorResources::default_sampler              = load_object<Sampler>("Editor::Samplers::DefaultSampler");
		EditorResources::grid_material                = load_object<Material>("Editor::Materials::GridMaterial");
		EditorResources::point_light_overlay_material = load_object<Material>("Editor::Materials::PointLightOverlay");
		EditorResources::spot_light_overlay_material  = load_object<Material>("Editor::Materials::SpotLightOverlay");
		EditorResources::texture_editor_material      = load_object<Material>("Editor::Materials::TextureEditorMaterial");

		create_point_light_overlay_positions();
		create_spot_light_overlay_positions();

		Icons::on_editor_package_loaded();
		GarbageCollector::on_unreachable_check.push(skip_destroy_assets);
	}

	static StartupResourcesInitializeController on_init(initialialize_editor, "Load Editor Package");


	static void load_configs()
	{
		Engine::Settings::e_splash_font = "resources/fonts/Source Code Pro/SourceCodePro-Bold.ttf";
		Engine::ConfigManager::load_config_from_file("editor/editor.config");
	}

	static Engine::ConfigsInitializeController configs_initializer(load_configs, "EditorConfig");
	static Engine::PreInitializeController on_preinit(preinit, "EditorPreinit");
}// namespace Engine
