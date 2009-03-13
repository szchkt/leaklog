CONFIG       += qt
TEMPLATE      = app
FORMS        += main_window.ui \
                about_widget.ui \
                import_dialogue.ui
HEADERS      += global.h \
                refrigerants.h \
                main_window.h \
                about_widget.h \
                extendedlineedit.h \
                mtwidgetpalettes.h \
                mtlistwidget.h \
                modify_dialogue.h \
                modify_warning_dialogue.h \
                mtdictionary.h \
                mtcolourcombobox.h \
                import_dialogue.h
RESOURCES    += resources.qrc \
                i18n.qrc \
                html.qrc
SOURCES      += global.cpp \
                refrigerants.cpp \
                main.cpp \
                main_window.cpp \
                about_widget.cpp \
                database.cpp \
                view.cpp \
                modify_dialogue.cpp \
                modify_warning_dialogue.cpp
QT           += network webkit sql
# QTPLUGIN     += qsqlite qsqlpsql
# fparser
HEADERS      += fparser/fpconfig.hh fparser/fptypes.hh fparser/fparser.hh
SOURCES      += fparser/fpoptimizer.cc fparser/fparser.cc
DEFINES      += NO_SUPPORT_OPTIMIZER
# *******
DESTDIR       = ./
win32 {
RC_FILE       = leaklog.rc
OBJECTS_DIR   = .tmp.win32
MOC_DIR       = .tmp.win32
RCC_DIR       = .tmp.win32
}
macx {
ICON          = images/leaklog128.icns
CONFIG       += x86 ppc
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.4
QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.4u.sdk
}
unix {
OBJECTS_DIR   = .tmp.unix
MOC_DIR       = .tmp.unix
RCC_DIR       = .tmp.unix
}
unix:!macx {
exists(/usr/bin/apgcc) {
QMAKE_CC      = apgcc
}
exists(/usr/bin/apg++) {
QMAKE_CXX     = apg++
}
}
TRANSLATIONS += i18n/Leaklog-Slovak.ts
QMAKE_RESOURCE_FLAGS += -compress 9
