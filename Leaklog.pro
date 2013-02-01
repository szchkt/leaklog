CONFIG        += qt warn_on
TEMPLATE       = app
FORMS         += ui/aboutwidget.ui \
                 ui/importcsvdialogue.ui \
                 ui/importdialogue.ui \
                 ui/mainwindow.ui \
                 ui/navigation.ui
HEADERS       += src/aboutwidget.h \
                 src/dbfile.h \
                 src/defs.h \
                 src/editassemblyrecorddialogue.h \
                 src/editcircuitdialogue.h \
                 src/editcircuitdialoguecompressorstab.h \
                 src/editcircuitdialogueunitstab.h \
                 src/editcustomerdialogue.h \
                 src/editdialogue.h \
                 src/editdialoguelayout.h \
                 src/editdialoguetable.h \
                 src/editdialoguetablegroups.h \
                 src/editdialoguewidgets.h \
                 src/editdialoguewithautoid.h \
                 src/editinspectiondialogue.h \
                 src/editinspectiondialogueaccess.h \
                 src/editinspectiondialogueassemblyrecordtab.h \
                 src/editinspectiondialoguecompressors.h \
                 src/editinspectiondialoguelayout.h \
                 src/editinspectordialogue.h \
                 src/editwarningdialogue.h \
                 src/extendedlineedit.h \
                 src/global.h \
                 src/highlighter.h \
                 src/htmlbuilder.h \
                 src/importcsvdialogue.h \
                 src/importdialogue.h \
                 src/inputwidgets.h \
                 src/linkparser.h \
                 src/main.h \
                 src/mainwindow.h \
                 src/mainwindowsettings.h \
                 src/mtaddress.h \
                 src/mtcheckboxgroup.h \
                 src/mtcolourcombobox.h \
                 src/mtdictionary.h \
                 src/mtlistwidget.h \
                 src/mtrecord.h \
                 src/mtsqlquery.h \
                 src/mtsqlqueryresult.h \
                 src/mttextstream.h \
                 src/mtvariant.h \
                 src/mtwebpage.h \
                 src/mtwidgetpalettes.h \
                 src/navigation.h \
                 src/partnerwidgets.h \
                 src/permissionsdialogue.h \
                 src/records.h \
                 src/refrigerants.h \
                 src/reportdata.h \
                 src/reportdatacontroller.h \
                 src/sha256.h \
                 src/tabbededitdialogue.h \
                 src/variableevaluation.h \
                 src/variables.h \
                 src/warnings.h
RESOURCES     += rc/html.qrc \
                 rc/i18n.qrc \
                 rc/resources.qrc
SOURCES       += src/aboutwidget.cpp \
                 src/database.cpp \
                 src/dbfile.cpp \
                 src/editassemblyrecorddialogue.cpp \
                 src/editcircuitdialogue.cpp \
                 src/editcircuitdialoguecompressorstab.cpp \
                 src/editcircuitdialogueunitstab.cpp \
                 src/editcustomerdialogue.cpp \
                 src/editdialogue.cpp \
                 src/editdialoguelayout.cpp \
                 src/editdialoguetable.cpp \
                 src/editdialoguetablegroups.cpp \
                 src/editdialoguewidgets.cpp \
                 src/editdialoguewithautoid.cpp \
                 src/editinspectiondialogue.cpp \
                 src/editinspectiondialogueaccess.cpp \
                 src/editinspectiondialogueassemblyrecordtab.cpp \
                 src/editinspectiondialoguecompressors.cpp \
                 src/editinspectiondialoguelayout.cpp \
                 src/editinspectordialogue.cpp \
                 src/editwarningdialogue.cpp \
                 src/global.cpp \
                 src/highlighter.cpp \
                 src/htmlbuilder.cpp \
                 src/importcsvdialogue.cpp \
                 src/importdialogue.cpp \
                 src/inputwidgets.cpp \
                 src/linkparser.cpp \
                 src/main.cpp \
                 src/mainwindow.cpp \
                 src/mainwindowsettings.cpp \
                 src/mtaddress.cpp \
                 src/mtrecord.cpp \
                 src/mtsqlquery.cpp \
                 src/mtsqlqueryresult.cpp \
                 src/mtvariant.cpp \
                 src/mtwebpage.cpp \
                 src/navigation.cpp \
                 src/partnerwidgets.cpp \
                 src/permissionsdialogue.cpp \
                 src/records.cpp \
                 src/refrigerants.cpp \
                 src/reportdata.cpp \
                 src/reportdatacontroller.cpp \
                 src/sha256.cpp \
                 src/tabbededitdialogue.cpp \
                 src/variableevaluation.cpp \
                 src/variables.cpp \
                 src/view.cpp \
                 src/warnings.cpp
lessThan(QT_MAJOR_VERSION, 5) {
QT            += network webkit sql
}
else {
QT            += widgets network webkitwidgets sql
}
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
LIBS          += -framework Foundation
OBJECTIVE_SOURCES += src/mainwindowmacx.mm
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
