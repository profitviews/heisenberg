#pragma once

#include <boost/algorithm/string.hpp>
#include <boost/describe/enumerators.hpp>
#include <boost/mp11.hpp>
#include <optional>
#include <string_view>
#include <type_traits>

namespace profitview 
{

template<class T>
concept BoostDescribeEnum = std::is_enum_v<T> && requires { typename boost::describe::describe_enumerators<T>; };

template<BoostDescribeEnum Enum>
std::string_view toString(Enum const value, std::string_view result = {})
{
    boost::mp11::mp_for_each< boost::describe::describe_enumerators<Enum> >([&result, value](auto describe){
        if (value == describe.value)
        {
            result = describe.name;
        }
    });
    return result;
}

template<BoostDescribeEnum Enum>
auto fromString(std::string_view const value)
{
    std::optional<Enum> result = std::nullopt;
    boost::mp11::mp_for_each< boost::describe::describe_enumerators<Enum> >([&result, value](auto describe){
        if (boost::iequals(value, describe.name))
        {
            result = describe.value;
        }
    });
    return result;
}

}