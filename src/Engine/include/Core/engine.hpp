#pragma once
#include <Core/constants.hpp>
#include <Core/engine_types.hpp>
#include <Core/etl/singletone.hpp>
#include <Core/system.hpp>
#include <Core/structures.hpp>

namespace Engine
{
    class System;
    class Thread;
    class Window;
    class System;

    namespace RHI
    {
        struct ApiInterface;
    }


    enum class EngineInstanceFlags : EnumerateType
    {
        IsInited,
        IsShutingDown,
        IsRequestingExit,
        __COUNT__
    };

    enum class ThreadType : EnumerateType
    {
        RenderThread = 0,

        __COUNT__,
    };

    class EngineSystem;

    class ENGINE_EXPORT EngineInstance final : public Singletone<EngineInstance, EmptyClass>
    {
    private:
        Array<Thread*, static_cast<size_t>(ThreadType::__COUNT__)> _M_threads;
        EngineSystem* _M_engine_system = nullptr;

        class Renderer* _M_renderer = nullptr;
        Window* _M_window           = nullptr;

        RHI::ApiInterface* _M_api_interface = nullptr;
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
        Window* window() const;
        SystemName system_type() const;
        EngineAPI api() const;
        bool is_inited() const;
        RHI::ApiInterface* api_interface() const;
        class Renderer* renderer() const;
        static bool is_on_stack(void* ptr);
        bool is_shuting_down() const;
        bool is_requesting_exit() const;
        EngineInstance& request_exit();
        bool check_format_support(ColorFormat format);
        Window* create_window();
        EngineSystem* engine_system() const;

        Thread* create_thread(ThreadType type);
        Thread* thread(ThreadType type) const;
        int_t launch_systems() const;

        float time_seconds() const;

        friend class Singletone;
        friend class Object;
    };

    ENGINE_EXPORT extern EngineInstance* engine_instance;
}// namespace Engine
