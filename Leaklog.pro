CONFIG        += qt warn_on
TEMPLATE       = app
FORMS         += ui/aboutwidget.ui \
                 ui/importcsvdialogue.ui \
                 ui/importdialogue.ui \
                 ui/mainwindow.ui \
                 ui/toolbarstack.ui \
                 ui/viewtab.ui
HEADERS       += src/aboutwidget.h \
                 src/activityeventfilter.h \
                 src/companyidvalidator.h \
                 src/dbfile.h \
                 src/dbrecord.h \
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
                 src/editinspectiondialogue.h \
                 src/editinspectiondialogueaccess.h \
                 src/editinspectiondialogueassemblyrecordtab.h \
                 src/editinspectiondialoguecompressors.h \
                 src/editinspectiondialoguelayout.h \
                 src/editinspectordialogue.h \
                 src/editwarningdialogue.h \
                 src/expression.h \
                 src/searchlineedit.h \
                 src/global.h \
                 src/highlighter.h \
                 src/htmlbuilder.h \
                 src/importcsvdialogue.h \
                 src/importdialogue.h \
                 src/inputwidgets.h \
                 src/leakagesbyapplication.h \
                 src/linkparser.h \
                 src/main.h \
                 src/mainwindow.h \
                 src/mainwindowsettings.h \
                 src/migrations.h \
                 src/models/servicecompany.h \
                 src/models/customer.h \
                 src/models/person.h \
                 src/models/circuit.h \
                 src/models/compressor.h \
                 src/models/inspection.h \
                 src/models/inspectioncompressor.h \
                 src/models/inspectionfile.h \
                 src/models/repair.h \
                 src/models/inspector.h \
                 src/models/variable.h \
                 src/models/table.h \
                 src/models/warning.h \
                 src/models/warningfilter.h \
                 src/models/warningcondition.h \
                 src/models/refrigerantrecord.h \
                 src/models/assemblyrecordtype.h \
                 src/models/assemblyrecorditemtype.h \
                 src/models/assemblyrecordtypecategory.h \
                 src/models/assemblyrecorditemcategory.h \
                 src/models/assemblyrecorditem.h \
                 src/models/file.h \
                 src/models/circuitunittype.h \
                 src/models/circuitunit.h \
                 src/models/dbinfo.h \
                 src/models/style.h \
                 src/models/journalentry.h \
                 src/mtaddress.h \
                 src/mtcheckboxgroup.h \
                 src/mtcolourcombobox.h \
                 src/mtdictionary.h \
                 src/mtlistwidget.h \
                 src/mtquery.h \
                 src/mtrecord.h \
                 src/mtsqlquery.h \
                 src/mtsqlqueryresult.h \
                 src/mttabwidget.h \
                 src/mttextstream.h \
                 src/mtvariant.h \
                 src/mtwebpage.h \
                 src/mtwidget.h \
                 src/mtwidgetpalettes.h \
                 src/partnerwidgets.h \
                 src/permissionsdialogue.h \
                 src/records.h \
                 src/refprop.h \
                 src/removedialogue.h \
                 src/reportdata.h \
                 src/reportdatacontroller.h \
                 src/sha256.h \
                 src/syncengine.h \
                 src/tabbededitdialogue.h \
                 src/toolbarstack.h \
                 src/undostack.h \
                 src/variableevaluation.h \
                 src/variables.h \
                 src/view.h \
                 src/views/agendaview.h \
                 src/views/assemblyrecorddetailsview.h \
                 src/views/assemblyrecorditemsview.h \
                 src/views/assemblyrecordsview.h \
                 src/views/assemblyrecordtypesview.h \
                 src/views/circuitsview.h \
                 src/views/circuitunittypesview.h \
                 src/views/customersview.h \
                 src/views/inspectiondetailsview.h \
                 src/views/inspectionimagesview.h \
                 src/views/inspectionsview.h \
                 src/views/inspectordetailsview.h \
                 src/views/inspectorsview.h \
                 src/views/leakagesbyapplicationview.h \
                 src/views/operatorreportview.h \
                 src/views/refrigerantmanagementview.h \
                 src/views/repairsview.h \
                 src/views/storeview.h \
                 src/views/tableview.h \
                 src/viewtab.h \
                 src/viewtabsettings.h \
                 src/warnings.h
RESOURCES     += rc/html.qrc \
                 rc/i18n.qrc \
                 rc/resources.qrc
SOURCES       += src/aboutwidget.cpp \
                 src/activityeventfilter.cpp \
                 src/companyidvalidator.cpp \
                 src/database.cpp \
                 src/dbfile.cpp \
                 src/dbrecord.cpp \
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
                 src/editinspectiondialogue.cpp \
                 src/editinspectiondialogueaccess.cpp \
                 src/editinspectiondialogueassemblyrecordtab.cpp \
                 src/editinspectiondialoguecompressors.cpp \
                 src/editinspectiondialoguelayout.cpp \
                 src/editinspectordialogue.cpp \
                 src/editwarningdialogue.cpp \
                 src/expression.cpp \
                 src/global.cpp \
                 src/highlighter.cpp \
                 src/htmlbuilder.cpp \
                 src/importcsvdialogue.cpp \
                 src/importdialogue.cpp \
                 src/inputwidgets.cpp \
                 src/leakagesbyapplication.cpp \
                 src/linkparser.cpp \
                 src/main.cpp \
                 src/mainwindow.cpp \
                 src/mainwindowsettings.cpp \
                 src/migrations.cpp \
                 src/models/servicecompany.cpp \
                 src/models/customer.cpp \
                 src/models/person.cpp \
                 src/models/circuit.cpp \
                 src/models/compressor.cpp \
                 src/models/inspection.cpp \
                 src/models/inspectioncompressor.cpp \
                 src/models/inspectionfile.cpp \
                 src/models/repair.cpp \
                 src/models/inspector.cpp \
                 src/models/variable.cpp \
                 src/models/table.cpp \
                 src/models/warning.cpp \
                 src/models/warningfilter.cpp \
                 src/models/warningcondition.cpp \
                 src/models/refrigerantrecord.cpp \
                 src/models/assemblyrecordtype.cpp \
                 src/models/assemblyrecorditemtype.cpp \
                 src/models/assemblyrecordtypecategory.cpp \
                 src/models/assemblyrecorditemcategory.cpp \
                 src/models/assemblyrecorditem.cpp \
                 src/models/file.cpp \
                 src/models/circuitunittype.cpp \
                 src/models/circuitunit.cpp \
                 src/models/dbinfo.cpp \
                 src/models/style.cpp \
                 src/models/journalentry.cpp \
                 src/mtaddress.cpp \
                 src/mtquery.cpp \
                 src/mtrecord.cpp \
                 src/mtsqlquery.cpp \
                 src/mtsqlqueryresult.cpp \
                 src/mttabwidget.cpp \
                 src/mtvariant.cpp \
                 src/mtwebpage.cpp \
                 src/mtwidget.cpp \
                 src/partnerwidgets.cpp \
                 src/permissionsdialogue.cpp \
                 src/refprop.cpp \
                 src/reportdata.cpp \
                 src/reportdatacontroller.cpp \
                 src/sha256.cpp \
                 src/syncengine.cpp \
                 src/tabbededitdialogue.cpp \
                 src/toolbarstack.cpp \
                 src/undostack.cpp \
                 src/variableevaluation.cpp \
                 src/variables.cpp \
                 src/view.cpp \
                 src/views/agendaview.cpp \
                 src/views/assemblyrecorddetailsview.cpp \
                 src/views/assemblyrecorditemsview.cpp \
                 src/views/assemblyrecordsview.cpp \
                 src/views/assemblyrecordtypesview.cpp \
                 src/views/circuitsview.cpp \
                 src/views/circuitunittypesview.cpp \
                 src/views/customersview.cpp \
                 src/views/inspectiondetailsview.cpp \
                 src/views/inspectionimagesview.cpp \
                 src/views/inspectionsview.cpp \
                 src/views/inspectordetailsview.cpp \
                 src/views/inspectorsview.cpp \
                 src/views/leakagesbyapplicationview.cpp \
                 src/views/operatorreportview.cpp \
                 src/views/refrigerantmanagementview.cpp \
                 src/views/repairsview.cpp \
                 src/views/storeview.cpp \
                 src/views/tableview.cpp \
                 src/viewtab.cpp \
                 src/viewtabsettings.cpp \
                 src/warnings.cpp

QT                += widgets network webenginewidgets sql printsupport
CONFIG            += c++11

# fparser
HEADERS           += include/fparser/fparser_gmpint.hh include/fparser/fparser_mpfr.hh include/fparser/fparser.hh include/fparser/fpconfig.hh
HEADERS           += include/fparser/extrasrc/fpaux.hh include/fparser/extrasrc/fptypes.hh
SOURCES           += include/fparser/fparser.cc include/fparser/fpoptimizer.cc
DEFINES           += FP_NO_SUPPORT_OPTIMIZER

# csvparser
HEADERS           += include/csvparser/mtcsvparser.h
SOURCES           += include/csvparser/mtcsvparser.cpp

DESTDIR            = bin
INCLUDEPATH       += src src/models src/views include
UI_DIR             = ui/include

win32 {
    RC_FILE        = rc/leaklog.rc
    OBJECTS_DIR    = build/win32
    MOC_DIR        = build/win32
    RCC_DIR        = build/win32
}

win32-msvc:greaterThan(QT_MAJOR_VERSION, 5) {
    QMAKE_LFLAGS  += /ENTRY:mainCRTStartup
}

win32-g++ {
    LIBS          += C:\MinGW\lib\libgdi32.a
}

macx {
    QMAKE_INFO_PLIST = Info.plist
    LIBS          += -framework Foundation
    OBJECTIVE_SOURCES += src/mainwindowmacx.mm
    ICON           = rc/images/leaklog.icns
    icons.path     = Contents/Resources
    icons.files    = rc/images/lklg.icns
    QMAKE_BUNDLE_DATA += icons
    CONFIG        += x86_64
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.14
    QMAKE_CC       = clang
    QMAKE_CXX      = clang++
    QMAKE_LINK     = clang++
    QMAKE_CXXFLAGS += -stdlib=libc++
    QMAKE_LFLAGS   += -stdlib=libc++
}

# Qt 6.5+
macx:greaterThan(QT_MAJOR_VERSION, 5):greaterThan(QT_MINOR_VERSION, 4) {
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 11.0
}

macx:greaterThan(QT_MAJOR_VERSION, 5):CONFIG(release) {
    QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64
}

win32-msvc:greaterThan(QMAKE_MSC_VER, 1929) {
    QMAKE_CXXFLAGS += /std:c++17
} else:win32-msvc {
    QMAKE_CXXFLAGS += /std:c++14
} else {
    QMAKE_CXXFLAGS += -std=c++17
}

unix {
    OBJECTS_DIR    = build/unix
    MOC_DIR        = build/unix
    RCC_DIR        = build/unix
}

defineTest(copyResourceFolder) {
    folder = $$1
    source = $$2
    macx {
        destination = $$quote(Contents/Resources/$$3)
    } else {
        destination = $$quote($$OUT_PWD/$$DESTDIR/$$3)
    }
    win32 {
        source      ~= s,/,\\,g
        destination ~= s,/,\\,g
    }

    macx {
        eval($${folder}.path           = $$destination)
        export($${folder}.path)
        eval($${folder}.files          = $$files($$source/*))
        export($${folder}.files)
        QMAKE_BUNDLE_DATA += $${folder}
        export(QMAKE_BUNDLE_DATA)
    } else {
        QMAKE_EXTRA_TARGETS           += $$folder
        export(QMAKE_EXTRA_TARGETS)
        POST_TARGETDEPS               += $$folder
        export(POST_TARGETDEPS)

        eval($${folder}.target         = $$folder)
        export($${folder}.target)
        eval($${folder}.depends        = $$files($$PWD/$$source/*))
        export($${folder}.depends)
        win32 {
            pwd                        = $$PWD
            pwd                       ~= s,/,\\,g
            eval($${folder}.commands   = $$quote((if not exist \"$$destination\". mkdir \"$$destination\") &&))
            eval($${folder}.commands  += $$quote($$QMAKE_COPY $$pwd\\$$source\\* \"$$destination\"))
        } else {
            eval($${folder}.commands   = $$quote(mkdir -p \'$$destination\';))
            eval($${folder}.commands  += $$quote($$QMAKE_COPY $$eval($${folder}.depends) \'$$destination\'))
        }
        export($${folder}.commands)
    }
}

# refprop
refprop:exists(refprop/include/refprop_lib.h) {
    message("Configuring with RefProp...")
    DEFINES       += REFPROP
    INCLUDEPATH   += refprop/include
    macx:LIBS     += $$PWD/refprop/macx/librefprop.a $$PWD/refprop/macx/libgcc.a $$PWD/refprop/macx/libgcc_eh.a $$PWD/refprop/macx/libgfortran.a $$PWD/refprop/macx/libgomp.a $$PWD/refprop/macx/libquadmath.a
    win32:LIBS    += $$PWD/refprop/win32/librefprop.a $$PWD/refprop/win32/libgcc.a $$PWD/refprop/win32/libgfortran.a $$PWD/refprop/win32/libgomp.a $$PWD/refprop/win32/libpthread.dll.a

    copyResourceFolder(fluids, refprop/fluids, fluids)
    copyResourceFolder(mixtures, refprop/mixtures, mixtures)
} else:macx {
    refpropdb.path     = Contents/Resources
    refpropdb.files    = rc/RefPropDatabase.sqlite
    QMAKE_BUNDLE_DATA += refpropdb
} else {
    destdir = $$quote($$OUT_PWD/$$DESTDIR)
    win32 {
        destdir ~= s,/,\\,g
    }

    QMAKE_EXTRA_TARGETS    += refpropdb
    POST_TARGETDEPS        += refpropdb
    refpropdb.target        = refpropdb
    refpropdb.depends       = $$PWD/rc/RefPropDatabase.sqlite
    win32 {
        depends             = $$refpropdb.depends
        depends            ~= s,/,\\,g
        refpropdb.commands  = $$quote($$QMAKE_COPY $$depends \"$$destdir\")
    } else {
        refpropdb.commands  = $$quote($$QMAKE_COPY $$refpropdb.depends \'$$destdir\')
    }
}

CODECFORTR         = UTF-8
TRANSLATIONS      += rc/i18n/Leaklog-Slovak.ts \
                     rc/i18n/Leaklog-Polish.ts \
                     rc/i18n/Leaklog-Czech.ts \
                     rc/i18n/Leaklog-Serbian.ts
QMAKE_RESOURCE_FLAGS += -compress 9
