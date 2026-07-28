#pragma once

#include <QString>

namespace Kub3::Common
{
    enum class ProcessMessageLevel
    {
        Info,
        Success,
        Warning,
        Error
    };

    struct ProcessMessage {
        QString text;
        ProcessMessageLevel level = ProcessMessageLevel::Info;
    };
}