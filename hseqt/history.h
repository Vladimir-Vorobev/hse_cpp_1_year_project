#ifndef HISTORY_H
#define HISTORY_H

#include <QCoreApplication>
#include <QString>
#include <QVariant>
#include <QFile>
#include <QByteArray>
#include <QTextStream>
#include <QDebug>

//json specific
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>


void create_file()
{
    QJsonObject jsonObj;

    QJsonArray jsonArray;
    jsonArray.append("You/nHello");
    jsonArray.append("AI/nHi");

    jsonObj["1"] = jsonArray;

    QByteArray byteArray;
    byteArray = QJsonDocument(jsonObj).toJson();

    QFile file;
    file.setFileName("./history.json");
    if(!file.open(QIODevice::WriteOnly)){
        qDebug() << "No write access for json file";
        return;
    }

    file.write(byteArray);
    file.close();

}

QJsonDocument read_history()
{
    QFile file("./history.json");
    QJsonDocument history;

    if(!file.open(QIODevice::ReadOnly)){
        qDebug() << "Json filef couldn't be opened/found";
        create_file();
        read_history();
    }

    QByteArray byteArray;
    byteArray = file.readAll();
    file.close();

    QJsonParseError parseError;
    history = QJsonDocument::fromJson(byteArray, &parseError);

    if (!history[QString::number(1)].isArray()) {
        create_file();
        read_history();
    }

    if(parseError.error != QJsonParseError::NoError){
        qWarning() << "Parse error at " << parseError.offset << ":" << parseError.errorString();
        create_file();
        read_history();
    }

    return history;

}

void save_history(QJsonDocument history)
{
    QByteArray byteArray = history.toJson();

    QFile file("./history.json");
    if(!file.open(QIODevice::WriteOnly)){
        qDebug() << "No write access for json file";
    }

    file.write(byteArray);
    file.close();
}

QJsonDocument add_chat(int n)
{
    QFile file("./history.json");
    QJsonDocument history;

    if(!file.open(QIODevice::ReadOnly)){
        qDebug() << "Json filef couldn't be opened/found";
        create_file();
        read_history();
    }

    QByteArray byteArray;
    byteArray = file.readAll();
    file.close();

    QJsonParseError parseError;
    history = QJsonDocument::fromJson(byteArray, &parseError);
    if(parseError.error != QJsonParseError::NoError){
        qWarning() << "Parse error at " << parseError.offset << ":" << parseError.errorString();
    }

    QJsonObject jObj;
    for (int i=1; i<=n; i++){
        if (!history[QString::number(i)].isArray()) break;

        QJsonValue chat = history[QString::number(i)];
        jObj.insert(QString::number(i), chat);
    }
    QJsonArray a;
    QJsonValue chat(a);
    jObj.insert(QString::number(n+1), chat);

    QJsonDocument history_new { jObj };


    byteArray = history_new.toJson();

    file.setFileName("./history.json");
    if(!file.open(QIODevice::WriteOnly)){
        qDebug() << "No write access for json file";
    }

    file.write(byteArray);
    file.close();

    return history_new;
}

QJsonDocument delete_chat(int n)
{
    QFile file("./history.json");
    QJsonDocument history;

    if(!file.open(QIODevice::ReadOnly)){
        qDebug() << "Json filef couldn't be opened/found";
        create_file();
        read_history();
    }

    QByteArray byteArray;
    byteArray = file.readAll();
    file.close();

    QJsonParseError parseError;
    history = QJsonDocument::fromJson(byteArray, &parseError);
    if(parseError.error != QJsonParseError::NoError){
        qWarning() << "Parse error at " << parseError.offset << ":" << parseError.errorString();
    }

    QJsonObject jObj;
    int i = 1;
    while (history[QString::number(i)].isArray()){
        if (i < n){
            QJsonValue chat = history[QString::number(i)];
            jObj.insert(QString::number(i), chat);
        } else if(i > n) {
            QJsonValue chat = history[QString::number(i)];
            jObj.insert(QString::number(i - 1), chat);
        }
        i++;
    }

    QJsonDocument history_new { jObj };


    byteArray = history_new.toJson();

    file.setFileName("./history.json");
    if(!file.open(QIODevice::WriteOnly)){
        qDebug() << "No write access for json file";
    }

    file.write(byteArray);
    file.close();

    return history_new;
}
#endif // HISTORY_H
