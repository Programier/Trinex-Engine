#pragma once
#include <Core/engine_types.hpp>
#include <Core/name.hpp>

namespace Engine
{
    class ENGINE_EXPORT Script final
    {
    private:
        Path m_path;
        Script(const Path& path);
        ~Script();

    public:
        const Path& path() const;
        Script& load();
        Script& replace_code(const char* new_code, size_t len = 0, size_t offset = 0);
        friend class ScriptEngine;
    };
}// namespace Engine
