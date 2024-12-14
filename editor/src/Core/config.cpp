#include <Core/editor_config.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <ScriptEngine/script_engine.hpp>

namespace Engine::Settings::Editor
{
	String font_path       = "resources/TrinexEditor/fonts/Source Code Pro/SourceCodePro-Bold.ttf";
	float small_font_size  = 12.f;
	float normal_font_size = 18.f;
	float large_font_size  = 24.f;
	bool show_grid         = true;

	static PreInitializeController initialize([]() {
		auto& e = ScriptEngine::instance();

		e.begin_config_group("editor/editor.config");
		{
			ScriptNamespaceScopedChanger changer("Engine::Settings::Editor");

			e.register_property("string font_path", &font_path);
			e.register_property("float small_font_size", &small_font_size);
			e.register_property("float normal_font_size", &normal_font_size);
			e.register_property("float large_font_size", &large_font_size);
			e.register_property("bool show_grid", &show_grid);
		}
		e.end_config_group();
	});
}// namespace Engine::Settings::Editor
