module;
#include <initializer_list>

export module Trinex.Core.ETL.InitializerList;

export namespace Trinex
{
	template<typename T>
	using InitializerList = std::initializer_list<T>;
}
