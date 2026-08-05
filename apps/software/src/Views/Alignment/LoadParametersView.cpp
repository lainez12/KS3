#include <QString>

#include <QPushButton>
#include <Views/Alignment/LoadParametersView.h>
#include <Views/Components/Colors.h>

#include "ui_LoadParametersView.h"

#define ID_BTN_BACK "B"
#define ID_BTN_SAVE "S"
#define ID_BTN_HOME "H"

LoadParametersView::LoadParametersView(Unique<LoadParametersViewModel> viewModel, QWidget *parent) :
    AlignmentViewBase(std::move(viewModel), parent),
    ui(new Ui::LoadParametersView)
{
    ui->setupUi(this);
    setDefaultTitleBar("Exposure settings");
    setNewNavButtonsConfigs();

    QVBoxLayout *layout = new QVBoxLayout(ui->parametersListWidget);
    ui->parametersListWidget->setLayout(layout);

    connect(ui->btnClose, &QPushButton::clicked, this, &LoadParametersView::onBackButtonClicked);
    connect(ui->btnLoad, &QPushButton::clicked, this, &LoadParametersView::onBtnLoadClicked);
    connect(ui->btnRemove, &QPushButton::clicked, this, &LoadParametersView::onBtnRemoveClicked);
    connect(ui->btnRename, &QPushButton::clicked, this, &LoadParametersView::onBtnRenameClicked);
}
LoadParametersView::~LoadParametersView()
{
}

void LoadParametersView::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);
}

void LoadParametersView::showEvent(QShowEvent *ev)
{
    ui->btnLoad->setEnabled(false);
    ui->btnRemove->setEnabled(false);
    ui->btnRename->setEnabled(false);
    populateParametersList();
    AlignmentViewBase::showEvent(ev);
}

void LoadParametersView::setNewNavButtonsConfigs()
{
    NavButtonConfig backBtn(
        "Back",
        ":/icons/back.svg",
        "B",
        std::bind(&LoadParametersView::onBackButtonClicked, this));
    addNavButton("left", backBtn);
}

void LoadParametersView::onBackButtonClicked()
{
    emit s_openView(Kub3::UI::ViewId::ALIGNMENT_VISUALISATION_VIEW);
}

void LoadParametersView::onValidateButtonClicked()
{
}

void LoadParametersView::populateParametersList()
{
    auto vm = getViewModel<LoadParametersViewModel>();
    if (!vm)
        return;

    auto namesRes = vm->getAllNamesSavedParameters();
    if (namesRes.is_err())
    {
        return;
    }

    const QList<QString> &names = namesRes.unwrap();

    // Clear existing buttons
    clearWidget(ui->parametersListWidget);
    m_parametersButtons.clear();
    m_selectedParameterButtons.clear();

    for (const QString &name : names)
    {
        QPushButton *button = new QPushButton(name, this);
        button->setProperty("class", "button-blue");
        button->setCheckable(true);
        button->setFixedHeight(80);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        connect(button, &QPushButton::toggled, this, [this, name, button](bool checked) {
            onBtnParameterClicked(name, button, checked);
        });

        ui->parametersListWidget->layout()->addWidget(button);
        m_parametersButtons.insert(name, button);
    }
}

void LoadParametersView::onBtnLoadClicked()
{
    if (m_selectedParameterButtons.size() != 1)
        return;
    if (m_selectedParameterButtons.isEmpty())
        return;
    auto vm = getViewModel<LoadParametersViewModel>();
    if (!vm)
        return;

    const QString &selectedName = m_selectedParameterButtons.firstKey();
    if (vm->uiRequestedLoadParameter(selectedName))
    {
        emit s_openView(Kub3::UI::ViewId::ALIGNMENT_VISUALISATION_VIEW);
    }
}
void LoadParametersView::onBtnRemoveClicked()
{
    auto vm = getViewModel<LoadParametersViewModel>();
    if (!vm)
        return;
    for (const QString &name : m_selectedParameterButtons.keys())
    {
        QPushButton *button = m_parametersButtons.value(name, nullptr);
        if (button)
        {
            ui->parametersListWidget->layout()->removeWidget(button);
            button->deleteLater();
        }
        auto removeRes = vm->removeParameterByName(name);
        if (removeRes.is_err())
        {
            qWarning() << "Error removing parameter:" << removeRes.unwrap_err();
        }
    }
    m_selectedParameterButtons.clear();
    updateButtonsStateBasedOnSelection();
}
void LoadParametersView::onBtnRenameClicked()
{
}
void LoadParametersView::onBtnParameterClicked(const QString &name, QPushButton *button, bool checked)
{
    if (checked)
    {
        m_selectedParameterButtons.insert(name, button);
        ui->btnRemove->setEnabled(true);
        updateButtonsStateBasedOnSelection();
    }
    else
    {
        m_selectedParameterButtons.remove(name);
        updateButtonsStateBasedOnSelection();
    }
}

void LoadParametersView::updateButtonsStateBasedOnSelection()
{
    int selectedCount = m_selectedParameterButtons.size();
    if (selectedCount == 0)
    {
        ui->btnLoad->setEnabled(false);
        ui->btnRemove->setEnabled(false);
        ui->btnRename->setEnabled(false);
    }
    else if (selectedCount == 1)
    {
        ui->btnLoad->setEnabled(true);
        ui->btnRemove->setEnabled(true);
        ui->btnRename->setEnabled(true);
    }
    else
    {
        ui->btnLoad->setEnabled(false);
        ui->btnRemove->setEnabled(true);
        ui->btnRename->setEnabled(false);
    }
}