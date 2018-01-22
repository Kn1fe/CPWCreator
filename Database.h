#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QMutex>
#include <QDebug>
#include <QThread>
#include <QHash>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QCoreApplication>
#include <QDir>

class Database : public QObject
{
    Q_OBJECT
public:
    explicit Database(QObject *parent = nullptr) : QObject(parent) {}
    static QSqlDatabase getInstance();

private:
    static QMutex s_databaseMutex;
    static QHash<QThread*, QSqlDatabase> s_instances;
};

#endif // DATABASE_H
