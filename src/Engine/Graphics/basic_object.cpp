#include <BasicFunctional/basic_functional.hpp>
#include <Graphics/basic_object.hpp>
#include <algorithm>
#include <glm/ext.hpp>

namespace Engine
{
    template<typename Object, typename Function, typename... Args>
    inline Object& sync(Object* _this, Function function, const Args&... args)
    {
        std::unordered_map<Object*, bool> moved_map;
        std::list<Object*> move_list = {_this};

        while (!move_list.empty())
        {
            Object* first = move_list.front();
            move_list.pop_front();
            auto& moved = moved_map[first];
            if (!moved)
            {
                moved = true;
                function(first, args...);
                move_list.insert(move_list.end(), first->_M_sync_objects.begin(), first->_M_sync_objects.end());
            }
        }
        return *_this;
    }

    template<typename Object>
    inline Object& sync_object_with(Object* base, Object& object, const double& double_sync)
    {
        if (&object == base)
            return *base;

        auto end = object._M_sync_objects.end();
        auto ell = std::find(object._M_sync_objects.begin(), end, base);
        if (ell != end)
            return *base;

        object._M_sync_objects.push_back(base);

        if (!double_sync)
            return *base;

        end = base->_M_sync_objects.end();
        ell = std::find(base->_M_sync_objects.begin(), end, &object);
        if (ell != end)
            return *base;
        base->_M_sync_objects.push_back(&object);
        return *base;
    }

    const glm::mat4& ModelMatrixObject::model() const
    {
        return _M_model;
    }

    void ModelMatrixObject::link_to(const ModelMatrixObject& object)
    {
        _M_model = object._M_model;
    }


    //      TRANSLATE

    void TranslateObject::private_move(TranslateObject* obj, const glm::vec3 move_vector, const glm::vec3& right, const glm::vec3& up,
                                       const glm::vec3& front, const bool& add_values)
    {
        glm::vec3 result_move = (right * move_vector.x) + (up * move_vector.y) + (front * move_vector.z);

        if (add_values)
            obj->_M_position += result_move;
        else
        {
            result_move = (-obj->_M_position) + move_vector;
            obj->_M_position = move_vector;
        }

        obj->_M_model = glm::translate(obj->_M_model, result_move);
    }

    const glm::vec3& TranslateObject::position() const
    {
        return _M_position;
    }

    TranslateObject& TranslateObject::move(const float& x, const float& y, const float& z, const bool& add_values)
    {
        return move({x, y, z}, OX, OY, OZ, add_values);
    }

    TranslateObject& TranslateObject::move(const glm::vec3& move_vector, const bool& add_values)
    {
        return move(move_vector, OX, OY, OZ, add_values);
    }

    TranslateObject& TranslateObject::move(const float& x, const float& y, const float& z, const glm::vec3& right, const glm::vec3& up,
                                           const glm::vec3& front, const bool& add_values)
    {
        return move({x, y, z}, right, up, front, add_values);
    }

    TranslateObject& TranslateObject::move(const glm::vec3 move_vector, const glm::vec3& right, const glm::vec3& up,
                                           const glm::vec3& front, const bool& add_values)
    {
        return sync(this, private_move, move_vector, right, up, front, add_values);
    }

    TranslateObject& TranslateObject::move(const float& distance, const glm::vec3& axis, const bool& add_value)
    {
        return move(axis * distance, OX, OY, OZ, add_value);
    }

    TranslateObject& TranslateObject::link_to(TranslateObject& object)
    {
        return move(object._M_position, false).sync_with(object, true);
    }

    TranslateObject& TranslateObject::sync_with(TranslateObject& object, const bool& double_sync)
    {
        return sync_object_with(this, object, double_sync);
    }

    TranslateObject::~TranslateObject()
    {
        for (auto address : _M_sync_objects)
        {
            auto end = address->_M_sync_objects.end();
            auto ell = std::find(address->_M_sync_objects.begin(), end, this);
            if (ell != end)
                address->_M_sync_objects.erase(ell);
        }
    }

    //          SCALE
    const glm::vec3& ScaleObject::scale() const
    {
        return _M_scale;
    }

    ScaleObject& ScaleObject::scale(const glm::vec3& sc, const bool& add_values)
    {
        glm::vec3 new_sc;
        if (add_values)
        {
            new_sc = sc;
            _M_scale *= sc;
        }
        else
        {
            new_sc = (1.f / _M_scale) * sc;
            _M_scale = sc;
        }
        _M_model = glm::scale(_M_model, new_sc);
        return *this;
    }

    ScaleObject& ScaleObject::scale(const float& x, const float& y, const float& z, const bool& add_values)
    {
        return scale({x, y, z}, add_values);
    }

    ScaleObject& ScaleObject::link_to(ScaleObject& object)
    {
        return scale(object.scale(), false).sync_with(object, true);
    }

    ScaleObject& ScaleObject::sync_with(ScaleObject& object, const bool& double_sync)
    {
        return sync_object_with(this, object, double_sync);
    }

    //      ROTATION
    const glm::vec3& RotateObject::euler_angles() const
    {
        return _M_euler_angles;
    }

    RotateObject& RotateObject::rotate(const float& x, const float& y, const float& z, const bool& add_values)
    {
        return rotate({x, y, z}, add_values);
    }

    void RotateObject::update_model(const glm::quat& q, const bool& add_values)
    {
        if (add_values)
            _M_quaternion *= q;
        else
        {
            _M_quaternion.w = -_M_quaternion.w;
            _M_model *= glm::mat4_cast(_M_quaternion);
            _M_quaternion = q;
        }

        _M_model *= glm::mat4_cast(q);
        _M_euler_angles = glm::eulerAngles(_M_quaternion);

        _M_front = glm::normalize(-OZ * _M_quaternion);
        _M_up = glm::normalize(OY * _M_quaternion);
        _M_right = glm::normalize(OX * _M_quaternion);
    }

    RotateObject& RotateObject::rotate(const glm::vec3& r, const bool& add_values)
    {
        glm::quat q(r);
        update_model(q, add_values);
        return *this;
    }

    RotateObject& RotateObject::rotate(const float& angle, const glm::vec3& axis, const bool& add_values)
    {
        glm::quat q = glm::rotate(glm::quat(glm::vec3(0.f, 0.f, 0.f)), angle, axis);
        update_model(q, add_values);
        return *this;
    }

    RotateObject& RotateObject::rotate(const glm::quat& q, const bool& add_values)
    {
        update_model(q, add_values);
        return *this;
    }

    const glm::quat& RotateObject::quaternion() const
    {
        return _M_quaternion;
    }

    const glm::vec3& RotateObject::front_vector() const
    {
        return _M_front;
    }

    const glm::vec3& RotateObject::right_vector() const
    {
        return _M_right;
    }

    const glm::vec3& RotateObject::up_vector() const
    {
        return _M_up;
    }

    RotateObject& RotateObject::link_to(RotateObject& object)
    {
        return rotate(object.quaternion(), false).sync_with(object, true);
    }

    RotateObject& RotateObject::sync_with(RotateObject& object, const bool& double_sync)
    {
        return sync_object_with(this, object, double_sync);
    }

}// namespace Engine
