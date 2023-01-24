#pragma once

#include <Core/string_format.hpp>
#include <stdexcept>

namespace Engine
{

#define check(conditional)                                                                                                  \
    if (!conditional)                                                                                                       \
        throw std::runtime_error(                                                                                           \
                Strings::format("Error at: {}:{}: Conditional {} must be true", __FILE__, __LINE__, #conditional));

#define check_with_message(conditional, message)                                                                            \
    if (!conditional)                                                                                                       \
        throw std::runtime_error(                                                                                           \
                Strings::format("Error at: {}:{}: Checking {{}} failed: {}", __FILE__, __LINE__, #conditional, message));
}// namespace Engine
