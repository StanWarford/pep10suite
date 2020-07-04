// File: termhelper.cpp
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

#include "termhelper.h"

#include <utility>

#include "amemorychip.h"
#include "amemorydevice.h"
#include "asmprogrammanager.h"
#include "asmprogram.h"
#include "asmcode.h"
#include "boundexecisacpu.h"
#include "isacpu.h"
#include "macroassemblerdriver.h"
#include "mainmemory.h"
#include "memorychips.h"
#include "pep.h"
#include "symboltable.h"
#include "symbolentry.h"

// Error messages potentially used in multiple places;
const QString errLogOpenErr = "Could not open file: %1";
const QString hadErr        = "Errors/warnings encountered while generating output for file: %1";
const QString assemble      = "About to assemble %1 into object file %2";

// Helper function that turns hexadecimal object code into a vector of
// unsigned characters, which is easier to copy into memory.
QVector<quint8> convertObjectCodeToIntArray(QString program)
{
    bool ok = false;
    quint8 temp;
    QVector<quint8> output;
    program.replace(QRegExp("\n")," ");
    for(const QString& byte : program.split(" ")) {
        // toShort(...) should never throw any errors, so there should be no concerns if byte is not a hex constant.
        temp = static_cast<quint8>(byte.toShort(&ok, 16));
        // There could be a loss in precision if given text outside the range of an uchar but in range of a ushort.
        if(ok && byte.length()>0) output.append(temp);
    }
    return output;
}

void buildDefaultOperatingSystem(AsmProgramManager &manager, QSharedPointer<MacroRegistry> registry)
{
    // Need to assemble operating system.
    QString defaultOSText = Pep::resToString(":/help-asm/figures/pep10os.pep", false);
    // If there is text, attempt to assemble it
    if(!defaultOSText.isEmpty()) {
        QSharedPointer<AsmProgram> prog;
        auto elist = QList<QPair<int, QString>>();
        MacroAssemblerDriver assembler(std::move(registry));
        auto asmResult = assembler.assembleOperatingSystem(defaultOSText);
        if(!asmResult.success) {
            qDebug() << "Failed to assemble OS.";
            throw 0xDEAD;
        }
        else if(!asmResult.program.isNull()) {
            prog = asmResult.program;
            manager.setOperatingSystem(prog);
        }
        // If the operating system failed to assembly, we can't progress any further.
        // All application functionality depends on the operating system being defined.
        else {
            qDebug() << "OS failed to assemble.";
            auto textList = defaultOSText.split("\n");
            for(const auto& errorPair : elist) {
                qDebug() << textList[errorPair.first] << errorPair.second << endl;
            }
            throw std::logic_error("The default operating system failed to assemble.");
        }
        QFile outputFile ("os.pepl");
        outputFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
        QString listing =  manager.getOperatingSystem()->getProgramListing();
        QTextStream (&outputFile) << listing
        << manager.getOperatingSystem()->getSymbolTable()->getSymbolTableListing();
        outputFile.flush();
        outputFile.close();
    }
    // If the operating system couldn't be found, we can't progress any further.
    // All application functionality depends on the operating system being defined.
    else {
        throw std::logic_error("Could not find default operating system.");
    }

}
