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


void create_file(QString user)
{
    QJsonObject jsonObj;
    QJsonObject jsonObj1;

    QJsonArray jsonArray;
    jsonArray.append("You\nHello");
    jsonArray.append("AI\nHi");

    jsonObj["1"] = jsonArray;
    jsonObj1[user] = jsonObj;

    QByteArray byteArray;
    byteArray = QJsonDocument(jsonObj1).toJson();

    QFile file;
    file.setFileName("./history.json");
    if(!file.open(QIODevice::WriteOnly)){
        qDebug() << "No write access for json file";
        return;
    }

    file.write(byteArray);
    file.close();

}

QJsonDocument read_history(QString user)
{
    QFile file("./history.json");
    QJsonDocument history;

    if(!file.open(QIODevice::ReadOnly)){
        qDebug() << "Json filef couldn't be opened/found";
        create_file(user);
        read_history(user);
    }

    QByteArray byteArray;
    byteArray = file.readAll();
    file.close();

    QJsonParseError parseError;
    history = QJsonDocument::fromJson(byteArray, &parseError);

    if (!history[user][QString::number(1)].isArray()) {
        create_file(user);
        read_history(user);
    }

    if(parseError.error != QJsonParseError::NoError){
        qWarning() << "Parse error at " << parseError.offset << ":" << parseError.errorString();
        create_file(user);
        read_history(user);
    }

    return history;

}

void save_history(QJsonDocument history, QString user)
{
    QByteArray byteArray = history.toJson();

    QFile file("./history.json");
    if(!file.open(QIODevice::WriteOnly)){
        qDebug() << "No write access for json file";
    }

    file.write(byteArray);
    file.close();
}

QJsonDocument add_chat(int n, QString user)
{
    QFile file("./history.json");
    QJsonDocument history;

    if(!file.open(QIODevice::ReadOnly)){
        qDebug() << "Json filef couldn't be opened/found";
        create_file(user);
        read_history(user);
    }

    QByteArray byteArray;
    byteArray = file.readAll();
    file.close();

    QJsonParseError parseError;
    history = QJsonDocument::fromJson(byteArray, &parseError);
    if(parseError.error != QJsonParseError::NoError){
        qWarning() << "Parse error at " << parseError.offset << ":" << parseError.errorString();
    }
    QJsonObject jObj = history.object();
    // for (int i=1; i<=n; i++){
    //     if (!history[QString::number(i)].isArray()) break;

    //     QJsonValue chat = history[QString::number(i)];
    //     jObj.insert(QString::number(i), chat);
    // }
    QJsonObject jObj2 = jObj[user].toObject();
    QJsonArray a;
    QJsonValue chat(a);
    jObj2.insert(QString::number(n+1), chat);
    jObj.remove(user);
    jObj.insert(user, jObj2);

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

QJsonDocument delete_chat(int n, QString user)
{
    QFile file("./history.json");
    QJsonDocument history;

    if(!file.open(QIODevice::ReadOnly)){
        qDebug() << "Json filef couldn't be opened/found";
        create_file(user);
        read_history(user);
    }

    QByteArray byteArray;
    byteArray = file.readAll();
    file.close();

    QJsonParseError parseError;
    history = QJsonDocument::fromJson(byteArray, &parseError);
    if(parseError.error != QJsonParseError::NoError){
        qWarning() << "Parse error at " << parseError.offset << ":" << parseError.errorString();
    }

    QJsonObject jObj = history.object();
    QJsonObject jObj2;
    int i = 1;
    while (history[user][QString::number(i)].isArray()){
        if (i < n){
            QJsonValue chat = history[user][QString::number(i)];
            jObj2.insert(QString::number(i), chat);
        } else if(i > n) {
            QJsonValue chat = history[user][QString::number(i)];
            jObj2.insert(QString::number(i - 1), chat);
        }
        i++;
    }
    jObj.remove(user);
    jObj.insert(user, jObj2);

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
