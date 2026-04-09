#ifndef HOMEVIEW_H
#define HOMEVIEW_H

#include <QWidget>

namespace Ui
{
    class HomeView;
}

class HomeView : public QWidget
{
    Q_OBJECT

public:
    explicit HomeView(QWidget *parent = nullptr);
    ~HomeView();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateMachineLogo(int h);

private:
    Ui::HomeView *ui;
};

#endif // HOMEVIEW_H
