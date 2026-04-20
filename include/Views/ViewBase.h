#pragma once

#include <QHideEvent>
#include <QShowEvent>
#include <QWidget>

#include <ViewModels/IViewModel.h>
#include <utils.h>

namespace Kub3::UI::Views
{
    class ViewBase : public QWidget
    {
        Q_OBJECT

    public:
        explicit ViewBase(Shared<ViewModels::IViewModel> viewModel, QWidget *parent = nullptr);
        ~ViewBase() = default;

    protected:
        void showEvent(QShowEvent *event) override;
        void hideEvent(QHideEvent *event) override;

    private:
        Shared<ViewModels::IViewModel> m_viewModel;
    };

}