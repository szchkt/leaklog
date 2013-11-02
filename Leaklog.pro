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
                 src/mtaddress.h \
                 src/mtcheckboxgroup.h \
                 src/mtcolourcombobox.h \
                 src/mtdictionary.h \
                 src/mtlistwidget.h \
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
                 src/leakagesbyapplication.cpp \
                 src/linkparser.cpp \
                 src/main.cpp \
                 src/mainwindow.cpp \
                 src/mainwindowsettings.cpp \
                 src/mtaddress.cpp \
                 src/mtrecord.cpp \
                 src/mtsqlquery.cpp \
                 src/mtsqlqueryresult.cpp \
                 src/mttabwidget.cpp \
                 src/mtvariant.cpp \
                 src/mtwebpage.cpp \
                 src/mtwidget.cpp \
                 src/partnerwidgets.cpp \
                 src/permissionsdialogue.cpp \
                 src/records.cpp \
                 src/refprop.cpp \
                 src/reportdata.cpp \
                 src/reportdatacontroller.cpp \
                 src/sha256.cpp \
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

lessThan(QT_MAJOR_VERSION, 5) {
    QT            += network webkit sql
    CONFIG        += depend_includepath
} else {
    QT            += widgets network webkitwidgets sql
}

# fparser
HEADERS           += include/fparser/fpconfig.hh include/fparser/fptypes.hh include/fparser/fparser.hh
SOURCES           += include/fparser/fpoptimizer.cc include/fparser/fparser.cc
DEFINES           += FP_NO_SUPPORT_OPTIMIZER

# csvparser
HEADERS           += include/csvparser/mtcsvparser.h
SOURCES           += include/csvparser/mtcsvparser.cpp

DESTDIR            = bin
INCLUDEPATH       += src src/views include
UI_HEADERS_DIR     = ui/include
UI_SOURCES_DIR     = ui

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
    icons.path     = Contents/Resources
    icons.files    = rc/images/lklg.icns
    QMAKE_BUNDLE_DATA += icons
    CONFIG        += x86_64
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.5
    # QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.5.sdk
}

macx-xcode {
    QMAKE_CC       = clang
    QMAKE_CXX      = clang++
}

unix {
    OBJECTS_DIR    = build/unix
    MOC_DIR        = build/unix
    RCC_DIR        = build/unix
}

defineTest(copyResourceFolder) {
    folder = $$1
    source = $$2
    macx-xcode {
        destination = $$quote($$DESTDIR/$${TARGET}.app/Contents/Resources/$$3)
    } else:macx {
        destination = $$quote($$OUT_PWD/$$DESTDIR/$${TARGET}.app/Contents/Resources/$$3)
    } else {
        destination = $$quote($$OUT_PWD/$$DESTDIR/$$3)
    }
    win32 {
        source      ~= s,/,\\,g
        destination ~= s,/,\\,g
    }

    macx-xcode {
        eval($${folder}_in             = $$files($$source/*))
        export($${folder}_in)
        eval($${folder}.input          = $${folder}_in)
        export($${folder}.input)
        eval($${folder}.output         = $$destination/${QMAKE_FILE_BASE}${QMAKE_FILE_EXT})
        export($${folder}.output)
        eval($${folder}.commands       = mkdir -p \'$$destination\'; $$QMAKE_COPY $$source/${QMAKE_FILE_BASE}${QMAKE_FILE_EXT} \'$$destination\')
        export($${folder}.commands)
        eval($${folder}.variable_out   = files_out)
        export($${folder}.variable_out)

        QMAKE_EXTRA_COMPILERS         += $$folder
        export(QMAKE_EXTRA_COMPILERS)
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
            eval($${folder}.commands   = $$quote((if not exist \'$$destination\'. mkdir \'$$destination\') &&))
            eval($${folder}.commands  += $$quote($$QMAKE_COPY $$pwd\\$$source\\* \'$$destination\'))
        } else {
            eval($${folder}.commands   = $$quote(mkdir -p \'$$destination\';))
            eval($${folder}.commands  += $$quote($$QMAKE_COPY $$eval($${folder}.depends) \'$$destination\'))
        }
        export($${folder}.commands)
    }
}

# refprop
exists(refprop/include/refprop_lib.h) {
    message("Configuring with RefProp...")
    DEFINES       += REFPROP
    INCLUDEPATH   += refprop/include
    macx:LIBS     += $$PWD/refprop/macx/librefprop.a $$PWD/refprop/macx/libgcc.a $$PWD/refprop/macx/libgcc_eh.a $$PWD/refprop/macx/libgfortran.a $$PWD/refprop/macx/libgomp.a $$PWD/refprop/macx/libquadmath.a
    win32:LIBS    += $$PWD/refprop/win32/librefprop.a $$PWD/refprop/win32/libgcc.a $$PWD/refprop/win32/libgfortran.a $$PWD/refprop/win32/libgomp.a $$PWD/refprop/win32/libpthread.dll.a

    copyResourceFolder(fluids, refprop/fluids, fluids)
    copyResourceFolder(mixtures, refprop/mixtures, mixtures)
} else {
    HEADERS       += src/refrigerants.h
    SOURCES       += src/refrigerants.cpp
}

TRANSLATIONS      += rc/i18n/Leaklog-Slovak.ts
QMAKE_RESOURCE_FLAGS += -compress 9
