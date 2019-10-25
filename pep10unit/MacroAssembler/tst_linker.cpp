#include "tst_linker.h"
#include "macroregistry.h"
#include "macroassembler.h"
#include "macrolinker.h"
#include "asmargument.h"
#include "asmcode.h"
#include "asmprogram.h"
#include "macropreprocessor.h"

LinkerTest::LinkerTest(): registry(new MacroRegistry())
{

}

LinkerTest::~LinkerTest() = default;

void LinkerTest::initTestCase()
{
    preprocessor = QSharedPointer<MacroPreprocessor>::create(registry.get());
    assembler = QSharedPointer<MacroAssembler>::create(registry.get());
    linker = QSharedPointer<MacroLinker>::create();
}

void LinkerTest::cleanupTestCase()
{

}

void LinkerTest::case_noOS_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");

    // Program lacking OS definition.
    QTest::newRow("Program lacking OS.")
            << "\n.END"
            << ModuleType::USER_PROGRAM
            << MacroLinker::noOperatingSystem;
}

void LinkerTest::case_noOS()
{
    //QFETCH(QString, ProgramText);
    QFETCH(QString, ExpectedError);
    QFETCH(ModuleType, MainModuleType);
    auto graph = ModuleAssemblyGraph();
    preprocess(graph, MainModuleType);
    assemble(graph);

    auto result = linker->link(graph);
    QVERIFY2(!result.success, "Assembly expected to fail in absence of OS.");
    QVERIFY2(result.errorList.size() == 1, "Expected an error list of size 1.");
    QCOMPARE(std::get<1>(result.errorList.at(0)), ExpectedError);
}

void LinkerTest::case_programTooLong_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QStringList>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");

    // Program of length 64k-1
    QTest::newRow("Valid block of length 65354.")
            << ".BLOCK 0xfffe\n.END"
            << ModuleType::USER_PROGRAM
            << QStringList({""})
            << true;

    // Program of length 64k
    QTest::newRow("Valid block of length 65355.")
            << ".BLOCK 0xffff\n.END"
            << ModuleType::USER_PROGRAM
            << QStringList({""})
            << true;

    // Program of length 64k + 1
    QTest::newRow("Invalid block of length 65356.")
            << ".BLOCK 0xffff\n.BLOCK 0x0001\n.END"
            << ModuleType::USER_PROGRAM
            << QStringList({MacroLinker::exceededMemory})
            << false;
    // Program of length 64k + 1
    QTest::newRow("Invalid block of length 2x65355.")
            << ".BLOCK 0xffff\n.BLOCK 0xffff\n.END"
            << ModuleType::USER_PROGRAM
            << QStringList({MacroLinker::exceededMemory})
            << false;


}

void LinkerTest::case_programTooLong()
{
    execute();
}

void LinkerTest::case_noSuchSymbol_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QStringList>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");

    // Program with a symbol that does not appear in the OS.
    QTest::newRow("Symbol not in OS.")
            << "LDWA noSym,i\n.END"
            << ModuleType::USER_PROGRAM
            << QStringList({MacroLinker::undefinedSymbol.arg("noSym")})
            << false;
}

void LinkerTest::case_noSuchSymbol()
{
    execute();
}

void LinkerTest::case_osNoBurn_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QStringList>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");

    // OS with no .BURN statement.
    QTest::newRow("No burn in OS.")
            << "\n.END"
            << ModuleType::OPERATING_SYSTEM
            << QStringList({MacroLinker::oneBURN})
            << false;
}

void LinkerTest::case_osNoBurn()
{
    execute();
}

void LinkerTest::case_osManyBurn_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QStringList>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");

    // OS with 2+ .BURN statements.
    QTest::newRow("Too many .BURN's in OS.")
            << ".BURN 0xffff\n .BURN 0xffff\n.END"
            << ModuleType::OPERATING_SYSTEM
            << QStringList({MacroLinker::oneBURN})
            << false;
}

void LinkerTest::case_osManyBurn()
{
    execute();
}

void LinkerTest::execute()
{
    //QFETCH(QString, ProgramText);
    QFETCH(QStringList, ExpectedError);
    QFETCH(ModuleType, MainModuleType);
    QFETCH(bool, ExpectPass);
    auto graph = ModuleAssemblyGraph();
    auto osSymbols = QSharedPointer<SymbolTable>::create();
    preprocess(graph, MainModuleType);
    assemble(graph);
    // If not assembling an OS, the linker expects a list os symbols defined by the OS.
    linker->clearOSSymbolTable();
    linker->setOSSymbolTable(osSymbols);

    auto result = linker->link(graph);
    QCOMPARE(result.success, ExpectPass);
    if(!result.success && !ExpectedError.isEmpty()) {
        QVERIFY2(!result.errorList.isEmpty(), "Expected an error message, got empty list.");
        QVERIFY2(result.errorList.size() == ExpectedError.size(), "Expected error list to be the same size.");
        for(int it = 0; it < result.errorList.size(); it ++) {
            if(ExpectedError.at(it) == "") continue;
            QCOMPARE(std::get<1>(result.errorList.at(it)), ExpectedError.at(it));
        }
    }
}

void LinkerTest::preprocess(ModuleAssemblyGraph &graph, ModuleType programType)
{
    QFETCH(QString, ProgramText);

    auto [rootPrototype, startRootInstance] = graph.createRoot(ProgramText, programType);
    preprocessor->setTarget(&graph);
    auto result = preprocessor->preprocess();

    QVERIFY2(result.succes, "Preprocessing should have no errors.");
    QVERIFY2(result.error.isNull(), "Preprocessing error should be nullptr.");
}

void LinkerTest::assemble(ModuleAssemblyGraph &graph)
{
    //QFETCH(QString, ProgramText);
    auto result = assembler->assemble(graph);
    QVERIFY2(result.success, "Assembly should have no errors.");
    QVERIFY2(result.error.isNull(), "Assembly error should be nullptr.");
}
