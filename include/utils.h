#ifndef KLOE_UTILS_H
#define KLOE_UTILS_H

#include <memory>
#include <optional>

namespace Kub3::Utils
{
    // Multiplies the input by 2
    int mul2(int value);

    struct StringViewHash
    {
        using is_transparent = void;

        [[nodiscard]] size_t operator()(std::string_view sv) const
        {
            return std::hash<std::string_view>{}(sv);
        }
    };
}

template <typename T>
using Unique = std::unique_ptr<T>;

template <typename T>
using Shared = std::shared_ptr<T>;

template <typename T>
using Weak = std::weak_ptr<T>;

template <typename T>
using Optional = std::optional<T>;

#endif // KLOE_UTILS_H
