export module Trinex.Core.Assert;

import Trinex.Core.BuildConfig;
import Trinex.Core.ETL;

export namespace Trinex::Assert
{
	enum class FailureKind
	{
		Assertion,
		Verification,
		Failure,
		Unreachable,
		NotImplemented,
	};

	using AssertHandler = void (*)(FailureKind kind, StringView message, SourceLocation location);

	void assert_handler(AssertHandler handler) noexcept;

	[[noreturn]] void report_failure(FailureKind kind, StringView message, SourceLocation location);

	inline void assertion(bool condition, StringView message, SourceLocation location = SourceLocation::current())
	{
		if constexpr (BuildConfig::assertions)
		{
			if (!condition) [[unlikely]]
				report_failure(FailureKind::Assertion, message, location);
		}
	}

	inline void verify(bool condition, StringView message, SourceLocation location = SourceLocation::current())
	{
		if (!condition) [[unlikely]]
			report_failure(FailureKind::Verification, message, location);
	}

	[[noreturn]] inline void failure(StringView message, SourceLocation location = SourceLocation::current())
	{
		if constexpr (BuildConfig::assertions)
			report_failure(FailureKind::Failure, message, location);
	}

	[[noreturn]] inline void unreachable(StringView message, SourceLocation location = SourceLocation::current())
	{
		report_failure(FailureKind::Unreachable, message, location);
	}

	[[noreturn]] inline void not_implemented(StringView message, SourceLocation location = SourceLocation::current())
	{
		report_failure(FailureKind::NotImplemented, message, location);
	}
} // namespace Trinex::Assert
