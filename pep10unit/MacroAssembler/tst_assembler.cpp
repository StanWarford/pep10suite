#include "tst_assembler.h"
#include "macroregistry.h"
#include "macropreprocessor.h"
#include "macromodules.h"
#include "macroassembler.h"

AssemblerTest::AssemblerTest():registry(new MacroRegistry())
{

}

AssemblerTest::~AssemblerTest()
{

}

void AssemblerTest::initTestCase()
{
    preprocessor = QSharedPointer<MacroPreprocessor>::create(registry.get());
    assembler = QSharedPointer<MacroAssembler>::create(registry.get());
}

void AssemblerTest::cleanupTestCase()
{

}

void AssemblerTest::case_syntaxError_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");

    QTest::addRow("Syntax error on an isolated line")
            << "!error"
            << ModuleType::USER_PROGRAM
            << MacroTokenizer::syntaxError
            << false;

    QTest::addRow("Syntax error in argument of EQUATE.")
            << "sy:.EQUATE ()no"
            << ModuleType::USER_PROGRAM
            << MacroTokenizer::syntaxError
            << false;

    QTest::addRow("Syntax error in operand of nonunary instruction.")
            << "ADDA %no,i"
            << ModuleType::USER_PROGRAM
            << MacroTokenizer::syntaxError
            << false;

    QTest::addRow("Syntax error in address of nonunary instruction.")
            << "ADDA 15,%d"
            << ModuleType::USER_PROGRAM
            // We have specific errors for addressing modes, so pick the "specific" error.
            << MacroTokenizer::malformedAddrMode
            << false;

    QTest::addRow("Syntax error in (extra) operand of unary instruction.")
            << "RET %no"
            << ModuleType::USER_PROGRAM
            << MacroTokenizer::syntaxError
            << false;

    QTest::addRow("Syntax error in symbol declaration.")
            << "%No:RET"
            << ModuleType::USER_PROGRAM
            << MacroTokenizer::syntaxError
            << false;
}

void AssemblerTest::case_syntaxError()
{
    execute();
}

void AssemblerTest::case_unexpectedToken_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");

    QTest::addRow("Unexpected decimal const at start of line.")
            << "6975\n"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::unexpectedToken.arg("6975")
            << false;

    QTest::addRow("Unexpected hex const at start of line.")
            << "0x6975\n"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::unexpectedToken.arg("0x6975")
            << false;

    QTest::addRow("Unexpected string const at start of line.")
            << "\"I am a string\"\n"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::unexpectedToken.arg("\"I am a string\"")
            << false;

    QTest::addRow("Unexpected char const at start of line.")
            << "'i'\n"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::unexpectedToken.arg("'i'")
            << false;
}

void AssemblerTest::case_unexpectedToken()
{
    execute();
}

void AssemblerTest::preprocess(ModuleAssemblyGraph &graph, ModuleType moduleType)
{
    QFETCH(QString, ProgramText);

    auto [rootPrototype, startRootInstance] = graph.createRoot(ProgramText, moduleType);
    preprocessor->setTarget(&graph);
    auto result = preprocessor->preprocess();

    QVERIFY2(result.succes, "Preprocessing should have no errors.");
    QVERIFY2(result.error.isNull(), "Preprocessing error should be nullptr.");
}

void AssemblerTest::execute()
{
    QFETCH(QString, ProgramText);
    QFETCH(QString, ExpectedError);
    QFETCH(ModuleType, MainModuleType);
    QFETCH(bool, ExpectPass);
    auto graph = ModuleAssemblyGraph();
    preprocess(graph, MainModuleType);
    auto result = assembler->assemble(graph);
    QCOMPARE(result.success, ExpectPass);
    if(!ExpectPass && !ExpectedError.isEmpty()) {
        QCOMPARE(result.error->getErrorMessage(), ExpectedError);
    }
}
