# -------------------------------------------------
# Project created by Matthew McRaven, 12/30/2018
# -------------------------------------------------
TEMPLATE = app
TARGET = Pep10CPU
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
QT += widgets webenginewidgets printsupport concurrent

# Mac icon/plist
ICON = images/icon.icns
QMAKE_INFO_PLIST = app.plist
#QMAKE_MAC_SDK = macosx10.14

#Windows RC file for icon:
RC_FILE = pep10resources.rc

FORMS += \
    cpuhelpdialog.ui \
    cpumainwindow.ui \

HEADERS += \
    cpuhelpdialog.h \
    cpumainwindow.h \

SOURCES += \
    cpuhelpdialog.cpp \
    cpumainwindow.cpp \
    cpumain.cpp

RESOURCES += \
    pep10cpu-resources.qrc \
    pep10cpu-helpresources.qrc \
    ../pep10common/pep10common-resources.qrc \
    ../pep10common/pep10common-helpresources.qrc

DISTFILES += \
    pep10resources.rc

INCLUDEPATH += $$PWD/../pep10common

VPATH += $$PWD
VPATH += $$PWD/../pep10common

include(../pep10common/pep10common.pro)
include(pep10cpu-common.pro)

#Add this include to the bottom of your project to enable automated installer creation
#Include the definitions file that sets all variables needed for the InstallerConfig Script
include("installer-config.pri")

#Lastly, include and run the installer config script
include("../installer/installer-creator.pri")
