#ifndef AUTHWINDOW_H
#define AUTHWINDOW_H

#include <QDialog>

namespace Ui {
class AuthWindow;
}

class AuthWindow : public QDialog
{
    Q_OBJECT

public:
    explicit AuthWindow(QWidget *parent = nullptr);
    ~AuthWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

signals:
    void logB_clicked(QString username, QString password);
    void regB_clicked();

private slots:
    void on_logB_clicked();

    void on_regB_clicked();

private:
    Ui::AuthWindow *ui;
};

#endif // AUTHWINDOW_H
