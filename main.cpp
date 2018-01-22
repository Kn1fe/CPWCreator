#include <QCoreApplication>
#include <QDebug>
#include <iostream>
#include <QStringList>
#include <QThreadPool>
#include <QtEndian>
#include <Patcher.h>
#include <Crypt.h>
#include <Settings.h>
#include <Utils.h>
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/err.h>

void Usage();

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QStringList args;
    args << "rsagen" << "x" << "initial" << "new" << "rebuild";
    if (argc < 2) {
        Usage();
    } else {
        SSL_load_error_strings();
        ERR_load_crypto_strings();
        OpenSSL_add_all_algorithms();
        std::cout << "OpenSSL version: " << OPENSSL_VERSION_TEXT << std::endl;
        Settings::init();
        int threadCount = Settings::get("ThreadCount").toInt();
        if (threadCount == 0)
            threadCount = QThread::idealThreadCount() * 8;
        QThreadPool::globalInstance()->setMaxThreadCount(threadCount);
        std::cout << "Set thread count: " << threadCount << std::endl;
        Patcher *p = new Patcher();
        switch (args.indexOf(argv[1])) {
        case 0:
            std::cout << "Generate new RSA keys" << std::endl;
            Crypt::generateKeys();
            break;
        case 1:
            std::cout << "Patching files" << std::endl;
            for (int i = 2; i < argc; ++i)
                p->patchExeFile(QString(argv[i]));
            break;
        case 2:
            std::cout << "Make initial patch with version 1" << std::endl;
            p->initial();
            break;
        case 3:
        {
            std::cout << "Creating new patch" << std::endl;
            int version = 1;
            if (argc > 2)
                version = atoi(argv[2]);
            p->createPatch(version);
            break;
        }
        case 4:
        {
            std::cout << "Rebuild files list" << std::endl;
            p->rebuild();
            break;
        }
        }
        std::cout << "Ready." << std::endl;
    }
}

void Usage()
{
    std::cout << "Usage command:" << std::endl;
    std::cout << "creator rsagen - generate new rsa keys" << std::endl;
    std::cout << "creator x {exe list separate by whitespace} - replace rsa keys in patcher files" << std::endl;
    std::cout << "creator initial - build initial patch with version 1" << std::endl;
    std::cout << "creator new {x} - build new patch with selected, or next version" << std::endl;
    std::cout << "creator rebuild - rebuild md5 file list" << std::endl;
}
