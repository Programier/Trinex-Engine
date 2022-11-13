#pragma once
#include <Core/export.hpp>
#include <string>

namespace Engine::Strings
{
    ENGINE_EXPORT std::wstring to_wstring(const std::string& str);
    ENGINE_EXPORT std::string to_string(const std::wstring& str);
}// namespace Engine::Strings
