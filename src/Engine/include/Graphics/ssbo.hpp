#pragma once
#include <Core/api_object.hpp>

namespace Engine
{
    ENGINE_EXPORT class BasicSSBO : public ApiObject
    {
    public:
        BufferUsage usage;
        declare_instance_info_hpp(BasicSSBO);

    public:
        delete_copy_constructors(BasicSSBO);
        constructor_hpp(BasicSSBO);
        BasicSSBO& gen();
        BasicSSBO& set_data();
        BasicSSBO& update_data(std::size_t elem_offset, std::size_t count);
        BasicSSBO& bind(unsigned int index = 0);
        const byte* data_ptr() const;
        virtual std::size_t value_size() const = 0;
        virtual std::size_t size() const = 0;
        virtual byte* data_ptr() = 0;
    };


    template<typename Element>
    class SSBO : public BasicSSBO
    {
    public:
        std::vector<Element> data;

        declare_instance_info_template(SSBO);

    public:
        delete_copy_constructors(SSBO);
        constructor_template(SSBO)
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
