#pragma once

#define constructor_template(name, ...) name(__VA_ARGS__)
#define constructor_hpp(name, ...) name(__VA_ARGS__)
#define constructor_cpp(name, ...) name::constructor_template(name, __VA_ARGS__)

#define delete_copy_constructors(class_name)                                                                           \
    class_name(const class_name&)            = delete;                                                                 \
    class_name(class_name&&)                 = delete;                                                                 \
    class_name& operator=(class_name&&)      = delete;                                                                 \
    class_name& operator=(const class_name&) = delete

#define copy_constructors_hpp(class_name)                                                                              \
    class_name(const class_name&);                                                                                     \
    class_name(class_name&&);                                                                                          \
    class_name& operator=(class_name&&);                                                                               \
    class_name& operator=(const class_name&)


#define default_copy_constructors_cpp(class_name)                                                                      \
    class_name::class_name(const class_name&)            = default;                                                    \
    class_name::class_name(class_name&&)                 = default;                                                    \
    class_name& class_name::operator=(class_name&&)      = default;                                                    \
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
