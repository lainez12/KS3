#ifndef KLOE_UTILS_H
#define KLOE_UTILS_H

#include <memory>
#include <optional>

namespace Kub3::Utils
{
    // Multiplies the input by 2
    int mul2(int value);

    struct StringViewHash {
        using is_transparent = void;

        [[nodiscard]] size_t operator()(std::string_view sv) const
        {
            return std::hash<std::string_view>{}(sv);
        }
    };

    template <typename A, typename B>
    auto zip(A &a, B &b)
    {
        struct It {
            decltype(a.begin()) it1;
            decltype(b.begin()) it2;
            bool operator!=(It const &o) const
            {
                return it1 != o.it1;
            }
            void operator++()
            {
                ++it1;
                ++it2;
            }
            auto operator*() const
            {
                return std::pair{*it1, *it2};
            }
        };
        struct Z {
            A &a;
            B &b;
            auto begin()
            {
                return It{a.begin(), b.begin()};
            }
            auto end()
            {
                return It{a.end(), b.end()};
            }
        };
        return Z{a, b};
    }
}

template <typename T>
using Unique = std::unique_ptr<T>;

template <typename T>
using Shared = std::shared_ptr<T>;

template <typename T>
using Weak = std::weak_ptr<T>;

template <typename T>
using Optional = std::optional<T>;

// ---------------------------------------
// --- C++20 OVERLOADED VISITOR HELPER
// ---------------------------------------
template <class... Ts>
struct overloadedCallable : Ts... {
    using Ts::operator()...;
};
// Deduction guide for compiler (Not strictly required in C++20 as type inference is smart, but good practice)
template <class... Ts>
overloadedCallable(Ts...) -> overloadedCallable<Ts...>;

#endif // KLOE_UTILS_H
