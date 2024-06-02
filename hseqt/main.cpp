#include "mainwindow.h"
#include "sqlite3.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    const char* SQL = "CREATE TABLE IF NOT EXISTS users(id INTEGER PRIMARY KEY AUTOINCREMENT, 'username' TEXT NOT NULL, 'password' TEXT NOT NULL);";

    sqlite3 *db = 0;
    char *err = 0;

    if( sqlite3_open("users.dblite", &db) ){
        qWarning() << "Ошибка открытия/создания БД";
        return 0;
    }
    else if (sqlite3_exec(db, SQL, 0, 0, &err))
    {
        qWarning() << "Ошибка sql";
        sqlite3_free(err);
        return 0;
    }
    sqlite3_close(db);

    return a.exec();
}
