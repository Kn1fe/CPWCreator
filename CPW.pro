QT -= gui
QT += sql concurrent

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

TARGET = creator

SOURCES += main.cpp \
    Patcher.cpp \
    Settings.cpp \
    Crypt.cpp \
    Utils.cpp \
    Database.cpp

HEADERS += \
    Patcher.h \
    Settings.h \
    Crypt.h \
    Utils.h \
    Database.h \
    patcher_entry.h \
    zlib_entry.h

win32 {
    INCLUDEPATH += $$PWD/library/win/include/
    LIBS += -L$$PWD/library/win/lib/ -lssleay32
    LIBS += -L$$PWD/library/win/lib/ -llibeay32
    LIBS += -lgdi32
}
linux-g++-32 {
    INCLUDEPATH += $$PWD/library/linux/include/
    LIBS += -L$$PWD/library/linux/lib/ -lssl
    LIBS += -L$$PWD/library/linux/lib/ -lcrypto
    QMAKE_LFLAGS += -static-libstdc++ -static-libgcc -static
}

