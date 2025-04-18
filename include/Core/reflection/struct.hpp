#pragma once
#include <Core/etl/set.hpp>
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
		struct StructCompare {
			inline bool operator()(const Struct* a, const Struct* b) const
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
		Set<Struct*> m_derived_structs;
		mutable Struct* m_parent = nullptr;

		class Group* m_group = nullptr;

	protected:
		void destroy_derived_structs();

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
		virtual size_t size() const;
		virtual bool serialize(void* object, Archive& ar);

		Struct* parent() const;
		size_t abstraction_level() const;
		Vector<Name> hierarchy(size_t offset = 0) const;
		const Set<Struct*>& derived_structs() const;
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

		static void register_layout(ScriptClassRegistrar& r, ClassInfo* info, DownCast downcast);

		~Struct();
	};

	template<typename T, typename Base = Struct>
	class NativeStruct : public Base
	{
	protected:
		template<typename F>
		using initializer_detector = decltype(F::static_initialize_struct());

		inline static Struct* super_of()
		    requires(has_super_type_v<T> && !std::is_same_v<typename T::Super, void> && !std::is_base_of_v<Object, T>)
		{
			return T::Super::static_struct_instance();
		}

		inline static Struct* super_of() { return nullptr; }

		NativeStruct& register_scriptable_instance() override { return *this; }

	public:
		NativeStruct(Struct* parent, BitMask flags = 0) : Base(parent, flags | Struct::native_type_flags<T>()) {}

		static Struct* create(StringView decl, BitMask flags = 0)
		{
			return Object::new_instance<NativeStruct<T, Base>>(decl, super_of(), flags);
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
			Base::initialize();

			if constexpr (is_detected_v<initializer_detector, T>)
			{
				T::static_initialize_struct();
			}

			return *this;
		}

		size_t size() const override { return sizeof(T); }

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
	};

#define trinex_implement_struct(decl, flags)                                                                                     \
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
	        Engine::Refl::Object::static_register_initializer([]() { decl::static_struct_instance(); }, #decl));                 \
                                                                                                                                 \
	void decl::static_initialize_struct()

#define trinex_implement_struct_default_init(decl, flags)                                                                        \
	trinex_implement_struct(decl, flags) {}
}// namespace Engine::Refl
