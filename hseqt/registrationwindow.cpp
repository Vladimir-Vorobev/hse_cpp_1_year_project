#include "registrationwindow.h"
#include "ui_registrationwindow.h"

RegistrationWindow::RegistrationWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegistrationWindow)
{
    ui->setupUi(this);
}


RegistrationWindow::~RegistrationWindow()
{
    delete ui;
}

void RegistrationWindow::on_regB_clicked()
{
    emit regB_clicked(ui->loginE->text(), ui->passE->text());
}


void RegistrationWindow::on_logB_clicked()
{
    emit logB_clicked();
}

