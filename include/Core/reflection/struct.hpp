#pragma once
#include <Core/etl/type_info.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/flags.hpp>
#include <Core/reflection/scoped_type.hpp>
#include <ScriptEngine/script_type_info.hpp>

namespace Engine
{
	class Group;
	class Archive;
}// namespace Engine

namespace Engine::Refl
{
	class ENGINE_EXPORT Struct : public ScopedType
	{
		declare_reflect_type(Struct, ScopedType);

	public:
		struct ENGINE_EXPORT StructCompare {
			bool operator()(const Struct* a, const Struct* b) const
			{
				return a->name().to_string() < b->name().to_string();
			}
		};

		enum Flag : BitMask
		{
			IsSingletone    = BIT(0),
			IsAbstract      = BIT(1),
			IsConstructible = BIT(2),
			IsFinal         = BIT(3),
			IsNative        = BIT(4),
			IsScriptable    = BIT(5),
			IsAsset         = BIT(6),
		};

		const Flags<Flag> flags;
		ScriptTypeInfo script_type_info;

	private:
		Vector<Property*> m_properties;
		Set<Struct*> m_childs;
		mutable Struct* m_parent = nullptr;

		class Group* m_group = nullptr;

	protected:
		void destroy_childs();

		template<typename T>
		static consteval BitMask native_type_flags()
		{
			BitMask mask = Flag::IsNative;

			if constexpr (std::is_final_v<T>)
			{
				mask |= Flag::IsFinal;
			}

			if constexpr (std::is_abstract_v<T>)
			{
				mask |= Flag::IsAbstract;
			}

			if constexpr (is_singletone_v<T>)
			{
				mask |= Flag::IsSingletone;
			}

			if constexpr (!(std::is_abstract_v<T> || (!std::is_constructible_v<T> && !Engine::is_singletone_v<T>) ))
			{
				mask |= Flag::IsConstructible;
			}

			return mask;
		}

		Struct& construct() override;
		virtual Struct& register_scriptable_instance();
		Struct& initialize() override;
		Struct& unregister_subobject(Object* subobject) override;
		Struct& register_subobject(Object* subobject) override;

	public:
		Struct(Struct* parent = nullptr, BitMask flags = 0);

		virtual void* create_struct();
		virtual Struct& destroy_struct(void* obj);
		virtual StringView type_name() const;
		virtual size_t size() const;
		virtual bool serialize(void* object, Archive& ar);

		Struct* parent() const;
		size_t abstraction_level() const;
		Vector<Name> hierarchy(size_t offset = 0) const;
		const Set<Struct*>& childs() const;
		bool is_asset() const;
		bool is_native() const;
		bool is_class() const;
		bool is_scriptable() const;

		using Super::is_a;
		bool is_a(const Struct* other) const;

		const Vector<Property*>& properties() const;
		Property* find_property(StringView name);
		bool serialize_properties(void* object, Archive& ar);

		Struct& group(class Group*);
		class Group* group() const;

		~Struct();
	};

	template<typename T>
	class NativeStruct : public Struct
	{
		template<typename F>
		using initializer_detector = decltype(F::static_initialize_struct());

		inline static Struct* super_of()
			requires(has_super_type_v<T> && !std::is_same_v<typename T::Super, void> && !std::is_base_of_v<Object, T>)
		{
			return T::Super::static_struct_instance();
		}

		inline static Struct* super_of()
		{
			return nullptr;
		}

		NativeStruct& register_scriptable_instance() override
		{
			return *this;
		}

	public:
		NativeStruct(BitMask flags = 0) : Struct(super_of(), flags | native_type_flags<T>())
		{
			bind_type_name(type_info<T>::name());
		}

		static Struct* create(StringView decl, BitMask flags = 0)
		{
			return Object::new_instance<NativeStruct<T>>(decl, flags);
		}

		void* create_struct() override
		{
			if constexpr (Concepts::struct_with_custom_allocation<T>)
			{
				return T::static_constructor();
			}
			else
			{
				if constexpr (std::is_default_constructible_v<T>)
				{
					return new T();
				}
				else
				{
					return nullptr;
				}
			}
		}

		NativeStruct& destroy_struct(void* mem) override
		{
			if constexpr (Concepts::struct_with_custom_allocation<T>)
			{
				T::static_destructor(reinterpret_cast<T*>(mem));
			}
			else
			{
				if constexpr (std::is_default_constructible_v<T>)
				{
					delete reinterpret_cast<T*>(mem);
				}
			}

			return *this;
		}

		NativeStruct& initialize() override
		{
			Struct::initialize();

			if constexpr (is_detected_v<T, initializer_detector>)
			{
				T::static_initialize_struct();
			}

			return *this;
		}

		StringView type_name() const override
		{
			return Engine::type_info<T>::name();
		}

		size_t size() const override
		{
			return sizeof(T);
		}

		bool serialize(void* object, Archive& ar) override
		{
			if constexpr (Concepts::is_serializable<T>)
			{
				return reinterpret_cast<T*>(object)->serialize(ar);
			}
			else
			{
				return Struct::serialize(object, ar);
			}
		}

		~NativeStruct()
		{
			unbind_type_name(type_info<T>::name());
		}
	};

#define implement_struct(decl, flags)                                                                                            \
    class Engine::Refl::Struct* decl::m_static_struct = nullptr;                                                                 \
                                                                                                                                 \
    class Engine::Refl::Struct* decl::static_struct_instance()                                                                   \
    {                                                                                                                            \
        if (!m_static_struct)                                                                                                    \
        {                                                                                                                        \
            m_static_struct = Engine::Refl::NativeStruct<decl>::create(#decl, flags);                                            \
        }                                                                                                                        \
        return m_static_struct;                                                                                                  \
    }                                                                                                                            \
                                                                                                                                 \
    static Engine::byte TRINEX_CONCAT(trinex_engine_refl_struct_, __LINE__) = static_cast<Engine::byte>(                         \
            Engine::ReflectionInitializeController([]() { decl::static_struct_instance(); }, #decl).id());                       \
                                                                                                                                 \
    void decl::static_initialize_struct()

#define implement_struct_default_init(decl, flags)                                                                               \
	implement_struct(decl, flags)                                                                                                \
	{}
}// namespace Engine::Refl
