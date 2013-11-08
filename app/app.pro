TEMPLATE = app
DESTDIR = ../bin
TARGET = bpv
QT += core \
    gui \
    opengl

#create directories for files generated by bin2h
!exists(include_gen):system(mkdir include_gen)
!exists(source_gen):system(mkdir source_gen)
!exists(include_gen/images.h){
  win*{
    system(..\\bin\\dir2hcpp images --header_dir=include_gen --source_dir=source_gen)
  }
  else {
    system(../bin/dir2hcpp images --header_dir=include_gen --source_dir=source_gen)
  }
}
!exists(include_gen/fonts.h){
  win*{
    system(..\\bin\\dir2hcpp fonts --header_dir=include_gen --source_dir=source_gen)
  }
  else {
    system(../bin/dir2hcpp fonts --header_dir=include_gen --source_dir=source_gen)
  }
}

HEADERS += include/textdialog.h \
    include/blankingdialog.h \
    include/colordialog.h \
    include/exportdialog.h \
    include/geometry.h \
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
    include/trajectorydialog.h \
    include_gen/images.h \
    include_gen/fonts.h
SOURCES += source/textdialog.cpp \
    source/blankingdialog.cpp \
    source/colordialog.cpp \
    source/exportdialog.cpp \
    source/geometry.cpp \
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
    source/trajectorydialog.cpp \
    source_gen/images.cpp \
    source_gen/fonts.cpp
INCLUDEPATH += ./include \
    ./include_gen
