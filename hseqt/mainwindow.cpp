#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "QListWidgetItem"
#include "history.h"
#include "QJsonArray"
#include "QPushButton"
#include "griddelete.h"
#include "authwindow.h"
#include "qlineedit.h"
#include "registrationwindow.h"
#include "usersdb.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->history = read_history();
    this->setup_chats();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::create_chat()
{
    save_chat();
    this->history = add_chat(this->chat_n);
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
    QJsonObject jObj;
    for (int i=1; i<=this->chat_n; i++){
        if (!history[QString::number(i)].isArray()) break;

        QJsonValue chat = history[QString::number(i)];
        jObj.insert(QString::number(i), chat);
    }
    QJsonArray chat;
    int i=0;
    while (ui->chatW->item(i)){
        chat.append(ui->chatW->item(i)->text());
        i++;
    }
    jObj[QString::number(this->current_chat)] = chat;
    QJsonDocument history_new { jObj };

    this->history = history_new;
    save_history(history_new);
}


void MainWindow::setup_chats()
{
    ui->chatW->clear();

    int count = 0;

    this->chat_list = new QGridLayout;

    for (int chat_i=1; chat_i<=100; chat_i++){
        if (!this->history[QString::number(chat_i)].isArray()) break;
        count = chat_i;

        if (chat_i == this->current_chat){
            QJsonArray chat = this->history[QString::number(chat_i)].toArray();
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
            this->history = delete_chat(chat_i);
            this->chat_n--;
            this->current_chat = std::min(this->current_chat, this->chat_n);
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
    AuthWindow *authW = new AuthWindow(this);
    authW->show();
    this->setEnabled(false);
    authW->setEnabled(true);

    QLineEdit *loginE = authW->findChild<QLineEdit *>("loginE", Qt::FindDirectChildrenOnly);
    QLineEdit *passE = authW->findChild<QLineEdit *>("passE", Qt::FindDirectChildrenOnly);
    QPushButton *logB = authW->findChild<QPushButton *>("logB", Qt::FindDirectChildrenOnly);

    authW->connect(logB, &QPushButton::clicked, this, [this, loginE, passE]() {
        // if ((loginE->text().toStdString().size() > 3) && (passE->text().toStdString().size() > 5)){
        //     create_user(loginE->text(), passE->text());
        // }
    });

}


void MainWindow::on_regB_clicked()
{
    RegistrationWindow *regW = new RegistrationWindow(this);
    regW->show();
    this->setEnabled(false);
    regW->setEnabled(true);

    QLineEdit *loginE = regW->findChild<QLineEdit *>("loginE", Qt::FindDirectChildrenOnly);
    QLineEdit *passE = regW->findChild<QLineEdit *>("passE", Qt::FindDirectChildrenOnly);
    QPushButton *regB = regW->findChild<QPushButton *>("regB", Qt::FindDirectChildrenOnly);

    regW->connect(regB, &QPushButton::clicked, this, [this, loginE, passE]() {
        if ((loginE->text().toStdString().size() > 3) && (passE->text().toStdString().size() > 5)){
            create_user(loginE->text(), passE->text());
        }
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


