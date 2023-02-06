CONFIG -= qt

TEMPLATE = lib

CONFIG += staticlib
CONFIG += c++17

INCLUDEPATH += /usr/local/include
INCLUDEPATH += src

SOURCES += \
    src/RandomNameGenerator.cpp

HEADERS += \
    src/RandomNameGenerator.h

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target
