#include <Core/localization.hpp>
#include <Core/object.hpp>


namespace Engine
{
    Localization* Localization::_M_instance = nullptr;

    const Name& Localization::language() const
    {
        return _M_language;
    }

    Localization& Localization::language(const String& lang)
    {
        if (_M_language == lang)
            return *this;

        _M_language = lang;
        reload();

        on_language_changed.trigger();
        return *this;
    }

    Localization& Localization::language(const char* lang)
    {
        if (_M_language == lang)
            return *this;

        _M_language = lang;
        reload();
        on_language_changed.trigger();
        return *this;
    }

    Localization& Localization::reload()
    {
        return *this;
    }

    ENGINE_EXPORT const String& Object::language()
    {
        return Localization::instance()->language().to_string();
    }

    ENGINE_EXPORT void Object::language(const String& new_language)
    {
        Localization::instance()->language(new_language);
    }

    ENGINE_EXPORT void language(const char* new_language)
    {
        Localization::instance()->language(new_language);
    }

}// namespace Engine
