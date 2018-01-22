#ifndef PATCHER_H
#define PATCHER_H

#include <QObject>
#include <QFile>
#include <QDataStream>
#include <QThreadPool>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QtConcurrent>
#include <QFutureSynchronizer>
#include <QDirIterator>
#include <QHash>
#include <iostream>
#include <Crypt.h>
#include <Utils.h>
#include <Database.h>
#include <patcher_entry.h>
#include <zlib_entry.h>

class Patcher : public QObject
{
    Q_OBJECT
public:
    explicit Patcher(QObject *parent = nullptr);
    void patchExeFile(const QString &path);
    void initial();
    int getTypeVersion(const QString &type);
    long getUpdateSize(const QString &type, const int &from, const int &to);
    void createPatch(const int &version);
    void rebuild();
    bool updateDirectory(const QString &path, const int &version);
    void updateFile(const QString &path, const QString &fp, const QString &op, const QString &type, const int &version);
    void createVLists(const QString &path);
    void createMd5List(const QString &path);
    void createVersionFile(const QString &path);
    void loadDatabase();
    void writeType(const QString &type);
    void removeFiles(const QString &path);

private:
    QString new_dir;
    QString cpw_dir;
    QStringList parts;
    int compressionLevel;
    mutable QMutex mutex;
    QHash<QString, QHash<QString, patcher_entry>> files;
};

#endif // PATCHER_H
