CONFIG       += qt
TEMPLATE      = app
FORMS        += main_window.ui \
                about_widget.ui
HEADERS      += main_window.h \
                about_widget.h \
                extendedlineedit.h \
                mtwidgetpalettes.h \
                mtlistwidget.h \
                modify_dialogue.h \
                modify_warning_dialogue.h \
                mtdictionary.h \
                i18n.h \
                mtcolourcombobox.h
RESOURCES    += resources.qrc \
                i18n.qrc \
                html.qrc \
                queries.qrc
SOURCES      += main.cpp \
                main_window.cpp \
                about_widget.cpp \
                document.cpp \
                view.cpp \
                modify_dialogue.cpp \
                modify_warning_dialogue.cpp
QT           += network webkit sql xml xmlpatterns
win32 {
DESTDIR       = ./
RC_FILE       = leaklog.rc
OBJECTS_DIR   = .tmp.win32/
MOC_DIR       = .tmp.win32/
RCC_DIR       = .tmp.win32/
}
macx {
ICON          = images/leaklog128.icns
CONFIG       += x86 ppc
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.4
QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.4u.sdk
}
unix:!macx {
DESTDIR       = ./
OBJECTS_DIR   = .tmp.unix/
MOC_DIR       = .tmp.unix/
RCC_DIR       = .tmp.unix/
exists(/usr/bin/apgcc) {
QMAKE_CC      = apgcc
}
exists(/usr/bin/apg++) {
QMAKE_CXX     = apg++
}
}
TRANSLATIONS += i18n/Leaklog-Slovak.ts
QMAKE_RESOURCE_FLAGS += -compress 9
