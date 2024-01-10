#include <Core/config.hpp>

namespace Engine
{
    Config& Config::update()
    {
        return *this;
    }

    Config& Config::update_using_args()
    {
        return *this;
    }

    Config::~Config()
    {}
}// namespace Engine
