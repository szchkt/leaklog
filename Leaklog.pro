CONFIG       += qt
TEMPLATE      = app
FORMS        += main_window.ui \
                about_widget.ui \
                import_dialogue.ui \
                navigation.ui
HEADERS      += global.h \
                defs.h \
                refrigerants.h \
                main.h \
                main_window.h \
                about_widget.h \
                extendedlineedit.h \
                mtwidgetpalettes.h \
                mtlistwidget.h \
                modify_dialogue.h \
                modify_warning_dialogue.h \
                mtdictionary.h \
                mtcolourcombobox.h \
                import_dialogue.h \
                mtaddress.h \
                mtrecord.h \
                mtsqlqueryresult.h \
                mtcheckboxgroup.h \
                input_widgets.h \
                records.h \
                report_data.h \
                report_data_controller.h \
                sha256.h \
                navigation.h \
                highlighter.h \
                variables.h \
                warnings.h \
                mtvariant.h \
                mtwebpage.h \
                mttextstream.h
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
                modify_warning_dialogue.cpp \
                mtaddress.cpp \
                mtrecord.cpp \
                mtsqlqueryresult.cpp \
                input_widgets.cpp \
                records.cpp \
                report_data.cpp \
                report_data_controller.cpp \
                sha256.cpp \
                navigation.cpp \
                highlighter.cpp \
                variables.cpp \
                warnings.cpp \
                mtwebpage.cpp
QT           += network webkit sql
# QTPLUGIN     += qsqlite qsqlpsql
# fparser
HEADERS      += fparser/fpconfig.hh fparser/fptypes.hh fparser/fparser.hh
SOURCES      += fparser/fpoptimizer.cc fparser/fparser.cc
DEFINES      += FP_NO_SUPPORT_OPTIMIZER
# *******
DESTDIR       = ./
win32 {
RC_FILE       = leaklog.rc
OBJECTS_DIR   = .build.win32
MOC_DIR       = .build.win32
RCC_DIR       = .build.win32
}
macx {
ICON          = images/leaklog128.icns
CONFIG       += x86
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.4
QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.5.sdk
}
unix {
OBJECTS_DIR   = .build.unix
MOC_DIR       = .build.unix
RCC_DIR       = .build.unix
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
