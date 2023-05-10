#pragma once
#include <Core/constants.hpp>
#include <Core/engine_types.hpp>
#include <Core/export.hpp>
#include <iomanip>
#include <ostream>
#include <utility>
#include <Core/render_types.hpp>
#include <Core/texture_types.hpp>


// PRINTING GLM OBJECT
template<typename T, typename = void>
struct is_member_of_glm : std::false_type {
};

template<typename T>
struct is_member_of_glm<T, decltype(adl_member_of_glm(std::declval<T>()))> : std::true_type {
};


namespace glm
{
    template<typename T>
    auto adl_member_of_glm(T&&) -> void;
}

template<typename Number>
typename std::enable_if<std::is_arithmetic<Number>::value, int>::type digits_of_number(const Number& number)
{
    signed long int value = static_cast<signed long int>(number);
    int digits = value <= 0 ? 1 : 0;
    while (value != 0)
    {
        digits++;
        value /= 10;
    }
    return digits;
}

template<typename Type>
typename std::enable_if<is_member_of_glm<Type>::value, int>::type digits_of_number(const Type& value)
{
    int length = value.length();
    int digits = 0;
    for (int i = 0; i < length; i++) digits = std::max(digits, digits_of_number(value[i]));
    return digits;
}

// Printing glm value

template<typename Type>
typename std::enable_if<!is_member_of_glm<Type>::value, std::ostream&>::type
print_glm_object(std::ostream& stream, const Type& value, const std::size_t& glm_print_width = 0)
{
    return stream << std::fixed << std::setw(glm_print_width) << value << std::flush;
}

template<typename Type>
typename std::enable_if<is_member_of_glm<Type>::value && !std::is_pointer<Type>::value, std::ostream&>::type
print_glm_object(std::ostream& stream, const Type& value, std::size_t glm_print_width = 0)

{

    if (glm_print_width == 1)
        glm_print_width = 7 + digits_of_number(value);
    int length = value.length();
    bool contain_glm = is_member_of_glm<decltype(value[0])>::value;

    if (!contain_glm)
        stream << "{";
    for (int i = 0; i < length; i++)
    {
        print_glm_object(stream, value[i], glm_print_width);

        if (!contain_glm)
            stream << (i == length - 1 ? "}" : ", ") << std::flush;
        else
            stream << std::endl;
    }
    return stream;
}

namespace glm
{
    template<typename Type>
    typename std::enable_if<is_member_of_glm<Type>::value && !std::is_pointer<Type>::value, std::ostream&>::type
    operator<<(std::ostream& stream, const Type& value)
    {
        return print_glm_object(stream, value);
    }
}// namespace glm


namespace Engine
{
    class Application;
    class Window;

    namespace GraphicApiInterface
    {
        class ApiInterface;
    }

    class ENGINE_EXPORT EngineInstance final
    {
    private:
        bool _M_is_inited = false;
        static EngineInstance* _M_instance;
        class Renderer* _M_renderer = nullptr;
        Application* _M_application = nullptr;
        EngineAPI _M_api;
        GraphicApiInterface::ApiInterface* _M_api_interface = nullptr;

        EngineInstance& trigger_terminate_functions();

        EngineInstance();

        ENGINE_EXPORT static EngineInstance* create_instance();
        EngineInstance& init_api();

        ~EngineInstance();

    public:
        int start(int argc, char** argv);
        ENGINE_EXPORT static EngineInstance* instance();
        const Window* window() const;
        SystemType system_type() const;
        EngineAPI api() const;
        bool is_inited() const;
        GraphicApiInterface::ApiInterface* api_interface() const;
        class Renderer* renderer() const;

        bool check_format_support(PixelType type, PixelComponentType component);
        void destroy();
        friend class Application;
    };

}// namespace Engine
