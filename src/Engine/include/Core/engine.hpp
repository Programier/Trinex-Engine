#pragma once
#include <Core/constants.hpp>
#include <Core/engine_types.hpp>
#include <Core/etl/singletone.hpp>
#include <Core/render_types.hpp>
#include <Core/texture_types.hpp>

namespace Engine
{
    class Window;
    class System;
    class Thread;

    namespace GraphicApiInterface
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

    class ENGINE_EXPORT EngineInstance final : public Singletone<EngineInstance>
    {
    private:
        Array<Thread*, static_cast<size_t>(ThreadType::__COUNT__)> _M_threads;

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
        EngineInstance& request_exit();
        EngineInstance& launch_systems();
        bool check_format_support(PixelType type, PixelComponentType component);

        Thread* create_thread(ThreadType type);
        Thread* thread(ThreadType type) const;
        friend class Singletone;
    };

    ENGINE_EXPORT extern EngineInstance* engine_instance;
}// namespace Engine
