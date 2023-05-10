#pragma once

#include <stdexcept>
#include <memory>


namespace Engine
{
    template <typename Type>
    void fake_delete(Type *)
    {}

    template <typename Type>
    void delete_value(Type* value)
    {
        delete value;
    }

    template <typename Type>
    void delete_array(Type *array)
    {
        delete[] array;
    }

    template <typename Type>
    using SmartPointer = std::shared_ptr<Type>;

//    enum class SmartPointerType
//    {
//        VALUE,
//        ARRAY
//    };

//    template<typename BasicType, SmartPointerType DeleteType = SmartPointerType::VALUE>
//    class SmartPointer
//    {
//    private:
//        BasicType* _M_data = nullptr;
//        bool _M_delete = true;
//        static Map<BasicType*, std::size_t> _M_references;

//        SmartPointer& change_references(const bool& increment = true)
//        {
//            if (_M_data == nullptr)
//                return *this;

//            auto& refs = _M_references[_M_data];
//            refs += -1 + 2 * static_cast<std::size_t>(increment);
//            if (refs == 0 && _M_delete)
//            {
//                DeleteType == SmartPointerType::VALUE ? delete _M_data : delete[] _M_data;
//                _M_references.erase(_M_data);
//            }

//            if (!increment)
//                _M_data = nullptr;

//            return *this;
//        }

//    public:
//        SmartPointer() = default;
//        SmartPointer(BasicType* value, const bool& delete_memory = true)
//        {
//            _M_data = value;
//            _M_delete = delete_memory;
//            change_references();
//        }

//        SmartPointer(const SmartPointer& ptr)
//        {
//            _M_data = ptr._M_data;
//            _M_delete = ptr._M_delete;
//            change_references();
//        }

//        SmartPointer& operator=(BasicType* value)
//        {
//            change_references(false);
//            _M_data = value;
//            change_references(true);
//            return *this;
//        }

//        SmartPointer& operator=(const SmartPointer& ptr)
//        {
//            change_references(false);
//            _M_data = ptr._M_data;
//            _M_delete = ptr._M_delete;
//            change_references(true);
//            return *this;
//        }

//        SmartPointer& delete_value(const bool& value)
//        {
//            _M_delete = value;
//            return *this;
//        }

//        BasicType* get() const noexcept
//        {
//            return _M_data;
//        }

//        std::size_t count() const
//        {
//            if (_M_delete == false || _M_data == nullptr)
//                return 0;
//            return _M_references[_M_data];
//        }

//        SmartPointer& reset()
//        {
//            change_references(false);
//            _M_delete = true;
//        }

//        BasicType* operator->()
//        {
//            if (!_M_data)
//                throw std::runtime_error("nullptr exception");
//            return _M_data;
//        }

//        BasicType& operator*()
//        {
//            if (!_M_data)
//                throw std::runtime_error("nullptr exception");
//            return *_M_data;
//        }

//        operator BasicType*()
//        {
//            return _M_data;
//        }

//        operator BasicType&()
//        {
//            return *(*this);
//        }

//        bool is_null() const
//        {
//            return _M_data == nullptr;
//        }

//        ~SmartPointer()
//        {
//            change_references(false);
//        }
//    };

//    template<typename BasicType, SmartPointerType DeleteType>
//    Map<BasicType*, std::size_t> SmartPointer<BasicType, DeleteType>::_M_references;


//    template<typename Type>
//    SmartPointer<Type, SmartPointerType::VALUE> smart_ptr(Type* ptr, const bool& delete_value = true)
//    {
//        return SmartPointer<Type, SmartPointerType::VALUE>(ptr, delete_value);
//    }
}// namespace Engine
