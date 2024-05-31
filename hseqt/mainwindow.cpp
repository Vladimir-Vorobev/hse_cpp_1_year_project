#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "QListWidgetItem"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_sendButton_clicked()
{

    QString text = "You\n" + ui->msgEdit->toPlainText().trimmed();

    if (!text.size()) return;

    QListWidgetItem *msg = new QListWidgetItem(text, ui->chatW);

    ui->msgEdit->setText("");

    ai_response();
}


void MainWindow::ai_response()
{
    QString text = "AI\nHi";

    QListWidgetItem *msg = new QListWidgetItem(text, ui->chatW);
}
