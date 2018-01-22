#include "Utils.h"

QString Utils::md5Hash(const QString &path)
{
    QCryptographicHash crypto(QCryptographicHash::Md5);
    QFile file(path);
    file.open(QFile::ReadOnly);
    while(!file.atEnd()) {
      crypto.addData(file.read(8388608));
    }
    return QString::fromUtf8(crypto.result().toHex().data());
}

zlib_entry Utils::zlibCompress(QByteArray data, const int &compressionLevel)
{
    uLongf len = compressBound(data.length());
    unsigned char* input = reinterpret_cast<unsigned char*>(data.data());
    z_Bytef *output = new z_Bytef[len];
    z_stream defstream;
    defstream.zalloc = Z_NULL;
    defstream.zfree = Z_NULL;
    defstream.opaque = Z_NULL;
    defstream.avail_in = (uInt)data.length(); // size of input, string + terminator
    defstream.next_in = input; // input char array
    defstream.avail_out = len; // size of output
    defstream.next_out = output; // output char array
    deflateInit(&defstream, compressionLevel);
    deflate(&defstream, Z_FINISH);
    deflateEnd(&defstream);
    zlib_entry e;
    e.file = output;
    e.size = defstream.total_out;
    return e;
}

QByteArray Utils::zlibDecompress(const QByteArray &data, const int &sizeDecompressed)
{
//    z_Bytef *buffer = new z_Bytef[sizeDecompressed];
//    unsigned long int uncompressLength = sizeDecompressed;
//    uncompress(buffer, &uncompressLength, reinterpret_cast<const unsigned char*>(data.data()), data.length());
//    QByteArray out = QByteArray::fromRawData(reinterpret_cast<char*>(buffer), sizeDecompressed);
//    return out;
    return data;
}

QString Utils::pathToBase64(QString path)
{
    QStringList parts = path.replace("\\", "/").split("/");
    QString out;
    foreach (const QString& part, parts) {
        out += "/" + toBase64(part);
    }
    out.replace("//", "/");
    return out;
}

QString Utils::toBase64(const QString &text)
{
    QByteArray b(text.toStdString().data());
    return QString::fromUtf8(b.toBase64());
}
