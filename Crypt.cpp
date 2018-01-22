#include "Crypt.h"

RSA *Crypt::keys = NULL;

void Crypt::generateKeys()
{
    RSA *r = RSA_new();
    BIGNUM *bne = BN_new();
    QString tmp = QDir::tempPath() + QDir::separator() + QUuid::createUuid().toString();
    BN_set_word(bne, RSA_F4);
    RSA_generate_key_ex(r, 1024, bne, NULL);
    //Private key
    BIO *bp_private = BIO_new_file(tmp.toStdString().data(), "w+");
    PEM_write_bio_RSAPrivateKey(bp_private, r, NULL, NULL, 0, NULL, NULL);
    BIO_free_all(bp_private);
    Settings::set("RSAPrivate", readFromFile(tmp));
    //Public key
    BIO *bp_public = BIO_new_file(tmp.toStdString().data(), "w+");
    PEM_write_bio_RSAPublicKey(bp_public, r);
    BIO_free_all(bp_public);
    Settings::set("RSAPublic", readFromFile(tmp));
    //Pub key
    BIO *bp_pub = BIO_new_file(tmp.toStdString().data(), "w+");
    PEM_write_bio_RSA_PUBKEY(bp_pub, r);
    BIO_free_all(bp_pub);
    Settings::set("RSAPub", readFromFile(tmp));
    RSA_free(r);
    BN_free(bne);
    if (QFile::exists(tmp))
        QFile::remove(tmp);
}

void Crypt::loadKeys()
{
    keys = RSA_new();
    BIO *mem = BIO_new(BIO_s_mem());
    BIO_puts(mem, Settings::get("RSAPrivate").toString().toStdString().data());
    keys = PEM_read_bio_RSAPrivateKey(mem, NULL, NULL, NULL);
    BIO_free(mem);
}

void Crypt::signFile(const QString &path)
{
    std::cout << "Sign file: " << path.toStdString() << std::endl;
    QFile qf(path);
    qf.open(QIODevice::ReadWrite | QIODevice::Text);
    QByteArray data = qf.readAll();
    QTextStream stream(&qf);
    stream << QByteArray("-----BEGIN ELEMENT SIGNATURE-----") << endl;
    unsigned char *sigret = new unsigned char[2048];
    unsigned int siglen = 0;
    unsigned char digest[2048];
    unsigned int dlen;
    const EVP_MD *evp_md = EVP_get_digestbyname("MD5");
    EVP_MD_CTX md;
    EVP_DigestInit(&md, evp_md);
    EVP_DigestUpdate(&md, (unsigned char*)data.data(), data.length());
    EVP_DigestFinal(&md, digest, &dlen);
    RSA_sign(NID_md5, digest, dlen, sigret, &siglen, keys);
    QString signature = QByteArray((char*)sigret, siglen).toBase64();
    signature.insert(128, "\n");
    signature.insert(64, "\n");
    stream << signature;
    qf.flush();
    qf.close();
}

QString Crypt::readFromFile(const QString &path)
{
    QFile qf(path);
    qf.open(QIODevice::ReadOnly);
    QTextStream text(&qf);
    return text.readAll();
}
