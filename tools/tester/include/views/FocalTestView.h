#pragma once

#include <QScrollArea>
#include <QVBoxLayout>
#include <memory>

#include <utils.h>
#include <view_models/FocalTestViewModel.h>
#include <views/ViewBase.h>

namespace Kub3::Tools::Tester
{

    class FocalTestView final : public UI::Views::ViewBase
    {
        Q_OBJECT

    public:
        explicit FocalTestView(Shared<FocalTestViewModel> vm, QWidget *parent = nullptr);
        ~FocalTestView() override = default;

    private:
        void bindViewModel();
        void buildDynamicModules(const QList<QString> &focalIds);

    private:
        Shared<FocalTestViewModel> m_vm;

        QVBoxLayout *m_modulesLayout = nullptr;
        QWidget *m_scrollContainer   = nullptr;
    };

} // namespace Kub3::Tools::Tester