#pragma once
#include <Core/executable_object.hpp>

namespace Engine
{
    template<typename Variable>
    class UpdateVariableCommand : public ExecutableObject
    {
        Variable src_variable;
        Variable& dst_variable;

    public:
        UpdateVariableCommand(const Variable& src, Variable& dst) : src_variable(src), dst_variable(dst)
        {}

        int_t execute()
        {
            dst_variable = src_variable;
            return sizeof(UpdateVariableCommand<Variable>);
        }
    };
}// namespace Engine
