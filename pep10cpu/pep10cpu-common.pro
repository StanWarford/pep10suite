#TEMPLATE = lib
#TARGET = Pep10Common
#CONFIG += staticlib
QT += webenginewidgets widgets printsupport concurrent
INCLUDEPATH += $$PWD\..\pep10common
VPATH += $$PWD\..\pep10common

FORMS += \
    cpupane.ui \
    microcodepane.ui \
    microobjectcodepane.ui \

HEADERS += \
    cpudata.h \
    cpupane.h \
    cpugraphicsitems.h \
    disableselectionmodel.h \
    interfacemccpu.h \
    microasm.h \
    microcode.h \
    microcodeeditor.h \
    microcodepane.h \
    microcodeprogram.h \
    microobjectcodepane.h \
    partialmicrocodedcpu.h \
    partialmicrocodedmemoizer.h \
    pepmicrohighlighter.h \
    rotatedheaderview.h \
    shapes_one_byte_data_bus.h \
    shapes_two_byte_data_bus.h \
    specification.h \
    tristatelabel.h \

SOURCES += \
    cpudata.cpp \
    cpupane.cpp \
    cpugraphicsitems.cpp \
    disableselectionmodel.cpp \
    interfacemccpu.cpp \
    microasm.cpp \
    microcode.cpp \
    microcodeeditor.cpp \
    microcodepane.cpp \
    microcodeprogram.cpp \
    microobjectcodepane.cpp \
    partialmicrocodedcpu.cpp \
    partialmicrocodedmemoizer.cpp \
    pepmicrohighlighter.cpp \
    rotatedheaderview.cpp \
    specification.cpp \
    tristatelabel.cpp \
