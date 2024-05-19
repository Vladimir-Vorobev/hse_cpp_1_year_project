#include "authwindow.h"
#include "./ui_authwindow.h"
#include "QString"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::AuthWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    ui->label->setText(QString::fromStdString(std::to_string(std::stoi(ui->label->text().toStdString()) + 1)));
    QWidget *wdg = new QWidget;
    wdg->show();
    hide();//this will disappear main window
    // ui->label->setText("2");
}

