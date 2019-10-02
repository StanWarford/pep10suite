s# Project created by Matthew McRaven, 12/30/2018
# -------------------------------------------------
# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# Console application specific configuration.
QT -= gui
CONFIG += c++17 console

TARGET = Pep10Term
#Prevent Windows from trying to parse the project three times per build.
CONFIG -= debug_and_release \
    debug_and_release_target
#Flag for enabling C++17 features.
#Due to support for C++17 features being added before the standard was finalized, and the placeholder text of "C++1z" has remained
CONFIG += c++1z
win32{
    #MSVC doesn't recognize c++1z flag, so use the MSVC specific flag here
    win32-msvc*: QMAKE_CXXFLAGS += /std:c++17
    #Flags needed to generate PDB information in release. Necessary information to profile program.
    #Flags also picked to provide a ~15% speed increase in release mode (at the cost of increased compile times).
    QMAKE_LFLAGS_RELEASE +=/MAP
    QMAKE_CFLAGS_RELEASE -= O2
    QMAKE_CFLAGS_RELEASE += /O3 /MD /zi
    QMAKE_LFLAGS_RELEASE +=/debug /opt:ref
}

# Mac icon/plist
ICON = images/icon.icns
QMAKE_INFO_PLIST = app.plist
QMAKE_MAC_SDK = macosx10.14

#Windows RC file for icon:
RC_FILE = pep10resources.rc

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    asmbuildhelper.cpp \
    asmrunhelper.cpp \
    boundexecmicrocpu.cpp \
    cpubuildhelper.cpp \
    cpurunhelper.cpp \
    microstephelper.cpp \
    termhelper.cpp \
    boundexecisacpu.cpp \
    termmain.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    asmbuildhelper.h \
    asmrunhelper.h \
    boundexecmicrocpu.h \
    cpubuildhelper.h \
    cpurunhelper.h \
    microstephelper.h \
    termhelper.h \
    boundexecisacpu.h

RESOURCES += \
    ../pep10asm/pep10asm-macros.qrc \
    ../pep10common/pep10common-helpresources.qrc\
    ../pep10asm/pep10asm-resources.qrc \
    ../pep10asm/pep10asm-helpresources.qrc \
    pep10term-helpresources.qrc \
    pep10term-resources.qrc

DISTFILES += \
    help-term/about.txt


INCLUDEPATH += $$PWD/../pep10common
INCLUDEPATH += $$PWD/../pep10asm
INCLUDEPATH += $$PWD/../pep10cpu
INCLUDEPATH += $$PWD/../pep10micro

#Include own directory in VPATH, otherwise qmake might accidentally import files with
#the same name from other directories.
VPATH += $$PWD
VPATH += $$PWD/../pep10common
VPATH += $$PWD/../pep10asm
VPATH += $$PWD/../pep10cpu
VPATH += $$PWD/../pep10micro

include(../pep10common/pep10common.pro)
include(../pep10asm/pep10asm-common.pro)
include(../pep10cpu/pep10cpu-common.pro)
include(../pep10micro/pep10micro-common.pro)

#Add this include to the bottom of your project to enable automated installer creation
#Include the definitions file that sets all variables needed for the InstallerConfig Script
include("installer-config.pri")

#Lastly, include and run the installer config script
include("../installer/installer-creator.pri")
