#TEMPLATE = lib
#TARGET = Pep10Common
#CONFIG += staticlib
QT += widgets printsupport concurrent
INCLUDEPATH += $$PWD\..\pep10common
VPATH += $$PWD\..\pep10common

FORMS += \
    asmobjectcodepane.ui \
    asmsourcecodepane.ui \
    executionstatisticswidget.ui \
    memorytracepane.ui \
    redefinemnemonicsdialog.ui \
    asmcpupane.ui \
    asmprogramtracepane.ui \
    asmprogramlistingpane.ui \
    assemblerpane.ui

HEADERS += \
    asmargument.h \
    asmcode.h \
    asmobjectcodepane.h \
    asmprogram.h \
    asmprogrammanager.h \
    asmsourcecodepane.h \
    cpphighlighter.h \
    executionstatisticswidget.h \
    fakemacrodriver.h \
    interfaceisacpu.h \
    isaasm.h \
    macro.h \
    macroassemblerdriver.h \
    macromodules.h \
    macropreprocessor.h \
    macroregistry.h \
    macrotokenizer.h \
    macroassembler.h \
    memorycellgraphicsitem.h \
    memorytracepane.h \
    ngraph.h \
    ngraph_path.h \
    ngraph_prune.h \
    pepasmhighlighter.h \
    setops.hpp \
    typetags.h \
    stacktrace.h \
    redefinemnemonicsdialog.h \
    asmcpupane.h \
    isacpu.h \
    isacpumemoizer.h \
    memoizerhelper.h \
    asmprogramtracepane.h \
    asmprogramlistingpane.h \
    assemblerpane.h

SOURCES += \
    asmargument.cpp \
    asmcode.cpp \
    asmobjectcodepane.cpp \
    asmprogram.cpp \
    asmprogrammanager.cpp \
    asmsourcecodepane.cpp \
    cpphighlighter.cpp \
    executionstatisticswidget.cpp \
    fakemacrodriver.cpp \
    interfaceisacpu.cpp \
    isaasm.cpp \
    macro.cpp \
    macroassemblerdriver.cpp \
    macromodules.cpp \
    macropreprocessor.cpp \
    macroregistry.cpp \
    macrotokenizer.cpp \
    macroassembler.cpp \
    memorycellgraphicsitem.cpp \
    memorytracepane.cpp \
    pepasmhighlighter.cpp \
    typetags.cpp \
    stacktrace.cpp \
    redefinemnemonicsdialog.cpp \
    asmcpupane.cpp \
    isacpu.cpp \
    isacpumemoizer.cpp \
    memoizerhelper.cpp \
    asmprogramtracepane.cpp \
    asmprogramlistingpane.cpp \
    assemblerpane.cpp