QT          += core gui serialport widgets

CONFIG      += qscintilla2

DEFINES += QT_DEPRECATED_WARNINGS QSCINTILLA_DLL

TARGET      = serial-tool
TEMPLATE    = app

DESTDIR     = $$PWD/build

include(../libs/libs.pri)
include(sessions/sessions.pri)
include(common/common.pri)

SOURCES += \
    AboutDialog.cpp \
    MainWindow.cpp \
    SettingsDialog.cpp \
    main.cpp

FORMS += \
    AboutDialog.ui \
    MainWindow.ui \
    SettingsDialog.ui

HEADERS += \
    AboutDialog.h \
    MainWindow.h \
    SettingsDialog.h

RC_ICONS = resources/icon.ico

RESOURCES += \
    resources/serial-tool.qrc

TRANSLATIONS += \
    resources/translations/serial-tool_zh.ts \
    resources/translations/serial-tool_en.ts
