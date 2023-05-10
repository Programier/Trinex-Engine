#pragma once
#include <Core/api_object.hpp>
#include <Core/buffer_types.hpp>

namespace Engine
{
    class ENGINE_EXPORT ShaderSharedBufferBase : public ApiObject
    {
    protected:
        ShaderSharedBufferBase& create_buffer(const byte* data, size_t size);
        ShaderSharedBufferBase& update_buffer(size_t offset, const byte* data, size_t size);

    public:
        ShaderSharedBufferBase& bind_buffer(BindingIndex index, size_t offset, size_t size);
    };

    class ENGINE_EXPORT BasicSSBO : public ApiObject
    {    
    public:
        delete_copy_constructors(BasicSSBO);
        constructor_hpp(BasicSSBO);
        BasicSSBO& gen();
        BasicSSBO& set_data();
        BasicSSBO& update_data(std::size_t elem_offset, std::size_t count);
        BasicSSBO& bind(unsigned int index = 0);
        const byte* data_ptr() const;
        virtual std::size_t value_size() const
        {
            return 0;
        }

        virtual std::size_t size() const
        {
            return 0;
        }

        virtual byte* data_ptr()
        {
            return 0;
        }
    };

    template<typename Type>
    class ShaderSSBO : public ShaderSharedBufferBase
    {
    public:
        Vector<Type> buffer;

        ShaderSSBO& create()
        {
            ShaderSharedBufferBase::create_buffer(reinterpret_cast<byte*>(buffer.data()), buffer.size() * sizeof(Type));
            return *this;
        }
    };


    template<typename Element>
    class SSBO : public BasicSSBO
    {
    public:
        Vector<Element> data;

    public:
        delete_copy_constructors(SSBO);
        SSBO()
        {}

        std::size_t value_size() const override
        {
            return sizeof(Element);
        }

        std::size_t size() const override
        {
            return data.size();
        }

        byte* data_ptr() override
        {
            return reinterpret_cast<byte*>(data.data());
        }
    };
}// namespace Engine
