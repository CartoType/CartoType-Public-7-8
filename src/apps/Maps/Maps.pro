#-------------------------------------------------
#
# Project created by QtCreator 2015-10-20T10:53:32
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CartoTypeMaps
TEMPLATE = app

CONFIG += c++14

DEFINES += NDEBUG \
           SQLITE_ENABLE_RTREE \
           SQLITE_RTREE_INT_ONLY \
           SQLITE_THREADSAFE=0 \
           XML_STATIC

INCLUDEPATH += ../../main/base \
    ../../main/library/rapidxml-1.13 \
    ../../main/platform/qt/CartoTypeQtMapRenderer

SOURCES += main.cpp\
    mainwindow.cpp \
    mapform.cpp \
    mapchildwindow.cpp \
    routedialog.cpp \
    finddialog.cpp \
    findaddressdialog.cpp \
    scaledialog.cpp \
    locationdialog.cpp \
    layerdialog.cpp \
    routeinstructionsdialog.cpp \
    routeprofiledialog.cpp \
    styledialog.cpp \
    stylemodel.cpp \
    styleitem.cpp \
    linestyledialog.cpp \
    util.cpp \
    polygonstyledialog.cpp \
    svgdialog.cpp \
    attributedialog.cpp

HEADERS += mainwindow.h \
    ../../main/base/cartotype_address.h \
    ../../main/base/cartotype_arithmetic.h \
    ../../main/base/cartotype_base.h \
    ../../main/base/cartotype_bidi.h \
    ../../main/base/cartotype_bitmap.h \
    ../../main/base/cartotype_char.h \
    ../../main/base/cartotype_color.h \
    ../../main/base/cartotype_epsg.h \
    ../../main/base/cartotype_errors.h \
    ../../main/base/cartotype_expression.h \
    ../../main/base/cartotype_find_param.h \
    ../../main/base/cartotype_framework.h \
    ../../main/base/cartotype_graphics_context.h \
    ../../main/base/cartotype_iter.h \
    ../../main/base/cartotype_legend.h \
    ../../main/base/cartotype_list.h \
    ../../main/base/cartotype_map_object.h \
    ../../main/base/cartotype_navigation.h \
    ../../main/base/cartotype_path.h \
    ../../main/base/cartotype_road_type.h \
    ../../main/base/cartotype_stream.h \
    ../../main/base/cartotype_string.h \
    ../../main/base/cartotype_transform.h \
    ../../main/base/cartotype_types.h \
    ../../main/base/pstdint.h \
    mapform.h \
    mapchildwindow.h \
    routedialog.h \
    finddialog.h \
    findaddressdialog.h \
    scaledialog.h \
    locationdialog.h \
    layerdialog.h \
    routeinstructionsdialog.h \
    routeprofiledialog.h \
    styledialog.h \
    ../../main/library/rapidxml-1.13/rapidxml.hpp \
    ../../main/library/rapidxml-1.13/rapidxml_iterators.hpp \
    ../../main/library/rapidxml-1.13/rapidxml_print.hpp \
    ../../main/library/rapidxml-1.13/rapidxml_utils.hpp \
    util.h \
    stylemodel.h \
    styleitem.h \
    linestyledialog.h \
    polygonstyledialog.h \
    svgdialog.h \
    attributedialog.h

FORMS    += mainwindow.ui \
    mapform.ui \
    routedialog.ui \
    finddialog.ui \
    findaddressdialog.ui \
    scaledialog.ui \
    locationdialog.ui \
    layerdialog.ui \
    routeinstructionsdialog.ui \
    routeprofiledialog.ui \
    styledialog.ui \
    linestyledialog.ui \
    polygonstyledialog.ui \
    svgdialog.ui \
    attributedialog.ui

RESOURCES += \
    icons.qrc

unix:!macx: LIBS += -ldl

win32: RC_ICONS = CT_Arrow-RGB.ico

ICON = CartoTypeMacMaps.icns

QT += printsupport
QT += gui

win32:
{
CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../bin/17.0/x64/DebugDLL/ -lcartotype -L$$PWD/../../main/platform/qt/CartoTypeQtMapRenderer/build64/debug -lCartoTypeQtMapRenderer
else:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../bin/17.0/x64/ReleaseDLL/ -lcartotype -L$$PWD/../../main/platform/qt/CartoTypeQtMapRenderer/build64/release -lCartoTypeQtMapRenderer
}

unix:!macx: LIBS += -L$$PWD/../../main/platform/qt/CartoTypeQtMapRenderer/build64/release/ -lCartoTypeQtMapRenderer -L$$PWD/../../main/single_library/unix/bin/ReleaseLicensed/ -lcartotype

unix:!macx: PRE_TARGETDEPS += $$PWD/../../main/platform/qt/CartoTypeQtMapRenderer/build64/release/libCartoTypeQtMapRenderer.a $$PWD/../../main/single_library/unix/bin/ReleaseLicensed/libcartotype.a

macx: LIBS += -L$$PWD/../../main/platform/qt/CartoTypeQtMapRenderer/build64/release/ -lCartoTypeQtMapRenderer -L$$PWD/../../main/single_library/mac/CartoType/build/Release/ -lCartoType

macx: PRE_TARGETDEPS += $$PWD/../../main/platform/qt/CartoTypeQtMapRenderer/build64/release/libCartoTypeQtMapRenderer.a $$PWD/../../main/single_library/mac/CartoType/build/Release/libCartoType.a
