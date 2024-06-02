#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "QListWidgetItem"
#include "history.h"
#include "QJsonArray"
#include "QPushButton"
#include "griddelete.h"
#include "authwindow.h"
#include "registrationwindow.h"
#include "usersdb.h"
#include "QMessageBox"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->history = read_history(user);
    this->setup_chats();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::create_chat()
{
    save_chat();
    this->history = add_chat(this->chat_n, user);
    this->chat_n++;
    this->current_chat = this->chat_n;
    GridLayoutUtil::removeRow(this->chat_list, chat_n - 1);
    setup_chats();
}


void MainWindow::swith_chat(int n)
{
    save_chat();
    this->current_chat = n;
    setup_chats();
}


void MainWindow::save_chat()
{
    QJsonObject jObj = history.object();
    QJsonObject jObj2;
    for (int i=1; i<=this->chat_n; i++){
        if (!history[user][QString::number(i)].isArray()) break;

        QJsonValue chat = history[user][QString::number(i)];
        jObj2.insert(QString::number(i), chat);
    }
    QJsonArray chat;
    int i=0;
    while (ui->chatW->item(i)){
        chat.append(ui->chatW->item(i)->text());
        i++;
    }
    jObj2[QString::number(this->current_chat)] = chat;

    jObj.remove(user);
    jObj.insert(user, jObj2);
    QJsonDocument history_new { jObj };

    this->history = history_new;
    save_history(history_new, user);
}


void MainWindow::setup_chats()
{
    ui->chatW->clear();

    int count = 0;

    this->chat_list = new QGridLayout;

    for (int chat_i=1; chat_i<=100; chat_i++){
        if (!this->history[user][QString::number(chat_i)].isArray()) break;
        count = chat_i;

        if (chat_i == this->current_chat){
            QJsonArray chat = this->history[user][QString::number(chat_i)].toArray();
            for (auto message: chat)
            {
                ui->chatW->addItem(message.toString());
            }
        }


        QPushButton *chatB = new QPushButton(tr("Chat ") + QString::number(chat_i));
        connect(chatB, &QPushButton::clicked, this, [this, chat_i]() {
            swith_chat(chat_i);
        });
        chatB->setStyleSheet(" background-color: rgb(255, 255, 255); border-radius: 9px; height: 30px; margin: 5px 0px 5px 5px; font: 11pt 'Cascadia Mono';");
        if (chat_i == this->current_chat) chatB->setStyleSheet(" background-color: rgb(207, 226, 243); border-radius: 9px; height: 30px; margin: 5px 0px 5px 5px; font: 11pt 'Cascadia Mono';");
        this->chat_list->addWidget(chatB, chat_i - 1, 0);


        QPushButton *deleteB = new QPushButton(tr("X"));
        connect(deleteB, &QPushButton::clicked, this, [this, chat_i]() {
            this->history = delete_chat(chat_i, user);
            // this->chat_n--;
            // this->current_chat = std::min(this->current_chat, this->chat_n);
            setup_chats();
        });
        deleteB->setStyleSheet(" background-color: rgb(255, 0, 0); border-radius: 4px; height: 30px; max-width: 15px; margin: 5px 5px 0px 5px; font: 11pt 'Cascadia Mono';");
        this->chat_list->addWidget(deleteB, chat_i - 1, 1);
    }

    QPushButton *createB = new QPushButton(tr("+ New Chat +"));
    connect(createB, &QPushButton::clicked, this, [this]() {
        create_chat();
        this->chat_list->itemAtPosition(0, 1)->widget()->setEnabled(true);
    });
    createB->setStyleSheet(" background-color: rgb(255, 255, 255); border-radius: 9px; min-height: 30px; margin: 5px; font: 11pt 'Cascadia Mono';");
    this->chat_list->addWidget(createB, count, 0, 1, 2);


    this->chat_list->setAlignment(Qt::AlignmentFlag::AlignTop);
    this->chat_n = count;
    this->current_chat = std::min(this->current_chat, this->chat_n);

    if(this->chat_n == 1){
        this->chat_list->itemAtPosition(0, 1)->widget()->setEnabled(false);
    }

    if ( ui->groupBox->layout() != NULL )
    {
        QLayoutItem* item;
        while ( ( item = ui->groupBox->layout()->takeAt( 0 ) ) != NULL )
        {
            delete item->widget();
            delete item;
        }
        delete ui->groupBox->layout();
    }

    ui->groupBox->setLayout(this->chat_list);
}


void MainWindow::on_sendButton_clicked()
{

    QString text = "You\n" + ui->msgEdit->toPlainText().trimmed();

    if (text.size() < 5) return;

    // QListWidgetItem *msg = new QListWidgetItem(text, ui->chatW);
    ui->chatW->addItem(text);
    ui->msgEdit->setText("");

    ai_response();
}


void MainWindow::ai_response()
{
    QString text = "AI\nHi";

    // QListWidgetItem *msg = new QListWidgetItem(text, ui->chatW);
    ui->chatW->addItem(text);
}


void MainWindow::on_loginB_clicked()
{
    this->authW = new AuthWindow(this);
    this->authW->show();
    this->setEnabled(false);
    this->authW->setEnabled(true);

    connect(this->authW, &AuthWindow::logB_clicked, this, [this](QString username, QString password) {
        if (get_user(username, password)){
            this->user = username;
            ui->loginB->setEnabled(false);
            ui->loginB->setText(username);
            QMessageBox msgError;
            msgError.setText("Logged successfully!");
            msgError.setWindowTitle("Notification");
            msgError.exec();
            read_history(this->user);
            setup_chats();
            this->authW->close();
        } else {
            QMessageBox msgError;
            msgError.setText("Wrong username or password");
            msgError.setWindowTitle("Notification");
            msgError.exec();
        }
    });

    connect(this->authW, &AuthWindow::regB_clicked, this, [this](){
        this->authW->close();
        this->on_regB_clicked();
    });

}


void MainWindow::on_regB_clicked()
{
    this->regW = new RegistrationWindow(this);
    this->regW->show();
    this->setEnabled(false);
    this->regW->setEnabled(true);

    connect(this->regW, &RegistrationWindow::regB_clicked, this, [this](QString username, QString password) {
        if ((username.size() >= 5) and (password.size() >= 5)){
            // create_user(username, password);
            if (!create_user(username, password)) {
                QMessageBox msgError;
                msgError.setText(username + " is already taken, try another one");
                msgError.setWindowTitle("Notification");
                msgError.exec();
                this->regW->close();
            } else {
                QMessageBox msgError;
                msgError.setText("New account created");
                msgError.setWindowTitle("Notification");
                msgError.exec();
            }
        } else {
            QMessageBox msgError;
            msgError.setText("Username and password must be at least 5 characters long!");
            msgError.setWindowTitle("Notification");
            msgError.exec();
        }
    });

    connect(this->regW, &RegistrationWindow::logB_clicked, this, [this](){
        this->regW->close();
        this->on_loginB_clicked();
    });
}


void AuthWindow::closeEvent(QCloseEvent *event)
{
    this->parentWidget()->setEnabled(true);
}


void RegistrationWindow::closeEvent(QCloseEvent *event)
{
    this->parentWidget()->setEnabled(true);
}


void MainWindow::on_logoutB_clicked()
{
    this->user = "none";
    ui->loginB->setEnabled(true);
    ui->loginB->setText("Login");
    read_history(this->user);
    setup_chats();
}
