#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "QPushButton"
#include <QJsonDocument>
#include <QGridLayout>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QPushButton chats[5];
    QJsonDocument history;
    QGridLayout *chat_list = new QGridLayout;
    int chat_n = 1;
    int current_chat = 1;

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void ai_response();
    void create_chat();
    void swith_chat(int n);
    void save_chat();


private slots:
    void setup_chats();
    void on_sendButton_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
