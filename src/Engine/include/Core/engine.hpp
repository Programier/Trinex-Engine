#pragma once
#include <Core/constants.hpp>
#include <Core/engine_types.hpp>
#include <Core/etl/singletone.hpp>
#include <Core/render_types.hpp>
#include <Core/texture_types.hpp>
#include <iomanip>
#include <ostream>
#include <utility>

namespace Engine
{
    class Window;
    class System;
    class Thread;

    namespace GraphicApiInterface
    {
        class ApiInterface;
    }


    enum class EngineInstanceFlags : EnumerateType
    {
        IsInited,
        IsShutingDown,
        IsRequestingExit,
        IsSystemsLaunched,
        __COUNT__
    };

    class ENGINE_EXPORT EngineInstance final : public Singletone<EngineInstance>
    {
    private:
        struct SystemEntry {
            String _M_name;
            System* _M_system = nullptr;
            Thread* _M_thread = nullptr;
        };

        Vector<SystemEntry> _M_systems;

        class Renderer* _M_renderer                         = nullptr;
        GraphicApiInterface::ApiInterface* _M_api_interface = nullptr;
        static EngineInstance* _M_instance;
        BitSet<static_cast<EnumerateType>(EngineInstanceFlags::__COUNT__)> _M_flags;
        EngineAPI _M_api;



        EngineInstance& trigger_terminate_functions();
        EngineInstance();
        EngineInstance& init_api();

        ~EngineInstance();
        int start(int argc, char** argv);

    public:
        ENGINE_EXPORT static const String& project_name();
        ENGINE_EXPORT static const String& project_name(const String& name);
        ENGINE_EXPORT static int initialize(int argc, char** argv);
        const Window* window() const;
        SystemName system_type() const;
        EngineAPI api() const;
        bool is_inited() const;
        GraphicApiInterface::ApiInterface* api_interface() const;
        class Renderer* renderer() const;
        static bool is_on_stack(void* ptr);
        bool is_shuting_down() const;
        bool is_requesting_exit() const;
        EngineInstance& requesting_exit();
        EngineInstance& launch_systems();
        bool check_format_support(PixelType type, PixelComponentType component);

        template<typename SystemType>
        EngineInstance& add_system(const String& name = "")
        {
            _M_systems.push_back({name, new SystemType(), nullptr});
            return *this;
        }

        friend class Singletone;
    };

    ENGINE_EXPORT extern EngineInstance* engine_instance;
}// namespace Engine
