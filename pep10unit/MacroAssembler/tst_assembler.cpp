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

    // Below are all tokens that may not appear at the start of the line.
    QTest::addRow("Unexpected addressing mode at start of line.")
            << ",i\n"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::unexpectedToken.arg("i")
            << false;


    QTest::addRow("Unexpected char const at start of line.")
            << "'i'\n"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::unexpectedToken.arg("'i'")
            << false;

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
}

void AssemblerTest::case_unexpectedToken()
{
    execute();
}

void AssemblerTest::case_unexpectedEOL_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");

    // Unary instructions.
    QTest::addRow("Unexpected EOL in unary instruction.")
            << "asra asra \n"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::unxpectedEOL
            << false;

    QTest::addRow("Unexpected EOL in sym:unary instruction.")
            << "ST: ASRA k\n"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::unxpectedEOL
            << false;

    QTest::addRow("Unexpected EOL in unary instruction ;comment.")
            << "asra asra ;Com\n"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::unxpectedEOL
            << false;

    QTest::addRow("Unexpected EOL in sym:unary instruction ;comment.")
            << "ST: ASRA k ;Com\n"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::unxpectedEOL
            << false;

    // Non-unary instructions.
    QTest::addRow("Unexpected EOL in nonunary instruction.")
            << "LDWA k,d,d \n"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::unxpectedEOL
            << false;

    QTest::addRow("Unexpected EOL in sym:nonunary instruction.")
            << "ST: LDWA k,d,d \n"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::unxpectedEOL
            << false;

    QTest::addRow("Unexpected EOL in nonunary instruction ;comment.")
            << "LDWA k,d,d ;Com\n"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::unxpectedEOL
            << false;

    QTest::addRow("Unexpected EOL in sym:nonunary instruction ;comment.")
            << "ST: LDWA k,d,d ;Com\n"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::unxpectedEOL
            << false;

    // Dot Commands.
    QTest::addRow("Unexpected EOL in sym:EQUATE instruction.")
            << "sym:equate 1 1"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::unxpectedEOL
            << false;

    QTest::addRow("Unexpected EOL in sym:EQUATE instruction.")
            << "sym:equate 1 k"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::unxpectedEOL
            << false;

    QTest::addRow("Unexpected EOL in WORD instruction.")
            << ".BLOCK 4,d"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::unxpectedEOL
            << false;

    QTest::addRow("Unexpected EOL in sym:WORD instruction.")
            << "sy:.BLOCK 4,d"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::unxpectedEOL
            << false;

    QTest::addRow("Unexpected EOL in sym:EQUATE instruction ;comment.")
            << "sym:equate 1 1 ;Com\n"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::unxpectedEOL
            << false;

    QTest::addRow("Unexpected EOL in sym:EQUATE instruction ;comment.")
            << "sym:equate 1 k ;Com\n"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::unxpectedEOL
            << false;

    QTest::addRow("Unexpected EOL in WORD instruction ;comment.")
            << ".BLOCK 4,d ;Com\n"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::unxpectedEOL
            << false;

    QTest::addRow("Unexpected EOL in sym:WORD instruction ;comment.")
            << "sy:.BLOCK 4,d ;Com\n"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::unxpectedEOL
            << false;
}

void AssemblerTest::case_unexpectedEOL()
{
    execute();
}

void AssemblerTest::case_unexpectedSymbolDeclaration_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");

    using element = typename std::tuple<QString,QString, bool, QString>;
    QList<element> identList;
    // For each token type, construct a test case with a symbol before it.
    // For tokens that identify multiple code lines (identifiers, dot commands)
    // must test multiple permutations of these symbols.
    identList  << element("addressing mode",",i", false, MacroAssembler::unexpectedSymbolDecl)
               << element("character const", "'l'", false, MacroAssembler::unexpectedSymbolDecl)
               << element("decimal const","72", false, MacroAssembler::unexpectedSymbolDecl)
               << element(".BLOCK",".BLOCK", false, MacroAssembler::badBlockArgument)
               << element(".BLOCK 5",".BLOCK 5\n.END\n", true, "")
               << element("sy:.BLOCK","sy:.BLOCK", false, MacroAssembler::unexpectedSymbolDecl)
               << element("sy:.BLOCK 5","sy:.BLOCK 5", false, MacroAssembler::unexpectedSymbolDecl)
               << element("sy:.EQUATE","sy:.EQUATE", false, MacroAssembler::unexpectedSymbolDecl)
               << element("sy:.EQUATE","sy:.EQUATE 5", false, MacroAssembler::unexpectedSymbolDecl)
               << element(".SCALL",".SCALL", false, MacroAssembler::unexpectedSymbolDecl)
               << element("sy:.SCALL","sy:.SCALL", false, MacroAssembler::unexpectedSymbolDecl)
               << element(".USCALL",".USCALL", false, MacroAssembler::unexpectedSymbolDecl)
               << element("sy:.USCALL","sy:.USCALL", false, MacroAssembler::unexpectedSymbolDecl)
               << element("hex constant","0x99", false, MacroAssembler::unexpectedSymbolDecl)
               << element("unary instruction","ASRA\n.END\n", true, "")
               << element("nonunary instruction","LDWA 0,d\n.END\n", true, "")
               << element("identifier","no", false, MacroAssembler::invalidMnemonic.arg("no"))
               << element("string constant","\"hi\"", false, MacroAssembler::unexpectedSymbolDecl)
               << element("symbol declaration","sy:", false, MacroAssembler::unexpectedSymbolDecl);

    QString symbol = "sy2:";
    for(auto ident : identList) {
        QTest::addRow("%s", QString("Symbol before %1.")
                      .arg(std::get<0>(ident)).toStdString().c_str())
                << QString("sy: %1").arg(std::get<1>(ident))
                << ModuleType::USER_PROGRAM
                << std::get<3>(ident)
                << std::get<2>(ident);
    }

}

void AssemblerTest::case_unexpectedSymbolDeclaration()
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
    if(!result.success && !ExpectedError.isEmpty()) {
        QVERIFY2(!result.error.isNull(), "Expected an error message, got nullptr.");
        QCOMPARE(result.error->getErrorMessage(), ExpectedError);
    }
}
