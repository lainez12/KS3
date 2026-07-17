#pragma once

#include <algorithm>
#include <cctype>
#include <crypt.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <shadow.h>
#include <string>

namespace Kub3
{
    /**
     * @brief Translates an ISO locale string (e.g., "fr_FR", "en_US.UTF-8") to an XKB layout code.
     */
    std::string translateLocaleToXkb(const std::string &locale);

    /**
     * @brief Invokes setxkbmap to change the active X11 keyboard layout.
     */
    bool applyKeyboardLayout(const std::string &layout);

    /**
     * @brief Checks the input against the root user password
     */
    bool verifyRootPassword(const std::string &inputPassword);
}
