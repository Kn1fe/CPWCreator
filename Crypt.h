#ifndef CRYPT_H
#define CRYPT_H

#include <QObject>
#include <QTemporaryFile>
#include <QUuid>
#include <QTextStream>
#include <QFile>
#include <QDebug>
#include <QThread>
#include <iostream>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <Settings.h>

class Crypt
{
public:
    Crypt() {}
    static void generateKeys();
    static void loadKeys();
    static void signFile(const QString &path);

private:
    static QString readFromFile(const QString &path);
    static void writeToFile(const QString &path, const QString &data);

    static RSA *keys;
};

#endif // CRYPT_H
