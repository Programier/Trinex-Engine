#pragma once
#include <Core/engine_types.hpp>
#include <Core/name.hpp>

namespace Engine
{
    class ENGINE_EXPORT Script final
    {
    private:
        Path _M_path;
        Script(const Path& path);
        ~Script();

    public:
        const Path& path() const;
        Script& load();
        friend class ScriptEngine;
    };
}// namespace Engine
