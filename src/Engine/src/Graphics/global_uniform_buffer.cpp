#include <Core/engine.hpp>
#include <Graphics/global_uniform_buffer.hpp>


namespace Engine
{
    GlobalUniformBuffer* GlobalUniformBuffer::_M_instance = nullptr;
    Name GlobalUniformBuffer::default_pass;

    GlobalUniformBuffer::GlobalUniformBuffer()
    {
        default_pass = "Default";
        allocate_new_buffer(default_pass);
        change_pass(default_pass);
    }

    GlobalUniformBuffer& GlobalUniformBuffer::allocate_new_buffer(Name pass_name)
    {
        Buffer& buffer = _M_buffers[pass_name.hash()];

        buffer._M_buffer            = Object::new_instance<UniformBuffer>();
        buffer._M_buffer->init_data = reinterpret_cast<const byte*>(&buffer._M_data);
        buffer._M_buffer->init_size = sizeof(Data);

        buffer._M_buffer->rhi_create();
        return *this;
    }

    GlobalUniformBuffer::Buffer* GlobalUniformBuffer::find_buffer(Name pass_name)
    {
        auto it = _M_buffers.find(pass_name.hash());
        if (it == _M_buffers.end())
            return nullptr;

        return &(it->second);
    }

    bool GlobalUniformBuffer::change_pass(Name pass_name)
    {
        if (_M_current_pass == pass_name)
            return true;

        Buffer* buffer = find_buffer(pass_name);
        if (!buffer)
            return false;

        _M_current_buffer = buffer;
        _M_current_pass   = pass_name;
        return true;
    }

    Name GlobalUniformBuffer::current_pass_name() const
    {
        return _M_current_pass;
    }

    GlobalUniformBuffer::Data* GlobalUniformBuffer::current_data()
    {
        return _M_current_buffer ? &_M_current_buffer->_M_data : nullptr;
    }

    GlobalUniformBuffer::Data* GlobalUniformBuffer::data_of(Name pass_name)
    {
        Buffer* buffer = find_buffer(pass_name);
        return buffer ? &buffer->_M_data : nullptr;
    }

    void GlobalUniformBuffer::private_rhi_update(Buffer* buffer, size_t size, size_t offset)
    {
        if (buffer == nullptr)
            return;

        offset = glm::min(sizeof(Data), offset);
        size   = glm::min(sizeof(Data) - offset, size);
        if (size > 0)
        {
            buffer->_M_buffer->rhi_update(offset, size, reinterpret_cast<const byte*>(&buffer->_M_data));
        }
    }

    GlobalUniformBuffer& GlobalUniformBuffer::update(float dt)
    {
        float time = engine_instance->time_seconds();

        for (auto& ubo : _M_buffers)
        {
            ubo.second._M_data.dt   = dt;
            ubo.second._M_data.time = time;

            ubo.second._M_buffer->rhi_update(0, sizeof(Data), reinterpret_cast<const byte*>(&ubo.second._M_data));
        }
        return *this;
    }

    void GlobalUniformBuffer::private_rhi_bind(Buffer* buffer, BindLocation location)
    {
        if (buffer == nullptr || !buffer->_M_buffer)
            return;
        buffer->_M_buffer->rhi_bind(location);
    }

    GlobalUniformBuffer& GlobalUniformBuffer::rhi_update(size_t size, size_t offset)
    {
        private_rhi_update(_M_current_buffer, size, offset);
        return *this;
    }


    GlobalUniformBuffer& GlobalUniformBuffer::rhi_update(Name pass_name, size_t size, size_t offset)
    {
        private_rhi_update(find_buffer(pass_name), size, offset);
        return *this;
    }

    GlobalUniformBuffer& GlobalUniformBuffer::rhi_bind(Name pass_name, BindLocation location)
    {
        private_rhi_bind(find_buffer(pass_name), location);
        return *this;
    }

    GlobalUniformBuffer& GlobalUniformBuffer::rhi_bind(BindLocation location)
    {
        private_rhi_bind(_M_current_buffer, location);
        return *this;
    }

    static void on_rhi_init()
    {
        GlobalUniformBuffer::create_instance();
    }

    static AfterRHIInitializeController controller(on_rhi_init);

}// namespace Engine
