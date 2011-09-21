TEMPLATE = app
TARGET = bpv
QT += core \
    gui \
    opengl
HEADERS += include/textdialog.h \
    include/blankingdialog.h \
    include/colordialog.h \
    include/exportdialog.h \
    include/geometry.h \
    include/gl2ps.h \
    include/glgeometry.h \
    include/glwidget.h \
    include/infowidgetbgd.h \
    include/infowidgetbnd.h \
    include/infowidgetgeo.h \
    include/infowidgetpar.h \
    include/mycolor.h \
    include/mymainwin.h \
    include/mywidget.h \
    include/particles.h \
    include/rotatedialog.h \
    include/trajectorydialog.h
SOURCES += source/textdialog.cpp \
    source/blankingdialog.cpp \
    source/colordialog.cpp \
    source/exportdialog.cpp \
    source/geometry.cpp \
    source/gl2ps.c \
    source/glgeometry.cpp \
    source/glwidget.cpp \
    source/infowidgetbgd.cpp \
    source/infowidgetbnd.cpp \
    source/infowidgetgeo.cpp \
    source/infowidgetpar.cpp \
    source/main.cpp \
    source/mycolor.cpp \
    source/mymainwin.cpp \
    source/mywidget.cpp \
    source/particles.cpp \
    source/rotatedialog.cpp \
    source/trajectorydialog.cpp
INCLUDEPATH += ./include
