#ifndef USERSDB_H
#define USERSDB_H

#include "sqlite3.h"
#include "QString"
#include <string>

void create_user(QString username, QString password)
{

    sqlite3 *db = 0;
    char *err = 0;

    std::hash<std::string> hasher;

    std::string p = password.toStdString();
    std::string n = username.toStdString();
    size_t hash = hasher(p);

    std::string str = "INSERT INTO users (username, password) VALUES('" + n + "', '" + std::to_string(hash) + "');";
    const char *SQL = str.c_str();


    if( sqlite3_open("users.dblite", &db) )
        return;

    else if (sqlite3_exec(db, SQL, 0, 0, &err))
    {
        sqlite3_free(err);
        return;
    }
    sqlite3_close(db);
}

#endif // USERSDB_H
