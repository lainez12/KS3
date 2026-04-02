#ifndef MACHINE_STATUS_VIEW_H
#define MACHINE_STATUS_VIEW_H

#include <QWidget>

namespace Ui
{
    class MachineStatusView;
}

class MachineStatusView : public QWidget
{
    Q_OBJECT

public:
    explicit MachineStatusView(QWidget *parent = nullptr);
    ~MachineStatusView();

signals:
    void s_home(void);

private slots:
    void on_goBackBtn_clicked(void);

private:
    Ui::MachineStatusView *ui;
};

#endif // MACHINE_STATUS_VIEW_H
