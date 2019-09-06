QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath
#Prevent Windows from trying to parse the project three times per build.
CONFIG -= debug_and_release \
    debug_and_release_target
#Flag for enabling C++17 features.
#Due to support for C++17 features being added before the standard was finalized, and the placeholder text of "C++1z" has remained
CONFIG += c++1z
win32{
    #MSVC doesn't recognize c++1z flag, so use the MSVC specific flag here
    win32-msvc*: QMAKE_CXXFLAGS += /std:c++17
}

SOURCES +=  \
    testmain.cpp \
    tst_assembleos.cpp \
    tst_assembleprograms.cpp \
    tst_assembler.cpp \
    tst_prepreocessorfail.cpp \
    tst_tokenbuffer.cpp \
    tst_tokenizer.cpp

HEADERS += \
    tst_assembleos.h \
    tst_assembleprograms.h \
    tst_assembler.h \
    tst_prepreocessorfail.h \
    tst_tokenbuffer.h \
    tst_tokenizer.h

INCLUDEPATH += $$PWD/../../pep10common
INCLUDEPATH += $$PWD/../../pep10asm
INCLUDEPATH += $$PWD/../../pep10cpu

#Include own directory in VPATH, otherwise qmake might accidentally import files with
#the same name from other directories.
VPATH += $$PWD
VPATH += $$PWD/../../pep10common
VPATH += $$PWD/../../pep10asm
VPATH += $$PWD/../../pep10cpu
include(../../pep10common/pep10common.pro)
include(../../pep10asm/pep10asm-common.pro)
include(../../pep10cpu/pep10cpu-common.pro)

#Must manually add resource files we care about.
RESOURCES += \
    ../../pep10asm/pep10asm-macros.qrc \
    ../../pep10asm/pep10asm-helpresources.qrc \
