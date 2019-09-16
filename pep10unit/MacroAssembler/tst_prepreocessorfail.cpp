#include "tst_prepreocessorfail.h"
#include "macromodules.h"

PreprocessorFailure::PreprocessorFailure(): registry(new MacroRegistry()),
    preprocessor(new MacroPreprocessor(registry.get()))
{
}

PreprocessorFailure::~PreprocessorFailure()
{

}

void PreprocessorFailure::initTestCase()
{

}

void PreprocessorFailure::cleanupTestCase()
{
    preprocessor->setTarget(nullptr);
}

void PreprocessorFailure::case_noSuchMacro_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");

    QTest::newRow("Mispell Core")
            << "@asr4\n.END"
            << noSuchMaro
            << false;

    QTest::newRow("Wildly Wrong")
            << "@hllowrld\n.END"
            << noSuchMaro
            << false;

    // Macro names should be case insensitive (e.g. @AsRa4 should be recognized)
    // as @ASRA4.
    QTest::newRow("Correct name, wrong capitalization")
            << "@AsRa4\n.END"
            << ""
            << true;
}

void PreprocessorFailure::case_noSuchMacro()
{
    QFETCH(QString, ProgramText);
    QFETCH(QString, ExpectedError);
    QFETCH(bool, ExpectPass);

    auto graph = ModuleAssemblyGraph();
    auto [rootPrototype, startRootInstance] = graph.createRoot(ProgramText, ModuleType::USER_PROGRAM);
    preprocessor->setTarget(&graph);
    auto result = preprocessor->preprocess();

    QCOMPARE(result.succes, ExpectPass);
    if(!ExpectPass) {
        QVERIFY2(!result.error.isNull(), "Expected an error message.");
        QCOMPARE(result.error->getErrorMessage(), ExpectedError);
    }

}

void PreprocessorFailure::case_macroRequiresIdentifier_data()
{
        QTest::addColumn<QString>("ProgramText");
        QTest::addColumn<QString>("PreferedError");
        QTest::addColumn<QString>("AlternateError");
        QTest::addColumn<bool>("ExpectPass");

        QStringList fakeMacros;
        fakeMacros << "4ASRA2"
                   << "()ASRA2"
                   << "_ASRA2"
                   << "ÁASRA4";

        registry->clearCustomMacros();
        for(auto macro : fakeMacros) {
            registry->registerCustomMacro(macro, "@"+macro+" 0\n.END");
        }

        QTest::newRow("Non-existant macro starting with integer.")
                << "@4m\n.END"
                << noIdentifier
                << noSuchMaro
                << false;

        QTest::newRow("Valid macro starting with integer.")
                << "@"+fakeMacros[0]+"\n.END"
                << noIdentifier
                << noSuchMaro
                << false;

        QTest::newRow("Non-existant macro starting with punctuation.")
                << "@()wrd\n.END"
                << noIdentifier
                << noSuchMaro
                << false;

        QTest::newRow("Valid macro starting starting with punctuation.")
                << "@"+fakeMacros[1]+"\n.END"
                << noIdentifier
                << noSuchMaro
                << false;


        QTest::newRow("Non-existant macro starting with underscore.")
                << "@_ASRa4\n.END"
                << noIdentifier
                << noSuchMaro
                << false;

        QTest::newRow("Valid macro starting with underscore.")
                << "@"+fakeMacros[2]+"\n.END"
                << ""
                << ""
                << true;

        QTest::newRow("Non-existant macro starting with non-ASCII7.")
                << "@ÁSRa4\n.END"
                << noIdentifier
                << noSuchMaro
                << false;

        QTest::newRow("Valid macro starting with non-ASCII7.")
                << "@"+fakeMacros[3]+"\n.END"
                << noIdentifier
                << noSuchMaro
                << false;

        // Older versions of Pep10 had issues where identifiers could be detected
        // non-contiguously with the @ in a macro. These unit tests are to
        // validate that this behavior does not return.

        QTest::newRow("Non-existant macro starting with integer followed by a comment.")
                << "@4m;Comment\n.END"
                << noIdentifier
                << noSuchMaro
                << false;

        QTest::newRow("Macro in registry starting with integer followed by a comment.")
                << "@"+fakeMacros[0]+";Comment\n.END"
                << noIdentifier
                << noSuchMaro
                << false;

        QTest::newRow("@ Followed by a comment with a non-existent macro.")
                << "@;4m\n.END"
                << noIdentifier
                << noSuchMaro
                << false;

        QTest::newRow("@ Followed by a comment with a macro starting with an integer.")
                << "@;"+fakeMacros[0]+"\n.END"
                << noIdentifier
                << noSuchMaro
                << false;

        QTest::newRow("@ Followed by a comment with a valid macro.")
                << "@;ASRA4\n.END"
                << noIdentifier
                << noSuchMaro
                << false;
}

void PreprocessorFailure::case_macroRequiresIdentifier()
{
        QFETCH(QString, ProgramText);
        QFETCH(QString, PreferedError);
        QFETCH(QString, AlternateError);
        QFETCH(bool, ExpectPass);

        auto graph = ModuleAssemblyGraph();
        auto [rootPrototype, startRootInstance] = graph.createRoot(ProgramText, ModuleType::USER_PROGRAM);
        preprocessor->setTarget(&graph);
        auto result = preprocessor->preprocess();
        QCOMPARE(result.succes, ExpectPass);
        if(!ExpectPass) {
            QVERIFY2(!result.error.isNull(), "Expected an error message.");
            QVERIFY2(result.error->getErrorMessage().compare(PreferedError) ||
                    result.error->getErrorMessage().compare(AlternateError),
                    "Error message did not match expected or alternate.");
        }
    }

void PreprocessorFailure::case_badArgCount_data()
{
    registry->clearCustomMacros();
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<bool>("ExpectSuccess");
    QTest::addColumn<QString>("ExpectedError");

    // Give varying number of arguments to a macro.
    // Test various permutations of 0-argument macros.
    QTest::newRow("0 argument macro given 1 argument(s).")
            << "@ASRA4\n.END"
            << true << badArgCount;
    QTest::newRow("0 argument macro given 1 argument(s).")
            << "@ASRA4 1\n.END"
            << false << badArgCount;
    QTest::newRow("0 argument macro given 2 argument(s).")
            << "@ASRA4 1,2\n.END"
            << false << badArgCount;

    // Test various permutations of 1-argument macros.
    registry->registerCustomMacro("onearg", "@onearg 1\n.END");
    QTest::newRow("1 argument macro given 0 argument(s).")
            << "@onearg \n.END"
            << false << badArgCount;
    QTest::newRow("1 argument macro given 1 argument(s).")
            << "@onearg 1\n.END"
            << true << badArgCount;
    QTest::newRow("1 argument macro given 2 argument(s).")
            << "@onearg 1,2\n.END"
            << false << badArgCount;

    // Test various permutations of 2-argument macros.
    registry->registerCustomMacro("onearg", "@onearg 1\n.END");
    QTest::newRow("2 argument macro given 0 argument(s).")
            << "@XCHGA \n.END"
            << false << badArgCount;
    QTest::newRow("2 argument macro given 1 argument(s).")
            << "@XCHGA 1\n.END"
            << false << badArgCount;
    QTest::newRow("2 argument macro given 2 argument(s).")
            << "@XCHGA 1,d\n.END"
            << true << badArgCount;
}

void PreprocessorFailure::case_badArgCount()
{
    QFETCH(QString, ProgramText);
    QFETCH(bool, ExpectSuccess);
    QFETCH(QString, ExpectedError);

    auto graph = ModuleAssemblyGraph();
    auto [rootPrototype, startRootInstance] = graph.createRoot(ProgramText, ModuleType::USER_PROGRAM);
    preprocessor->setTarget(&graph);
    auto result = preprocessor->preprocess();

    QCOMPARE(result.succes, ExpectSuccess);
    if(!result.succes) {
        QCOMPARE(result.error->getErrorMessage(), ExpectedError);
    }
}

void PreprocessorFailure::case_noDollarInArgument_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<bool>("ExpectSuccess");
    QTest::addColumn<QString>("ExpectedError");


    QVERIFY2(!registry->getMacro("XCHGA").isNull(),
             "Expected @XCHGA 2 to exist.");

    QTest::newRow("Pass $0 to a macro.")
            << "@XCHGA $0,d\n.END"
            << false
            << invalidArg.arg("$0");

    QTest::newRow("Pass $1 to a macro.")
            << "@XCHGA $1,d\n.END"
            << false
            << invalidArg.arg("$1");

    QTest::newRow("Pass $2 to a macro.")
            << "@XCHGA $2,d\n.END"
            << false
            << invalidArg.arg("$2");

    // Even though identifier macro substitutions are invalid,
    // these are things caught by the assembler, allowing the preprocessor
    // do less work.
    QTest::newRow("Pass $ followed by text to a macro.")
            << "@XCHGA $cat,d\n.END"
            << true
            << "";

    QTest::newRow("Pass $ _ followed by text to a macro.")
            << "@XCHGA $_cat,d\n.END"
            << true
            << "";
}

void PreprocessorFailure::case_noDollarInArgument()
{
    QFETCH(QString, ProgramText);
    QFETCH(bool, ExpectSuccess);
    QFETCH(QString, ExpectedError);

    auto graph = ModuleAssemblyGraph();
    auto [rootPrototype, startRootInstance] = graph.createRoot(ProgramText, ModuleType::USER_PROGRAM);
    preprocessor->setTarget(&graph);
    auto result = preprocessor->preprocess();
    QCOMPARE(result.succes, ExpectSuccess);
    if(!ExpectSuccess) {
        QVERIFY2(!result.error.isNull(), "Expected an error message.");
        QCOMPARE(result.error->getErrorMessage(), ExpectedError);
    }
}

void PreprocessorFailure::case_circularInclude_data()
{
    registry->clearCustomMacros();

    // Loop of 2
    registry->registerCustomMacro("LOOPA", "@LOOPA 0\n@LOOPB\n.END\n");
    registry->registerCustomMacro("LOOPB", "@LOOPB 0\n@LOOPA\n.END\n");

    // Loop of 4
    registry->registerCustomMacro("LOOPC", "@LOOPC 0\n@LOOPD\n.END\n");
    registry->registerCustomMacro("LOOPD", "@LOOPD 0\n@LOOPE\n.END\n");
    registry->registerCustomMacro("LOOPE", "@LOOPE 0\n@LOOPF\n.END\n");
    registry->registerCustomMacro("LOOPF", "@LOOPF 0\n@LOOPC\n.END\n");

    // Trigger loop of 2 at depth of 1.
    registry->registerCustomMacro("TRIGA", "@TRIGA 0\n@LOOPA\n.END\n");
    // Trigger loop of 2 at depth of 2.
    registry->registerCustomMacro("TTRIGA", "@TTRIGA 0\n@TRIGA\n.END\n");

    // Trigger the loop of 2 at depth 2 and the loop of 4 at depth 1.
    registry->registerCustomMacro("DBLLOOP", "@DBLLOOP 0\n@TRIGA\n@LOOPC\n.END\n");

    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<QString>("ExpectedError");

    QTest::newRow("Loop of 2, spawned in root module.")
            << "@LOOPA\n.END"
            << circularInclude;

    QTest::newRow("Loop of 4, spawned in root module.")
            << "@LOOPC\n.END"
            << circularInclude;

    QTest::newRow("Loop of 2, spawned by module included by root (depth 1).")
            << "@TRIGA\n.END"
            << circularInclude;

    QTest::newRow("Loop of 2, spawned by module included by root (depth 2).")
            << "@TTRIGA\n.END"
            << circularInclude;

    QTest::newRow("Trigger multiple include loops.")
            << "@DBLLOOP\n.END"
            << circularInclude;

}

void PreprocessorFailure::case_circularInclude()
{
    QFETCH(QString, ProgramText);
    QFETCH(QString, ExpectedError);

    auto graph = ModuleAssemblyGraph();
    auto [rootPrototype, startRootInstance] = graph.createRoot(ProgramText, ModuleType::USER_PROGRAM);
    preprocessor->setTarget(&graph);
    auto result = preprocessor->preprocess();

    QVERIFY2(!result.error.isNull(), "Expected an error message.");
    QCOMPARE(result.error->getErrorMessage(), ExpectedError);
}

void PreprocessorFailure::case_selfReference_data()
{
    registry->clearCustomMacros();
    QString macroName = "selfRef";
    registry->registerCustomMacro(macroName, "@"+macroName+" 0\n@"+macroName+"\n.END");
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<QString>("ExpectedError");

    QTest::newRow("Self-referencing macro.")
            << "@"+macroName+"\n.END"
            << selfRefence;
}

void PreprocessorFailure::case_selfReference()
{
    QFETCH(QString, ProgramText);
    QFETCH(QString, ExpectedError);

    auto graph = ModuleAssemblyGraph();
    auto [rootPrototype, startRootInstance] = graph.createRoot(ProgramText, ModuleType::USER_PROGRAM);
    preprocessor->setTarget(&graph);
    auto result = preprocessor->preprocess();

    QVERIFY2(!result.error.isNull(), "Expected an error message.");
    QCOMPARE(result.error->getErrorMessage(), ExpectedError);
}
