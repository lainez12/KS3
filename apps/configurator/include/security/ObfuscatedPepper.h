#pragma once

#include <array>
#include <cstddef>
#include <string>

namespace Kub3::Security
{
    // Compile-time string obfuscator
    template <size_t N>
    class ObfuscatedPepper
    {
    public:
        // Constructor runs strictly at compile-time
        constexpr explicit ObfuscatedPepper(const char (&str)[N])
        {
            for (size_t i = 0; i < N; ++i)
            {
                m_cipher[i] = str[i] ^ XOR_KEY;
            }
        }

        // Decrypts dynamically at runtime into RAM
        [[nodiscard]] std::string decrypt() const
        {
            std::string plain;

            plain.resize(N - 1); // Exclude null terminator
            for (size_t i = 0; i < N - 1; ++i)
            {
                plain[i] = m_cipher[i] ^ XOR_KEY;
            }
            return plain;
        }

    private:
        std::array<char, N> m_cipher{};
        static constexpr char XOR_KEY = 0xA4; // Random, constant, single-byte key
    };
}
