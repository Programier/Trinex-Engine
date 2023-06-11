#pragma once
#include <Core/constants.hpp>
#include <Core/engine_types.hpp>
#include <Core/export.hpp>
#include <iomanip>
#include <ostream>
#include <utility>
#include <Core/render_types.hpp>
#include <Core/texture_types.hpp>


namespace Engine
{
    class Window;

    namespace GraphicApiInterface
    {
        class ApiInterface;
    }

    class ENGINE_EXPORT EngineInstance final
    {
    private:
        class Renderer* _M_renderer = nullptr;
        GraphicApiInterface::ApiInterface* _M_api_interface = nullptr;

        EngineAPI _M_api;
        bool _M_is_inited = false;

        EngineInstance& trigger_terminate_functions();
        EngineInstance();
        EngineInstance& init_api();

        ~EngineInstance();
        int start(int argc, char** argv);
        void destroy();

    public:
        ENGINE_EXPORT static EngineInstance* instance();
        ENGINE_EXPORT static const String& project_name();
        ENGINE_EXPORT static const String& project_name(const String& name);
        ENGINE_EXPORT static int initialize(int argc, char** argv);
        const Window* window() const;
        SystemType system_type() const;
        EngineAPI api() const;
        bool is_inited() const;
        GraphicApiInterface::ApiInterface* api_interface() const;
        class Renderer* renderer() const;


        bool check_format_support(PixelType type, PixelComponentType component);
    };

    ENGINE_EXPORT extern EngineInstance* engine_instance;
}// namespace Engine
