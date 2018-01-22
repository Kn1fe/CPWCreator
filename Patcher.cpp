#include "Patcher.h"

Patcher::Patcher(QObject *parent) : QObject(parent)
{
    new_dir = QCoreApplication::applicationDirPath() + QDir::separator() + Settings::get("InputFolder").toString();
    cpw_dir = QCoreApplication::applicationDirPath() + QDir::separator() + Settings::get("OutputFolder").toString();
    parts << "element" << "launcher" << "patcher";
    compressionLevel = Settings::get("ZlibCompressionLevel").toInt();
}

void Patcher::patchExeFile(const QString &path)
{
    std::cout << "Input file: " << path.toStdString() << std::endl;
    const QString sig = "-----BEGIN PUBLIC KEY-----";
    QFile qf(path);
    qf.open(QIODevice::ReadWrite);
    QByteArray data = qf.readAll();
    int beginIndex = data.indexOf(sig);
    std::cout << "Signature offset: " << beginIndex << std::endl;
    data.remove(beginIndex, Settings::get("RSAPub").toString().length());
    data.insert(beginIndex, Settings::get("RSAPub").toByteArray());
    qf.seek(0);
    qf.write(data);
    qf.close();
}

void Patcher::initial()
{
    Crypt::loadKeys();
    std::cout << "Removing CPW directory" << std::endl;
    QDir(cpw_dir).removeRecursively();
    QSqlQuery sql(Database::getInstance());
    sql.exec("DELETE FROM files");
    sql.exec("VACUUM");
    foreach (const QString &part, parts) {
        updateDirectory(part, 1);
        createMd5List(part);
        createVersionFile(part);
    }
    QString pid = new_dir + QDir::separator() + "patcher/server/pid.ini";
    if (QFile::exists(pid)) {
        QSettings p(pid, QSettings::IniFormat);
        int pv = p.value("Version/pid").toInt();
        QString outpid = cpw_dir + QDir::separator() + "info";
        QDir().mkpath(outpid);
        outpid += "/pid";
        QFile qf(outpid);
        qf.open(QIODevice::WriteOnly);
        qf.write(QByteArray::number(pv));
        qf.close();
    }
}

int Patcher::getTypeVersion(const QString &type)
{
    QSqlQuery sql(Database::getInstance());
    sql.prepare("SELECT revision FROM files WHERE type=? ORDER BY revision DESC LIMIT 1");
    sql.addBindValue(type);
    sql.exec();
    if (sql.next())
        return sql.value(0).toInt();
    return 1;
}

long Patcher::getUpdateSize(const QString &type, const int &from, const int &to)
{
    QSqlQuery sql(Database::getInstance());
    sql.prepare("SELECT size FROM files WHERE type=? AND revision BETWEEN ? AND ?");
    sql.addBindValue(type);
    sql.addBindValue(from);
    sql.addBindValue(to);
    sql.exec();
    long out = 0;
    while (sql.next())
        out += sql.value(0).toInt();
    return out;
}

void Patcher::createPatch(const int &version)
{
    loadDatabase();
    Crypt::loadKeys();
    foreach (const QString &part, parts) {
        if (updateDirectory(part, getTypeVersion(part) + version)) {
            createVLists(part);
            createMd5List(part);
            createVersionFile(part);
            int rf = Settings::get("RemoveFiles").toInt();
            if (rf == 1)
                removeFiles(new_dir + QDir::separator() + part);
            else if (rf == 2) {
                QDir(new_dir + QDir::separator() + part).removeRecursively();
                QDir().mkpath(new_dir + QDir::separator() + part);
            }
        }
    }
}

void Patcher::rebuild()
{
    Crypt::loadKeys();
    foreach (const QString &part, parts) {
        createVLists(part);
        createMd5List(part);
        createVersionFile(part);
    }
}

bool Patcher::updateDirectory(const QString &path, const int &version)
{
    QFutureSynchronizer<void> sync;
    //New folder
    QString fp = QDir::cleanPath(new_dir + QDir::separator() + path);
    //Cpw folder
    QString op = QDir::cleanPath(cpw_dir + QDir::separator() + path + QDir::separator() + path);
    std::cout << "Parse directory: " << path.toStdString() << std::endl;
    QDirIterator it(fp, QDir::Files, QDirIterator::Subdirectories);
    int ready = 0;
    while (it.hasNext()) {
        sync.addFuture(QtConcurrent::run(QThreadPool::globalInstance(), this, &Patcher::updateFile, it.next(), fp, op, path, version));
        std::cout << "\rFiles in action: " << ready;
         ++ready;
    }
    sync.waitForFinished();
    std::cout << std::endl;
    writeType(path);
    QSqlQuery sql(Database::getInstance());
    sql.prepare("SELECT * FROM files WHERE type=? AND revision=?");
    sql.addBindValue(path);
    sql.addBindValue(version);
    sql.exec();
    return sql.next();
}

void Patcher::updateFile(const QString &path, const QString &fp, const QString &op, const QString &type, const int &version)
{
    //New folder file path
    QString filepin = QDir::cleanPath(path);
    filepin.replace(fp, "");
    //Base64 path
    QString filepin_base64 = Utils::pathToBase64(filepin);
    QFile in(path);
    QFile out(op + QDir::separator() + filepin_base64);
    QFileInfo outi(out);
    if (QFile::exists(outi.absoluteFilePath()))
        QFile::remove(outi.absoluteFilePath());
    QDir().mkpath(outi.absolutePath());
    in.open(QIODevice::ReadOnly);
    out.open(QIODevice::WriteOnly);
    QDataStream out_stream(&out);
    out_stream.setByteOrder(QDataStream::LittleEndian);
    QByteArray buffer = in.readAll();
    int size = buffer.length();
    zlib_entry e = Utils::zlibCompress(buffer, compressionLevel);
    QByteArray cfile = QByteArray(reinterpret_cast<char*>(e.file), e.size);
    out_stream << size;
    out_stream.writeRawData(cfile.data(), cfile.length());
    in.close();
    out.close();
    QString md5 = Utils::md5Hash(outi.absoluteFilePath());
    filepin = QDir::cleanPath(filepin);
    QMutexLocker locker(&mutex);
    if (files[type].contains(filepin)) {
        if (files[type][filepin].md5 != md5) {
            files[type][filepin].revision = version;
            files[type][filepin].size = size;
            files[type][filepin].md5 = md5;
        }
    } else {
        //File on exists
        patcher_entry entry;
        entry.added = version;
        entry.size = size;
        entry.revision = version;
        entry.md5 = md5;
        entry.type = type;
        entry.file = filepin;
        files[type].insert(entry.file, entry);
    }
    delete e.file;
}

void Patcher::createVLists(const QString &path)
{
    std::cout << "Creating v-X.inc files" << std::endl;
    QString op = QDir::cleanPath(cpw_dir + QDir::separator() + path);
    QSqlQuery sql(Database::getInstance());
    sql.first();
    int currentVersion = 1;
    int version = getTypeVersion(path);
    while (currentVersion < version) {
        int v = version - currentVersion;
        sql.prepare("SELECT added, md5, file FROM files WHERE revision != 1 AND revision > ?");
        sql.addBindValue(currentVersion);
        sql.exec();
        if (!sql.next())
            break;
        else
            sql.previous();
        QString x = QDir::cleanPath(op + QDir::separator() + QString("v-%1.inc").arg(v));
        std::cout << QString("v-%1.inc").arg(v).toStdString() << std::endl;
        if (QFile::exists(x))
            QFile::remove(x);
        QFile qf(x);
        qf.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream stream(&qf);
        stream << QString("# %1 %2 %3").arg(currentVersion).arg(version).arg(getUpdateSize(path, currentVersion, version)) << endl;
        QString root_path = "155comeinfeb";
        while (sql.next()) {
            QString t = "!";
            if (sql.value(0).toInt() >= currentVersion)
                t = "+";
            QString md5 = sql.value(1).toString();
            QString file = Utils::pathToBase64(sql.value(2).toString());
            QString file_root = file;
            file_root.replace(file_root.split("/").last(), "");
            if (root_path != file_root) {
                stream << QString("%1%2 %3").arg(t).arg(md5).arg(file) << endl;
                root_path = file;
                root_path.replace(root_path.split("/").last(), "");
            } else {
                stream << QString("%1%2 %3").arg(t).arg(md5).arg(file.split("/").last()) << endl;
            }
        }
        qf.close();
        Crypt::signFile(x);
        ++currentVersion;
    }
}

void Patcher::createMd5List(const QString &path)
{
    std::cout << "Generate files.md5" << std::endl;
    QString op = QDir::cleanPath(cpw_dir + QDir::separator() + path);
    QString md5_path = QDir::cleanPath(op + QDir::separator() + "files.md5");
    if (QFile::exists(md5_path))
        QFile::remove(md5_path);
    QFile qf(md5_path);
    qf.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream stream(&qf);
    QSqlQuery sql(Database::getInstance());
    sql.prepare("SELECT md5, file FROM files WHERE type=? ORDER by file");
    sql.addBindValue(path);
    sql.exec();
    QString root_path = "155comeinfeb";
    stream << QString("# %1").arg(getTypeVersion(path)) << endl;
    while (sql.next()) {
        QString md5 = sql.value(0).toString();
        QString file = Utils::pathToBase64(sql.value(1).toString());
        QString file_root = file;
        file_root.replace(file_root.split("/").last(), "");
        if (root_path != file_root) {
            stream << QString("%1 %2").arg(md5).arg(file) << endl;
            root_path = file;
            root_path.replace(root_path.split("/").last(), "");
        } else {
            stream << QString("%1 %2").arg(md5).arg(file.split("/").last()) << endl;
        }
    }
    stream.flush();
    qf.close();
    Crypt::signFile(md5_path);
}

void Patcher::createVersionFile(const QString &path)
{
    QString op = QDir::cleanPath(cpw_dir + QDir::separator() + path);
    QString v = QDir::cleanPath(op + QDir::separator() + "version");
    if (QFile::exists(v))
        QFile::remove(v);
    QFile qf(v);
    qf.open(QIODevice::WriteOnly | QIODevice::Text);
    qf.write(QByteArray::number(getTypeVersion(path)));
    qf.close();
}

void Patcher::loadDatabase()
{
    std::cout << "Loading database..." << std::endl;
    QSqlQuery sql(Database::getInstance());
    sql.exec("SELECT * FROM files");
    while (sql.next()) {
        patcher_entry entry;
        entry.added = sql.value(0).toInt();
        entry.size = sql.value(1).toInt();
        entry.revision = sql.value(2).toInt();
        entry.md5 = sql.value(3).toString();
        entry.type = sql.value(4).toString();
        entry.file = sql.value(5).toString();
        files[entry.type].insert(entry.file, entry);
    }
}

void Patcher::writeType(const QString &type)
{
    std::cout << "Write to database: " << type.toStdString() << std::endl;
    QSqlQuery sql(Database::getInstance());
    sql.exec("DELETE FROM files WHERE type=?");
    sql.addBindValue(type);
    sql.exec();
    sql.exec("VACUUM");
    QHashIterator<QString, patcher_entry> i(files[type]);
    while (i.hasNext()) {
        QHash<QString, patcher_entry>::const_iterator e = i.next();
        patcher_entry entry = e.value();
        sql.prepare("INSERT INTO files VALUES (?, ?, ?, ?, ?, ?)");
        sql.addBindValue(entry.added);
        sql.addBindValue(entry.size);
        sql.addBindValue(entry.revision);
        sql.addBindValue(entry.md5);
        sql.addBindValue(entry.type);
        sql.addBindValue(QDir::cleanPath(entry.file));
        sql.exec();
    }
}

void Patcher::removeFiles(const QString &path)
{
    QDirIterator it(path, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
        QFile::remove(it.next());
}
