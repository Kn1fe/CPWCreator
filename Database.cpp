#include "Database.h"

QMutex Database::s_databaseMutex;
QHash<QThread*, QSqlDatabase> Database::s_instances;

QSqlDatabase Database::getInstance()
{
    QMutexLocker locker(&s_databaseMutex);
    QThread *thread = QThread::currentThread();
    if (s_instances.contains(thread))
        return s_instances[thread];
    QSqlDatabase connection = QSqlDatabase::addDatabase("QSQLITE", QString::number((int)thread));
    connection.setDatabaseName(QCoreApplication::applicationDirPath() + QDir::separator() + "files.db");
    connection.open();
    QSqlQuery sql(connection);
    sql.exec("PRAGMA synchronous = OFF;");
    sql.exec("PRAGMA journal_mode = OFF;");
    sql.exec("PRAGMA cache_size=128000;");
    sql.exec("PRAGMA busy_timeout = 12000000");
    s_instances.insert(thread, connection);
    return connection;
}
