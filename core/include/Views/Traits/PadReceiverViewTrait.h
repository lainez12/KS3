#pragma once

#include <QWidget>
#include <functional>
#include <map>
#include <utility>

namespace Kub3::UI::Views
{
    // The physical subsystems the PAD can control
    enum class PadTarget
    {
        LeftCamera,
        RightCamera,
        XStage,
        YStage,
        ThetaStage,
        ZElevator
    };

    // The logical directional actions on the PAD
    enum class PadAction
    {
        Up,
        Down,
        Left,
        Right,
        Front,
        Back,
        CW,
        CCW
    };

    // The physical state of the PAD button
    enum class PadTrigger
    {
        Pressed,
        Held,
        Released
    };

    using PadCallback = std::function<void(PadTrigger)>;

    /**
     * @brief Trait that maps generic keyboard events to specific Hardware PAD semantics.
     */
    class PadReceiverViewTrait
    {
    public:
        explicit PadReceiverViewTrait(QWidget *targetWidget);
        virtual ~PadReceiverViewTrait() = default;

        /**
         * @brief Links a semantic PAD target and action to a specific callback.
         */
        void link(PadTarget target, PadAction action, PadCallback callback);

    private:
        void handleEvent(Qt::Key key, PadTrigger trigger);

        // Uses a std::map to map a Target/Action pair to the bound callback
        std::map<std::pair<PadTarget, PadAction>, PadCallback> m_callbacks;
    };

} // namespace Kub3::UI::Views
