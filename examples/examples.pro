TEMPLATE = app
DESTDIR = ../bin
TARGET = examples
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    source/main.c \
    source/particles.c \
    source/box.c \
    source/benzene.c \
    source/wave.c

HEADERS += \
    include/particles.h \
    include/box.h \
    include/benzene.h \
    include/wave.h

INCLUDEPATH += \
    ./include
