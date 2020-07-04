#TEMPLATE = lib
#TARGET = Pep10Common
#CONFIG += staticlib
QT += widgets printsupport concurrent

FORMS += \
    aboutpep.ui \
    byteconverterbin.ui \
    byteconverterchar.ui \
    byteconverterdec.ui \
    byteconverterhex.ui \
    byteconverterinstr.ui \
    inputpane.ui \
    iowidget.ui \
    memorydumppane.ui \
    outputpane.ui \
    terminalpane.ui \

HEADERS += \
    aboutpep.h \
    acpumodel.h \
    amemorychip.h \
    amemorydevice.h \
    byteconverterbin.h \
    byteconverterchar.h \
    byteconverterdec.h \
    byteconverterhex.h \
    byteconverterinstr.h \
    colors.h \
    enu.h \
    inputpane.h \
    interrupthandler.h \
    iowidget.h \
    mainmemory.h \
    memorychips.h \
    memorydumppane.h \
    optional_helper.h \
    outputpane.h \
    pep.h \
    symbolentry.h \
    symboltable.h \
    symbolvalue.h \
    terminalpane.h \
    updatechecker.h \
    registerfile.h \
    darkhelper.h \


SOURCES += \
    aboutpep.cpp \
    acpumodel.cpp \
    amemorychip.cpp \
    amemorydevice.cpp \
    byteconverterbin.cpp \
    byteconverterchar.cpp \
    byteconverterdec.cpp \
    byteconverterhex.cpp \
    byteconverterinstr.cpp \
    colors.cpp \
    inputpane.cpp \
    interrupthandler.cpp \
    iowidget.cpp \
    mainmemory.cpp \
    memorychips.cpp \
    memorydumppane.cpp \
    outputpane.cpp \
    pep.cpp \
    symbolentry.cpp \
    symboltable.cpp \
    symbolvalue.cpp \
    terminalpane.cpp \
    updatechecker.cpp \
    enu.cpp \
    registerfile.cpp

macx{
    QT += macextras
    LIBS += -framework Foundation -framework AppKit
    OBJECTIVE_SOURCES = darkhelper_mac.mm
}
else {
    SOURCES += darkhelper_other.cpp
}





