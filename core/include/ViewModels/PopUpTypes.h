#pragma once

#include <functional>

#include <QString>
#include <QVector>

typedef struct popup_action_s {
    QString text;
    std::function<void()> callback;
} popup_action_t;

using PopUpActions = QVector<popup_action_t>;
