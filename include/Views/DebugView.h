#ifndef DEBUGVIEW_H
#define DEBUGVIEW_H

#include <QWidget>

namespace Ui
{
    class DebugView;
}

class DebugView : public QWidget
{
    Q_OBJECT

public:
    explicit DebugView(QWidget *parent = nullptr);
    ~DebugView();

    void updateMachineState(const QString &stateName);

signals:
    void s_openMachineStatus(void);
    void s_initializationRequest(void);

private slots:
    void on_openMachineStatusViewBtn_clicked(void);
    void on_startInitializationBtn_clicked(void);

private:
    Ui::DebugView *ui;
};

#endif // DEBUGVIEW_H
