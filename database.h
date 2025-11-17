#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

class Database {
public:
    // Initializes the database and creates the tables with dummy data
    static bool initialize();
};

#endif // DATABASE_H
