#ifndef PATCHER_ENTRY_H
#define PATCHER_ENTRY_H

#include <QObject>

struct patcher_entry
{
    int added = 0;
    int size = 0;
    int revision = 0;
    QString md5 = "";
    QString type = "";
    QString file = "";
};

#endif // PATCHER_ENTRY_H
