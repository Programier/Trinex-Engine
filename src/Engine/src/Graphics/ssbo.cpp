#include <Graphics/ssbo.hpp>
#include <api_funcs.hpp>

namespace Engine
{
    declare_instance_info_cpp(BasicSSBO);

    constructor_cpp(BasicSSBO)
    {}

    BasicSSBO& BasicSSBO::gen()
    {
        destroy();
        create_ssbo(_M_ID);
        return *this;
    }

    BasicSSBO& BasicSSBO::set_data()
    {
        set_ssbo_data(_M_ID, data_ptr(), size() * value_size(), usage);
        return *this;
    }

    BasicSSBO& BasicSSBO::update_data(std::size_t elem_offset, std::size_t count)
    {
        if (elem_offset + count > size())
            return *this;
        const auto _M_value_size = value_size();
        update_ssbo_data(_M_ID, data_ptr() + _M_value_size * elem_offset, count * _M_value_size,
                         elem_offset * _M_value_size);
        return *this;
    }

    BasicSSBO& BasicSSBO::bind(unsigned int index)
    {
        bind_ssbo(_M_ID);
        return *this;
    }

    const byte* BasicSSBO::data_ptr() const
    {
        return const_cast<BasicSSBO*>(this)->data_ptr();
    }
}// namespace Engine
