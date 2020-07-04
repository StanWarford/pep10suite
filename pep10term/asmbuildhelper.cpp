// File: asmbuildhelper.cpp
/*
    Pep10Term is a  command line tool utility for assembling Pep/10 programs to
    object code and executing object code programs.

    Copyright (C) 2019-2020 J. Stanley Warford & Matthew McRaven, Pepperdine University

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "asmbuildhelper.h"

#include "asmcode.h"
#include "asmprogram.h"
#include "asmprogrammanager.h"
#include "isaasm.h"
#include "macroassemblerdriver.h"
#include "macroregistry.h"
#include "pep.h"
#include "symbolentry.h"
#include "symboltable.h"
#include "termhelper.h"


ASMBuildHelper::ASMBuildHelper(const QString source, QFileInfo objFileInfo,
                         AsmProgramManager &manager, QSharedPointer<MacroRegistry> registry,
                         QObject *parent): QObject(parent),
    source(source), objFileInfo(objFileInfo),
    manager(manager), registry(std::move(std::move(registry)))
{
    // Default error log name to the base name of the file with an _errLog.txt extension.
    this->error_log = objFileInfo.absoluteDir().absoluteFilePath(objFileInfo.baseName() + "_errLog.txt");
}

// All of our memory is owned by sharedpointers, so we
// should not attempt to delete anything ourselves.
ASMBuildHelper::~ASMBuildHelper() = default;

void ASMBuildHelper::run()
{
    // All set up work is done in build program, so all run needs to do is attempt
    if(buildProgram()) {
       // Placeholder for potential work needing to be done after successful assembly.
    }

    // Application will live forever if we don't signal it to die.
    emit finished();
}

void ASMBuildHelper::set_error_file(QString error_file)
{
    this->error_log = error_file;
}

bool ASMBuildHelper::buildProgram()
{
    // Construct files that will be needed for assembly
    QFile objectFile(objFileInfo.absoluteFilePath());
    QFile errorLog(error_log.absoluteFilePath());

    MacroAssemblerDriver assembler(registry);
    //QString osText = Pep::resToString(":/help-asm/figures/pep10os.pep", false);
    //auto osResult = assembler.assembleOperatingSystem(osText);
    // Returns true if object code is successfully generated (i.e. program is non-null).
    auto asmResult = assembler.assembleUserProgram(source,
                                            AsmProgramManager::getInstance()->
                                            getOperatingSystem()->getSymbolTable());

    // If there were errors, attempt to write all of them to the error file.
    // If the error file can't be opened, log that failure to standard output.
    if(!asmResult.errors.sourceMapped.isEmpty()) {
        if(!errorLog.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            qDebug().noquote() << errLogOpenErr.arg(errorLog.fileName());
        }
        else {
            QTextStream errAsStream(&errorLog);
            auto textList = source.split("\n");
            for(const auto& errorList : asmResult.errors.sourceMapped) {
                for(const auto& error : errorList) {
                    int index = error->getSourceLineNumber();
                    // Sometimes errors will be at (or past) the end of the source list.
                    // This occurs when a .END isn't specified.
                    // In this case, there is not text to print, so ignore existing text.
                    if(index >= textList.size()) {
                        errAsStream << error->getErrorMessage() << endl;
                    }
                    else {
                        errAsStream << textList[error->getSourceLineNumber()]
                                    << error->getErrorMessage() << endl;
                    }
                }
            }
            // Error log should be flushed automatically.
            errorLog.close();
        }
    }

    // Only open & write object code file if assembly was successful.
    if(asmResult.success) {
        QSharedPointer<AsmProgram> program = asmResult.program;
        // Program assembly can succeed despite the presence of errors in the
        // case of trace tag warnings. Must gaurd against this.
        if(asmResult.errors.sourceMapped.isEmpty()) {
            qDebug() << "Program assembled successfully.";
        }
        else {
            qDebug() << "Warning(s) generated. See error log.";
        }
        // Attempt to open object code file. Write error to standard out if it fails.
        if(!objectFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            qDebug().noquote() << errLogOpenErr.arg(objectFile.fileName());
        }
        else {
            // Below code copied from object code pane's formatting
            QString objectCodeString = "";
            auto objectCode = program->getObjectCode();
            for (int i = 0; i < objectCode.length(); i++) {
                objectCodeString.append(QString("%1").arg(objectCode[i], 2, 16, QLatin1Char('0')).toUpper());
                objectCodeString.append((i % 16) == 15 ? '\n' : ' ');
            }
            objectCodeString.append("zz");
            QTextStream objStream(&objectFile);
            objStream << objectCodeString << "\n";
            objectFile.close();
        }

        // Also attempt to generate listing file from assembled program as well.
        QFile listingFile(QFileInfo(objectFile).absoluteDir().absoluteFilePath(
                              QFileInfo(objectFile).baseName() + ".pepl"));
        if(!listingFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            qDebug().noquote() << errLogOpenErr.arg(listingFile.fileName());
        }
        else {
            QTextStream listingStream(&listingFile);
            listingStream << program->getProgramListing();
            if(!program->getSymbolTable()->getSymbolMap().isEmpty()) {
                listingStream << "\n";
                listingStream << program->getSymbolTable()->getSymbolTableListing();
            }
            listingFile.close();
        }
    }
    else {
        qDebug() << "Error(s) generated. See error log.";
    }
    return asmResult.success;
}

