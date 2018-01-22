#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QByteArray>
#include <QFile>
#include <QDir>
#include <QCryptographicHash>
#include <QDebug>
#include <QBuffer>
#include <QtZlib/zlib.h>
#include <zlib_entry.h>

class Utils : public QObject
{
    Q_OBJECT
public:
    explicit Utils(QObject *parent = nullptr) : QObject(parent) {}
    static QString md5Hash(const QString &path);
    static zlib_entry zlibCompress(QByteArray data, const int &compressionLevel);
    static QByteArray zlibDecompress(const QByteArray &data, const int &sizeDecompressed);
    static QString pathToBase64(QString path);
    static QString toBase64(const QString &text);
};

#endif // UTILS_H
