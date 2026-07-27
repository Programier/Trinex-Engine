#include <Core/etl/templates.hpp>
#include <Core/string_functions.hpp>
#include <UI/element.hpp>
#include <UI/reflection.hpp>
#include <imgui.h>

namespace Trinex::UI::Refl
{
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

	bool Type::binding_path_resolver(void* dst, const void* src, Property::Flags mask)
	{
		if (Element* element = Element::current())
		{
			element->bind(dst, this, *static_cast<const Markup::BindingPath*>(src));
			return true;
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

	bool Type::assign(const ValueRef& dst, const ConstValueRef& src, Property::Flags mask)
	{
		if (dst.type == src.type)
			return dst.type->assign(dst.address, src.address);

		auto it = dst.type->m_resolvers.find(src.type);

		if (it != dst.type->m_resolvers.end())
		{
			return it->second(dst.address, src.address, mask);
		}

		return false;
	}

	bool Type::assign(const ValueRef& dst, const ConstPropertyRef& src, Property::Flags mask)
	{
		if (src.field)
		{
			if (Property* prop = src.type->property(*src.field))
			{
				if ((prop->flags() & mask) == Property::Undefined)
				{
					return false;
				}

				return prop->load(src.address, [&](const void* address, Type* type) -> bool {
					if (src.fields > 1)
					{
						ConstPropertyRef child = {
						        .address = address,
						        .type    = type,
						        .field   = src.field + 1,
						        .fields  = src.fields - 1,
						};

						return Type::assign(dst, child, mask);
					}
					else
					{
						ConstValueRef child = {
						        .address = address,
						        .type    = type,
						};

						return Type::assign(dst, child, mask);
					}
				});
			}

			return false;
		}

		return assign(dst, ConstValueRef{.address = src.address, .type = src.type}, mask);
	}

	bool Type::assign(const PropertyRef& dst, const ConstValueRef& src, Property::Flags mask)
	{
		if (dst.field)
		{
			if (Property* prop = dst.type->property(*dst.field))
			{
				if ((prop->flags() & mask) == Property::Undefined)
				{
					return false;
				}

				return prop->store(dst.address, [&](void* address, Type* type) -> bool {
					if (dst.fields > 1)
					{
						PropertyRef child = {
						        .address = address,
						        .type    = type,
						        .field   = dst.field + 1,
						        .fields  = dst.fields - 1,
						};

						return Type::assign(child, src, mask);
					}
					else
					{
						ValueRef child = {
						        .address = address,
						        .type    = type,
						};

						return Type::assign(child, src, mask);
					}
				});
			}

			return false;
		}

		return assign(ValueRef{.address = dst.address, .type = dst.type}, src, mask);
	}

	bool Type::assign(const PropertyRef& dst, const ConstPropertyRef& src, Property::Flags mask)
	{
		if (dst.field)
		{
			if (Property* prop = dst.type->property(*dst.field))
			{
				if ((prop->flags() & mask) == Property::Undefined)
				{
					return false;
				}

				return prop->store(dst.address, [&](void* address, Type* type) -> bool {
					if (dst.fields > 1)
					{
						PropertyRef child = {
						        .address = address,
						        .type    = type,
						        .field   = dst.field + 1,
						        .fields  = dst.fields - 1,
						};

						return Type::assign(child, src, mask);
					}
					else
					{
						ValueRef child = {
						        .address = address,
						        .type    = type,
						};

						return Type::assign(child, src, mask);
					}
				});
			}

			return false;
		}

		return assign(ValueRef{.address = dst.address, .type = dst.type}, src, mask);
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

	bool Type::is_a(Refl::Type* type) const
	{
		const Type* self = this;

		do
		{
			if (self == type)
				return self;
			self = self->parent();
		} while (self);

		return false;
	}

	ElementRegistry* ElementRegistry::instance()
	{
		static ElementRegistry registry;
		return &registry;
	}

	template<typename Dst, typename Src>
	static bool static_cast_resolver(void* dst, const void* src, Property::Flags mask)
	{
		*static_cast<Dst*>(dst) = static_cast<Dst>(*static_cast<const Src*>(src));
		return true;
	}

	template<typename Dst, typename Src>
	static bool copy_resolver(void* dst, const void* src, Property::Flags mask)
	{
		*static_cast<Dst*>(dst) = *static_cast<const Src*>(src);
		return true;
	}

	static bool string_to_bool(void* dst, const void* src, Property::Flags mask)
	{
		const String& value = *static_cast<const String*>(src);
		return Strings::boolean_of(value, *static_cast<bool*>(dst));
	}

	static bool string_to_i32(void* dst, const void* src, Property::Flags mask)
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

	static bool string_to_f32(void* dst, const void* src, Property::Flags mask)
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
	static bool bool_to_string(void* dst, const void* src, Property::Flags mask)
	{
		*static_cast<StringType*>(dst) = *static_cast<const bool*>(src) ? "true" : "false";
		return true;
	}

	template<typename Src, typename StringType = String>
	static bool number_to_string(void* dst, const void* src, Property::Flags mask)
	{
		*static_cast<StringType*>(dst) = Strings::format("{}", *static_cast<const Src*>(src));
		return true;
	}

	template<typename StringType = String>
	static bool identifier_to_string(void* dst, const void* src, Property::Flags mask)
	{
		*static_cast<StringType*>(dst) = *static_cast<const Markup::Identifier*>(src);
		return true;
	}

	static bool parse_value(const ValueRef& dst, const Markup::ValueDesc& src, Property::Flags mask)
	{
		auto visitor = [&]<typename T>(const T& value) -> bool {
			Refl::ConstValueRef src = {
			        .address = &value,
			        .type    = NativeType<T>::instance(),
			};
			return Type::assign(dst, src, mask);
		};

		return etl::visit(visitor, src.value);
	}

	static bool value_to_f32(f32& dst, const Markup::ValueDesc& src, Property::Flags mask)
	{
		return parse_value({.address = &dst, .type = NativeType<f32>::instance()}, src, mask);
	}

	static bool value_to_unit(Unit& dst, const Markup::ValueDesc& src, Property::Flags mask)
	{
		return parse_value({.address = &dst, .type = NativeType<Unit>::instance()}, src, mask);
	}

	static bool f32_to_unit(void* dst, const void* src, Property::Flags mask)
	{
		*static_cast<Unit*>(dst) = Unit(*static_cast<const f32*>(src));
		return true;
	}

	static bool i32_to_unit(void* dst, const void* src, Property::Flags mask)
	{
		*static_cast<Unit*>(dst) = Unit(static_cast<f32>(*static_cast<const i32*>(src)));
		return true;
	}

	static bool name_to_unit_type(Unit::Type& dst, Name name)
	{
		if (name == "px")
			dst = Unit::Px;
		else if (name == "rem")
			dst = Unit::Rem;
		else if (name == "percent" || name == "%")
			dst = Unit::Percent;
		else if (name == "fill")
			dst = Unit::Fill;
		else
			return false;

		return true;
	}

	static bool name_to_unit(void* dst, const void* src, Property::Flags mask)
	{
		Unit::Type type;

		if (!name_to_unit_type(type, *static_cast<const Name*>(src)))
		{
			return false;
		}

		*static_cast<Unit*>(dst) = Unit(type, type == Unit::Fill ? 1.0f : 0.0f);
		return true;
	}

	static bool string_to_unit(void* dst, const void* src, Property::Flags mask)
	{
		Name name = *static_cast<const String*>(src);
		return name_to_unit(dst, &name, mask);
	}

	static bool identifier_to_unit(void* dst, const void* src, Property::Flags mask)
	{
		Name name = *static_cast<const Markup::Identifier*>(src);
		return name_to_unit(dst, &name, mask);
	}

	static bool name_to_unit_type(void* dst, const void* src, Property::Flags mask)
	{
		return name_to_unit_type(*static_cast<Unit::Type*>(dst), *static_cast<const Name*>(src));
	}

	static bool string_to_unit_type(void* dst, const void* src, Property::Flags mask)
	{
		return name_to_unit_type(*static_cast<Unit::Type*>(dst), *static_cast<const String*>(src));
	}

	static bool identifier_to_unit_type(void* dst, const void* src, Property::Flags mask)
	{
		return name_to_unit_type(*static_cast<Unit::Type*>(dst), *static_cast<const Markup::Identifier*>(src));
	}

	static bool i32_to_unit_type(void* dst, const void* src, Property::Flags mask)
	{
		const i32 value = *static_cast<const i32*>(src);

		if (value < Unit::Px || value > Unit::Fill)
		{
			return false;
		}

		*static_cast<Unit::Type*>(dst) = static_cast<Unit::Type>(value);
		return true;
	}

	template<typename VectorType>
	static bool container_to_vec2(void* dst, const void* src, Property::Flags mask)
	{
		const auto& values = *static_cast<const Markup::Container*>(src);

		if (values.size() != 2)
		{
			return false;
		}

		VectorType result;
		if (!value_to_f32(result.x, values[0], mask) || !value_to_f32(result.y, values[1], mask))
		{
			return false;
		}

		*static_cast<VectorType*>(dst) = result;
		return true;
	}

	template<typename VectorType>
	static bool container_to_vec3(void* dst, const void* src, Property::Flags mask)
	{
		const auto& values = *static_cast<const Markup::Container*>(src);

		if (values.size() != 3)
		{
			return false;
		}

		VectorType result;
		if (!value_to_f32(result.x, values[0], mask) || !value_to_f32(result.y, values[1], mask) ||
		    !value_to_f32(result.z, values[2], mask))
		{
			return false;
		}

		*static_cast<VectorType*>(dst) = result;
		return true;
	}

	template<typename VectorType>
	static bool container_to_vec4(void* dst, const void* src, Property::Flags mask)
	{
		const auto& values = *static_cast<const Markup::Container*>(src);

		if (values.size() != 4)
		{
			return false;
		}

		VectorType result;
		if (!value_to_f32(result.x, values[0], mask) || !value_to_f32(result.y, values[1], mask) ||
		    !value_to_f32(result.z, values[2], mask) || !value_to_f32(result.w, values[3], mask))
		{
			return false;
		}

		*static_cast<VectorType*>(dst) = result;
		return true;
	}

	static bool container_to_size(void* dst, const void* src, Property::Flags mask)
	{
		const auto& values = *static_cast<const Markup::Container*>(src);

		if (values.size() != 2)
		{
			return false;
		}

		Size result;
		if (!value_to_unit(result.width, values[0], mask) || !value_to_unit(result.height, values[1], mask))
		{
			return false;
		}

		*static_cast<Size*>(dst) = result;
		return true;
	}

	static Vec4 to_vec4(const Vec2& value)
	{
		return Vec4(value.x, value.y, 0.0f, 1.0f);
	}

	static Vec4 to_vec4(const Vec3& value)
	{
		return Vec4(value.x, value.y, value.z, 1.0f);
	}

	static Vec4 to_vec4(const Vec4& value)
	{
		return value;
	}

	static Vec4 to_vec4(const ImVec2& value)
	{
		return Vec4(value.x, value.y, 0.0f, 1.0f);
	}

	static Vec4 to_vec4(const ImVec4& value)
	{
		return Vec4(value.x, value.y, value.z, value.w);
	}

	static Vec4 to_vec4(const Size& value)
	{
		return Vec4(value.width.value, value.height.value, 0.0f, 1.0f);
	}

	template<typename Dst, typename Src>
	static bool vector_resolver(void* dst, const void* src, Property::Flags mask)
	{
		const Vec4 value = to_vec4(*static_cast<const Src*>(src));

		if constexpr (std::is_same_v<Dst, Vec2>)
		{
			*static_cast<Dst*>(dst) = Vec2(value.x, value.y);
		}
		else if constexpr (std::is_same_v<Dst, Vec3>)
		{
			*static_cast<Dst*>(dst) = Vec3(value.x, value.y, value.z);
		}
		else if constexpr (std::is_same_v<Dst, Vec4>)
		{
			*static_cast<Dst*>(dst) = value;
		}
		else if constexpr (std::is_same_v<Dst, ImVec2>)
		{
			*static_cast<Dst*>(dst) = ImVec2(value.x, value.y);
		}
		else if constexpr (std::is_same_v<Dst, ImVec4>)
		{
			*static_cast<Dst*>(dst) = ImVec4(value.x, value.y, value.z, value.w);
		}
		else if constexpr (std::is_same_v<Dst, Size>)
		{
			*static_cast<Dst*>(dst) = Size(value.x, value.y);
		}

		return true;
	}

	template<typename Dst, typename Src>
	static void bind_vector_resolver()
	{
		if constexpr (!std::is_same_v<Dst, Src>)
		{
			NativeType<Dst>::instance()->template bind<Src>(vector_resolver<Dst, Src>);
		}
	}

	template<typename Dst, typename Types>
	struct VectorResolverBinder;

	template<typename Dst, typename... Src>
	struct VectorResolverBinder<Dst, TypesList<Src...>> {
		static void bind() { (bind_vector_resolver<Dst, Src>(), ...); }
	};

	template<typename DstType>
	static bool object_to_type(void* dst, const void* src, Property::Flags mask)
	{
		const auto& object = *static_cast<const Markup::Object*>(src);

		for (const Markup::ObjectField& field : object)
		{
			PropertyRef dst_ref = {
			        .address = dst,
			        .type    = NativeType<DstType>::instance(),
			        .field   = &field.name,
			        .fields  = 1,
			};

			auto visitor = [&]<typename Value>(const Value& value) -> bool {
				ConstValueRef src_ref = {
				        .address = &value,
				        .type    = NativeType<Value>::instance(),
				};

				return Type::assign(dst_ref, src_ref, mask);
			};

			if (!etl::visit(visitor, field.value.value))
			{
				return false;
			}
		}

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
		NativeType<Unit>::instance()->bind<Name>(name_to_unit);
		NativeType<Unit>::instance()->bind<String>(string_to_unit);
		NativeType<Unit>::instance()->bind<Markup::Identifier>(identifier_to_unit);
		NativeType<Unit::Type>::instance()->bind<Name>(name_to_unit_type);
		NativeType<Unit::Type>::instance()->bind<String>(string_to_unit_type);
		NativeType<Unit::Type>::instance()->bind<Markup::Identifier>(identifier_to_unit_type);
		NativeType<Unit::Type>::instance()->bind<i32>(i32_to_unit_type);
		NativeType<Unit>::instance()->bind<Markup::Object>(object_to_type<Unit>);
		NativeType<Vec2>::instance()->bind<Markup::Container>(container_to_vec2<Vec2>);
		NativeType<Vec3>::instance()->bind<Markup::Container>(container_to_vec3<Vec3>);
		NativeType<Vec4>::instance()->bind<Markup::Container>(container_to_vec4<Vec4>);
		NativeType<ImVec2>::instance()->bind<Markup::Container>(container_to_vec2<ImVec2>);
		NativeType<ImVec4>::instance()->bind<Markup::Container>(container_to_vec4<ImVec4>);
		NativeType<Size>::instance()->bind<Markup::Container>(container_to_size);
		NativeType<Vec2>::instance()->bind<Markup::Object>(object_to_type<Vec2>);
		NativeType<Vec3>::instance()->bind<Markup::Object>(object_to_type<Vec3>);
		NativeType<Vec4>::instance()->bind<Markup::Object>(object_to_type<Vec4>);
		NativeType<ImVec2>::instance()->bind<Markup::Object>(object_to_type<ImVec2>);
		NativeType<ImVec4>::instance()->bind<Markup::Object>(object_to_type<ImVec4>);
		NativeType<Size>::instance()->bind<Markup::Object>(object_to_type<Size>);

		using VectorTypesList = TypesList<Vec2, Vec3, Vec4, ImVec2, ImVec4, Size>;
		VectorTypesList::for_each([]<typename Dst>() { VectorResolverBinder<Dst, VectorTypesList>::bind(); });

		NativeType<Unit>::instance()->bind("type", &Unit::type);
		NativeType<Unit>::instance()->bind("unit", &Unit::type);
		NativeType<Unit>::instance()->bind("value", &Unit::value);

		NativeType<Vec2>::instance()->bind("x", &Vec2::x);
		NativeType<Vec2>::instance()->bind("r", &Vec2::x);
		NativeType<Vec2>::instance()->bind("y", &Vec2::y);
		NativeType<Vec2>::instance()->bind("g", &Vec2::y);

		NativeType<Vec3>::instance()->bind("x", &Vec3::x);
		NativeType<Vec3>::instance()->bind("r", &Vec3::x);
		NativeType<Vec3>::instance()->bind("y", &Vec3::y);
		NativeType<Vec3>::instance()->bind("g", &Vec3::y);
		NativeType<Vec3>::instance()->bind("z", &Vec3::z);
		NativeType<Vec3>::instance()->bind("b", &Vec3::z);

		NativeType<Vec4>::instance()->bind("x", &Vec4::x);
		NativeType<Vec4>::instance()->bind("r", &Vec4::x);
		NativeType<Vec4>::instance()->bind("y", &Vec4::y);
		NativeType<Vec4>::instance()->bind("g", &Vec4::y);
		NativeType<Vec4>::instance()->bind("z", &Vec4::z);
		NativeType<Vec4>::instance()->bind("b", &Vec4::z);
		NativeType<Vec4>::instance()->bind("w", &Vec4::w);
		NativeType<Vec4>::instance()->bind("a", &Vec4::w);

		NativeType<ImVec2>::instance()->bind("x", &ImVec2::x);
		NativeType<ImVec2>::instance()->bind("r", &ImVec2::x);
		NativeType<ImVec2>::instance()->bind("y", &ImVec2::y);
		NativeType<ImVec2>::instance()->bind("g", &ImVec2::y);

		NativeType<ImVec4>::instance()->bind("x", &ImVec4::x);
		NativeType<ImVec4>::instance()->bind("r", &ImVec4::x);
		NativeType<ImVec4>::instance()->bind("y", &ImVec4::y);
		NativeType<ImVec4>::instance()->bind("g", &ImVec4::y);
		NativeType<ImVec4>::instance()->bind("z", &ImVec4::z);
		NativeType<ImVec4>::instance()->bind("b", &ImVec4::z);
		NativeType<ImVec4>::instance()->bind("w", &ImVec4::w);
		NativeType<ImVec4>::instance()->bind("a", &ImVec4::w);

		NativeType<Size>::instance()->bind("x", &Size::width);
		NativeType<Size>::instance()->bind("width", &Size::width);
		NativeType<Size>::instance()->bind("y", &Size::height);
		NativeType<Size>::instance()->bind("height", &Size::height);


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
		trinex_ui_bind_type_name(Unit::Type);
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
		trinex_ui_bind_type_name(Markup::Object);
	}
}// namespace Trinex::UI::Refl
