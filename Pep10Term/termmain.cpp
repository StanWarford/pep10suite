// File: main.cpp
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
#include <QCoreApplication>
#include <QtCore>
#include <QDebug>
#include <QThreadPool>
#include "termhelper.h"
#include "mainmemory.h"
#include "memorychips.h"
#include "asmprogrammanager.h"
#include "pep.h"
#include <QCommandLineOption>
#include <QCommandLineParser>
#include "boundexecisacpu.h"
#include "macroregistry.h"
const QString asmInputFileText = "Input Pep/9 source program for assembler";
const QString asmOutputFileText = "Output object code generated from source.";
const QString objInputFileText = "Input Pep/9 object code program for simulator";
const QString charinFileText = "File which will be buffered behind the charIn";
const QString charoutFileText = "File which charOut will be written to.";
const QString maxStepText = "The maximum number of steps executed before aborting. Defaults to %1";
const QString listMacroText = "View the listing of a macro.";
int main(int argc, char *argv[])
{
    // Initialize global state maps.
    Pep::initEnumMnemonMaps();
    Pep::initMnemonicMaps();
    Pep::initAddrModesMap();
    Pep::initDecoderTables();

    // Construct an application so that we have a Qt main event loop.
    QCoreApplication a(argc, argv);

    QCoreApplication::setOrganizationName("Pepperdine Computer Science Lab");
    QCoreApplication::setOrganizationDomain("cslab.pepperdine.edu");
    QCoreApplication::setApplicationName("Pep10Term");
    QCoreApplication::setApplicationVersion("10.0");

    // Create a parser
    QCommandLineParser parser;
    parser.addHelpOption();
    // Placeholder for one of the execution modes. Ideally, the parser would
    // have different options and positional arguments depending on the value of
    // the leading positional argument. However, the parser lacks this functionality.
    parser.addPositionalArgument("mode", "The mode Pep10 is to be executed in: Options are \"asm\", \"run\"\
, \"macros\", and \"listing\". \
Run pep9term 'mode' --help for more options.");
    parser.addOption(QCommandLineOption("about",
                                        "Display information about Qt & developers."));
    parser.parse(QCoreApplication::arguments());

    // Fetch the positional argument list, the first of which is the "mode"
    // Pep10term is to be run in.
    const QStringList args = parser.positionalArguments();
    const QString command = args.isEmpty() ? QString() : args.first();
    // Reconstruct the parser options based on the application's mode.
    if(parser.isSet("about")) {
        QFile aboutFile(":/help-term/about.txt");
        aboutFile.open(QIODevice::ReadOnly | QIODevice::Text);
        qDebug().noquote() << aboutFile.readAll();
        aboutFile.close();
        return 0;
    }
    else if (command == "asm") {
        parser.clearPositionalArguments();
        parser.addPositionalArgument("asm", "Assemble a Pep/10 source code program", "pep10term asm -s source.pep -o assembled.pepo");
        parser.addOption(QCommandLineOption("s", asmInputFileText, "source_file"));
        parser.addOption(QCommandLineOption("o", asmOutputFileText, "object_file"));
    }
    else if (command == "run") {
        parser.clearPositionalArguments();
        parser.addPositionalArgument("run", "Run an object code program.",
                                     "pep10term run -s asm.pepo -i charIn.txt, -o charOut.txt");
        parser.addOption(QCommandLineOption("s", objInputFileText, "object_source_file"));
        // Batch input that will be loaded into charIn.
        parser.addOption(QCommandLineOption("i", charinFileText, "char_input"));
        // File where values written to charOut will be stored.
        parser.addOption(QCommandLineOption("o", charoutFileText, "char_output"));
        // Maximum number of steps.
        parser.addOption(QCommandLineOption("m",
                                            maxStepText.arg(BoundExecIsaCpu::getDefaultMaxSteps()),
                                            "max_Steps"));
    }
    else if (command == "macros") {
        parser.clearPositionalArguments();
        parser.addPositionalArgument("macros", "View all macros available to the Pep/10 assembler.");
    } else if (command == "listing") {
        parser.clearPositionalArguments();
        parser.addPositionalArgument("listing", "View the listing of a macro.");
        parser.addOption(QCommandLineOption("n", listMacroText, "name"));
    }
    // Otherwise it's an invalid mode, return an error and have the help
    // documentation appear
    else {
        parser.showHelp(-1);
    }

    // Must re-add help documentation to ensure -h is still a valid option.
    auto helpOption = parser.addHelpOption();
    parser.process(a);

    // Task that will be
    QRunnable *run = nullptr;
    QThreadPool pool;
    QSharedPointer<MacroRegistry> registry = QSharedPointer<MacroRegistry>::create("");
    AsmProgramManager::getInstance()->setMacroRegistry(registry);
    // Assembly the default operating system from this thread, so that
    // no worker threads have to check for the presence of an operating system.
    buildDefaultOperatingSystem(*AsmProgramManager::getInstance(), registry);

    if (command == "asm") {
        // Needs both an input and output source to be a well-defined command.
        if(!parser.isSet("s") || !parser.isSet("o")) {
            qDebug() << "Invalid option combination";
            parser.showHelp(-1);
        }

        // File names associated with cli parameters.
        QString sourceFileString = parser.value("s");
        QString objectFileString = parser.value("o");

        QFile sourceFile(sourceFileString);
        QString sourceText;
        if(!sourceFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug().noquote() << errLogOpenErr.arg(sourceFile.fileName());
            parser.showHelp(-1);
        }
        else {
            QTextStream sourceStream(&sourceFile);
            sourceText = sourceStream.readAll();
            sourceFile.close();

            BuildHelper *helper = new BuildHelper(sourceText, objectFileString, *AsmProgramManager::getInstance(), registry);
            QObject::connect(helper, &BuildHelper::finished, &a, &QCoreApplication::quit);

            run = helper;
        }
    }
    else if(command == "run") {
        // Needs both an source program, input & output to be a well-defined command.
        if(!parser.isSet("s") || !parser.isSet("o")) {
            qDebug() << "Invalid option combination";
            parser.showHelp(-1);
        }

        // File names associated with cli parameters.
        QString objCodeFileName = parser.value("s");
        QString textInputFileName = parser.value("i");
        QString textOutputFileName = parser.value("o");
        QString stepMaxValue = parser.value("m");
        // Load object code string from file if possible, else print error log.
        QFile objFile(objCodeFileName);
        if(!objFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug().noquote() << errLogOpenErr.arg(objFile.fileName());
            parser.showHelp(-1);
        }

        QTextStream objStream(&objFile);
        QString objText = objStream.readAll();
        objFile.close();

        // Attempt to parse stepMax string as an integer.
        bool stepConvWorked;
        quint64 maxSimSteps = stepMaxValue.toULong(&stepConvWorked);
        if(!stepConvWorked || maxSimSteps == 0) {
            maxSimSteps = BoundExecIsaCpu::getDefaultMaxSteps();
        }
        RunHelper *helper = new RunHelper(objText,maxSimSteps, textOutputFileName,
                                          textInputFileName, *AsmProgramManager::getInstance());
        QObject::connect(helper, &RunHelper::finished, &a, &QCoreApplication::quit);

        run = helper;

    }
    else if(command == "macros") {
        QStringList macroNames;
        macroNames << "Builtins:";
        for(const auto& builtin : registry->getCoreMacros()) {
            macroNames << QString("\t%1").arg(builtin->macroName);
        }
        macroNames << "Syscalls:";
        for(const auto& syscall : registry->getSytemCalls()) {
            macroNames << QString("\t%1").arg(syscall->macroName);
        }
        macroNames << "User Defined Macros:";
        for(const auto& udm : registry->getCustomMacros()) {
            macroNames << QString("\t%1").arg(udm->macroName);
        }
        qDebug().noquote().nospace() << "Available Macros\n" << macroNames.join("\n");
        return 0;
    }
    else if(command == "listing") {
        if (!parser.isSet("n")) {
            qDebug() << "Macro name is required.";
            parser.showHelp(-1);
        }
        QString macroName = parser.value("n");
        if(!registry->hasMacro(macroName)) {
            qDebug() << "No such macro.";
            return 0;
        }
        qDebug().noquote() << registry->getMacro(macroName)->macroText;
        return 0;
    }
    else {
        parser.showHelp(0);
    }

    /*
     * This asynchronous approach must be used, because if quit() is called
     * before a.exec() happens, then the application will not actually quit.
     * So, by causing the task to be scheduled via the event loop, we allow
     * the task to be scheduled via the main event loop.
     *
     */
    if(run != nullptr) {
        pool.start(run);
    }
    return a.exec();
}
