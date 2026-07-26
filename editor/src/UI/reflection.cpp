#include <Core/etl/templates.hpp>
#include <Core/string_functions.hpp>
#include <UI/element.hpp>
#include <UI/reflection.hpp>
#include <imgui.h>

namespace Trinex::UI::Refl
{
	bool Property::assign(void* object, const void* src, const Type* src_type, Flags mask, const AssignHistory* history)
	{
		if ((flags() & mask) == Flags::Undefined)
			return true;

		if (object == nullptr || src == nullptr || src_type == nullptr)
		{
			return false;
		}

		AssignHistory frame;
		frame.prev     = history;
		frame.dst      = object;
		frame.dst_type = owner();
		frame.src      = src;
		frame.src_type = src_type;

		return type()->assign(resolve(object), src, src_type, mask, &frame);
	}

	Type::Type(Type* parent) : m_parent(parent)
	{
		if (parent)
		{
			parent->m_childs.push_back(this);
		}
	}

	Type::~Type()
	{
		for (auto& [name, prop] : m_properties)
		{
			trx_delete prop;
		}

		m_properties.clear();
	}

	bool Type::binding_path_resolver(void* dst, const void* src, Property::Flags mask, const AssignHistory* history)
	{
		while (history)
		{
			if (auto element = Element::cast(history->dst, history->dst_type))
			{
				element->bind(dst, this, *static_cast<const Markup::BindingPath*>(src));
				return true;
			}

			history = history->prev;
		}

		return false;
	}

	Type& Type::bind(Name name, Property* prop)
	{
		struct DownCastChecker {
			static void check(Type* type, Name name)
			{
				for (Type* child : type->m_childs)
				{
					trinex_assert(!child->m_properties.contains(name));
					check(child, name);
				}
			}
		};

		trinex_assert(property(name) == nullptr);
		DownCastChecker::check(this, name);

		m_properties.insert({name, prop});
		return *this;
	}

	Type& Type::bind(Type* type, Resolver resolver)
	{
		trinex_assert(type && !m_resolvers.contains(type));
		m_resolvers[type] = resolver;
		return *this;
	}

	Pair<void*, const Type*> Type::resolve(void* address, const Name* names, usize count) const
	{
		if (address == nullptr)
			return {nullptr, nullptr};

		const Type* self = this;

		for (usize i = 0; i < count; ++i)
		{
			Property* prop = self->property(names[i]);

			if (prop == nullptr)
				return {nullptr, nullptr};

			address = prop->resolve(address);
			self    = prop->type();
		}

		return {address, self};
	}

	bool Type::assign(void* object, Name name, const void* src, const Type* type, Property::Flags mask,
	                  const AssignHistory* history) const
	{
		if (Property* prop = property(name))
		{
			return prop->assign(object, src, type, mask, history);
		}

		return false;
	}

	bool Type::assign(void* dst, const void* src, const Type* type, Property::Flags mask, const AssignHistory* history) const
	{
		if (this == type)
			return assign(dst, src);

		auto it = m_resolvers.find(type);

		if (it != m_resolvers.end())
		{
			return it->second(dst, src, mask, history);
		}

		return false;
	}

	Property* Type::property(Name name) const
	{
		const Type* self = this;

		while (self)
		{
			auto it = self->m_properties.find(name);

			if (it != self->m_properties.end())
			{
				return it->second;
			}

			self = self->m_parent;
		}

		return nullptr;
	}

	ElementRegistry* ElementRegistry::instance()
	{
		static ElementRegistry registry;
		return &registry;
	}

	template<typename Dst, typename Src>
	static bool static_cast_resolver(void* dst, const void* src, Property::Flags mask, const AssignHistory* history)
	{
		*static_cast<Dst*>(dst) = static_cast<Dst>(*static_cast<const Src*>(src));
		return true;
	}

	template<typename Dst, typename Src>
	static bool copy_resolver(void* dst, const void* src, Property::Flags mask, const AssignHistory* history)
	{
		*static_cast<Dst*>(dst) = *static_cast<const Src*>(src);
		return true;
	}

	static bool string_to_bool(void* dst, const void* src, Property::Flags mask, const AssignHistory* history)
	{
		const String& value = *static_cast<const String*>(src);
		return Strings::boolean_of(value, *static_cast<bool*>(dst));
	}

	static bool string_to_i32(void* dst, const void* src, Property::Flags mask, const AssignHistory* history)
	{
		const String& value = *static_cast<const String*>(src);
		i64 result;

		if (Strings::signed_of(value, result))
		{
			*static_cast<i32*>(dst) = static_cast<i32>(result);
			return true;
		}

		return false;
	}

	static bool string_to_f32(void* dst, const void* src, Property::Flags mask, const AssignHistory* history)
	{
		const String& value = *static_cast<const String*>(src);
		f64 result;

		if (Strings::floating_of(value, result))
		{
			*static_cast<f32*>(dst) = static_cast<f32>(result);
			return true;
		}

		return false;
	}

	template<typename StringType = String>
	static bool bool_to_string(void* dst, const void* src, Property::Flags mask, const AssignHistory* history)
	{
		*static_cast<StringType*>(dst) = *static_cast<const bool*>(src) ? "true" : "false";
		return true;
	}

	template<typename Src, typename StringType = String>
	static bool number_to_string(void* dst, const void* src, Property::Flags mask, const AssignHistory* history)
	{
		*static_cast<StringType*>(dst) = Strings::format("{}", *static_cast<const Src*>(src));
		return true;
	}

	template<typename StringType = String>
	static bool identifier_to_string(void* dst, const void* src, Property::Flags mask, const AssignHistory* history)
	{
		*static_cast<StringType*>(dst) = *static_cast<const Markup::Identifier*>(src);
		return true;
	}

	static bool value_to_f32(f32& dst, Property::Flags mask, const Markup::ValueDesc& src)
	{
		auto visitor = [&]<typename T>(const T& value) -> bool {
			return NativeType<f32>::instance()->assign(&dst, &value, NativeType<T>::instance(), mask);
		};

		return etl::visit(visitor, src.value);
	}

	static bool f32_to_unit(void* dst, const void* src, Property::Flags mask, const AssignHistory* history)
	{
		*static_cast<Unit*>(dst) = Unit(*static_cast<const f32*>(src));
		return true;
	}

	static bool i32_to_unit(void* dst, const void* src, Property::Flags mask, const AssignHistory* history)
	{
		*static_cast<Unit*>(dst) = Unit(static_cast<f32>(*static_cast<const i32*>(src)));
		return true;
	}

	template<typename VectorType>
	static bool container_to_vec2(void* dst, const void* src, Property::Flags mask, const AssignHistory* history)
	{
		const auto& values = *static_cast<const Markup::Container*>(src);

		if (values.size() != 2)
		{
			return false;
		}

		VectorType result;
		if (!value_to_f32(result.x, mask, values[0]) || !value_to_f32(result.y, mask, values[1]))
		{
			return false;
		}

		*static_cast<VectorType*>(dst) = result;
		return true;
	}

	template<typename VectorType>
	static bool container_to_vec3(void* dst, const void* src, Property::Flags mask, const AssignHistory* history)
	{
		const auto& values = *static_cast<const Markup::Container*>(src);

		if (values.size() != 3)
		{
			return false;
		}

		VectorType result;
		if (!value_to_f32(result.x, mask, values[0]) || !value_to_f32(result.y, mask, values[1]) ||
		    !value_to_f32(result.z, mask, values[2]))
		{
			return false;
		}

		*static_cast<VectorType*>(dst) = result;
		return true;
	}

	template<typename VectorType>
	static bool container_to_vec4(void* dst, const void* src, Property::Flags mask, const AssignHistory* history)
	{
		const auto& values = *static_cast<const Markup::Container*>(src);

		if (values.size() != 4)
		{
			return false;
		}

		VectorType result;
		if (!value_to_f32(result.x, mask, values[0]) || !value_to_f32(result.y, mask, values[1]) ||
		    !value_to_f32(result.z, mask, values[2]) || !value_to_f32(result.w, mask, values[3]))
		{
			return false;
		}

		*static_cast<VectorType*>(dst) = result;
		return true;
	}

	static bool container_to_size(void* dst, const void* src, Property::Flags mask, const AssignHistory* history)
	{
		Vec2 result;

		if (!container_to_vec2<Vec2>(&result, src, mask, history))
		{
			return false;
		}

		*static_cast<Size*>(dst) = Size(result);
		return true;
	}

	trinex_on_pre_init()
	{
		using ScalarTypesList = TypesList<bool, i8, u8, i16, u16, i32, u32, i64, u64, f16, f32, f64>;

		ScalarTypesList::for_each([]<typename LHS>() {
			ScalarTypesList::for_each([]<typename RHS>() {
				auto instance = NativeType<LHS>::instance();
				instance->template bind<RHS>(static_cast_resolver<LHS, RHS>);
			});
		});

		NativeType<bool>::instance()->bind<String>(string_to_bool);
		NativeType<i32>::instance()->bind<String>(string_to_i32);
		NativeType<f32>::instance()->bind<String>(string_to_f32);

		NativeType<String>::instance()->bind<bool>(bool_to_string);
		NativeType<String>::instance()->bind<i32>(number_to_string<i32>);
		NativeType<String>::instance()->bind<f32>(number_to_string<f32>);
		NativeType<String>::instance()->bind<Markup::Identifier>(identifier_to_string);

		NativeType<Name>::instance()->bind<String>(copy_resolver<Name, String>);
		NativeType<Name>::instance()->bind<bool>(bool_to_string<Name>);
		NativeType<Name>::instance()->bind<i32>(number_to_string<i32, Name>);
		NativeType<Name>::instance()->bind<f32>(number_to_string<f32, Name>);
		NativeType<Name>::instance()->bind<Markup::Identifier>(identifier_to_string<Name>);

		NativeType<Unit>::instance()->bind<f32>(f32_to_unit);
		NativeType<Unit>::instance()->bind<i32>(i32_to_unit);
		NativeType<Vec2>::instance()->bind<Markup::Container>(container_to_vec2<Vec2>);
		NativeType<Vec3>::instance()->bind<Markup::Container>(container_to_vec3<Vec3>);
		NativeType<Vec4>::instance()->bind<Markup::Container>(container_to_vec4<Vec4>);
		NativeType<ImVec2>::instance()->bind<Markup::Container>(container_to_vec2<ImVec2>);
		NativeType<ImVec4>::instance()->bind<Markup::Container>(container_to_vec4<ImVec4>);
		NativeType<Size>::instance()->bind<Markup::Container>(container_to_size);


		trinex_ui_bind_type_name(bool);
		trinex_ui_bind_type_name(i8);
		trinex_ui_bind_type_name(i16);
		trinex_ui_bind_type_name(i32);
		trinex_ui_bind_type_name(i64);
		trinex_ui_bind_type_name(u8);
		trinex_ui_bind_type_name(u16);
		trinex_ui_bind_type_name(u32);
		trinex_ui_bind_type_name(u64);
		trinex_ui_bind_type_name(f16);
		trinex_ui_bind_type_name(f32);
		trinex_ui_bind_type_name(f64);
		trinex_ui_bind_type_name(String);
		trinex_ui_bind_type_name(Name);
		trinex_ui_bind_type_name(Unit);
		trinex_ui_bind_type_name(Vec2);
		trinex_ui_bind_type_name(Vec3);
		trinex_ui_bind_type_name(Vec4);
		trinex_ui_bind_type_name(ImVec2);
		trinex_ui_bind_type_name(ImVec4);
		trinex_ui_bind_type_name(Size);
		trinex_ui_bind_type_name(Markup::Identifier);
		trinex_ui_bind_type_name(Markup::BindingPath);
		trinex_ui_bind_type_name(Markup::LocalizationKey);
		trinex_ui_bind_type_name(Markup::Null);
	}
}// namespace Trinex::UI::Refl
