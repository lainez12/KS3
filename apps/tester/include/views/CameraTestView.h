#pragma once

#include <QGroupBox>
#include <QScrollArea>
#include <QVBoxLayout>
#include <memory>

#include <utils.h>
#include <view_models/CameraTestViewModel.h>
#include <views/ViewBase.h>

namespace Kub3::Tools::Tester
{

    class CameraTestView final : public UI::Views::ViewBase
    {
        Q_OBJECT

    public:
        explicit CameraTestView(Shared<CameraTestViewModel> vm, QWidget *parent = nullptr);
        ~CameraTestView() override = default;

    private:
        void bindViewModel();
        void buildFocalModules(const QStringList &focalIds);
        void buildLightModules(const QStringList &lightIds);

    private:
        Shared<CameraTestViewModel> m_vm;

        QWidget *m_scrollContainer = nullptr;
        QVBoxLayout *m_mainLayout  = nullptr;

        QGroupBox *m_focalsGroup    = nullptr;
        QVBoxLayout *m_focalsLayout = nullptr;

        QGroupBox *m_lightsGroup    = nullptr;
        QVBoxLayout *m_lightsLayout = nullptr;
    };

} // namespace Kub3::Tools::Tester
