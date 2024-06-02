#ifndef USERSDB_H
#define USERSDB_H

#include "qdebug.h"
#include "sqlite3.h"
#include "QString"
#include <string>

static int create_call(void *data, int argc, char **argv, char **azColName) {
    int i;
    if (argc){
        bool* res = reinterpret_cast<bool*> (data);
        *res = false;
    }
    return 0;
}

bool create_user(QString username, QString password)
{

    sqlite3 *db = 0;
    char *err = 0;

    std::hash<std::string> hasher;

    std::string p = password.toStdString();
    std::string n = username.toStdString();
    size_t hash = hasher(p);
    bool res = true;

    std::string str = "INSERT INTO users (username, password) VALUES('" + n + "', '" + std::to_string(hash) + "');";
    const char *SQL_insert = str.c_str();

    std::string str2 = "SELECT 1 FROM users WHERE username = '" + n + "'";
    const char *SQL_select = str2.c_str();

    if( sqlite3_open("users.dblite", &db) ){
        qWarning() << "Ошибка открытия/создания БД";
        return false;
    }
    else if (sqlite3_exec(db, SQL_select, create_call, &res, &err))
    {
        qWarning() << "Ошибка sql";
        sqlite3_free(err);
        return false;
    } else if (res) {
        if (sqlite3_exec(db, SQL_insert, 0, 0, &err)) {
            qWarning() << "Ошибка sql";
            sqlite3_free(err);
            return false;
        }
    } else return false;
    sqlite3_close(db);
    return true;
}


static int get_user_call(void *data, int argc, char **argv, char **azColName) {
    int i;
    if (argc){
        bool* correct_user = reinterpret_cast<bool*> (data);
        *correct_user = true;
    }
    return 0;
}


bool get_user(QString username, QString password)
{
    sqlite3 *db = 0;
    char *err = 0;

    std::hash<std::string> hasher;

    std::string p = password.toStdString();
    std::string n = username.toStdString();
    size_t hash = hasher(p);
    bool correct_user = false;

    std::string str2 = "SELECT 1 FROM users WHERE username = '" + n + "' AND password = '" + std::to_string(hash) + "'";
    const char *SQL_select = str2.c_str();

    if( sqlite3_open("users.dblite", &db) ){
        qWarning() << "Ошибка открытия/создания БД";
        return false;
    }
    else if (sqlite3_exec(db, SQL_select, get_user_call, &correct_user, &err))
    {
        qWarning() << "Ошибка sql";
        sqlite3_free(err);
        return false;
    }
    sqlite3_close(db);
    return correct_user;
}

#endif // USERSDB_H
