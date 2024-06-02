#ifndef REGISTRATIONWINDOW_H
#define REGISTRATIONWINDOW_H

#include <QDialog>

namespace Ui {
class RegistrationWindow;
}

class RegistrationWindow : public QDialog
{
    Q_OBJECT

public:
    explicit RegistrationWindow(QWidget *parent = nullptr);
    ~RegistrationWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::RegistrationWindow *ui;
};

#endif // REGISTRATIONWINDOW_H
