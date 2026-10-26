#include <Core/editor_config.hpp>
#include <ScriptEngine/script_engine.hpp>

namespace Trinex::Settings::Editor
{
	String font_path       = "[content]:/TrinexEditor/fonts/Source Code Pro/SourceCodePro-Bold.ttf";
	float small_font_size  = 12.f;
	float normal_font_size = 18.f;
	float large_font_size  = 24.f;
	bool show_grid         = true;

	trinex_on_pre_init()
	{
		ScriptEngine::begin_config_group("editor/editor.config");
		{
			ScriptNamespaceScopedChanger changer("Trinex::Settings::Editor");

			ScriptEngine::register_property("string font_path", &font_path);
			ScriptEngine::register_property("float small_font_size", &small_font_size);
			ScriptEngine::register_property("float normal_font_size", &normal_font_size);
			ScriptEngine::register_property("float large_font_size", &large_font_size);
			ScriptEngine::register_property("bool show_grid", &show_grid);
		}
		ScriptEngine::end_config_group();
	}
} // namespace Trinex::Settings::Editor
