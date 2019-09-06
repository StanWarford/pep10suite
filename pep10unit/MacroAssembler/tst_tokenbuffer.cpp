#include "tst_tokenbuffer.h"
#include "macromodules.h"
#include "macrotokenizer.h"
#include "macroregistry.h"

TokenBufferTest::TokenBufferTest(): registry(new MacroRegistry())
{

}

TokenBufferTest::~TokenBufferTest()
{

}

void TokenBufferTest::initTestCase()
{
    buffer = QSharedPointer<TokenizerBuffer>::create();
}

void TokenBufferTest::cleanupTestCase()
{

}

void TokenBufferTest::case_malformedDecConst_data()
{
    using MacroTokenizerHelper::ELexicalToken;
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<QList<MacroTokenizerHelper::ELexicalToken>>("MatchList");
    QTest::addColumn<QList<MacroTokenizerHelper::ELexicalToken>>("ExpectedMatches");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");

    // Tests expected to fail.
    QStringList interestingChars;
    interestingChars << ":" << "()"<< "!";
    for(auto combo : interestingChars) {
        QTest::newRow(QString("Special character inside decimal constant %1.")
                      .arg(combo).toStdString().c_str())
                << QString("6%1").arg(combo)+"7"
                << (QList<MacroTokenizerHelper::ELexicalToken>()
                    << ELexicalToken::LT_DEC_CONSTANT << ELexicalToken::LTE_ERROR)
                << (QList<MacroTokenizerHelper::ELexicalToken>() << ELexicalToken::LTE_ERROR)
                << MacroTokenizer::syntaxError
                << false;

        QTest::newRow(QString("Special character after decimal constant %1.")
                      .arg(combo).toStdString().c_str())
                << QString("67%1").arg(combo)
                << (QList<MacroTokenizerHelper::ELexicalToken>()
                    << ELexicalToken::LT_DEC_CONSTANT << ELexicalToken::LTE_ERROR)
                << (QList<MacroTokenizerHelper::ELexicalToken>() << ELexicalToken::LTE_ERROR)
                << MacroTokenizer::syntaxError
                << false;
    }


    // Tests expected to pass.
    QTest::newRow("Identifier joined with decimal constant.")
            << "6id"
            << (QList<MacroTokenizerHelper::ELexicalToken>()
                << ELexicalToken::LT_DEC_CONSTANT << ELexicalToken::LT_IDENTIFIER)
            << (QList<MacroTokenizerHelper::ELexicalToken>()
                << ELexicalToken::LT_DEC_CONSTANT << ELexicalToken::LT_IDENTIFIER)
            << ""
            << true;
}

void TokenBufferTest::case_malformedDecConst()
{
    execute();
}

void TokenBufferTest::case_malformedHexConst_data()
{
    using MacroTokenizerHelper::ELexicalToken;
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<QList<MacroTokenizerHelper::ELexicalToken>>("MatchList");
    QTest::addColumn<QList<MacroTokenizerHelper::ELexicalToken>>("ExpectedMatches");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");

    // Tests expected to fail.
    QStringList interestingChars;
    interestingChars << ":" << "()"<< "!";
    for(auto combo : interestingChars) {
        QTest::newRow(QString("Special character inside hex constant %1.")
                      .arg(combo).toStdString().c_str())
                << QString("0x6%1").arg(combo)+"7"
                << (QList<MacroTokenizerHelper::ELexicalToken>()
                    << ELexicalToken::LT_HEX_CONSTANT << ELexicalToken::LTE_ERROR)
                << (QList<MacroTokenizerHelper::ELexicalToken>() << ELexicalToken::LTE_ERROR)
                << MacroTokenizer::syntaxError
                << false;

        QTest::newRow(QString("Special character after hex constant %1.")
                      .arg(combo).toStdString().c_str())
                << QString("0x67%1").arg(combo)
                << (QList<MacroTokenizerHelper::ELexicalToken>()
                    << ELexicalToken::LT_HEX_CONSTANT << ELexicalToken::LTE_ERROR)
                << (QList<MacroTokenizerHelper::ELexicalToken>() << ELexicalToken::LTE_ERROR)
                << MacroTokenizer::syntaxError
                << false;
    }

    QTest::newRow("Invalid hex constant 0x9p1")
            << QString("0x9p1")
            << (QList<MacroTokenizerHelper::ELexicalToken>()
                << ELexicalToken::LT_HEX_CONSTANT << ELexicalToken::LT_IDENTIFIER)
            << (QList<MacroTokenizerHelper::ELexicalToken>()
                << ELexicalToken::LT_HEX_CONSTANT << ELexicalToken::LT_IDENTIFIER)
            << ""
            << true;

    QTest::newRow("Invalid hex constant 0x0x9a")
            << QString("0x0x9a")
            << (QList<MacroTokenizerHelper::ELexicalToken>()
                << ELexicalToken::LT_HEX_CONSTANT << ELexicalToken::LT_IDENTIFIER)
            << (QList<MacroTokenizerHelper::ELexicalToken>()
                << ELexicalToken::LT_HEX_CONSTANT << ELexicalToken::LT_IDENTIFIER)
            << ""
            << true;

    QTest::newRow("Invalid hex constant 0x9p")
            << QString("0x9p")
            << (QList<MacroTokenizerHelper::ELexicalToken>()
                << ELexicalToken::LT_HEX_CONSTANT << ELexicalToken::LT_IDENTIFIER)
            << (QList<MacroTokenizerHelper::ELexicalToken>()
                << ELexicalToken::LT_HEX_CONSTANT << ELexicalToken::LT_IDENTIFIER)
            << ""
            << true;

    // Tests expected to pass.
    QTest::newRow("Identifier joined with decimal constant.")
            << "0x6id"
            << (QList<MacroTokenizerHelper::ELexicalToken>()
                << ELexicalToken::LT_HEX_CONSTANT << ELexicalToken::LT_IDENTIFIER)
            << (QList<MacroTokenizerHelper::ELexicalToken>()
                << ELexicalToken::LT_HEX_CONSTANT << ELexicalToken::LT_IDENTIFIER)
            << ""
            << true;

    QTest::newRow("Invalid hex constant 0x9p")
            << QString("0x9p")
            << (QList<MacroTokenizerHelper::ELexicalToken>()
                << ELexicalToken::LT_HEX_CONSTANT << ELexicalToken::LT_IDENTIFIER)
            << (QList<MacroTokenizerHelper::ELexicalToken>()
                << ELexicalToken::LT_HEX_CONSTANT << ELexicalToken::LT_IDENTIFIER)
            << ""
            << true;
}

void TokenBufferTest::case_malformedHexConst()
{
    execute();
}

void TokenBufferTest::case_malformedDot_data()
{
    using MacroTokenizerHelper::ELexicalToken;
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<QList<MacroTokenizerHelper::ELexicalToken>>("MatchList");
    QTest::addColumn<QList<MacroTokenizerHelper::ELexicalToken>>("ExpectedMatches");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");

    // Tests expected to fail.
    QStringList interestingChars;
    interestingChars << ":" << "()"<< "!";
    for(auto combo : interestingChars) {
        QTest::newRow(QString("Special character inside hex constant %1.")
                      .arg(combo).toStdString().c_str())
                << QString(".EQU%1").arg(combo)+"TE"
                << (QList<MacroTokenizerHelper::ELexicalToken>()
                    << ELexicalToken::LT_DOT_COMMAND << ELexicalToken::LTE_ERROR)
                << (QList<MacroTokenizerHelper::ELexicalToken>() << ELexicalToken::LTE_ERROR)
                << MacroTokenizer::syntaxError
                << false;

        QTest::newRow(QString("Special character after hex constant %1.")
                      .arg(combo).toStdString().c_str())
                << QString("0xEQUATE%1").arg(combo)
                << (QList<MacroTokenizerHelper::ELexicalToken>()
                    << ELexicalToken::LT_DOT_COMMAND << ELexicalToken::LTE_ERROR)
                << (QList<MacroTokenizerHelper::ELexicalToken>() << ELexicalToken::LTE_ERROR)
                << MacroTokenizer::syntaxError
                << false;
    }

    QTest::newRow("Multiple dot commands fused together.")
            << QString(".EQUA.TE")
            << (QList<MacroTokenizerHelper::ELexicalToken>()
                << ELexicalToken::LT_DOT_COMMAND << ELexicalToken::LT_DOT_COMMAND)
            << (QList<MacroTokenizerHelper::ELexicalToken>()
                << ELexicalToken::LT_DOT_COMMAND << ELexicalToken::LT_DOT_COMMAND)
            << ""
            << true;

}

void TokenBufferTest::case_malformedDot()
{
    execute();
}

void TokenBufferTest::case_malformedIdentifier_data()
{
    using MacroTokenizerHelper::ELexicalToken;
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<QList<MacroTokenizerHelper::ELexicalToken>>("MatchList");
    QTest::addColumn<QList<MacroTokenizerHelper::ELexicalToken>>("ExpectedMatches");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");

    // Tests expected to fail.
    QStringList interestingChars;
    interestingChars << "::" << "()"<< "!";
    for(auto combo : interestingChars) {
        QTest::newRow(QString("Special character inside symbol definition %1.")
                      .arg(combo).toStdString().c_str())
                << QString("hell%1").arg(combo)+"lo:"
                << (QList<MacroTokenizerHelper::ELexicalToken>()
                    << ELexicalToken::LT_SYMBOL_DEF << ELexicalToken::LTE_ERROR)
                << (QList<MacroTokenizerHelper::ELexicalToken>() << ELexicalToken::LTE_ERROR)
                << MacroTokenizer::syntaxError
                << false;

        QTest::newRow(QString("Special character after symbol symbol definition %1.")
                      .arg(combo).toStdString().c_str())
                << QString("hello:%1").arg(combo)
                << (QList<MacroTokenizerHelper::ELexicalToken>()
                    << ELexicalToken::LT_SYMBOL_DEF << ELexicalToken::LTE_ERROR)
                << (QList<MacroTokenizerHelper::ELexicalToken>() << ELexicalToken::LTE_ERROR)
                << MacroTokenizer::syntaxError
                << false;
    }
    QTest::newRow("Special characters in identifier h-_-i.")
            << QString("h-_-i")
            << (QList<MacroTokenizerHelper::ELexicalToken>()
                << ELexicalToken::LT_IDENTIFIER << ELexicalToken::LTE_ERROR)
            << (QList<MacroTokenizerHelper::ELexicalToken>()<< ELexicalToken::LTE_ERROR)
            << "" //Doesn't really matter what the error message is.
            << false;

    QTest::newRow("Identifier followed by an addressing mode.")
            << QString("HELLOWORL,d")
            << (QList<MacroTokenizerHelper::ELexicalToken>()
                << ELexicalToken::LT_IDENTIFIER << ELexicalToken::LT_ADDRESSING_MODE)
            << (QList<MacroTokenizerHelper::ELexicalToken>()
                << ELexicalToken::LT_IDENTIFIER << ELexicalToken::LT_ADDRESSING_MODE)
            << ""
            << true;

    QTest::newRow("Identifier followed by two addressing modes.")
            << QString("HELLOWORL,d,d")
            << (QList<MacroTokenizerHelper::ELexicalToken>()
                << ELexicalToken::LT_IDENTIFIER << ELexicalToken::LT_ADDRESSING_MODE
                << ELexicalToken::LT_ADDRESSING_MODE)
            << (QList<MacroTokenizerHelper::ELexicalToken>()
                << ELexicalToken::LT_IDENTIFIER << ELexicalToken::LT_ADDRESSING_MODE
                << ELexicalToken::LT_ADDRESSING_MODE)
            << ""
            << true;

}

void TokenBufferTest::case_malformedIdentifier()
{
    execute();
}

void TokenBufferTest::execute()
{

    QFETCH(QString, ProgramText);
    QFETCH(QList<MacroTokenizerHelper::ELexicalToken>, MatchList);
    QFETCH(QList<MacroTokenizerHelper::ELexicalToken>, ExpectedMatches);
    QFETCH(QString, ExpectedError);
    QFETCH(bool, ExpectPass);

    auto graph = ModuleAssemblyGraph();
    auto [rootPrototype, startRootInstance] = graph.createRoot(ProgramText, ModuleType::USER_PROGRAM);
    buffer->setTokenizerInput(ProgramText.split("\n"));
    bool pass = true;
    for(auto match : MatchList) {
        pass &= buffer->match(match);
    }
    QCOMPARE(pass, ExpectPass);
    QCOMPARE(buffer->getMatches().size(), ExpectedMatches.size());
    auto cat=buffer->getMatches().first();
    for(int it = 0; it< buffer->getMatches().size(); it++) {
        /*qDebug() << "Comparing items: "
                 << buffer->getMatches().at(it).first
                 << ExpectedMatches.at(it);*/
        QVERIFY2(buffer->getMatches().at(it).first == ExpectedMatches.at(it),
                 "Mistmatched tokens in buffer");
    }
    // If we expected an error, and were given an error message, compare the errors.
    if(ExpectedMatches.size()>=1 &&
            ExpectedMatches.first() == MacroTokenizerHelper::ELexicalToken::LTE_ERROR &&
            !ExpectedError.isEmpty()) {
        QCOMPARE(buffer->getMatches().first().second.toString(),
                ExpectedError);
    }
}
