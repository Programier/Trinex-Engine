#pragma once
#include <Core/decode_typeid_name.hpp>
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>
#include <Core/string_convert.hpp>
#include <Core/string_format.hpp>
#include <stdexcept>
#include <Core/logger.hpp>

namespace Engine
{

    class IApiFunction
    {
    protected:
        String _M_name;
        String _M_prototype_name;
        String _M_base_name = L"";

    public:
        IApiFunction& base_name(const String& base_name);
        const String& base_name() const;
        const String& prototype_name() const;
        const String& name() const;

        virtual bool has_function() const = 0;
        virtual void operator=(void*) = 0;
    };


    template<typename ReturnType, typename... Args>
    class ApiFunction : public IApiFunction
    {
    private:
        ReturnType (*_M_function)(Args...) = nullptr;

    public:
        ApiFunction(const String& function_base_name)
        {
            _M_prototype_name = decode_name(typeid(_M_function));
            base_name(function_base_name);
            _M_function = nullptr;
        }

        ApiFunction(const ApiFunction&) = default;
        ApiFunction(ApiFunction&&) = default;
        ApiFunction& operator=(const ApiFunction&) = default;
        ApiFunction& operator=(ApiFunction&&) = default;

        void operator=(void* _M_func) override
        {
            _M_function = reinterpret_cast<ReturnType (*)(Args...)>(_M_func);
        }

        ReturnType operator()(Args... args)
        {
            if (_M_function)
                return _M_function(args...);

#ifdef THROW_ON_NULL_FUNC
            throw std::runtime_error(Strings::to_string(Strings::format(L"Null function: {}", name())));
#else
            logger->log("No function found: %ls\n", name().c_str());
            return ReturnType();
#endif
        }

        bool has_function() const override
        {
            return _M_function != nullptr;
        }
    };
}// namespace Engine
