TEMPLATE = app
CONFIG -= moc app_bundle
CONFIG += console
INCLUDEPATH += .

# Input
SOURCES += main.cpp configutils.cpp
HEADERS += configutils.h

CONFIG -= debug_and_release
build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}

QT = core
