#include <Graphics/rhi.hpp>

namespace Engine
{
    ENGINE_EXPORT RHI* rhi = nullptr;

    RHI_Object::RHI_Object(size_t init_ref_count) : m_references(init_ref_count)
    {}

    void RHI_Object::add_reference() const
    {
        ++m_references;
    }

    void RHI_Object::release() const
    {
        if (m_references > 0)
            --m_references;

        if (m_references == 0)
        {
            destroy();
        }
    }

    size_t RHI_Object::references() const
    {
        return m_references;
    }

    RHI_Object::~RHI_Object()
    {}
}// namespace Engine
