include($$PWD/../../SQLiteStudio3/plugins.pri)

QT += widgets

TARGET = MultiEditorVarints
TEMPLATE = lib

DEFINES += MULTIEDITORVARINTS_LIBRARY

SOURCES += multieditorvarints.cpp

HEADERS += multieditorvarints.h \
        multieditorvarints_global.h

OTHER_FILES += \
    multieditorvarints.json

RESOURCES += \
    multieditorvarints.qrc

TRANSLATIONS += translations/MultiEditorVarints.ts
