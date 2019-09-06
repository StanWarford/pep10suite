#include "tst_tokenizerfail.h"
#include "macroregistry.h"
#include "macrotokenizer.h"
#include "macromodules.h"

TokenizerFailure::TokenizerFailure(): registry(new MacroRegistry())
{

}

TokenizerFailure::~TokenizerFailure()
{

}

void TokenizerFailure::initTestCase()
{
    tokenizer = QSharedPointer<MacroTokenizer>::create();
}

void TokenizerFailure::cleanupTestCase()
{

}

void TokenizerFailure::case_malformedMacroInvoke_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");

    // Tests expected to fail.
    QTest::newRow("Special character after @.")
            << "@!n"
            << MacroTokenizer::malformedMacroInvocation
            << false;

    QTest::newRow("Space between @ and identifier.")
            << "@ name"
            << MacroTokenizer::malformedMacroInvocation
            << false;

    QTest::newRow("Comment after @.")
            << "@;Comment"
            << MacroTokenizer::malformedMacroInvocation
            << false;

    QTest::newRow("Special character in identifier.")
            << "@AS!RA4"
            << MacroTokenizer::malformedMacroInvocation
            << false;

    // Tests expected to pass.
    QTest::newRow("Valid macro name.")
            << "@XCHGA"
            << ""
            << true;
    QTest::newRow("Valid macro name with integer.")
            << "@ASRA4"
            << ""
            << true;
    QTest::newRow("Valid macro name with _.")
            << "@AS_RA"
            << ""
            << true;
    QTest::newRow("Valid macro name followed by a comment.")
            << "@ASRA4;HelloWorld"
            << ""
            << true;
}

void TokenizerFailure::case_malformedMacroInvoke()
{
    execute();
}

void TokenizerFailure::case_malformedMacroSubstitution_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");

    QTest::newRow("Special character after $.")
            << "$!n"
            << MacroTokenizer::malformedMacroSubstitution
            << false;

    QTest::newRow("Space between $ and number.")
            << "$ 1"
            << MacroTokenizer::malformedMacroSubstitution
            << false;

    QTest::newRow("$ followed by identifier.")
            << "$failure"
            << MacroTokenizer::malformedMacroSubstitution
            << false;

    QTest::newRow("Comment after $.")
            << "$;Comment"
            << MacroTokenizer::malformedMacroSubstitution
            << false;

    QTest::newRow("Two $'s.")
            << "$$"
            << MacroTokenizer::malformedMacroSubstitution
            << false;

    //While this is a valid substitution, the tokenizer expects all
    // preprocessing to have been completed, meaning all
    // macro substitution placeholders should have been replaced with
    // their target text before tokenization.
    QTest::newRow("Valid substitution, expect tokenization failure.")
            << "$1"
            << MacroTokenizer::malformedMacroSubstitution
            << false;
}

void TokenizerFailure::case_malformedMacroSubstitution()
{
    execute();
}

void TokenizerFailure::case_malformedAddrMode_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");

    // Tests expected to fail.
    QTest::newRow("Integer in place of address mode.")
            << ",22"
            << MacroTokenizer::malformedAddrMode
            << false;

    QTest::newRow("$ in place of address mode.")
            << ",$$"
            << MacroTokenizer::malformedAddrMode
            << false;

    QTest::newRow("No address mode.")
            << ","
            << MacroTokenizer::malformedAddrMode
            << false;

    QTest::newRow("Comment in address mode.")
            << ",;Comment"
            << MacroTokenizer::malformedAddrMode
            << false;

    QTest::newRow("Single digit integer followed by comment.")
            << ",1;Comment"
            << MacroTokenizer::malformedAddrMode
            << false;

    QTest::newRow("Space followed by comment.")
            << ", ;Comment"
            << MacroTokenizer::malformedAddrMode
            << false;

    QTest::newRow("Special character followed by a space.")
            << ",!;Comment"
            << MacroTokenizer::malformedAddrMode
            << false;

    QTest::newRow("Invalid addressing mode.")
            << ",fact"
            << MacroTokenizer::malformedAddrMode
            << false;

    QTest::newRow("SXF addressing mode (from Pep8).")
            << ",sxf"
            << MacroTokenizer::malformedAddrMode
            << false;

    // Check that the tokenizer doesn't "skip" to the 'i' in light.
    QTest::newRow("Nonsensense identifier.")
            << ", light"
            << MacroTokenizer::malformedAddrMode
            << false;

    //Test expected to pass.
    QTest::newRow("Comma followed by a space, then an identifier.")
            << ", i"
            << ""
            << true;

    QTest::newRow("Comma followed by a tab, then an identifier.")
            << ",\ti"
            << ""
            << true;
}

void TokenizerFailure::case_malformedAddrMode()
{
    execute();
}

void TokenizerFailure::case_malformedCharConst_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");

    QTest::newRow("No character data.")
            << "''"
            << MacroTokenizer::malformedCharConst
            << false;

    QTest::newRow("Two characters in char constant.")
            << "'ok'"
            << MacroTokenizer::malformedCharConst
            << false;

    QTest::newRow("Invalid escape sequence \\k.")
            << "'\\k'"
            << MacroTokenizer::malformedCharConst
            << false;

    QTest::newRow("Invalid escape sequence \\0.")
            << "'\\0'"
            << MacroTokenizer::malformedCharConst
            << false;

    QTest::newRow("Invalid escape sequence \\!.")
            << "'\\!'"
            << MacroTokenizer::malformedCharConst
            << false;

    QTest::newRow("Invalid hex escape sequence \\xf.")
            << "'\\xf'"
            << MacroTokenizer::malformedCharConst
            << false;

    QTest::newRow("Invalid hex escape sequence \\x123.")
            << "'\\x123'"
            << MacroTokenizer::malformedCharConst
            << false;

    QTest::newRow("Double escape sequence..")
            << "'\\r\\n'"
            << MacroTokenizer::malformedCharConst
            << false;

    QTest::newRow("Double hex escape sequence..")
            << "'\\x16\\x25'"
            << MacroTokenizer::malformedCharConst
            << false;

    QTest::newRow("Well formed hex escape sequence.")
            << "'\\xab'"
            << ""
            << true;

    QTest::newRow("Well formed escape sequence.")
            << "'\\n'"
            << ""
            << true;

    QTest::newRow("Space character.")
            << "' '"
            << ""
            << true;
}

void TokenizerFailure::case_malformedCharConst()
{
    execute();
}

void TokenizerFailure::case_malformedDecConst_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");

    QTest::newRow("Identifier in dec const.")
            << "6a7"
            << MacroTokenizer::malformedDecConst
            << false;

    QTest::newRow("Special character in dec const (6!7).")
            << "6!7"
            << MacroTokenizer::malformedDecConst
            << false;

    QTest::newRow("Special character in dec const (-67_).")
            << "-67_"
            << MacroTokenizer::malformedDecConst
            << false;

    QTest::newRow("Double negative decimal constant.")
            << "--67"
            << MacroTokenizer::malformedDecConst
            << false;

    QTest::newRow("Double positive decimal constant.")
            << "++67"
            << MacroTokenizer::malformedDecConst
            << false;

    QTest::newRow("Positive-Negative decimal constant.")
            << "+-67"
            << MacroTokenizer::malformedDecConst
            << false;

    QTest::newRow("Negative-Positive decimal constant.")
            << "-+67"
            << MacroTokenizer::malformedDecConst
            << false;

    // Tests that should pass.
    QTest::newRow("Negative decimal constant.")
            << "-67"
            << ""
            << true;

    QTest::newRow("Positive decimal constant.")
            << "+67"
            << ""
            << true;

}

void TokenizerFailure::case_malformedDecConst()
{
    execute();
}

void TokenizerFailure::case_malformedHexConst_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");

    QTest::newRow("Empty hex value.")
            << "0x"
            << MacroTokenizer::malformedHexConst
            << false;

    QTest::newRow("0x followed by space.")
            << "0x  "
            << MacroTokenizer::malformedHexConst
            << false;

    QTest::newRow("0x!.")
            << "0x!"
            << MacroTokenizer::malformedHexConst
            << false;

    QTest::newRow("Too many x's.")
            << "0xx"
            << MacroTokenizer::malformedHexConst
            << false;

    QTest::newRow("Space between x and hex value.")
            << "0x 98"
            << MacroTokenizer::malformedHexConst
            << false;

    QTest::newRow("Invalid hex value 0x9p.")
            << "0x9p"
            << MacroTokenizer::malformedHexConst
            << false;

    QTest::newRow("Double 0x.")
            << "0x0x9595"
            << MacroTokenizer::malformedHexConst
            << false;

    // Tests expected to pass.
    QTest::newRow("1 character hex constant.")
            << "0x1"
            << ""
            << true;

    QTest::newRow("2 character hex constant.")
            << "0x1a"
            << ""
            << true;

    QTest::newRow("3 character hex constant.")
            << "0x1b3"
            << ""
            << true;

    QTest::newRow("4 character hex constant.")
            << "0xa23f"
            << ""
            << true;
}

void TokenizerFailure::case_malformedHexConst()
{
    execute();
}

void TokenizerFailure::case_malformedDot_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");

    QTest::newRow("Number starting dot command.")
            << ".5dot"
            << MacroTokenizer::malformedDot
            << false;

    QTest::newRow("Number inside dot command.")
            << ".d04t"
            << MacroTokenizer::malformedDot
            << false;

    QTest::newRow("Special character inside dot command.")
            << ".d()t"
            << MacroTokenizer::malformedDot
            << false;

    QTest::newRow("Special character starting dot command.")
            << ".()t"
            << MacroTokenizer::malformedDot
            << false;

    QTest::newRow("Too many .'s.")
            << "..EQUATE"
            << MacroTokenizer::malformedDot
            << false;

    QTest::newRow(". inside dot command.")
            << ".EQUA.TE"
            << MacroTokenizer::malformedDot
            << false;

    QTest::newRow("Empty dot command.")
            << "."
            << MacroTokenizer::malformedDot
            << false;

    QTest::newRow("Two .'s.")
            << ".."
            << MacroTokenizer::malformedDot
            << false;
}

void TokenizerFailure::case_malformedDot()
{
    execute();
}

void TokenizerFailure::case_malformedIdentifier_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");

    // Tests designed to fail.
    QTest::newRow("Special character in identifier (h!i).")
            << "h!i"
            << MacroTokenizer::malformedIdentifier
            << false;

    QTest::newRow("Special character in identifier (h-_+i).")
            << "h-_+i"
            << MacroTokenizer::malformedIdentifier
            << false;

    // Tests designed to pass.
    QTest::newRow("Identifier looking like hex constant.")
            << "l0x5d"
            << MacroTokenizer::malformedIdentifier
            << false;

    QTest::newRow("Well formed identifier.")
            << "hi"
            << ""
            << true;

    QTest::newRow("Long well formed identifier.")
            << "hello_world_this_is_a_long_identifier"
            << ""
            << true;

    QTest::newRow("Well formed identifier with accented character.")
            << "hÁi"
            << ""
            << true;

    QTest::newRow("Well formed identifier starting with accented character.")
            << "Áhi"
            << ""
            << true;
}

void TokenizerFailure::case_malformedIdentifier()
{
    execute();
}

void TokenizerFailure::case_malformedStringConst_data()
{

}

void TokenizerFailure::case_malformedStringConst()
{

}

void TokenizerFailure::case_syntaxError_data()
{

}

void TokenizerFailure::case_syntaxError()
{

}

void TokenizerFailure::preprocess(ModuleAssemblyGraph &graph)
{
    QFETCH(QString, ProgramText);
    QFETCH(QString, ExpectedError);

    auto [rootPrototype, startRootInstance] = graph.createRoot(ProgramText, ModuleType::USER_PROGRAM);
    //preprocessor->setTarget(&graph);
    //auto result = preprocessor->preprocess();

    //QVERIFY2(!result.error.isNull(), "Expected an error message.");
            //QCOMPARE(result.error->getErrorMessage(), ExpectedError);
}

void TokenizerFailure::execute()
{
    QFETCH(QString, ProgramText);
    QFETCH(QString, ExpectedError);
    QFETCH(bool, ExpectPass);

    auto graph = ModuleAssemblyGraph();
    preprocess(graph);
    int offset = 0;
    MacroTokenizerHelper::ELexicalToken token;
    QString errorMessage;
    QStringRef outString;
    bool pass = tokenizer->getToken(ProgramText, offset, token, outString, errorMessage);
    QCOMPARE(pass, ExpectPass);
    if(!pass) {
        QVERIFY2(!errorMessage.isEmpty(), "Expected an error message.");
        QCOMPARE(errorMessage, ExpectedError);
    }
}

