#include <Views/Components/KeyboardFilter.h>
#include <Views/Traits/PadReceiverViewTrait.h>
#include <unordered_map>

namespace Kub3::UI::Views
{
    // The hardware mapping for the physical PAD keys.
    static const std::unordered_map<Qt::Key, std::pair<PadTarget, PadAction>> PAD_HARDWARE_MAP = {
        // Left Camera
        {Qt::Key_A, {PadTarget::LeftCamera, PadAction::Up}},
        {Qt::Key_K, {PadTarget::LeftCamera, PadAction::Left}},
        {Qt::Key_B, {PadTarget::LeftCamera, PadAction::Down}},
        {Qt::Key_J, {PadTarget::LeftCamera, PadAction::Right}},
        // Right Camera
        {Qt::Key_H, {PadTarget::RightCamera, PadAction::Up}},
        {Qt::Key_N, {PadTarget::RightCamera, PadAction::Left}},
        {Qt::Key_Z, {PadTarget::RightCamera, PadAction::Down}},
        {Qt::Key_O, {PadTarget::RightCamera, PadAction::Right}},
        // X Stage
        {Qt::Key_L, {PadTarget::XStage, PadAction::Left}},
        {Qt::Key_Semicolon, {PadTarget::XStage, PadAction::Right}},
        // Y Stage
        {Qt::Key_T, {PadTarget::YStage, PadAction::Front}},
        {Qt::Key_E, {PadTarget::YStage, PadAction::Back}},
        // Theta Stage
        {Qt::Key_F, {PadTarget::ThetaStage, PadAction::CW}},
        {Qt::Key_D, {PadTarget::ThetaStage, PadAction::CCW}},
        // Z Elevators
        {Qt::Key_U, {PadTarget::ZElevator, PadAction::Up}},
        {Qt::Key_S, {PadTarget::ZElevator, PadAction::Down}},
    };

    PadReceiverViewTrait::PadReceiverViewTrait(QWidget *targetWidget)
    {
        if (!targetWidget)
            return;

        auto *keyboardFilter = new KeyboardFilter(targetWidget, targetWidget);

        QObject::connect(keyboardFilter, &KeyboardFilter::keyPressed, targetWidget,
                         [this](Qt::Key key, Qt::KeyboardModifiers) { handleEvent(key, PadTrigger::Pressed); });

        QObject::connect(keyboardFilter, &KeyboardFilter::keyHeld, targetWidget,
                         [this](Qt::Key key, Qt::KeyboardModifiers) { handleEvent(key, PadTrigger::Held); });

        QObject::connect(keyboardFilter, &KeyboardFilter::keyReleased, targetWidget,
                         [this](Qt::Key key, Qt::KeyboardModifiers) { handleEvent(key, PadTrigger::Released); });
    }

    void PadReceiverViewTrait::link(PadTarget target, PadAction action, PadCallback callback)
    {
        m_callbacks[{target, action}] = std::move(callback);
    }

    void PadReceiverViewTrait::handleEvent(Qt::Key key, PadTrigger trigger)
    {
        // Identify what target and action the raw key maps to
        if (auto itKey = PAD_HARDWARE_MAP.find(key); itKey != PAD_HARDWARE_MAP.end())
        {
            // See if the View linked a callback to this specific target/action pair
            if (auto itCb = m_callbacks.find(itKey->second); itCb != m_callbacks.end() && itCb->second)
            {
                // Execute the callback, injecting the current trigger state
                itCb->second(trigger);
            }
        }
    }

} // namespace Kub3::UI::Views
