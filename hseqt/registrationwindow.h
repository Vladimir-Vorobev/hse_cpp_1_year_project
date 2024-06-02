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

signals:
    void regB_clicked(QString username, QString password);

private slots:
    void on_regB_clicked();

private:
    Ui::RegistrationWindow *ui;
};

#endif // REGISTRATIONWINDOW_H
