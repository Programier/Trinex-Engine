#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
	class ENGINE_EXPORT Name
	{
	public:
		static const Name undefined;
		static const Name color;
		static const Name offset;
		static const Name ambient_color;
		static const Name radius;
		static const Name fall_off_exponent;
		static const Name height;
		static const Name spot_angles;
		static const Name intensivity;
		static const Name location;
		static const Name direction;
		static const Name inverse_rotation;
		static const Name scale;
		static const Name out_of_range;
		static const Name model;
		static const Name texture;

		static const Name clear_render_targets;
		static const Name base_pass;
		static const Name depth_pass;
		static const Name deferred_light_pass;
		static const Name light_pass;
		static const Name scene_output_pass;
		static const Name post_process;

		static const Name color_scene_rendering;
		static const Name depth_scene_rendering;


	public:
		struct Entry {
			String name;
			HashIndex hash;
		};

		struct HashFunction {
			FORCE_INLINE HashIndex operator()(const Name& name) const
			{
				return name.m_index;
			}
		};

		struct Less {
			FORCE_INLINE bool operator()(const Name& x, const Name& y) const
			{
				return std::less<String>()(x.to_string(), y.to_string());
			}
		};

		static ENGINE_EXPORT Name none;

	private:
		Index m_index;
		Name& init(const StringView& view);

	public:
		Name();
		Name(const Name&);
		Name(Name&&);
		Name& operator=(const Name&);
		Name& operator=(Name&&);

		Name(const char* name);
		Name(const char* name, size_t len);
		Name(const String& name);
		Name(const StringView& name);

		Name& operator=(const char* name);
		Name& operator=(const String& name);
		Name& operator=(const StringView& name);

		static Name find_name(const StringView& name);

		bool is_valid() const;
		HashIndex hash() const;
		bool operator==(const StringView& name) const;
		bool operator!=(const StringView& name) const;
		bool operator==(const char* name) const;
		bool operator!=(const char* name) const;
		bool operator==(const String& name) const;
		bool operator!=(const String& name) const;


		bool equals(const String& name) const;
		bool equals(const char* name) const;
		bool equals(const char* name, size_t len) const;
		bool equals(const StringView& name) const;
		bool equals(const Name& name) const;

		const String& to_string() const;
		const char* c_str() const;
		const Name& to_string(String& out) const;
		operator const String&() const;
		operator StringView() const;

		static const Vector<Name::Entry>& entries();

		FORCE_INLINE bool operator==(const Name& name) const
		{
			return name.m_index == m_index;
		}

		FORCE_INLINE bool operator!=(const Name& name) const
		{
			return name.m_index != m_index;
		}

		FORCE_INLINE bool operator<(const Name& name) const
		{
			return m_index < name.m_index;
		}

		FORCE_INLINE bool operator<=(const Name& name) const
		{
			return m_index <= name.m_index;
		}

		FORCE_INLINE bool operator>(const Name& name) const
		{
			return m_index > name.m_index;
		}

		FORCE_INLINE bool operator>=(const Name& name) const
		{
			return m_index >= name.m_index;
		}
	};

	ENGINE_EXPORT bool operator&(class Archive&, Name& name);
}// namespace Engine
