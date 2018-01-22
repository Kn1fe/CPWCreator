#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QCoreApplication>
#include <QSettings>
#include <QDir>

class Settings : public QObject
{
    Q_OBJECT
public:
    explicit Settings(QObject *parent = nullptr) : QObject(parent) {}
    static void init();
    static QVariant get(const QString &key);
    static void set(const QString &key, const QVariant &value);

    static QSettings *settings;
};

#endif // SETTINGS_H
