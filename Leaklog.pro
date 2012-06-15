CONFIG        += qt warn_on
TEMPLATE       = app
FORMS         += ui/main_window.ui \
                 ui/about_widget.ui \
                 ui/import_dialogue.ui \
                 ui/navigation.ui \
                 ui/import_csv_dialogue.ui
HEADERS       += src/global.h \
                 src/defs.h \
                 src/refrigerants.h \
                 src/main.h \
                 src/main_window.h \
                 src/about_widget.h \
                 src/extendedlineedit.h \
                 src/mtwidgetpalettes.h \
                 src/mtlistwidget.h \
                 src/edit_dialogue.h \
                 src/edit_warning_dialogue.h \
                 src/mtdictionary.h \
                 src/mtcolourcombobox.h \
                 src/import_dialogue.h \
                 src/mtaddress.h \
                 src/mtrecord.h \
                 src/mtsqlqueryresult.h \
                 src/mtcheckboxgroup.h \
                 src/input_widgets.h \
                 src/records.h \
                 src/report_data.h \
                 src/report_data_controller.h \
                 src/sha256.h \
                 src/navigation.h \
                 src/highlighter.h \
                 src/variables.h \
                 src/warnings.h \
                 src/mtvariant.h \
                 src/mtwebpage.h \
                 src/mttextstream.h \
                 src/import_csv_dialogue.h \
                 src/permissions_dialogue.h \
                 src/tabbed_edit_dialogue.h \
                 src/dbfile.h \
                 src/htmlbuilder.h \
                 src/edit_dialogue_table.h \
                 src/edit_customer_dialogue.h \
                 src/variable_evaluation.h \
                 src/edit_circuit_dialogue.h \
                 src/edit_inspection_dialogue.h \
                 src/edit_assembly_record_dialogue.h \
                 src/edit_dialogue_table_groups.h \
                 src/edit_inspector_dialogue.h \
                 src/partner_widgets.h \
                 src/link_parser.h \
                 src/edit_circuit_dialogue_units_tab.h \
                 src/edit_circuit_dialogue_compressors_tab.h \
                 src/edit_inspection_dialogue_compressors.h \
                 src/edit_dialogue_widgets.h \
                 src/edit_dialogue_layout.h \
                 src/edit_inspection_dialogue_assembly_record_tab.h \
                 src/main_window_settings.h \
                 src/mtsqlquery.h \
                 src/edit_inspection_dialogue_access.h \
                 src/edit_dialogue_with_auto_id.h
RESOURCES     += rc/resources.qrc \
                 rc/i18n.qrc \
                 rc/html.qrc
SOURCES       += src/global.cpp \
                 src/refrigerants.cpp \
                 src/main.cpp \
                 src/main_window.cpp \
                 src/about_widget.cpp \
                 src/database.cpp \
                 src/view.cpp \
                 src/edit_dialogue.cpp \
                 src/edit_warning_dialogue.cpp \
                 src/import_dialogue.cpp \
                 src/mtaddress.cpp \
                 src/mtrecord.cpp \
                 src/mtsqlqueryresult.cpp \
                 src/input_widgets.cpp \
                 src/records.cpp \
                 src/report_data.cpp \
                 src/report_data_controller.cpp \
                 src/sha256.cpp \
                 src/navigation.cpp \
                 src/highlighter.cpp \
                 src/variables.cpp \
                 src/warnings.cpp \
                 src/mtvariant.cpp \
                 src/mtwebpage.cpp \
                 src/import_csv_dialogue.cpp \
                 src/permissions_dialogue.cpp \
                 src/tabbed_edit_dialogue.cpp \
                 src/dbfile.cpp \
                 src/htmlbuilder.cpp \
                 src/edit_dialogue_table.cpp \
                 src/edit_customer_dialogue.cpp \
                 src/variable_evaluation.cpp \
                 src/edit_circuit_dialogue.cpp \
                 src/edit_inspection_dialogue.cpp \
                 src/edit_assembly_record_dialogue.cpp \
                 src/edit_dialogue_table_groups.cpp \
                 src/edit_inspector_dialogue.cpp \
                 src/partner_widgets.cpp \
                 src/link_parser.cpp \
                 src/edit_circuit_dialogue_units_tab.cpp \
                 src/edit_circuit_dialogue_compressors_tab.cpp \
                 src/edit_inspection_dialogue_compressors.cpp \
                 src/edit_dialogue_widgets.cpp \
                 src/edit_dialogue_layout.cpp \
                 src/edit_inspection_dialogue_assembly_record_tab.cpp \
                 src/main_window_settings.cpp \
                 src/mtsqlquery.cpp \
                 src/edit_inspection_dialogue_access.cpp \
                 src/edit_dialogue_with_auto_id.cpp
QT            += network webkit sql
# QTPLUGIN      += qsqlite qsqlpsql
# fparser
HEADERS       += include/fparser/fpconfig.hh include/fparser/fptypes.hh include/fparser/fparser.hh
SOURCES       += include/fparser/fpoptimizer.cc include/fparser/fparser.cc
DEFINES       += FP_NO_SUPPORT_OPTIMIZER
# *******
# csvparser
HEADERS       += include/csvparser/mtcsvparser.h
SOURCES       += include/csvparser/mtcsvparser.cpp
# *******
DESTDIR        = bin
INCLUDEPATH   += src include
UI_HEADERS_DIR = ui/include
UI_SOURCES_DIR = ui
win32 {
RC_FILE        = rc/leaklog.rc
OBJECTS_DIR    = build/win32
MOC_DIR        = build/win32
RCC_DIR        = build/win32
}
macx {
ICON           = rc/images/leaklog.icns
CONFIG        += x86_64
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.5
# QMAKE_MAC_SDK  = /Developer/SDKs/MacOSX10.5.sdk
}
unix {
OBJECTS_DIR    = build/unix
MOC_DIR        = build/unix
RCC_DIR        = build/unix
}
unix:!macx {
exists(/usr/bin/apgcc) {
QMAKE_CC       = apgcc
}
exists(/usr/bin/apg++) {
QMAKE_CXX      = apg++
}
}
TRANSLATIONS  += rc/i18n/Leaklog-Slovak.ts
QMAKE_RESOURCE_FLAGS += -compress 9
