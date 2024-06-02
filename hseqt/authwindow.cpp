#include "authwindow.h"
#include "ui_authwindow.h"

AuthWindow::AuthWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AuthWindow)
{
    ui->setupUi(this);
}

AuthWindow::~AuthWindow()
{
    delete ui;
}

void AuthWindow::on_logB_clicked()
{
    emit logB_clicked(ui->loginE->text(), ui->passE->text());
}


void AuthWindow::on_regB_clicked()
{
    emit regB_clicked();
}

