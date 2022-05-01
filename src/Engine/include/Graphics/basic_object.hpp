#pragma once
#include <BasicFunctional/basic_functional.hpp>
#include <BasicFunctional/reference_wrapper.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <list>


namespace Engine
{
    class ModelMatrix
    {
    protected:
        ReferenceWrapper<glm::mat4> _M_models[3] = {ReferenceWrapper<glm::mat4>(identity_matrix),
                                                    ReferenceWrapper<glm::mat4>(identity_matrix),
                                                    ReferenceWrapper<glm::mat4>(identity_matrix)};

        ReferenceWrapper<glm::mat4> _M_shift_models[4] = {
                ReferenceWrapper<glm::mat4>(identity_matrix), ReferenceWrapper<glm::mat4>(identity_matrix),
                ReferenceWrapper<glm::mat4>(identity_matrix), ReferenceWrapper<glm::mat4>(identity_matrix)};

        glm::mat4& _M_rotation_matrix();
        glm::mat4& _M_scale_matrix();
        glm::mat4& _M_translate_matrix();

        ModelMatrix& recalculate_shift();


    public:
        const glm::mat4& rotation_matrix() const;
        const glm::mat4& scale_matrix() const;
        const glm::mat4& translate_matrix() const;
        glm::mat4 model() const;
    };


    class Translate : public virtual ModelMatrix
    {

    protected:
        ReferenceWrapper<glm::vec3> _M_position = glm::vec3(0.f);
        glm::vec3 _M_shift_position = zero_vector;

    public:
        Translate& move(const float& x, const float& y, const float& z, const bool& add_values = true);
        Translate& move(const glm::vec3& move_vector, const bool& add_values = true);
        Translate& move(const float& x, const float& y, const float& z, const glm::vec3& right, const glm::vec3& up,
                        const glm::vec3& front, const bool& add_values = true);
        Translate& move(const glm::vec3 move_vector, const glm::vec3& right, const glm::vec3& up, const glm::vec3& front,
                        const bool& add_values = true);
        Translate& move(const float& distance, const glm::vec3& axis, const bool& add_value = true);
        glm::vec3 position() const;

        auto& shift()
        {
            return _M_shift_position;
        }


        Translate& link_to(Translate& obj);
        ~Translate();
    };


    class Scale : public virtual ModelMatrix
    {
    protected:
        ReferenceWrapper<glm::vec3> _M_scale = glm::vec3(1.f);

    public:
        const glm::vec3& scale() const;
        Scale& scale(const glm::vec3& sc, const bool& add_values = true);
        Scale& scale(const float& x, const float& y, const float& z, const bool& add_values = true);
        Scale& link_to(Scale& obj);
    };


    class Rotate : public virtual ModelMatrix
    {
    private:
        void update_model(const glm::quat& q, const bool& add_values);

    protected:
        ReferenceWrapper<glm::quat> _M_quaternion = glm::quat(glm::vec3(0.f, 0.f, 0.f));
        ReferenceWrapper<glm::vec3> _M_euler_angles = glm::vec3(0.f, 0.f, 0.f);
        ReferenceWrapper<glm::vec3> _M_front = OZ;
        ReferenceWrapper<glm::vec3> _M_right = OX;
        ReferenceWrapper<glm::vec3> _M_up = OY;

    public:
        const glm::vec3& euler_angles() const;
        Rotate& rotate(const float& x, const float& y, const float& z, const bool& add_values = true);
        Rotate& rotate(const glm::vec3& r, const bool& add_values = true);
        Rotate& rotate(const float& angle, const glm::vec3& axis, const bool& add_values = true);
        Rotate& rotate(const glm::quat& q, const bool& add_values = true);
        const glm::quat& quaternion() const;
        const glm::vec3& front_vector() const;
        const glm::vec3& right_vector() const;
        const glm::vec3& up_vector() const;

        Rotate& link_to(Rotate& obj);
    };


    template<typename FirstType = void, typename... Args>
    constexpr bool is_basic__base_classes(const bool& void_status = false)
    {
        if (std::is_void<FirstType>::value)
            return void_status;
        constexpr bool status = std::is_same<FirstType, ModelMatrix>::value || std::is_same<FirstType, Rotate>::value ||
                                std::is_same<FirstType, Translate>::value || std::is_same<FirstType, Scale>::value;
        return status && is_basic__base_classes<Args...>(true);
    }

    template<typename... BaseClasses>
    class BasicObject : public BaseClasses...
    {
    };

}// namespace Engine
