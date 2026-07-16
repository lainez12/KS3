#include <system.h>

std::string translateLocaleToXkb(const std::string &locale)
{
    if (locale.empty())
    {
        return "us"; // Fallback safe default
    }

    std::string layout;
    size_t underscorePos = locale.find('_');

    if (underscorePos != std::string::npos && underscorePos + 1 < locale.length())
    {
        // Extract the 2-letter country code (e.g., "FR" from "fr_FR.UTF-8")
        layout = locale.substr(underscorePos + 1, 2);
    }
    else
    {
        // Fallback: use the first 2 letters of the language code (e.g., "fr" from "fr")
        layout = locale.substr(0, 2);
    }

    // Convert to lowercase (XKB layout codes are strictly lowercase)
    std::transform(layout.begin(), layout.end(), layout.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    // Strict Sanitization: Ensure the layout consists ONLY of 2 to 3 alphanumeric characters.
    // This absolutely eliminates any risk of command injection when executed via std::system.
    if (layout.length() < 2 || layout.length() > 3)
    {
        return "us";
    }
    for (char c : layout)
    {
        if (!std::isalnum(static_cast<unsigned char>(c)))
        {
            return "us";
        }
    }

    return layout;
}

bool applyKeyboardLayout(const std::string &layout)
{
    // Double-check sanitization to prevent shell injections
    for (char c : layout)
    {
        if (!std::isalnum(static_cast<unsigned char>(c)))
        {
            return false;
        }
    }

    // Construct standard command: "setxkbmap <layout>"
    std::string command = "setxkbmap " + layout;

    // std::system is clean and safe here due to the strict alphanumeric validation above
    int result = std::system(command.c_str());
    return (result == 0);
}
