#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/templates.hpp>
#include <Core/exception.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_engine.hpp>

namespace Engine
{
	static void on_init()
	{
		ScriptClassRegistrar::ValueInfo info;
		info.pod	  = true;
		info.all_ints = true;

		ScriptClassRegistrar registrar =
				ScriptClassRegistrar::value_class("Engine::StringView", sizeof(StringView), info);

		registrar.behave(ScriptClassBehave::Construct, "void f()", ScriptClassRegistrar::constructor<StringView>);
		registrar.behave(ScriptClassBehave::Construct, "void f(const string& in)",
						 ScriptClassRegistrar::constructor<StringView, const String&>);

		registrar.method("bool empty() const", &StringView::empty);
		registrar.method("uint64 length() const", &StringView::length);
		registrar.method("uint64 size() const", &StringView::size);
		registrar.method("uint64 max_size() const", &StringView::max_size);
		registrar.method("uint8 at(uint64) const", &StringView::at);
		registrar.method("uint8 opIndex(uint64) const", &StringView::at);
		registrar.method("StringView substr(uint64 = 0, uint64 = -1) const", &StringView::substr);
		registrar.method("void remove_prefix(uint64)", &StringView::remove_prefix);
		registrar.method("void remove_suffix(uint64)", &StringView::remove_suffix);

		registrar.method("bool starts_with(uint8) const", method_of<bool, char>(&StringView::starts_with));
		registrar.method("bool starts_with(StringView) const", method_of<bool, StringView>(&StringView::starts_with));

		registrar.method("uint64 find(uint8, uint64 = 0) const",
						 method_of<StringView::size_type, char, StringView::size_type>(&StringView::find));
		registrar.method("uint64 find(StringView, uint64 = 0) const",
						 method_of<StringView::size_type, StringView, StringView::size_type>(&StringView::find));

		registrar.method("uint64 find_first_not_of(uint8, uint64 = 0) const",
						 method_of<StringView::size_type, char, StringView::size_type>(&StringView::find_first_not_of));
		registrar.method(
				"uint64 find_first_not_of(StringView, uint64 = 0) const",
				method_of<StringView::size_type, StringView, StringView::size_type>(&StringView::find_first_not_of));

		registrar.method("uint64 find_first_of(uint8, uint64 = 0) const",
						 method_of<StringView::size_type, char, StringView::size_type>(&StringView::find_first_of));
		registrar.method(
				"uint64 find_first_of(StringView, uint64 = 0) const",
				method_of<StringView::size_type, StringView, StringView::size_type>(&StringView::find_first_of));

		registrar.method("uint64 find_last_not_of(uint8, uint64 = 0) const",
						 method_of<StringView::size_type, char, StringView::size_type>(&StringView::find_last_not_of));
		registrar.method(
				"uint64 find_last_not_of(StringView, uint64 = 0) const",
				method_of<StringView::size_type, StringView, StringView::size_type>(&StringView::find_last_not_of));

		registrar.method("uint64 find_last_of(uint8, uint64 = 0) const",
						 method_of<StringView::size_type, char, StringView::size_type>(&StringView::find_last_of));
		registrar.method(
				"uint64 find_last_of(StringView, uint64 = 0) const",
				method_of<StringView::size_type, StringView, StringView::size_type>(&StringView::find_last_of));
	}

	static ReflectionInitializeController initialize(on_init, "Engine::StringView");
}// namespace Engine
