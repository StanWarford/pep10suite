// File: termhelper.h
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.

    Copyright (C) 2019  J. Stanley Warford & Matthew McRaven, Pepperdine University

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

#ifndef TERMHELPER_H
#define TERMHELPER_H
#include <QtCore>
#include <QRunnable>

#include "macroregistry.h"

extern const QString errLogOpenErr;
extern const QString hadErr;
extern const QString assemble;

class MainMemory;
class AsmProgramManager;
class BoundExecIsaCpu;

// Assemble the default operating system for the help documentation,
// and install it into the program manager.
void buildDefaultOperatingSystem(AsmProgramManager& manager, QSharedPointer<MacroRegistry> registry);

// Helper function that turns hexadecimal object code into a vector of
// unsigned characters, which is easier to copy into memory.
QVector<quint8> convertObjectCodeToIntArray(QString program);

/*
 * This class is responsible for assembling a single assembly language source file.
 * Takes an assembly language program's text as input, in addition to a program manager
 * which already has the default operating system installed
 *
 * If there are warnings or errors, a error log will be written to a file
 * of the same name as objName with the .extension replaced by -errLog.txt.
 *
 * If program assembly was successful (or the only warnings were trace tag issues),
 * then the object code text will be written to objFile. If objFile doesn't exist,
 * it will bre created, and if it does, it will be truncated.
 *
 * If objFile doesn't exist at the end of the execution of this script, then
 * the file failed to assemble, and as such there must be an error log.
 *
 * When the assembler finishes running, or is terminated, finished() will be emitted
 * so that the application may shut down safely.
 */
class BuildHelper: public QObject, public QRunnable {
    };

#endif // TERMHELPER_H
