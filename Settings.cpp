#include "Settings.h"

QSettings *Settings::settings = NULL;

void Settings::init()
{
    settings = new QSettings(QCoreApplication::applicationDirPath() + QDir::separator() + "cpw.conf", QSettings::IniFormat);
}

QVariant Settings::get(const QString &key)
{
    return settings->value(QString("CPW/%1").arg(key));
}

void Settings::set(const QString &key, const QVariant &value)
{
    settings->setValue(QString("CPW/%1").arg(key), value);
    settings->sync();
}
