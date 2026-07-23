#pragma once

#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>

namespace Kub3
{
    // ---------------------------------------------------------
    // Type Traits for Transparent Arrow Operator
    // ---------------------------------------------------------
    namespace detail
    {
        template <typename, typename = void>
        struct has_arrow : std::false_type {};

        template <typename U>
        struct has_arrow<U, std::void_t<decltype(std::declval<U &>().operator->())>> : std::true_type {};

        template <typename U>
        inline constexpr bool is_transparent_ptr_v = std::is_pointer_v<U> || has_arrow<U>::value;
    }

    // ---------------------------------------------------------
    // Tag Types (mimics Rust's Ok(val) and Err(err))
    // ---------------------------------------------------------

    template <typename T>
    struct Ok {
        T value;
        explicit constexpr Ok(const T &v) : value(v) {}
        explicit constexpr Ok(T &&v) : value(std::move(v)) {}
    };

    template <typename E>
    struct Err {
        E value;
        explicit constexpr Err(const E &v) : value(v) {}
        explicit constexpr Err(E &&v) : value(std::move(v)) {}
    };

    // Deduction guides (C++17) to be able to write `Ok(5)` instead of `Ok<int>(5)`
    template <typename T>
    Ok(T) -> Ok<T>;
    template <typename E>
    Err(E) -> Err<E>;

    // Exception Type for bad unwraps (mimics Rust panics)
    class BadResultAccess : public std::logic_error
    {
    public:
        BadResultAccess(const char *msg) : std::logic_error(msg) {}
    };

    // ---------------------------------------------------------
    // The Result Class
    // ---------------------------------------------------------

    template <typename T, typename E>
    class Result
    {
    public:
        // Implicit constructors
        constexpr Result(Ok<T> ok) : m_data(std::move(ok)) {}
        constexpr Result(Err<E> err) : m_data(std::move(err)) {}

        // --- Status Checks ---
        [[nodiscard]] constexpr bool is_ok() const noexcept
        {
            return std::holds_alternative<Ok<T>>(m_data);
        }

        [[nodiscard]] constexpr bool is_err() const noexcept
        {
            return std::holds_alternative<Err<E>>(m_data);
        }

        // --- C++ Ergonomics: Boolean Context & Dereferencing ---

        [[nodiscard]] constexpr explicit operator bool() const noexcept
        {
            return is_ok();
        }

        [[nodiscard]] constexpr T &operator*()
        {
            return unwrap();
        }
        [[nodiscard]] constexpr const T &operator*() const
        {
            return unwrap();
        }

        // TRANSPARENT ARROW: If T is a smart pointer or raw pointer, flatten it.
        template <typename U = T>
        [[nodiscard]] constexpr std::enable_if_t<detail::is_transparent_ptr_v<U>, U &> operator->()
        {
            return unwrap();
        }
        template <typename U = T>
        [[nodiscard]] constexpr std::enable_if_t<detail::is_transparent_ptr_v<U>, const U &> operator->() const
        {
            return unwrap();
        }

        // STANDARD ARROW: If T is an object (e.g. std::string), return address.
        template <typename U = T>
        [[nodiscard]] constexpr std::enable_if_t<!detail::is_transparent_ptr_v<U>, U *> operator->()
        {
            return &unwrap();
        }
        template <typename U = T>
        [[nodiscard]] constexpr std::enable_if_t<!detail::is_transparent_ptr_v<U>, const U *> operator->() const
        {
            return &unwrap();
        }

        // --- Rust Ergonomics: Unwrapping ---

        [[nodiscard]] T &unwrap()
        {
            if (is_ok())
                return std::get<Ok<T>>(m_data).value;
            throw BadResultAccess("Called unwrap() on an Err value");
        }

        [[nodiscard]] const T &unwrap() const
        {
            if (is_ok())
                return std::get<Ok<T>>(m_data).value;
            throw BadResultAccess("Called unwrap() on an Err value");
        }

        [[nodiscard]] const E &unwrap_err() const
        {
            if (is_err())
                return std::get<Err<E>>(m_data).value;
            throw BadResultAccess("Called unwrap_err() on an Ok value");
        }

        [[nodiscard]] T unwrap_or(T default_value) const
        {
            if (is_ok())
                return std::get<Ok<T>>(m_data).value;
            return default_value;
        }

        // --- Monadic Operations ---
        template <typename F>
        auto map(F &&func) const -> Result<std::invoke_result_t<F, T>, E>
        {
            using U = std::invoke_result_t<F, T>;
            if (is_ok())
                return Ok<U>(std::forward<F>(func)(std::get<Ok<T>>(m_data).value));
            return Err<E>(std::get<Err<E>>(m_data).value);
        }

        template <typename F>
        auto and_then(F &&func) const -> std::invoke_result_t<F, T>
        {
            if (is_ok())
                return std::forward<F>(func)(std::get<Ok<T>>(m_data).value);
            return Err<E>(std::get<Err<E>>(m_data).value);
        }

        template <typename F>
        auto or_else(F &&func) const -> std::invoke_result_t<F, E>
        {
            if (is_err())
                return std::forward<F>(func)(std::get<Err<E>>(m_data).value);
            return Ok<T>(std::get<Ok<T>>(m_data).value);
        }

    private:
        std::variant<Ok<T>, Err<E>> m_data;
    };

    // Helper for "Void" Results
    struct Unit {};

} // namespace Kub3::Utils