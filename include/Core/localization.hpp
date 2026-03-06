#pragma once
#include <Core/callback.hpp>
#include <Core/engine_types.hpp>
#include <Core/etl/map.hpp>
#include <Core/etl/singletone.hpp>

namespace Engine
{
	class ENGINE_EXPORT Localization : public Singletone<Localization, EmptySingletoneParent>
	{
	private:
		static Localization* s_instance;
		Map<u64, String> m_translation_map;
		mutable Map<u64, String> m_default_translation_map;

	public:
		CallBacks<void()> on_language_changed;

		const String& localize(const StringView& line) const;

		const String& language() const;
		Localization& language(const StringView&);
		Localization& reload(bool clear = true, bool with_default = false);

		friend class Singletone<Localization, EmptySingletoneParent>;
	};


}// namespace Engine
