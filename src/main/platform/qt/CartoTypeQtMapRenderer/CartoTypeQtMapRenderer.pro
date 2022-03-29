#-------------------------------------------------
#
# Project created by QtCreator 2017-11-20T15:25:45
#
#-------------------------------------------------

TARGET = CartoTypeQtMapRenderer
TEMPLATE = lib
CONFIG += staticlib
CONFIG += c++14

DEFINES += CARTOTYPE_FOR_QT
DEFINES += NDEBUG

INCLUDEPATH += ../../../base \
    ../../../framework \
    ../../../graphics \
    ../../../mapdata \
    ../../../svg \
    ../../../xml \
    ../../../library/libtess2

SOURCES += cartotype_qt_map_renderer.cpp \
    ../../../graphics/cartotype_opengl_es_render.cpp \
    ../../../library/libtess2/bucketalloc.c \
    ../../../library/libtess2/dict.c \
    ../../../library/libtess2/geom.c \
    ../../../library/libtess2/mesh.c \
    ../../../library/libtess2/priorityq.c \
    ../../../library/libtess2/sweep.c \
    ../../../library/libtess2/tess.c

HEADERS += cartotype_qt_map_renderer.h

unix
    {
    target.path = /usr/lib
    INSTALLS += target
    }
