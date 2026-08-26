#include <Core/assert.hpp>
#include <Core/console.hpp>
#include <Core/etl/algorithm.hpp>
#include <Core/etl/flat_set.hpp>
#include <Core/etl/variant.hpp>
#include <Core/string_functions.hpp>

#include <Core/math/vector.hpp>

namespace Trinex::Console
{
	class Manager
	{
	private:
		struct Compare {
			bool operator()(const Entry* first, const Entry* second) const { return first->name() < second->name(); }
		};

		FlatSet<Entry*, Compare> m_entries;

	public:
		Manager& bind(Entry* entry)
		{
			trinex_assert(find(entry->name()) == nullptr);
			const bool inserted = m_entries.insert(entry).second;
			trinex_assert(inserted);
			return *this;
		}

		Manager& unbind(Entry* entry)
		{
			trinex_assert(find(entry->name()) == entry);
			m_entries.erase(entry);
			return *this;
		}

		Entry* find(StringView name) const
		{
			auto it = etl::lower_bound(m_entries.begin(), m_entries.end(), name,
			                           [](const Entry* entry, StringView name) { return entry->name() < name; });

			if (it == m_entries.end())
				return nullptr;

			Entry* entry = *it;

			if (entry->name() != name)
				return nullptr;

			return entry;
		}

		usize find(StringView name, const FunctionRef<void(Entry*)>& action) const
		{
			auto it = etl::lower_bound(m_entries.begin(), m_entries.end(), name,
			                           [](const Entry* entry, StringView name) { return entry->name() < name; });

			usize count = 0;

			for (; it != m_entries.end(); ++it)
			{
				Entry* entry = *it;

				if (!entry->name().starts_with(name))
					return count;

				action(entry);
				++count;
			}

			return count;
		}

		static Manager* instance()
		{
			static Manager manager;
			return &manager;
		}
	};


	struct BareWord : public StringView {
		using StringView::StringView;
	};

	struct ArgumentArray : Vector<Argument> {
		using Vector<Argument>::Vector;
	};

	struct Argument : public Variant<bool, i64, u64, f64, StringView, BareWord, ArgumentArray> {
		using Super = Variant<bool, i64, u64, f64, StringView, BareWord, ArgumentArray>;

		using Super::Super;
	};

	template<typename T>
	concept ArgumentString = etl::same_as<T, StringView> || etl::same_as<T, BareWord>;

	template<typename Dst, typename Src>
	static inline ExecuteStatus (*s_type_convertor)(Dst*, const Src*) =
	        [](Dst*, const Src*) -> ExecuteStatus { return ExecuteStatus::ValueParseFailed; };

	template<typename T>
	static inline ExecuteStatus (*s_type_convertor<T, T>)(T*, const T*) = [](T* dst, const T* src) -> ExecuteStatus {
		*dst = *src;
		return ExecuteStatus::Success;
	};

	template<etl::arithmetic Dst, etl::arithmetic Src>
	static inline ExecuteStatus (*s_type_convertor<Dst, Src>)(Dst*, const Src*) = [](Dst* dst, const Src* src) -> ExecuteStatus {
		*dst = static_cast<Dst>(*src);
		return ExecuteStatus::Success;
	};

	template<etl::arithmetic Dst, ArgumentString Src>
	static inline ExecuteStatus (*s_type_convertor<Dst, Src>)(Dst*, const Src*) = [](Dst* dst, const Src* src) -> ExecuteStatus {
		if constexpr (etl::same_as<Dst, bool>)
		{
			if (Strings::boolean_of(*src, *dst))
				return ExecuteStatus::Success;

			return ExecuteStatus::ValueParseFailed;
		}
		else if constexpr (etl::unsigned_integral<Dst>)
		{
			if (Strings::unsigned_of(*src, *dst))
				return ExecuteStatus::Success;

			return ExecuteStatus::ValueParseFailed;
		}
		else if constexpr (etl::signed_integral<Dst>)
		{
			if (Strings::signed_of(*src, *dst))
				return ExecuteStatus::Success;

			return ExecuteStatus::ValueParseFailed;
		}
		else if constexpr (etl::floating_point<Dst>)
		{
			if (Strings::floating_of(*src, *dst))
				return ExecuteStatus::Success;

			return ExecuteStatus::ValueParseFailed;
		}

		return ExecuteStatus::ValueParseFailed;
	};

	template<ArgumentString Src>
	static inline ExecuteStatus (*s_type_convertor<String, Src>)(String*, const Src*) =
	        [](String* dst, const Src* src) -> ExecuteStatus {
		*dst = *src;
		return ExecuteStatus::Success;
	};

	template<etl::arithmetic Src>
	static inline ExecuteStatus (*s_type_convertor<String, Src>)(String*, const Src*) =
	        [](String* dst, const Src* src) -> ExecuteStatus {
		if constexpr (etl::same_as<Src, bool>)
		{
			(*dst) = (*src) ? "true" : "false";
			return ExecuteStatus::Success;
		}
		else
		{
			(*dst) = Strings::format("{}", *src);
			return ExecuteStatus::Success;
		}

		trinex_unreachable();
	};

	template<typename T>
	static ExecuteStatus store_internal(T* dst, const Argument* src)
	{
		auto visitor = [dst]<typename Src>(const Src& value) -> ExecuteStatus { return s_type_convertor<T, Src>(dst, &value); };
		return etl::visit(visitor, *src);
	}

	ExecuteStatus Entry::store(bool* dst, const Argument* src)
	{
		return store_internal(dst, src);
	}

	ExecuteStatus Entry::store(u64* dst, const Argument* src)
	{
		return store_internal(dst, src);
	}

	ExecuteStatus Entry::store(i64* dst, const Argument* src)
	{
		return store_internal(dst, src);
	}

	ExecuteStatus Entry::store(f64* dst, const Argument* src)
	{
		return store_internal(dst, src);
	}

	ExecuteStatus Entry::store(String* dst, const Argument* src)
	{
		return store_internal(dst, src);
	}

	ExecuteStatus Entry::store(ArrayInterface* dst, const Argument* src)
	{
		dst->clear();

		if (const ArgumentArray* array = etl::get_if<ArgumentArray>(src))
		{
			for (const Argument& argument : *array)
			{
				ExecuteStatus status = dst->append(&argument);

				if (status != ExecuteStatus::Success)
					return status;
			}
		}
		else
		{
			return dst->append(src);
		}

		return ExecuteStatus::Success;
	}

	ExecuteStatus Entry::store(String* dst, bool* src)
	{
		return ExecuteStatus::Success;
	}

	ExecuteStatus Entry::store(String* dst, u64* src)
	{
		return ExecuteStatus::Success;
	}

	ExecuteStatus Entry::store(String* dst, i64* src)
	{
		return ExecuteStatus::Success;
	}

	ExecuteStatus Entry::store(String* dst, f64* src)
	{
		return ExecuteStatus::Success;
	}

	ExecuteStatus Entry::store(String* dst, String* src)
	{
		return ExecuteStatus::Success;
	}

	ExecuteStatus Entry::store(String* dst, ArrayInterface* src)
	{
		return ExecuteStatus::Success;
	}

	EntryType VariableEntry::type() const
	{
		return EntryType::Variable;
	}

	Entry::Entry(StringView name, StringView description, EntryFlags flags)
	    : m_name(name), m_description(description), m_flags(flags)
	{
		Manager::instance()->bind(this);
	}

	Entry::~Entry()
	{
		Manager::instance()->unbind(this);
	}

	Entry* Entry::find(StringView name)
	{
		return Manager::instance()->find(name);
	}

	usize Entry::find(StringView name, const FunctionRef<void(Entry*)>& action)
	{
		return Manager::instance()->find(name, action);
	}

	trinex_on_pre_init()
	{
		Variable<Vector3f> test = {
		        "r.example.1",
		};
		Variable<int> test2 = {
		        "r.example.2",
		        10,
		};
		Variable<int> test3 = {
		        "r.example.3",
		        10,
		};
		Variable<int> test4 = {
		        "r.test.3",
		        10,
		};

		Entry::find("r.example", [](Entry* entry) { printf("%s\n", entry->name().data()); });
		exit(0);
	}
}// namespace Trinex::Console
