#pragma once
#include <vulkan_api.hpp>

namespace Engine
{
    template<typename Type>
    struct VulkanUniquePerFrame {
        Vector<Type*> m_instances;

        template<typename... Args>
        VulkanUniquePerFrame(Args&&... args)
        {
            m_instances.resize(API->m_framebuffers_count);
            for (uint32_t i = 0; i < API->m_framebuffers_count; ++i)
            {
                m_instances[i] = new Type(std::forward<Args>(args)...);
            }
        }

        FORCE_INLINE Type& current()
        {
            return *m_instances[API->m_current_buffer];
        }

        ~VulkanUniquePerFrame()
        {
            for (Type* instance : m_instances)
            {
                delete instance;
            }

            m_instances.clear();
        }
    };
}// namespace Engine
