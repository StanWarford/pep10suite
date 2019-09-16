#include "tst_tokenizer.h"
#include "macroregistry.h"
#include "macrotokenizer.h"
#include "macromodules.h"

TokenizerTest::TokenizerTest(): registry(new MacroRegistry())
{

}

TokenizerTest::~TokenizerTest()
{

}

void TokenizerTest::initTestCase()
{
    tokenizer = QSharedPointer<MacroTokenizer>::create();
}

void TokenizerTest::cleanupTestCase()
{

}

void TokenizerTest::case_malformedMacroInvoke_data()
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

void TokenizerTest::case_malformedMacroInvoke()
{
    execute();
}

void TokenizerTest::case_malformedMacroSubstitution_data()
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

void TokenizerTest::case_malformedMacroSubstitution()
{
    execute();
}

void TokenizerTest::case_malformedAddrMode_data()
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

void TokenizerTest::case_malformedAddrMode()
{
    execute();
}

void TokenizerTest::case_malformedCharConst_data()
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

void TokenizerTest::case_malformedCharConst()
{
    execute();
}

void TokenizerTest::case_malformedDecConst_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");

    // Detecting identifiers or special joined with decimal constants
    // requires evaluating a chain of tokens, which is beyond the ability
    // of our tokenizer. The TokenizerBuffer exists for this reason,
    // and as such these unit tests have been moved to tst_tokenbuffer.
    /*QTest::newRow("Identifier in dec const.")
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
            << false;*/

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

    QTest::newRow("1 leading zero on integer literal.")
            << "067"
            << ""
            << true;

    QTest::newRow("5 leading zeros on integer literal.")
            << "0000067"
            << ""
            << true;

    QTest::newRow("1 leading zero on explicitly positive integer literal.")
            << "+067"
            << ""
            << true;

    QTest::newRow("5 leading zeros on explicitly positive integer literal.")
            << "+0000067"
            << ""
            << true;

    QTest::newRow("1 leading zero on negative integer literal.")
            << "-067"
            << ""
            << true;

    QTest::newRow("5 leading zeros on negative integer literal.")
            << "-0000067"
            << ""
            << true;
}

void TokenizerTest::case_malformedDecConst()
{
    execute();
}

void TokenizerTest::case_malformedHexConst_data()
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

    // Detecting identifiers or special joined with hex constants
    // requires evaluating a chain of tokens, which is beyond the ability
    // of our tokenizer. The TokenizerBuffer exists for this reason,
    // and as such these unit tests have been moved to tst_tokenbuffer.
    /*
    QTest::newRow("Invalid hex value 0x9p.")
            << "0x9p"
            << MacroTokenizer::malformedHexConst
            << false;

    QTest::newRow("Double 0x.")
            << "0x0x9595"
            << MacroTokenizer::malformedHexConst
            << false;*/

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

    QTest::newRow("1 leading zero of 4 character hex constant.")
            << "0x0a23f"
            << ""
            << true;

    // Zero padding makes this look like a DWORD to the tokenizer
    QTest::newRow("4 leading zers of 4 character hex constant.")
            << "0x0000a23f"
            << ""
            << true;
}

void TokenizerTest::case_malformedHexConst()
{
    execute();
}

void TokenizerTest::case_malformedDot_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");


    // After discussion with Dr. Warford, the tokenizer SHOULD NOT pick up on this error.
    // A bad dot command (including those with numbers) is not a syntax error,
    // it is a semantics error that should be detected by the assembler.
    QTest::newRow("Number inside dot command.")
            << ".d04t"
            << ""
            << true;

    // Detecting identifiers or special joined with dot commands
    // requires evaluating a chain of tokens, which is beyond the ability
    // of our tokenizer. The TokenizerBuffer exists for this reason,
    // and as such these unit tests have been moved to tst_tokenbuffer.
    /*QTest::newRow("Special character inside dot command.")
            << ".d()t"
            << MacroTokenizer::malformedDot
            << false;

    QTest::newRow(". inside dot command.")
            << ".EQUA.TE"
            << MacroTokenizer::malformedDot
            << false;*/

    QTest::newRow("Number starting dot command.")
            << ".5dot"
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

    QTest::newRow("Empty dot command.")
            << "."
            << MacroTokenizer::malformedDot
            << false;

    QTest::newRow("Two .'s.")
            << ".."
            << MacroTokenizer::malformedDot
            << false;
}

void TokenizerTest::case_malformedDot()
{
    execute();
}

void TokenizerTest::case_malformedIdentifier_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");

    // Tests designed to fail.
    // Detecting identifiers or special joined with hex constants
    // requires evaluating a chain of tokens, which is beyond the ability
    // of our tokenizer. The TokenizerBuffer exists for this reason,
    // and as such these unit tests have been moved to tst_tokenbuffer.
    /*
    QTest::newRow("Special character in identifier (h!i).")
            << "h!i"
            << MacroTokenizer::malformedIdentifier
            << false;

    QTest::newRow("Special character in identifier (h-_+i).")
            << "h-_+i"
            << MacroTokenizer::malformedIdentifier
            << false;
    QTest::newRow("Identifier looking like hex constant.")
            << "l0x5d"
            << MacroTokenizer::malformedIdentifier
            << false;*/

    // Tests designed to pass.

    QTest::newRow("Well formed identifier.")
            << "hi"
            << ""
            << true;

    QTest::newRow("Well formed, long, identifier.")
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

void TokenizerTest::case_malformedIdentifier()
{
    execute();
}

void TokenizerTest::case_malformedStringConst_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");

    // Tests designed to fail.
    QTest::newRow("Invalid escape sequence \\k.")
            << "\"\\k\""
            << MacroTokenizer::malformedStringConst
            << false;

    QTest::newRow("Invalid escape sequence \\N.")
            << "\"\\N\""
            << MacroTokenizer::malformedStringConst
            << false;

    QTest::newRow("Invalid escape sequence \\x2 9.")
            << "\"\\x2 9\""
            << MacroTokenizer::malformedStringConst
            << false;

    QTest::newRow("Invalid escape sequence \\x 29.")
            << "\"\\x 29\""
            << MacroTokenizer::malformedStringConst
            << false;

    QTest::newRow("Invalid escape sequence \\x0.")
            << "\"\\x0\""
            << MacroTokenizer::malformedStringConst
            << false;

    QTest::newRow("Unclosed string literal.")
            << "\""
            << MacroTokenizer::malformedStringConst
            << false;

    QTest::newRow("Unclosed string and a \\n.")
            << "\"\n"
            << MacroTokenizer::malformedStringConst
            << false;

    // The higher levels of asbtraction of the token stream
    // and assembler strip all newline characters from the tokenizer
    // input. Therefore, the tokenizer does not expect to see newline
    // characters, and will properly match a string that spans multiple lines.
    /*QTest::newRow("Unexpected \\n.")
            << " \"\n\""
            << MacroTokenizer::malformedStringConst
            << false;*/


    // Tests designed to pass.
    QTest::newRow("Escape quote \".")
            << "\"\\\"\""
            << ""
            << true;

    // Demonstrate that hex escape sequence will parse
    QTest::newRow("Escape sequence \\xff.")
            << "\"\\xff\""
            << ""
            << true;
    // Demonstrate that two escape sequences in a row will parse.
    QTest::newRow("Escape sequence \\xfe\\xed.")
            << "\"\\xfe\\xed\""
            << ""
            << true;
}

void TokenizerTest::case_malformedStringConst()
{
    execute();
}

void TokenizerTest::case_syntaxError_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");

    QTest::newRow("Single special character.")
            << "!"
            << MacroTokenizer::syntaxError
            << false;

    QTest::newRow("Multiple special characters.")
            << "()"
            << MacroTokenizer::syntaxError
            << false;

    QTest::newRow("Unexpected :.")
            << ": hello world"
            << MacroTokenizer::syntaxError
            << false;
}

void TokenizerTest::case_syntaxError()
{
    execute();
}

void TokenizerTest::preprocess(ModuleAssemblyGraph &graph)
{
    QFETCH(QString, ProgramText);
    QFETCH(QString, ExpectedError);

    auto [rootPrototype, startRootInstance] = graph.createRoot(ProgramText, ModuleType::USER_PROGRAM);
    //preprocessor->setTarget(&graph);
    //auto result = preprocessor->preprocess();

    //QVERIFY2(!result.error.isNull(), "Expected an error message.");
            //QCOMPARE(result.error->getErrorMessage(), ExpectedError);
}

void TokenizerTest::execute()
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

