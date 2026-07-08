#ifndef KLOE_CONFIG_UTILS_H_
#define KLOE_CONFIG_UTILS_H_

#include <format>
#include <string>
#include <vector>

// Macro for standard primitive types (int, double, bool)
#define MERGE_FIELD(target, source, empty_value, field_name, path_prefix, logs)                       \
    if ((target).field_name == (empty_value) && (source).field_name != (empty_value))                 \
    {                                                                                                 \
        (target).field_name = (source).field_name;                                                    \
        (logs).push_back(std::format("{}/{} = {}", (path_prefix), #field_name, (target).field_name)); \
    }

// Macro for std::string
#define MERGE_STRING_FIELD(target, source, field_name, path_prefix, logs)                             \
    if ((target).field_name.empty() && !(source).field_name.empty())                                  \
    {                                                                                                 \
        (target).field_name = (source).field_name;                                                    \
        (logs).push_back(std::format("{}/{} = {}", (path_prefix), #field_name, (target).field_name)); \
    }

// Macro for QString
#define MERGE_QSTRING_FIELD(target, source, field_name, path_prefix, logs)                                          \
    if ((target).field_name.isEmpty() && !(source).field_name.isEmpty())                                            \
    {                                                                                                               \
        (target).field_name = (source).field_name;                                                                  \
        (logs).push_back(std::format("{}/{} = {}", (path_prefix), #field_name, (target).field_name.toStdString())); \
    }

// Macro for Optional<QString>
#define MERGE_OPTIONAL_QSTRING_FIELD(target, source, field_name, path_prefix, logs)                                         \
    if (!(target).field_name.has_value() && (source).field_name.has_value())                                                \
    {                                                                                                                       \
        (target).field_name = (source).field_name;                                                                          \
        (logs).push_back(std::format("{}/{} = {}", (path_prefix), #field_name, (target).field_name.value().toStdString())); \
    }

#endif // KLOE_CONFIG_UTILS_H_
