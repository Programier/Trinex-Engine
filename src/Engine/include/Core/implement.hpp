#pragma once


#define declare_instance_info_hpp(name)                                                                                \
public:                                                                                                                \
    static constexpr ObjectType instance_type = ObjectType::name;                                                      \
                                                                                                                       \
private:                                                                                                               \
    name(__FOR_PRIVATE_USAGE__);

#define declare_instance_info_template(name, ...)                                                                      \
public:                                                                                                                \
    static constexpr ObjectType instance_type = ObjectType::name;                                                      \
                                                                                                                       \
private:                                                                                                               \
    name(__FOR_PRIVATE_USAGE__) __VA_OPT__( :) __VA_ARGS__                                                             \
    {                                                                                                                  \
        std::size_t index = static_cast<std::size_t>(name::instance_type);                                             \
        _M_instance_info[index]._M_has_instance = true;                                                                \
        _M_instance_info[index]._M_offset =                                                                            \
                reinterpret_cast<byte*>(dynamic_cast<Object*>(this)) - reinterpret_cast<byte*>(this);                  \
    }


#define declare_instance_info_cpp(name, ...)                                                                           \
    name::name(__FOR_PRIVATE_USAGE__) __VA_OPT__( :) __VA_ARGS__                                                       \
    {                                                                                                                  \
        std::size_t index = static_cast<std::size_t>(name::instance_type);                                             \
        _M_instance_info[index]._M_has_instance = true;                                                                \
        _M_instance_info[index]._M_offset =                                                                            \
                reinterpret_cast<byte*>(dynamic_cast<Object*>(this)) - reinterpret_cast<byte*>(this);                  \
    }


#define constructor_template(name, ...) name(__VA_ARGS__) : name(__FOR_PRIVATE_USAGE__())
#define constructor_hpp(name, ...) name(__VA_ARGS__)
#define constructor_cpp(name, ...) name::constructor_template(name, __VA_ARGS__)

#define delete_copy_constructors(class_name)                                                                           \
    class_name(const class_name&) = delete;                                                                            \
    class_name(class_name&&) = delete;                                                                                 \
    class_name& operator=(class_name&&) = delete;                                                                      \
    class_name& operator=(const class_name&) = delete;

#define copy_constructors_hpp(class_name)                                                                              \
    class_name(const class_name&);                                                                                     \
    class_name(class_name&&);                                                                                          \
    class_name& operator=(class_name&&);                                                                               \
    class_name& operator=(const class_name&);


#define default_copy_constructors_cpp(class_name)                                                                      \
    class_name::class_name(const class_name&) = default;                                                               \
    class_name::class_name(class_name&&) = default;                                                                    \
    class_name& class_name::operator=(class_name&&) = default;                                                         \
    class_name& class_name::operator=(const class_name&) = default;


#define property_hpp(class_name, type, name, _default)                                                                 \
private:                                                                                                               \
    type _M_##name = _default;                                                                                         \
                                                                                                                       \
public:                                                                                                                \
    const type& name() const;                                                                                          \
    class_name& name(const type& value)

#define property_cpp(_class, type, name)                                                                               \
    const type& _class::name() const                                                                                   \
    {                                                                                                                  \
        return _M_##name;                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    _class& _class::name(const type& value)                                                                            \
    {                                                                                                                  \
        this->_M_##name = value;                                                                                       \
        return *this;                                                                                                  \
    }
