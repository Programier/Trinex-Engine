#pragma once
#include <vulkan_api.hpp>

namespace Engine
{
    template<typename Type>
    struct VulkanUniquePerFrame {
        Vector<Type*> _M_instances;

        template<typename... Args>
        VulkanUniquePerFrame(Args&&... args)
        {
            _M_instances.resize(MAIN_FRAMEBUFFERS_COUNT);
            for (int i = 0; i < MAIN_FRAMEBUFFERS_COUNT; ++i)
            {
                _M_instances[i] = new Type(std::forward<Args>(args)...);
            }
        }

        FORCE_INLINE Type& current()
        {
            return *_M_instances[API->_M_current_buffer];
        }

        ~VulkanUniquePerFrame()
        {
            for (Type* instance : _M_instances)
            {
                delete instance;
            }

            _M_instances.clear();
        }
    };
}// namespace Engine
