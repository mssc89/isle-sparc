#pragma once

#include "lego1_export.h"

#include <string>
#include <utility>

template <typename T>
class MinimalOptional {
	bool m_hasValue;
	T m_value;

public:
	MinimalOptional() : m_hasValue(false) {}

	MinimalOptional(const T& value)
		: m_hasValue(true), m_value(value) {}

	MinimalOptional(T&& value)
		: m_hasValue(true), m_value(std::move(value)) {}

	bool has_value() const { return m_hasValue; }

	T& value() { return m_value; }
	const T& value() const { return m_value; }

	operator bool() const { return m_hasValue; }

	T value_or(const T& default_value) const {
		return m_hasValue ? m_value : default_value;
	}
};

namespace Extensions
{
constexpr const char* availableExtensions[] = {"extensions:texture loader"};

LEGO1_EXPORT void Enable(const char* p_key);

template <typename T>
struct Extension {
	template <typename Function, typename... Args>
	static auto Call(Function&& function, Args&&... args)
		-> MinimalOptional<decltype(function(std::forward<Args>(args)...))>
	{
#ifdef EXTENSIONS
		if (T::enabled) {
			auto result = function(std::forward<Args>(args)...);
			return MinimalOptional<decltype(result)>(std::move(result));
		}
#endif
		typedef decltype(function(std::forward<Args>(args)...)) ReturnType;
		return MinimalOptional<ReturnType>();
	}
};
}; // namespace Extensions
