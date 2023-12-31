#pragma once

namespace Engine
{
    void make_dock_window(const char* name, unsigned int flags = 0, void (*callback)(void* userdata) = nullptr,
                          void* userdata = nullptr);

    template<typename Instance>
    void make_dock_window(const char* name, unsigned int flags, void (Instance::*callback)(void* userdata) = nullptr,
                          Instance* instance = nullptr, void* userdata = nullptr)
    {
        struct InternalData {
            Instance* instance                = nullptr;
            void (Instance::*callback)(void*) = nullptr;
            void* userdata;
        } data{instance, callback, userdata};

        struct InternalFunction {
            static void execute(void* _data)
            {
                InternalData* data = reinterpret_cast<InternalData*>(_data);
                (data->instance->*data->callback)(data->userdata);
            }
        };

        return make_dock_window(name, flags, InternalFunction::execute, &data);
    }
}// namespace Engine
