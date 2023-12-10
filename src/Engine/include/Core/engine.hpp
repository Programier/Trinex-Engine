#pragma once
#include <Core/color_format.hpp>
#include <Core/constants.hpp>
#include <Core/engine_types.hpp>
#include <Core/etl/singletone.hpp>
#include <Core/structures.hpp>
#include <Core/system.hpp>

namespace Engine
{
    class System;
    class Thread;
    class Window;
    class System;

    struct RHI;

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


    class ENGINE_EXPORT EngineInstance final : public Singletone<EngineInstance, EmptyClass>
    {
    private:
        static EngineInstance* _M_instance;

        Array<Thread*, static_cast<size_t>(ThreadType::__COUNT__)> _M_threads;
        class Renderer* _M_renderer = nullptr;
        RHI* _M_rhi                 = nullptr;
        BitSet<static_cast<EnumerateType>(EngineInstanceFlags::__COUNT__)> _M_flags;
        EngineAPI _M_api;
        Index _M_frame_index = 0;


        EngineInstance& trigger_terminate_functions();
        EngineInstance();
        EngineInstance& init_api();

        ~EngineInstance();
        int start(int argc, char** argv);

    private:
        void init_engine_for_rendering();
        void create_window();
        void create_render_targets();

    public:
        ENGINE_EXPORT static const String& project_name();
        ENGINE_EXPORT static const String& project_name(const String& name);
        ENGINE_EXPORT static int initialize(int argc, char** argv);
        SystemName system_type() const;
        EngineAPI api() const;
        const String& api_name() const;
        bool is_inited() const;
        RHI* rhi() const;
        class Renderer* renderer() const;
        bool is_shuting_down() const;
        bool is_requesting_exit() const;
        EngineInstance& request_exit();

        Thread* create_thread(ThreadType type);
        Thread* thread(ThreadType type) const;
        int_t launch();

        float time_seconds() const;
        Index frame_index() const;

        friend class Singletone;
        friend class Object;
        friend class EngineSystem;
    };

    ENGINE_EXPORT extern EngineInstance* engine_instance;
}// namespace Engine
