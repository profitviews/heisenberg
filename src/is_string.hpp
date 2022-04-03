#pragma once

#include <string>
#include <string_view>

namespace profitview {

template <typename T> constexpr bool is_string = false;
template <> constexpr bool is_string<std::string> = true;
template <> constexpr bool is_string<std::string_view> = true;

template <class T> concept IsString = is_string<T>;

} // namespace profitview