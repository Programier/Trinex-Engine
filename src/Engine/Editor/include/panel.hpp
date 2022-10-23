#pragma once
#include <string>

namespace Engine
{
    class Panel
    {
    public:
        bool need_render = true;
        std::string name;
        virtual void render() = 0;
        virtual ~Panel(){};
    };
}// namespace Engine
