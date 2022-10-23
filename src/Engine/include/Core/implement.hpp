#pragma once

#define implement_class_cpp(name)                                                                                           \
    name::name() = default;                                                                                                 \
    name::name(const name&) = default;                                                                                      \
    name::name(name&&) = default;                                                                                           \
    name& name::operator=(const name&) = default;                                                                           \
    name& name::operator=(name&&) = default;

#define implement_class_hpp(name)                                                                                           \
    name();                                                                                                                 \
    name(const name&);                                                                                                      \
    name(name&&);                                                                                                           \
    name& operator=(const name&);                                                                                           \
    name& operator=(name&&);
