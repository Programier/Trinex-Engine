#pragma once
#include <Core/callback.hpp>
#include <Core/engine_types.hpp>
#include <Core/etl/singletone.hpp>

namespace Engine
{
    class ENGINE_EXPORT Localization : public Singletone<Localization, EmptySingletoneParent>
    {
    private:
        static Localization* _M_instance;
        Map<HashIndex, String> _M_translation_map;
        mutable Map<HashIndex, String> _M_default_translation_map;

    public:
        CallBacks<void()> on_language_changed;

        const String& localize(const StringView& line) const;

        const String& language() const;
        Localization& language(const StringView&);
        Localization& reload(bool clear = true, bool with_default = false);

        friend class Singletone<Localization, EmptySingletoneParent>;
    };


}// namespace Engine
