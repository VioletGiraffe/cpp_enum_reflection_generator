#pragma once

#include <algorithm>
#include <array>
#include <utility>
#include <stdint.h>
#include <string_view>

// % 1 - enum type
// % 2 - number of items
// % 3 - items list: {"name", value}, ...

namespace EnumReflection {
namespace detail {
	template <class>
	class EnumReflection;

	template <>
	class EnumReflection<%1>
	{
		using EnumType = %1;
		using ValueType = int64_t;
	public:
		static constexpr std::string_view enum_name(EnumType v) noexcept {
			const auto it = std::find_if(_items.begin(), _items.end(), [&v](auto&& item) {
				return item.second == static_cast<ValueType>(v);
			});

			return it != _items.end() ? it->first : std::string_view{}; 
		}

	private:
		static constexpr std::array<std::pair<std::string_view, ValueType>, %2> _items{{
			%3
		}};
	};
} // detail

	template <typename E>
	[[nodiscard]] inline constexpr std::string_view enum_name(E v) noexcept {
		return detail::EnumReflection<E>::enum_name(v);
	}
}
