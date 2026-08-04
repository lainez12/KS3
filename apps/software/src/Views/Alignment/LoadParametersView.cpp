#include <QString>

#include <QCheckBox>
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
    m_parametersDeleteButtons.clear();

    for (const QString &name : names)
    {
        QCheckBox *button = new QCheckBox(name, this);
        button->setObjectName("parameterButton");
        button->setProperty("class", "button-blue");
        button->setFixedHeight(80);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        connect(button, &QCheckBox::checkStateChanged, this, [this, name, button](Qt::CheckState state) {
            if (state == Qt::CheckState::Checked)
            {
                m_parametersDeleteButtons.insert(name, button);
            }
            else
            {
                m_parametersDeleteButtons.remove(name);
            }
        });

        ui->parametersListWidget->layout()->addWidget(button);
        m_parametersButtons.insert(name, button);
    }
}