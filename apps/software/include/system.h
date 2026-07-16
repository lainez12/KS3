#pragma once

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <string>

/**
 * @brief Translates an ISO locale string (e.g., "fr_FR", "en_US.UTF-8") to an XKB layout code.
 */
std::string translateLocaleToXkb(const std::string &locale);

/**
 * @brief Invokes setxkbmap to change the active X11 keyboard layout.
 */
bool applyKeyboardLayout(const std::string &layout);
