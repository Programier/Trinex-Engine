#include "bind_glm.hpp"
#include <Core/engine_loading_controllers.hpp>


namespace Engine
{

    struct QuaternionWrapper : public Quaternion {
        Quaternion& obj()
        {
            return *this;
        }

        const Quaternion& obj() const
        {
            return *this;
        }

        QuaternionWrapper()
        {}

        template<typename... Args>
        QuaternionWrapper(Args... args) : Quaternion(args...)
        {}

        QuaternionWrapper(const QuaternionWrapper& vec)
        {
            obj() = vec.obj();
        }

        QuaternionWrapper(Quaternion::value_type v)
        {
            obj().x = v;
            obj().y = v;
            obj().z = v;
            obj().w = v;
        }

        bool operator==(const QuaternionWrapper& wrap) const
        {
            return obj() == wrap.obj();
        }

        QuaternionWrapper& operator=(const QuaternionWrapper& new_obj)
        {
            obj() = new_obj.obj();
            return *this;
        }

        QuaternionWrapper& operator+=(const QuaternionWrapper& new_obj)
        {
            obj() += new_obj.obj();
            return *this;
        }

        QuaternionWrapper& operator-=(const QuaternionWrapper& new_obj)
        {
            obj() -= new_obj.obj();
            return *this;
        }

        QuaternionWrapper& operator*=(const QuaternionWrapper& new_obj)
        {
            obj() *= new_obj.obj();
            return *this;
        }

        QuaternionWrapper& operator/=(Quaternion::value_type v)
        {
            obj() /= v;
            return *this;
        }

        QuaternionWrapper& operator*=(Quaternion::value_type v)
        {
            obj() *= v;
            return *this;
        }

        QuaternionWrapper operator+(const QuaternionWrapper& new_obj) const
        {
            return obj() + new_obj.obj();
        }


        QuaternionWrapper operator-(const QuaternionWrapper& new_obj) const
        {
            return obj() - new_obj.obj();
        }

        QuaternionWrapper operator*(const QuaternionWrapper& new_obj) const
        {
            return obj() * new_obj.obj();
        }

        QuaternionWrapper operator*(Quaternion::value_type v) const
        {
            return obj() * v;
        }

        QuaternionWrapper operator/(Quaternion::value_type v) const
        {
            return obj() / v;
        }

        String as_string() const
        {
            std::stringstream s;
            s << obj();
            return s.str();
        }
    };

    static void bind_quat_operators(ScriptClassRegistrar& registrar)
    {
        using T = QuaternionWrapper;


        registrar.opfunc("Quaternion& opAssign(const Quaternion& in)", func_of<T&>(&T::operator=),
                         ScriptCallConv::THISCALL);

        registrar.opfunc("bool opEquals(const Quaternion& in) const", func_of<bool>(&T::operator==),
                         ScriptCallConv::THISCALL);

        registrar.opfunc("Quaternion& opAddAssign(const Quaternion& in)", func_of<T&, T, const T&>(&T::operator+=),
                         ScriptCallConv::THISCALL);

        registrar.opfunc("Quaternion& opSubAssign(const Quaternion& in)", func_of<T&, T, const T&>(&T::operator-=),
                         ScriptCallConv::THISCALL);

        registrar.opfunc("Quaternion& opMulAssign(const Quaternion& in)", func_of<T&, T, const T&>(&T::operator*=),
                         ScriptCallConv::THISCALL);
        registrar.opfunc("Quaternion& opMulAssign(float)", func_of<T&, T, typename T::value_type>(&T::operator*=),
                         ScriptCallConv::THISCALL);

        registrar.opfunc("Quaternion& opDivAssign(Quaternion)", func_of<T&, T, typename T::value_type>(&T::operator/=),
                         ScriptCallConv::THISCALL);

        registrar.opfunc("Quaternion opAdd(const Quaternion& in) const", func_of<T, T, const T&>(&T::operator+),
                         ScriptCallConv::THISCALL);

        registrar.opfunc("Quaternion opSub(const Quaternion& in) const", func_of<T, T, const T&>(&T::operator-),
                         ScriptCallConv::THISCALL);

        registrar.opfunc("Quaternion opMul(const Quaternion& in) const", func_of<T, T, const T&>(&T::operator*),
                         ScriptCallConv::THISCALL);
        registrar.opfunc("Quaternion opMul(float) const", func_of<T, T, typename T::value_type>(&T::operator*),
                         ScriptCallConv::THISCALL);
        registrar.opfunc("Quaternion  opMul_r(float) const", func_of<T, T, typename T::value_type>(&T::operator*),
                         ScriptCallConv::THISCALL);

        registrar.opfunc("Quaternion opDiv(float) const", func_of<T, T, typename T::value_type>(&T::operator/),
                         ScriptCallConv::THISCALL);
    }


    static void bind_quat_props(ScriptClassRegistrar& registrar)
    {
        using T = QuaternionWrapper;
        registrar.property("float w", &T::w);
        registrar.property("float x", &T::x);
        registrar.property("float y", &T::y);
        registrar.property("float z", &T::z);
    }


    static void on_init()
    {
        using RegistryType1 = QuaternionWrapper;

        {
            ScriptClassRegistrar registrar("Engine::Quaternion", info_of<RegistryType1>());
            bind_glm_behaviours<QuaternionWrapper>(registrar, "float");
            bind_quat_operators(registrar);
            bind_quat_props(registrar);
        }
    }

    static InitializeController controller(on_init, "Bind Engine::Quaternion");
}// namespace Engine
