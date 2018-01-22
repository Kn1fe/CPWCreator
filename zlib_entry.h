#ifndef ZLIB_ENTRY_H
#define ZLIB_ENTRY_H

#include <QObject>

struct zlib_entry
{
    unsigned char *file = 0;
    int size = 0;
};

#endif // ZLIB_ENTRY_H
