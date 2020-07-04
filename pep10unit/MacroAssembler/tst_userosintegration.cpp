#include "tst_userosintegration.h"

#include "macroregistry.h"
#include "macroassembler.h"
#include "macrolinker.h"
#include "asmargument.h"
#include "asmcode.h"
#include "asmprogram.h"
#include "macropreprocessor.h"

UserOSIntegrationTest::UserOSIntegrationTest(): registry(new MacroRegistry())
{

}

UserOSIntegrationTest::~UserOSIntegrationTest() = default;

void UserOSIntegrationTest::initTestCase()
{
    preprocessor = QSharedPointer<MacroPreprocessor>::create(registry.get());
    assembler = QSharedPointer<MacroAssembler>::create(registry.get());
    linker = QSharedPointer<MacroLinker>::create();
}

void UserOSIntegrationTest::cleanupTestCase()
{

}

void UserOSIntegrationTest::case_testExport_data()
{
    QTest::addColumn<QString>("OSProgramText");
    QTest::addColumn<QString>("UserProgramText");
    QTest::addColumn<bool>("ExpectUserLinkPass");
    QTest::addColumn<QStringList>("ExpectedUserError");

    QTest::newRow("Check that values are exported by EXPORT.")
            << ".BURN 0xFFFF\n sym:.EQUATE 10\n .EXPORT sym\n.END"
            << ".ADDRSS sym \n .END"
            << true
            << QStringList({""});

#pragma message("See https://github.com/StanWarford/pep10suite/issues/10")
    QTest::newRow("Check that local symbolic definitions may not override those from the OS.")
            << ".BURN 0xFFFF\n sym:.EQUATE 10\n .EXPORT sym\n.END"
            << ".ADDRSS sym\n sym: .EQUATE 20 \n .END"
            << false
            << QStringList({MacroLinker::redefineExportSymbol.arg("sym")});
}

void UserOSIntegrationTest::case_testExport()
{
    auto osGraph = ModuleAssemblyGraph();
    auto userGraph = ModuleAssemblyGraph();
    execute(osGraph, userGraph);
    auto addressCode = userGraph.getRootInstance()->codeList[0];
    auto addressLine = dynamic_cast<DotAddrss*>(addressCode.get());
    QVERIFY2(addressLine->getArgument()->getArgumentValue()==10,
             "Expected exported value to be equal to 10");
}

void UserOSIntegrationTest::case_testScall_data()
{
    QTest::addColumn<QString>("OSProgramText");
    QTest::addColumn<QString>("UserProgramText");
    QTest::addColumn<bool>("ExpectUserLinkPass");
    QTest::addColumn<QStringList>("ExpectedUserError");

    QTest::newRow("Non-unary system call with no export.")
            << ".BURN 0xFFFF\n sym:.EQUATE 10\n .SCALL sym\n.END"
            << "@sym 0,i\n .END"
            << false
            << QStringList({MacroLinker::undefinedSymbol.arg("sym")});
    QTest::newRow("Check injection of non-unary system call macros into user program.")
            << ".BURN 0xFFFF\n sym:.EQUATE 10\n .SCALL sym\n .EXPORT sym \n.END"
            << "@sym 0,i\n .END"
            << true
            << QStringList();

}

void UserOSIntegrationTest::case_testScall()
{
    auto osGraph = ModuleAssemblyGraph();
    auto userGraph = ModuleAssemblyGraph();
    execute(osGraph, userGraph);
}

void UserOSIntegrationTest::case_testUScall_data()
{
    QTest::addColumn<QString>("OSProgramText");
    QTest::addColumn<QString>("UserProgramText");
    QTest::addColumn<bool>("ExpectUserLinkPass");
    QTest::addColumn<QStringList>("ExpectedUserError");

    QTest::newRow("Unary system call with no export.")
            << ".BURN 0xFFFF\n sym:.EQUATE 10\n .USCALL sym\n.END"
            << "@sym\n .END"
            << false
            << QStringList({MacroLinker::undefinedSymbol.arg("sym")});

    QTest::newRow("Check injection of unary system call macros into user program.")
            << ".BURN 0xFFFF\n sym:.EQUATE 10\n .USCALL sym\n .EXPORT sym \n.END"
            << "@sym\n .END"
            << true
            << QStringList();
}

void UserOSIntegrationTest::case_testUScall()
{
    auto osGraph = ModuleAssemblyGraph();
    auto userGraph = ModuleAssemblyGraph();
    execute(osGraph, userGraph);
}

void UserOSIntegrationTest::case_syscallInOS_data()
{
    QTest::addColumn<QString>("OSProgramText");
    QTest::addColumn<QString>("UserProgramText");
    QTest::addColumn<bool>("ExpectUserLinkPass");
    QTest::addColumn<QStringList>("ExpectedUserError");

    QTest::newRow("Operating system using system call macros.")
            << ".BURN 0xFFFF\n sym:.EQUATE 10\n .USCALL sym\n @sym.END"
            << "@sym\n .END"
            << false
            << QStringList({MacroLinker::undefinedSymbol.arg("sym")});
}

void UserOSIntegrationTest::case_syscallInOS()
{
    auto osGraph = ModuleAssemblyGraph();
    QFETCH(QString, OSProgramText);
    QFETCH(QString, UserProgramText);
    auto [OSRootPrototype, OSStartRootInstance] =
            osGraph.createRoot(OSProgramText, ModuleType::OPERATING_SYSTEM);

    // Must clear latent system calls between runs.
    registry->clearSystemCalls();
    preprocessor->setTarget(&osGraph);

    auto result = preprocessor->preprocess();

    QVERIFY2(!result.succes, "Preprocessing should fail with undefined macro.");
    QVERIFY2(result.error->getErrorMessage() == noSuchMaro,
            "System call in operating system should yield \"no such macro..\" exception");
}

void UserOSIntegrationTest::execute(ModuleAssemblyGraph& osGraph,
                                    ModuleAssemblyGraph& userGraph)
{

    QFETCH(QString, OSProgramText);
    QFETCH(QString, UserProgramText);
    QFETCH(bool, ExpectUserLinkPass);
    QFETCH(QStringList, ExpectedUserError);

    // Create assembly projects for OS, user
    auto [OSRootPrototype, OSStartRootInstance] =
            osGraph.createRoot(OSProgramText, ModuleType::OPERATING_SYSTEM);
    auto [UserRootPrototype, UserStartRootInstance] =
            userGraph.createRoot(UserProgramText, ModuleType::USER_PROGRAM);

    // Must remove any existing system calls anytime an OS is compiled.
    registry->clearSystemCalls();

    // Must assemble & link OS before starting user program,
    // otherwise macros, EXPORT'ed values will not be available.
    preprocess(osGraph);
    auto osResult = assembler->assemble(osGraph);

    QVERIFY2(osResult.success, "OS assembly should have no errors.");
    QVERIFY2(osResult.error.isNull(), "OS assembly error should be nullptr.");

    linker->clearOSSymbolTable();
    linker->setOSSymbolTable(OSStartRootInstance->symbolTable);
    auto osLinkResult = linker->link(osGraph);
    QVERIFY2(osLinkResult.errorList.isEmpty(), "OS should link successfully");

    // OS must be linked before user program assembly is attempted.
    // Otherwise, system calls will not be registered correctly.
    preprocess(userGraph);
    auto userResult = assembler->assemble(userGraph);
    QVERIFY2(userResult.success, "User program assembly should have no errors.");
    QVERIFY2(userResult.error.isNull(), "User program assembly error should be nullptr.");

    auto userLinkResult = linker->link(userGraph);

    QVERIFY2(ExpectUserLinkPass == userLinkResult.success,
            "Expected success state of linker did not match actual success state.");
    if(!userLinkResult.success && !ExpectedUserError.isEmpty()) {
        QVERIFY2(!userLinkResult.errorList.isEmpty(),
                 "Expected an error message, got empty list.");
        QVERIFY2(userLinkResult.errorList.size() == ExpectedUserError.size(),
                 "Expected error list to be the same size.");
        for(int it = 0; it < userLinkResult.errorList.size(); it ++) {
            if(ExpectedUserError.at(it) == "") continue;
            // Compare messages line-by-line.
            QCOMPARE(std::get<1>(userLinkResult.errorList.at(it)), ExpectedUserError.at(it));
        }
    }

}

void UserOSIntegrationTest::preprocess(ModuleAssemblyGraph &graph)
{
    preprocessor->setTarget(&graph);
    auto result = preprocessor->preprocess();
    QVERIFY2(result.succes, "Preprocessing should have no errors.");
    QVERIFY2(result.error.isNull(), "Preprocessing error should be nullptr.");
}

