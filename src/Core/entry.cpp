#include <Core/command_line.hpp>
#include <Core/console.hpp>
#include <Core/entry.hpp>
#include <Core/reflection/class.hpp>

namespace Trinex
{
	class DefaultEntry;

	static trinex_console_var(Refl::Class*, s_entry_point_class, "trinex.entry", nullptr);

	Ref<EntryPoint> EntryPoint::create()
	{
		Refl::Class* entry = s_entry_point_class;
		StringView value   = CommandLine::value("entry");

		if (value.empty())
		{
			if (entry == nullptr)
				entry = Refl::class_of<DefaultEntry>();
		}
		else
		{
			Refl::class_of<Refl::Class>()->parse_indirect(&entry, value);
		}

		if (entry)
		{
			return adopt_ref(entry->allocate<EntryPoint>());
		}

		return nullptr;
	}

	i32 EntryPoint::execute()
	{
		return 0;
	}
} // namespace Trinex
