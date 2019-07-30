// File: termhelper.cpp
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.

    Copyright (C) 2019  J. Stanley Warford & Matthew McRaevn, Pepperdine University

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
#include "amemorychip.h"
#include "amemorydevice.h"
#include "asmprogrammanager.h"
#include "asmprogram.h"
#include "asmcode.h"
#include "boundexecisacpu.h"
#include "isacpu.h"
#include "pep.h"
#include "symboltable.h"
#include "symbolentry.h"
#include "macroassemblerdriver.h"
#include "mainmemory.h"
#include "memorychips.h"

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
    for(QString byte : program.split(" ")) {
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
        MacroAssemblerDriver assembler(registry);
        prog = assembler.assembleOperatingSystem(defaultOSText);
        if(!prog.isNull()) {
            manager.setOperatingSystem(prog);
        }
        // If the operating system failed to assembly, we can't progress any further.
        // All application functionality depends on the operating system being defined.
        else {
            qDebug() << "OS failed to assemble.";
            auto textList = defaultOSText.split("\n");
            for(auto errorPair : elist) {
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

RunHelper::RunHelper(const QString objectCodeString,quint64 maxSimSteps,
                     QFileInfo programOutput, QFileInfo programInput, AsmProgramManager &manager,
                     QObject *parent):
    QObject(parent), QRunnable (), objectCodeString(objectCodeString),
    programOutput(programOutput), programInput(programInput) ,manager(manager),
    // Explicitly initialize both simulation objects to nullptr,
    // so that it is clear to that neither object has been allocated
    memory(nullptr), cpu(nullptr), outputFile(nullptr), maxSimSteps(maxSimSteps)

{

}

RunHelper::~RunHelper()
{
    // All of our memory is owned by sharedpointers, so we should not attempt
    // to delete anything ourselves.
    if(outputFile != nullptr) {
        outputFile->flush();
        // It might seem like we should close the file here, but it causes read / write violations to do so.
        // Instead, delete it later under the assumption that the operating system will handle that for us.
        // Schedule the output file for deletion via the event loop.
        outputFile->deleteLater();
    }
}

void RunHelper::loadOperatingSystem()
{
    QVector<quint8> values;
    quint16 startAddress;
    values = manager.getOperatingSystem()->getObjectCode();
    startAddress = manager.getOperatingSystem()->getBurnAddress();
    // Get addresses for I/O chips
    auto osSymTable = manager.getOperatingSystem()->getSymbolTable();
    diskIn = static_cast<quint16>(osSymTable->getValue("diskIn")->getValue());
    charIn = static_cast<quint16>(osSymTable->getValue("charIn")->getValue());
    charOut = static_cast<quint16>(osSymTable->getValue("charOut")->getValue());
    powerOff  = static_cast<quint16>(osSymTable->getValue("pwrOff")->getValue());

    // Construct main memory according to the current configuration of the operating system.
    QList<MemoryChipSpec> list;
    // Make sure RAM will fill any accidental gaps in the memory map by making it go
    // right up to the start of the operating system.
    list.append({AMemoryChip::ChipTypes::RAM, 0, startAddress});
    list.append({AMemoryChip::ChipTypes::ROM, startAddress, static_cast<quint32>(values.length())});
    // Character input / output ports are only 1 byte wide by design.
    list.append({AMemoryChip::ChipTypes::IDEV, diskIn, 1});
    list.append({AMemoryChip::ChipTypes::IDEV, charIn, 1});
    list.append({AMemoryChip::ChipTypes::ODEV, charOut, 1});
    list.append({AMemoryChip::ChipTypes::ODEV, powerOff, 1});
    memory->constructMemoryDevice(list);

    memory->autoUpdateMemoryMap(true);
    memory->loadValues(manager.getOperatingSystem()->getBurnAddress(), values);
}

void RunHelper::onInputRequested(quint16 address)
{
    // All the input a program will ever receive is loaded into the memory
    // buffer as the program is started. So we can't satisfy the IO request,
    // and thus we need to signal the simulation that the IO request was denied.
    memory->onInputAborted(address);
}

static QDebug* dbg = new QDebug(QtDebugMsg);
void RunHelper::onOutputReceived(quint16 address, quint8 value)
{
    if(address == powerOff) {
        this->cpu->onCancelExecution();
    }
    if(address == charOut &&outputFile != nullptr) {
        // Use a temporary (anonymous) text stream to make writing easy.
        QTextStream (&*outputFile) << QChar(value);
        // Try to block and make sure the IO actually completes.
        outputFile->waitForBytesWritten(300);
        dbg->noquote().nospace() << QChar(value);
    }
}

void RunHelper::onSimulationFinished()
{
    // There migh be outstanding IO events. Give them a chance to finish
    // before initiating shutdown.
    QCoreApplication::processEvents();
    emit finished();
}

void RunHelper::runProgram()
{

    // Buffer input file into memory mapped input if possible.
    QFile input(programInput.absoluteFilePath());

    // If there is not input, append a newline so that there is a least one character buffered.
    if(!programInput.exists()) {
        memory->onInputReceived(charIn, "\n");
    }
    else if(!input.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug().noquote() << errLogOpenErr.arg(input.fileName());
        throw std::logic_error("Can't open input file.");
    } else {
        QTextStream inputStream(&input);
        memory->onInputReceived(charIn, inputStream.readAll() % "\n");
        input.close();
    }

    // Open up program output file if possible.
    // If output can't be opened up, abort.
    QFile *output = new QFile(programOutput.absoluteFilePath());
    if(!output->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qDebug().noquote() << errLogOpenErr.arg(output->fileName());
        throw std::logic_error("Can't open output file.");
    } else {
        // If it could be opened, map charOut to the file.
        outputFile = output;
    }

    // Make sure to set up any last minute flags needed by CPU to perform simulation.
    cpu->onSimulationStarted();
    cpu->runUntilLoaded();
    qDebug().noquote() << "Yeet yeet i'm loaded \n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
    QThread::currentThread()->msleep(1000);
    if(!cpu->onRun()) {
        qDebug().noquote() << "The CPU failed for the following reason: "<<cpu->getErrorMessage();
        QTextStream (&*outputFile) << "[[" << cpu->getErrorMessage() << "]]";
    }

}

void RunHelper::run()
{

    // Construct all needed simulation objects in run, so that the owning
    // thread is the one doing the computation, not the main thread.
    if(memory.isNull()) {
        // Assume memory will always be 64k.
        memory = QSharedPointer<MainMemory>::create(nullptr);
        QSharedPointer<RAMChip> ramChip(new RAMChip(1<<16, 0, memory.get()));
        memory->insertChip(ramChip, 0);

        cpu = QSharedPointer<BoundExecIsaCpu>::create(maxSimSteps, &manager, memory, nullptr);

        // Connect IO events. IO *MUST* complete before execution moves forward.
        // Use a blocking connection to serialize IO. Use asynchronous connection
        // so that memory and helper don't need to reside in the same thread.
        connect(memory.get(), &MainMemory::inputRequested, this, &RunHelper::onInputRequested, Qt::BlockingQueuedConnection);
        connect(memory.get(), &MainMemory::outputWritten, this, &RunHelper::onOutputReceived, Qt::BlockingQueuedConnection);
    }

    // Load operating system & user program into memory.
    loadOperatingSystem();
    //auto objCode = convertObjectCodeToIntArray(objectCodeString);
    memory->onInputReceived(diskIn, objectCodeString);
    //memory->loadValues(0, objCode);

    // Clear & initialize all values in CPU before starting simulation.
    cpu->reset();
    cpu->initCPU();

    // Instead of directly allowing run() to kill itself, uses events to "schedule"
    // shutting down the application. This should ensure all IO completes. We were
    // having an error where closing IO streams directly after simulation completion would
    // cause a race condition with IO pending for the file. The overhead of the simulation events
    // seems to "serialize" writes / closing.
    connect(cpu.get(), &IsaCpu::simulationFinished, this, &RunHelper::onSimulationFinished);
    runProgram();

    // Make sure any outstanding events are handled.
    QCoreApplication::processEvents();
}

BuildHelper::BuildHelper(const QString source, QFileInfo objFileInfo,
                         AsmProgramManager &manager, QSharedPointer<MacroRegistry> registry,
                         QObject *parent): QObject(parent),
    QRunnable(), source(source), objFileInfo(objFileInfo),
    manager(manager), registry(registry)
{

}

BuildHelper::~BuildHelper()
{
    // All of our memory is owned by sharedpointers, so we
    // should not attempt to delete anything ourselves.
}

void BuildHelper::run()
{
    // All set up work is done in build program, so all run needs to do is attempt
    if(buildProgram()) {
       // Placeholder for potential work needing to be done after successful assembly.
    }

    // Application will live forever if we don't signal it to die.
    emit finished();
}

bool BuildHelper::buildProgram()
{
    // Construct files that will be needed for assembly
    QFile objectFile(objFileInfo.absoluteFilePath());
    QFile errorLog(QFileInfo(objectFile).absoluteDir().absoluteFilePath(
                       QFileInfo(objectFile).baseName() + "_errLog.txt"));
    QSharedPointer<AsmProgram> program;
    auto elist = QList<QPair<int, QString> >();
#pragma message("Remove dummy macros")
    registry->registerCustomMacro("asla6", "@asla6 0\n@asla5\nasla\n.END\n");
    registry->registerCustomMacro("asla7", "@asla7 0\n@asla6\nasla\n.END\n");
    registry->registerCustomMacro("DIVA", "@DIVA 2\nLDWX $1,$2\nCPWX 0,i\n@asla2\n.END\n");
    // Trigger loop detection
    registry->registerCustomMacro("LOOPA", "@LOOPA 0\n@LOOPB\n.END\n");
    registry->registerCustomMacro("LOOPB", "@LOOPB 0\n@LOOPA\n.END\n");

    // Angry diamond
    registry->registerCustomMacro("L1A", "@L1A 0\n@L1B\n@L2\n.END\n");
    registry->registerCustomMacro("L1B", "@L1B 0\n@L2\n.END\n");
    registry->registerCustomMacro("L2",  "@L2 0\n@asla7\n@L3\n.END\n");
    // Angry diamond
    registry->registerCustomMacro("L3", "@L3 0\n@L4A\n@L4B\n@L4C\n.END\n");
    registry->registerCustomMacro("L4A", "@L4A 0\n@L4B\n@L4C\n.END\n");
    registry->registerCustomMacro("L4B", "@L4B 0\n@L4C\n@DIVA l,sfx\n.END\n");
    // Works normally.
    registry->registerCustomMacro("L4C", "@L4C 0\n@DIVA k,d\n.END\n");

    MacroAssemblerDriver assembler(registry);
    //QString osText = Pep::resToString(":/help-asm/figures/pep10os.pep", false);
    //auto osResult = assembler.assembleOperatingSystem(osText);
    // Returns true if object code is successfully generated (i.e. program is non-null).
    program = assembler.assembleUserProgram(source,
                                            AsmProgramManager::getInstance()->
                                            getOperatingSystem()->getSymbolTable());

    // If there were errors, attempt to write all of them to the error file.
    // If the error file can't be opened, log that failure to standard output.
    if(!elist.isEmpty()) {
        if(!errorLog.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            qDebug().noquote() << errLogOpenErr.arg(errorLog.fileName());
        }
        else {
            QTextStream errAsStream(&errorLog);
            auto textList = source.split("\n");
            for(auto errorPair : elist) {
                errAsStream << textList[errorPair.first] << errorPair.second << endl;
            }
            // Error log should be flushed automatically.
            errorLog.close();
        }
    }

    // Only open & write object code file if assembly was successful.
    if(true /*success*/) {
        // Program assembly can succeed despite the presence of errors in the
        // case of trace tag warnings. Must gaurd against this.
        if(elist.isEmpty()) {
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
            // Below code copeid from object code pane's formatting
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
            listingStream << "\n";
            listingStream << program->getSymbolTable()->getSymbolTableListing();
            listingFile.close();
        }
    }
    else {
        qDebug() << "Error(s) generated. See error log.";
    }
    return true /*success*/;
}
