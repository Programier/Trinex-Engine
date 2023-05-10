#include <Core/engine.hpp>
#include <Graphics/ssbo.hpp>
#include <api.hpp>

namespace Engine
{
    ShaderSharedBufferBase& ShaderSharedBufferBase::create_buffer(const byte* data, size_t size)
    {
        if (_M_ID)
            destroy();

        EngineInstance::instance()->api_interface()->create_ssbo(_M_ID, data, size);
        return *this;
    }

    ShaderSharedBufferBase& ShaderSharedBufferBase::update_buffer(size_t offset, const byte* data, size_t size)
    {
        if (_M_ID)
            EngineInstance::instance()->api_interface()->update_ssbo(_M_ID, data, offset, size);
        return *this;
    }

    ShaderSharedBufferBase& ShaderSharedBufferBase::bind_buffer(BindingIndex index, size_t offset, size_t size)
    {
        if (_M_ID)
            EngineInstance::instance()->api_interface()->bind_ssbo(_M_ID, index, offset, size);
        return *this;
    }


    BasicSSBO::BasicSSBO()
    {}

    BasicSSBO& BasicSSBO::gen()
    {
        destroy();
        // EngineInstance::instance()->api_interface()->create_ssbo(_M_ID);
        return *this;
    }

    BasicSSBO& BasicSSBO::set_data()
    {
        // EngineInstance::instance()->api_interface()->ssbo_data(_M_ID, data_ptr(), size() * value_size(), usage);
        return *this;
    }

    BasicSSBO& BasicSSBO::update_data(std::size_t elem_offset, std::size_t count)
    {
        if (elem_offset + count > size())
            return *this;
        //  EngineInstance::instance()->api_interface()->update_ssbo_data(
        //      _M_ID, data_ptr() + _M_value_size * elem_offset, count * _M_value_size, elem_offset * _M_value_size);
        return *this;
    }

    BasicSSBO& BasicSSBO::bind(unsigned int index)
    {
        //EngineInstance::instance()->api_interface()->bind_ssbo(_M_ID, index);
        return *this;
    }

    const byte* BasicSSBO::data_ptr() const
    {
        return const_cast<BasicSSBO*>(this)->data_ptr();
    }
}// namespace Engine
