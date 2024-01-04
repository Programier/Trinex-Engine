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
        Name _M_language;


    public:
        CallBacks<void()> on_language_changed;

        const String& localize(const String& line);
        const String& localize(const char* line);

        const Name& language() const;
        Localization& language(const String&);
        Localization& language(const char*);
        Localization& reload();

        friend class Singletone<Localization, EmptySingletoneParent>;
    };


}// namespace Engine
