#ifndef KLOE_UTILS_H
#define KLOE_UTILS_H

#include <memory>
#include <optional>
#include <utility>

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

    // --------------------------------
    // --- Constexpr O(1) hash array
    // --------------------------------

    // A compile-time hash map for const char* / std::string_view
    template <size_t N, size_t Capacity = N * 2> // 50% load factor to minimize collisions
    struct ConstexprHashMap {
        struct Entry {
            std::string_view key{};
            std::string_view value{};
            bool is_valid = false;
        };

        std::array<Entry, Capacity> table{};

        // Compile-time FNV-1a hash function
        static constexpr uint32_t hash(std::string_view s)
        {
            uint32_t h = 2166136261u;
            for (char c : s)
            {
                h = (h ^ static_cast<uint32_t>(c)) * 16777619u;
            }
            return h;
        }

        // Constructor builds the hash table at compile time
        constexpr ConstexprHashMap(const std::pair<std::string_view, std::string_view> (&init)[N])
        {
            for (const auto &kv : init)
            {
                uint32_t h = hash(kv.first) % Capacity;

                while (table[h].is_valid)
                {
                    h = (h + 1) % Capacity; // Linear probing
                }
                table[h] = {kv.first, kv.second, true};
            }
        }

        // O(1) lookup
        constexpr std::optional<std::string_view> get(std::string_view key) const
        {
            uint32_t h     = hash(key) % Capacity;
            uint32_t start = h;

            while (table[h].is_valid)
            {
                if (table[h].key == key)
                    return table[h].value;
                h = (h + 1) % Capacity;
                if (h == start)
                    break; // Table full, not found
            }
            return std::nullopt;
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

template <typename K, typename V>
using Pair = std::pair<K, V>;

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
