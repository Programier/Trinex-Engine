#pragma once
#include <Core/arguments.hpp>
#include <Core/color_format.hpp>
#include <Core/constants.hpp>
#include <Core/engine_types.hpp>
#include <Core/etl/singletone.hpp>
#include <Core/structures.hpp>

namespace Engine
{
    class Thread;
    class Window;

    struct RHI;


    enum class ThreadType : EnumerateType
    {
        RenderThread = 0,
        __COUNT__,
    };


    class ENGINE_EXPORT EngineInstance final : public Singletone<EngineInstance, EmptyClass, false>
    {
    public:
        enum Flag : EnumerateType
        {
            PreInitTriggered              = BIT(0),
            InitTriggered                 = BIT(1),
            PostInitTriggered             = BIT(2),
            ClassInitTriggered            = BIT(3),
            DefaultResourcesInitTriggered = BIT(4),
            IsInited                      = BIT(5),
            IsShutingDown                 = BIT(6),
            IsRequestingExit              = BIT(7),
        };

    private:
        static EngineInstance* _M_instance;

        Arguments _M_args;
        Array<Thread*, static_cast<size_t>(ThreadType::__COUNT__)> _M_threads;
        RHI* _M_rhi = nullptr;
        Flags<EngineInstance::Flag> _M_flags;
        Index _M_frame_index       = 0;
        GCFlag _M_current_gc_stage = GCFlag::None;

        float _M_delta_time;


        EngineInstance();
        bool init_api();

        ~EngineInstance();
        int start(int argc, char** argv);
        void load_default_resources();

    private:
        void create_window();
        void create_render_targets();

    public:
        ENGINE_EXPORT static const String& project_name();
        ENGINE_EXPORT static const String& project_name(const String& name);
        ENGINE_EXPORT static int initialize(int argc, char** argv);
        const String& api_name() const;
        bool is_inited() const;
        RHI* rhi() const;
        bool is_shuting_down() const;
        bool is_requesting_exit() const;
        EngineInstance& request_exit();
        const Arguments& args() const;
        Arguments& args();
        const Flags<EngineInstance::Flag>& flags() const;
        EngineInstance& start_garbage_collection();

        Thread* create_thread(ThreadType type);
        Thread* thread(ThreadType type) const;
        int_t launch();

        float time_seconds() const;
        float delta_time() const;
        Index frame_index() const;

        friend class Singletone;
        friend class Object;
        friend class EngineSystem;
    };

    ENGINE_EXPORT extern EngineInstance* engine_instance;
}// namespace Engine
