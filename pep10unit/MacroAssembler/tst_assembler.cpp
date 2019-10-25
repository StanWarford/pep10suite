#include "tst_assembler.h"
#include "macroregistry.h"
#include "macropreprocessor.h"
#include "macromodules.h"
#include "macroassembler.h"

AssemblerTest::AssemblerTest():registry(new MacroRegistry())
{

}

AssemblerTest::~AssemblerTest() = default;

void AssemblerTest::initTestCase()
{
    preprocessor = QSharedPointer<MacroPreprocessor>::create(registry.get());
    assembler = QSharedPointer<MacroAssembler>::create(registry.get());
}

void AssemblerTest::cleanupTestCase()
{

}

void AssemblerTest::case_missingEnd_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");


    // Validate that all program combinations must terminate in a .END

    QTest::addRow("Empty user program missing .END sentinel.")
            << "\n"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::missingEND
            << false;

    QTest::addRow("Non-empty user program missing .END sentinel.")
            << "asra\n"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::missingEND
            << false;

    QTest::addRow("Empty operating system missing .END sentinel.")
            << "\n"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::missingEND
            << false;

    QTest::addRow("Non-empty operating system missing .END sentinel.")
            << "asra\n"
            << ModuleType::OPERATING_SYSTEM
            << MacroAssembler::missingEND
            << false;

    registry->clearCustomMacros();
    registry->registerCustomMacro("test", "@test 0\n\n");
    QTest::addRow("User program including a macro missing .END sentinel.")
            << "@test\n.END"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::missingEND
            << false;

    QTest::addRow("Operating system including a macro missing .END sentinel.")
            << "@test\n.END"
            << ModuleType::OPERATING_SYSTEM
            << MacroAssembler::missingEND
            << false;

}

void AssemblerTest::case_missingEnd()
{
    execute();
}

void AssemblerTest::case_syntaxError_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");

    // Validate that various kinds of syntax errors are passed
    // up from the tokenizer to the error output.
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

    QTest::addRow("Unexpected unsigned decimal const at start of line.")
            << "6975\n"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::unexpectedToken.arg("6975")
            << false;

    QTest::addRow("Unexpected unsigned decimal const at start of line.")
            << "-6975\n"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::unexpectedToken.arg("-6975")
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
            << "sym:.equate 1 1"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::unxpectedEOL
            << false;

    QTest::addRow("Unexpected EOL in sym:EQUATE instruction.")
            << "sym:.equate 1 k"
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
            << "sym:.equate 1 1 ;Com\n"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::unxpectedEOL
            << false;

    QTest::addRow("Unexpected EOL in sym:EQUATE instruction ;comment.")
            << "sym:.equate 1 k ;Com\n"
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
    // For tokens that describe multiple unique code lines (identifiers, dot commands)
    // must test multiple permutations of these symbols.
    // SCALL and USCALL provide a special error message when defining a symbol
    // so they will be tested case_noSymbolUscall, and case_noSymbolScall
    // respectively.
    identList  << element("addressing mode",",i", false, MacroAssembler::unexpectedSymbolDecl)
               << element("character const", "'l'", false, MacroAssembler::unexpectedSymbolDecl)
               << element("decimal const","72", false, MacroAssembler::unexpectedSymbolDecl)
               << element(".BLOCK",".BLOCK", false, MacroAssembler::badBlockArgument)
               << element(".BLOCK 5",".BLOCK 5\n.END\n", true, "")
               << element("sy:.BLOCK","sy:.BLOCK", false, MacroAssembler::unexpectedSymbolDecl)
               << element("sy:.BLOCK 5","sy:.BLOCK 5", false, MacroAssembler::unexpectedSymbolDecl)
               << element("sy:.EQUATE","sy:.EQUATE 5", false, MacroAssembler::unexpectedSymbolDecl)
               << element("hex constant","0x99", false, MacroAssembler::unexpectedSymbolDecl)
               << element("unary instruction","ASRA\n.END\n", true, "")
               << element("nonunary instruction","LDWA 0,d\n.END\n", true, "")
               << element("identifier","no", false, MacroAssembler::invalidMnemonic.arg("no"))
               << element("string constant","\"hi\"", false, MacroAssembler::unexpectedSymbolDecl)
               << element("symbol declaration","sy:", false, MacroAssembler::unexpectedSymbolDecl);

    // Insert a symbol before each of the above lines, and check the resulting expression.
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

void AssemblerTest::case_invalidMnemonic_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");


    // Test that obviously incorrect mnemonics will not assemble.
    QTest::addRow("Invalid mnemonic hello.")
            << "hello\n.END\n"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::invalidMnemonic.arg("hello")
            << false;

    QTest::addRow("Invalid mnemonic x95.")
            << "x95\n.END\n"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::invalidMnemonic.arg("x95")
            << false;

    QTest::addRow("Invalid mnemonic x_95.")
            << "x_95\n.END\n"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::invalidMnemonic.arg("x_95")
            << false;

    QTest::addRow("Invalid mnemonic _hi.")
            << "_hi\n.END\n"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::invalidMnemonic.arg("_hi")
            << false;

    // Check that a sample unary instruction will assemble correctly.
    QTest::addRow("Parse unary instruction ASRA.")
            << "asra\n.END\n"
            << ModuleType::USER_PROGRAM
            << ""
            << true;

    // Check that a sample nonunary instruction will assemble correctly.
    QTest::addRow("Parse nonunary instruction LDWA 10,d.")
            << "LDWA 10,d\n.END\n"
            << ModuleType::USER_PROGRAM
            << ""
            << true;
}

void AssemblerTest::case_invalidMnemonic()
{
    execute();
}

void AssemblerTest::case_onlyInOS_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");


    // Some dot command may only appear in the operating system -
    // namely BURN, SCALL, USCALL, and EXPORT.
    // Check that the assembler enforces these requirements

    // Check that the operating system allows the symbol.
    // Must use unique symbol per statement, to prevent collision
    // in the macro registry
    QList<QPair<QString,QString>> osList;
    osList << QPair<QString,QString>{".SCALL", "os1"}
    << QPair<QString,QString>{".USCALL", "os2"}
    << QPair<QString,QString>{".BURN", "0xff"}
    << QPair<QString,QString>{".EXPORT", "os4"};

    // Create a assembly program for each input.
    for(const auto& ident : osList) {
        QTest::addRow("%s", QString("OS %1")
                      .arg(ident.first).toStdString().c_str())
                << QString("%1 %2 \n.END").arg(ident.first, ident.second)
                << ModuleType::OPERATING_SYSTEM
                << ""
                << true;

    }

    // Check that there symbols are forbidden in a user program.
    QList<QPair<QString,QString>> userList;
    userList << QPair<QString,QString>{".SCALL", "up1"}
    << QPair<QString,QString>{".USCALL", "up2"}
    << QPair<QString,QString>{".BURN", "0xff"}
    << QPair<QString,QString>{".EXPORT", "up4"};
    for(const auto& ident : userList) {
        QTest::addRow("%s", QString("User program %1")
                      .arg(ident.first).toStdString().c_str())
                << QString("%1 %2 \n.END").arg(ident.first, ident.second)
                << ModuleType::USER_PROGRAM
                << MacroAssembler::onlyInOperatingSystem.arg(ident.first)
                << false;
    }

    // Check that the symbols are banned inside a macro included from a user program,
    // but they are allowed in macros included by the operating system
    QList<QPair<QString,QString>> macroUse;
    macroUse << QPair<QString,QString>{".SCALL", "em1"}
    << QPair<QString,QString>{".USCALL", "em2"}
    << QPair<QString,QString>{".BURN", "0xff"}
    << QPair<QString,QString>{".EXPORT", "em4"};

    // Must make each macro unique, to prevent collisions in the macro registry.
    int count = 0;
    for(const auto& ident : macroUse) {

        registry->registerCustomMacro(QString("test%1").arg(count),
                                      QString("@test%1 0\n%2 %3\n.END")
                                      .arg(count)
                                      .arg(ident.first, ident.second));

        QTest::addRow("%s", QString("Embeded macro in user program %1")
                      .arg(ident.first).toStdString().c_str())
                << QString("@test%1 \n.END").arg(count)
                << ModuleType::USER_PROGRAM
                << MacroAssembler::onlyInOperatingSystem.arg(ident.first)
                << false;

        QTest::addRow("%s", QString("Embeded macro in operating system %1")
                      .arg(ident.first).toStdString().c_str())
                << QString("@test%1 \n.END").arg(count)
                << ModuleType::OPERATING_SYSTEM
                << ""
                << true;
        count++;
    }

}

void AssemblerTest::case_onlyInOS()
{
    execute();
}

void AssemblerTest::case_invalidDotCommand_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");


    // Check that clearly invalid dot commands are rejected.
    QTest::addRow("Invalid dot command .WHAT.")
            << ".WHAT\n"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::invalidDotCommand.arg("WHAT")
            << false;

    QTest::addRow("Invalid dot command .W_HAT.")
            << ".W_HAT\n"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::invalidDotCommand.arg("W_HAT")
            << false;

    QTest::addRow("Invalid dot command W_9HAT.")
            << ".W_9HAT\n"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::invalidDotCommand.arg("W_9HAT")
            << false;

    // There are helpers that test the various conditions and properties
    // of each valid dot command, therefore the code should not be duplicated here.

}

void AssemblerTest::case_invalidDotCommand()
{
    execute();
}

void AssemblerTest::case_symbolTooLong_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");


    // Tests that show the cutoff point between short and too long.
    QTest::newRow("Symbol of length 7.")
            << "abcdefg: asra\n.END\n"
            << ModuleType::USER_PROGRAM
            << ""
            << true;

    QTest::newRow("Symbol of length 8.")
            << "abcdefgh: asra\n.END\n"
            << ModuleType::USER_PROGRAM
            << ""
            << true;

    QTest::newRow("Symbol of length 9.")
            << "abcdefghi: asra\n.END\n"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::longSymbol.arg("abcdefghi")
            << false;

    // Tests expected to fail.
    QString longStr = "long_identifier";
    QTest::newRow("Otherwise valid symbol that is too long.")
            << QString("%1: asra\n.END\n").arg(longStr)
            << ModuleType::USER_PROGRAM
            << MacroAssembler::longSymbol.arg(longStr)
            << false;

    // Tests expected to pass.
    QTest::newRow("Well formed symbol of length 3.")
            << "hi: asra\n.END\n"
            << ModuleType::USER_PROGRAM
            << ""
            << true;

    QTest::newRow("Well formed symbol with accented character.")
            << "hÁi: asra\n.END\n"
            << ModuleType::USER_PROGRAM
            << ""
            << true;
}

void AssemblerTest::case_symbolTooLong()
{
    execute();
}

void AssemblerTest::case_badAddrMode_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");


    QSet<QString> mnemonics;
    QMetaObject meta = Enu::staticMetaObject;
    QMetaEnum metaEnum = meta.enumerator(meta.indexOfEnumerator("EMnemonic"));
    QMetaEnum addrEnum = meta.enumerator(meta.indexOfEnumerator("EAddrMode"));
    QString tempqs;

    // Iterate over all mnemonics
    // This catches bugs in the implementation of the assembler
    // (e.g. ADDA being hardcoded to be unary, i/x addressing modes being
    // banned from XORX instructions), it does NOT catch bugs in the specification
    // created in pep.h and enu.h.
    // That being said, the results of this test suite may be viewed to ensure
    // that the specification is sane - I found several faults in the Pep10
    // specification on account of these tests.
    for(int it = 0; it < metaEnum.keyCount(); it++)
    {
        auto mnemonic = static_cast<Enu::EMnemonic>(metaEnum.value(it));
        // Test cases for nonunary instructions requiring an addressing mode.
        if(Pep::addrModeRequiredMap[mnemonic]) {
            // Test that nonunary instruction fails to assemble when missing addressing mode.
            QTest::newRow(QString("Require addressing mode for %1.")
                          .arg(QString(metaEnum.key(it)).toUpper()).toStdString().c_str())
                    << QString(metaEnum.key(it)).toUpper() + " k \n.END\n"
                    << ModuleType::USER_PROGRAM
                    << MacroAssembler::reqAddrMode
                    << false;

            // Validate that instructions forbidding certain addressing modes
            // fail when assembeld with that addressing mode (e.g STWA k,i).
            if(auto allowed = Pep::addrModesMap[mnemonic];
                    allowed != static_cast<int>(Enu::EAddrMode::ALL)) {
                for(int addrIt = 0; addrIt < addrEnum.keyCount(); addrIt++) {
                    // Skip cases where the addressing mode is allowed.
                    if(allowed & addrEnum.value(addrIt)) {
                        continue;
                    }
                    // The "none" addressing mode is an element of the address
                    // enumeration, indicating the lack of an addressing mode. Skip.
                    else if(static_cast<Enu::EAddrMode>(addrEnum.value(addrIt))
                            == Enu::EAddrMode::NONE) {
                        continue;
                    }
                    QTest::newRow(QString("Test illegal addressing mode for %1 k,%2.")
                                  .arg(QString(metaEnum.key(it)).toUpper())
                                  .arg(QString(addrEnum.key(addrIt)).toLower()).toStdString().c_str())
                            << QString(metaEnum.key(it)).toUpper() + " k," +
                               QString(addrEnum.key(addrIt)).toLower() + "\n.END\n"
                            << ModuleType::USER_PROGRAM
                            << MacroAssembler::illegalAddrMode
                            << false;
                }

            }
        }

        // Generate test cases where unary instructions are treated
        // as nonunary instructions.
        else if(Pep::isUnaryMap[mnemonic]){
            QTest::newRow(QString("Test forbidden addressing mode for unary instruction %1.")
                          .arg(QString(metaEnum.key(it)).toUpper()).toStdString().c_str())
                    << QString(metaEnum.key(it)).toUpper() + " k,n \n.END\n"
                    << ModuleType::USER_PROGRAM
                    << MacroAssembler::unxpectedEOL
                    << false;
        }
        // Generate test cases for BR / CALL instructions where illegal addressing
        // are provided. Separate from normal non-unary instructions since they
        // do not require an addressing mode to be present.
        else{
            for(int addrIt = 0; addrIt < addrEnum.keyCount(); addrIt++) {
                // Skip cases where the addressing mode is allowed.
                if(Pep::addrModesMap[mnemonic] & addrEnum.value(addrIt)) {
                    continue;
                }
                // The "none" addressing mode is an element of the address
                // enumeration, indicating the lack of an addressing mode. Skip.
                else if(static_cast<Enu::EAddrMode>(addrEnum.value(addrIt))
                        == Enu::EAddrMode::NONE) {
                    continue;
                }
                QTest::newRow(QString("Test illegal addressing mode for %1 k,%2.")
                              .arg(QString(metaEnum.key(it)).toUpper())
                              .arg(QString(addrEnum.key(addrIt)).toLower()).toStdString().c_str())
                        << QString(metaEnum.key(it)).toUpper() + " k," +
                           QString(addrEnum.key(addrIt)).toLower() + "\n.END\n"
                        << ModuleType::USER_PROGRAM
                        << MacroAssembler::illegalAddrMode
                        << false;
            }
        }
    }
}

void AssemblerTest::case_badAddrMode()
{
    execute();
}

void AssemblerTest::case_badMacroSub_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");


    // Validate that macro substitution succededs with semantically
    // incorrect tokens, propogates errors of sub-problems to the main
    // module.

    // Macros pass all tokens (except other macros, newlines, and comments)
    // as argument to the next level of depth.
    QTest::newRow("Pass a symbol to XCHGA.")
            << "@XCHGA hello: ,i\n.END"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::opsecAfterMnemonic
            << false;

    QTest::newRow("Pass valid string and invalid addressing mode to XCHGA.")
            << "@XCHGA \"SO\" ,i\n.END"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::illegalAddrMode
            << false;

    QTest::newRow("Pass a string that is too long to XCHGA.")
            << "@XCHGA \"SOO\",d\n.END"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::wordStringOutOfRange
            << false;

    QTest::newRow("Pass gibberish to XCHGA.")
            << "@XCHGA %Hi ,i\n.END"
            << ModuleType::USER_PROGRAM
            << MacroTokenizer::syntaxError
            << false;

}

void AssemblerTest::case_badMacroSub()
{
    execute();
}

void AssemblerTest::case_expectOperand_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");


    // Test that various permutations of BR will fail when an
    // operand specifier is ommitted. Early development versions
    // of Pep10 had several bugs allowing for mistakes of this
    // form to assemble, so unit testing attempts to be thorough.
    QTest::newRow("No argument to BR: br.")
            << "BR\n.END"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::opsecAfterMnemonic
            << false;

    QTest::newRow("No argument to BR: br;Useless comment.")
            << "BR;Useless comment\n.END"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::opsecAfterMnemonic
            << false;

    QTest::newRow("No argument to BR: br;0x10.")
            << "BR;0x10\n.END"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::opsecAfterMnemonic
            << false;

    QTest::newRow("No argument to BR: here:br;here.")
            << "here:BR;here\n.END"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::opsecAfterMnemonic
            << false;

    QTest::newRow("No argument to BR: br,i.")
            << "BR ,i\n.END"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::opsecAfterMnemonic
            << false;

    QTest::newRow("No argument to BR: br ,i;Useless comment.")
            << "BR ,i;Useless comment\n.END"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::opsecAfterMnemonic
            << false;

    QTest::newRow("No argument to BR: br ,i;0x10.")
            << "BR ,i;0x10\n.END"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::opsecAfterMnemonic
            << false;

    QTest::newRow("No argument to BR: here:br ,i;here.")
            << "here:BR ,i;here\n.END"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::opsecAfterMnemonic
            << false;

    // Test non-br-like instruction, with an addressing mode but no operand.
    // Previous tests ensure errors are raised with missing,
    // incorrect addressing modes
    QTest::newRow("No argument to ADDA: adda,i.")
        << "ADDA ,i\n.END"
        << ModuleType::USER_PROGRAM
        << MacroAssembler::opsecAfterMnemonic
        << false;

    QTest::newRow("No argument to ADDA: adda ,i;Useless comment.")
            << "ADDA ,i;Useless comment\n.END"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::opsecAfterMnemonic
            << false;

    QTest::newRow("No argument to ADDA: adda ,i;0x10.")
            << "ADDA ,i;0x10\n.END"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::opsecAfterMnemonic
            << false;

    QTest::newRow("No argument to ADDA: here:adda ,i;here.")
            << "here:ADDA ,i;here\n.END"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::opsecAfterMnemonic
            << false;
}

void AssemblerTest::case_expectOperand()
{
    execute();
}

void AssemblerTest::case_byteOutOfRange_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");


    // Tests the maximum acceptable values of
    // decimal, hex, string, and character operands
    // for byte sized instruction.
    QStringList inRange;
    inRange << "-1" << "-128" << "254" << "255"
            << R"('\x00')" << R"('\xff')"
            << "\"\\x00\"" << "\"\\xff\""
            << "0x00" << "0xff" << "0xFFFFFFFFFFFF";

    for(const auto& ident : inRange) {
        QTest::addRow("%s", QString("Byte constant in range: .BYTE %1")
                      .arg(ident).toStdString().c_str())
                << QString(".BYTE %1 \n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << ""
                << true;

    }

    // Check that byte pseudo-ops fail when given a 2,3 byte dec const.
    QStringList decOutOfRange;
    decOutOfRange << "-129" << "-32768" << "-32769"
                  << "256" << "65535" << "65536";
    for(const auto& ident : decOutOfRange) {
        QTest::addRow("%s", QString("Byte constant out of range: .BYTE %1")
                      .arg(ident).toStdString().c_str())
                << QString(".BYTE %1 \n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << MacroAssembler::byteDecOutOfRange
                << false;
    }

    // Check that byte pseudo-ops fail when given a 2,3 byte hex const.
    QStringList hexOutOfRange;
    hexOutOfRange << "0x0100" << "0xffff" << "0x10000";
    for(const auto& ident : hexOutOfRange) {
        QTest::addRow("%s", QString("Byte constant out of range: .BYTE %1")
                      .arg(ident).toStdString().c_str())
                << QString(".BYTE %1 \n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << MacroAssembler::byteHexOutOfRange
                << false;
    }

    // Check that byte pseudo-ops fail when given a 2,3 byte string.
    QStringList strOutOfRange;
    strOutOfRange << "\"ab\"" << R"("\x00\x00")"
                  << "\"abc\"" << "\"\\x00\\x00\\x00\"";
    for(const auto& ident : strOutOfRange) {
        QTest::addRow("%s", QString("Byte constant out of range: .BYTE %1")
                      .arg(ident).toStdString().c_str())
                << QString(".BYTE %1 \n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << MacroAssembler::byteStringOutOfRange
                << false;
    }
}

void AssemblerTest::case_byteOutOfRange()
{
    execute();
}

void AssemblerTest::case_wordOutOfRange_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");


    // Since LDBA and STBA may perform memory ops, they need
    // an operand capable of accessing the entire address space,
    // and thus are in the word range check handler.

    // There is a test case for each dot command, as well as multiple
    // tests for non-unary instructions. As the dot commands all have
    // separate handlers in the assembler, they all must be tested separately.

    // Tests the maximum acceptable values of
    // decimal, hex, string, and character operands
    // for word sized instruction.
    QStringList inRange;
    inRange << "-1" << "-128" << "254" << "255"
            << "-129" << "-32768" << "256" << "65535"
            << R"('\x00')" << R"('\xff')"
            << R"("\x00")" << R"("\xff")" << R"("\x00\x00")" << R"("\xff\xff")"
            << "\"a\"" << "\"ab\""
            << "0x00" << "0xff" << "0x100" << "0xffff";

    for(const auto& ident : inRange) {
        QTest::addRow("%s", QString("Word constant in range: ldba %1,i")
                      .arg(ident).toStdString().c_str())
                << QString("ldba %1, i\n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << ""
                << true;
        QTest::addRow("%s", QString("Word constant in range: stbx %1,d")
                      .arg(ident).toStdString().c_str())
                << QString("stbx %1, d\n.END").arg(ident)
                << ModuleType::OPERATING_SYSTEM
                << ""
                << true;
        QTest::addRow("%s", QString("Word constant in range: ldwa %1,i")
                      .arg(ident).toStdString().c_str())
                << QString("ldwa %1, i\n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << ""
                << true;
        QTest::addRow("%s", QString("Word constant in range: stwx %1,d")
                      .arg(ident).toStdString().c_str())
                << QString("stwx %1, d\n.END").arg(ident)
                << ModuleType::OPERATING_SYSTEM
                << ""
                << true;
        QTest::addRow("%s", QString("Word constant in range: .WORD %1")
                      .arg(ident).toStdString().c_str())
                << QString(".WORD %1 \n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << ""
                << true;
        QTest::addRow("%s", QString("Word constant in range: sy:.EQUATE %1")
                      .arg(ident).toStdString().c_str())
                << QString("sy:.EQUATE %1 \n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << ""
                << true;

    }

    // Check that word mnemonics, pseudo-ops fail when given a 3,4,8 byte decimal const.
    QStringList decOutOfRange;
    decOutOfRange << "-32769" << "65536" << "655350"
    // Some versions of Pep10 have issue parsing a string that is
    // out of bounds of a 32 bit signed integer.
                  << "2147483647" << "2147483648"
                  << "4294967295" << "4294967296"
                  << "-2147483648" << "-2147483649"
                  << "-9223372036854775808";
    for(const auto& ident : decOutOfRange) {
        // BLOCK is the only instruction in the language requiring
        // an unsigned integer
        QTest::addRow("%s", QString("Word constant out of range: .BLOCK %1")
                      .arg(ident).toStdString().c_str())
                << QString(".BLOCK %1\n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << MacroAssembler::wordUnsignDecOutOfRange
                << false;
        QTest::addRow("%s", QString("Word constant out of range: ldba %1,i")
                      .arg(ident).toStdString().c_str())
                << QString("ldba %1, i\n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << MacroAssembler::wordSignDecOutOfRange
                << false;
        QTest::addRow("%s", QString("Word constant out of range: stbx %1,d")
                      .arg(ident).toStdString().c_str())
                << QString("stbx %1, d\n.END").arg(ident)
                << ModuleType::OPERATING_SYSTEM
                << MacroAssembler::wordSignDecOutOfRange
                << false;
        QTest::addRow("%s", QString("Word constant out of range: ldwa %1,i")
                      .arg(ident).toStdString().c_str())
                << QString("ldwa %1, i\n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << MacroAssembler::wordSignDecOutOfRange
                << false;
        QTest::addRow("%s", QString("Word constant out of range: stwx %1,d")
                      .arg(ident).toStdString().c_str())
                << QString("stwx %1, d\n.END").arg(ident)
                << ModuleType::OPERATING_SYSTEM
                << MacroAssembler::wordSignDecOutOfRange
                << false;
        QTest::addRow("%s", QString("Word constant out of range: .WORD %1")
                      .arg(ident).toStdString().c_str())
                << QString(".WORD %1 \n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << MacroAssembler::wordSignDecOutOfRange
                << false;
        QTest::addRow("%s", QString("Word constant out of range: sy:.EQUATE %1")
                      .arg(ident).toStdString().c_str())
                << QString("sy:.EQUATE %1 \n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << MacroAssembler::wordSignDecOutOfRange
                << false;

    }

    // Check that word mnemonics, pseudo-ops fail when given a 3,4,8 byte hex const.
    QStringList hexOutOfRange;
    hexOutOfRange << "0x10000" << "0xFFFF1"
    // Some versions of Pep10 have issues with very large hex constants.
                  << "0xFFFFFF" << "0xFFFFFFFF" << "0xFFFFFFFFFFFFF";
    for(const auto& ident : hexOutOfRange) {
        QTest::addRow("%s", QString("Word constant out of range: ldba %1,i")
                      .arg(ident).toStdString().c_str())
                << QString("ldba %1, i\n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << MacroAssembler::wordHexOutOfRange
                << false;
        QTest::addRow("%s", QString("Word constant out of range: stbx %1,d")
                      .arg(ident).toStdString().c_str())
                << QString("stbx %1, d\n.END").arg(ident)
                << ModuleType::OPERATING_SYSTEM
                << MacroAssembler::wordHexOutOfRange
                << false;
        QTest::addRow("%s", QString("Word constant out of range: ldwa %1,i")
                      .arg(ident).toStdString().c_str())
                << QString("ldwa %1, i\n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << MacroAssembler::wordHexOutOfRange
                << false;
        QTest::addRow("%s", QString("Word constant out of range: stwx %1,d")
                      .arg(ident).toStdString().c_str())
                << QString("stwx %1, d\n.END").arg(ident)
                << ModuleType::OPERATING_SYSTEM
                << MacroAssembler::wordHexOutOfRange
                << false;
        QTest::addRow("%s", QString("Word constant out of range: .WORD %1")
                      .arg(ident).toStdString().c_str())
                << QString(".WORD %1 \n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << MacroAssembler::wordHexOutOfRange
                << false;
        QTest::addRow("%s", QString("Word constant out of range: sy:.EQUATE %1")
                      .arg(ident).toStdString().c_str())
                << QString("sy:.EQUATE %1 \n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << MacroAssembler::wordHexOutOfRange
                << false;

    }

    // Check that word mnemonics, pseudo-ops fail when given a 3+ byte string const.
    QStringList strOutOfRange;
    strOutOfRange << "\"abc\"" << R"("\x00\x00\x00")"
                  << "\"String way too long.\"";
    for(const auto& ident : strOutOfRange) {
        QTest::addRow("%s", QString("Word constant out of range: ldba %1,i")
                      .arg(ident).toStdString().c_str())
                << QString("ldba %1, i\n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << MacroAssembler::wordStringOutOfRange
                << false;
        QTest::addRow("%s", QString("Word constant out of range: stbx %1,d")
                      .arg(ident).toStdString().c_str())
                << QString("stbx %1, d\n.END").arg(ident)
                << ModuleType::OPERATING_SYSTEM
                << MacroAssembler::wordStringOutOfRange
                << false;
        QTest::addRow("%s", QString("Word constant out of range: ldwa %1,i")
                      .arg(ident).toStdString().c_str())
                << QString("ldwa %1, i\n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << MacroAssembler::wordStringOutOfRange
                << false;
        QTest::addRow("%s", QString("Word constant out of range: stwx %1,d")
                      .arg(ident).toStdString().c_str())
                << QString("stwx %1, d\n.END").arg(ident)
                << ModuleType::OPERATING_SYSTEM
                << MacroAssembler::wordStringOutOfRange
                << false;
        QTest::addRow("%s", QString("Word constant out of range: .WORD %1")
                      .arg(ident).toStdString().c_str())
                << QString(".WORD %1 \n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << MacroAssembler::wordStringOutOfRange
                << false;
        QTest::addRow("%s", QString("Word constant out of range: sy:.EQUATE %1")
                      .arg(ident).toStdString().c_str())
                << QString("sy:.EQUATE %1 \n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << MacroAssembler::wordStringOutOfRange
                << false;
    }

}

void AssemblerTest::case_wordOutOfRange()
{
    execute();
}

void AssemblerTest::case_badAddrssArg_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");


    // Check that .ADDRSS will assemble when given a symbolic argument.
    QTest::newRow("Good argument for .ADDRSS.")
            << "sy:.ADDRSS sy \n.END"
            << ModuleType::USER_PROGRAM
            << ""
            << true;

    // Check that .ADDRSS fails when given a symbol that is too long.
    QTest::newRow("Bad argument for .ADDRSS symboltoolong.")
            << ".ADDRSS symboltoolong \n.END"
            << ModuleType::USER_PROGRAM
            << MacroAssembler::longSymbol.arg("symboltoolong")
            << false;

    // Check that .ADDRSS fails to assemble when given any other value as an argument.
    QStringList badArgs;
    badArgs << "-1" << "0x0"
            << "\"hi\"" << "'c'" << "65535" << "@asra2";
    for(const auto& ident : badArgs) {
        QTest::addRow("%s", QString("Bad argument for .ADDRSS: %1")
                      .arg(ident).toStdString().c_str())
                << QString(".ADDRSS %1\n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << MacroAssembler::badAddrssArgument
                << false;
    }

    // Bad symbolic arguments (those where the symbols are undefined)
    // can't be caught until the linker is invoked, and will be tested in the linker
    // test suite.
}

void AssemblerTest::case_badAddrssArg()
{
    execute();
}

void AssemblerTest::case_badAlignArg_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");


    // Check that .ALIGN works with the proper sizes (2,4,8).
    QStringList goodArgs;
    goodArgs << "2" << "4" << "8";
    for(const auto& ident : goodArgs) {
        QTest::addRow("%s", QString("Good argument for .ALIGN: %1")
                      .arg(ident).toStdString().c_str())
                << QString(".ALIGN %1\n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << ""
                << true;
    }

    // Check that .ALIGN fails to assemble when given a non-integer argument.
    QStringList nonIntArgs;
    nonIntArgs << "0x0" << "0x1" << "0x7" << "asra" << "udefsym"
               << "\"hi\"" << "'c'" << "@asra2";
    for(const auto& ident : nonIntArgs) {
        QTest::addRow("%s", QString("Bad argument for .ALIGN: %1")
                      .arg(ident).toStdString().c_str())
                << QString(".ALIGN %1\n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << MacroAssembler::badAlignArgument
                << false;
    }

    // Check that .ALIGN fails when given an integer outside of {2,4,8}
    QStringList badIntArgs;
    badIntArgs << "0" << "1" << "7" << "65535" << "65536";
    for(const auto& ident : badIntArgs) {
        QTest::addRow("%s", QString("Bad argument for .ALIGN: %1")
                      .arg(ident).toStdString().c_str())
                << QString(".ALIGN %1\n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << MacroAssembler::decConst248
                << false;
    }
}

void AssemblerTest::case_badAlignArg()
{
    execute();
}

void AssemblerTest::case_badAsciiArg_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");


    // Check that .ASCII assembles when given various string arguments.
    QStringList goodArgs;
    goodArgs << "\"a\"" << "\"ab\"" << R"("abcdefgh\x00")";
    for(const auto& ident : goodArgs) {
        QTest::addRow("%s", QString("Good argument for .ASCII: %1")
                      .arg(ident).toStdString().c_str())
                << QString(".ASCII %1\n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << ""
                << true;
    }

    // Check that .ASCII fails to assemble when given a non-string argument.
    QStringList badArgs;
    badArgs << "-1" << "0" << "asra" << "udefsym" <<"defSym"
            << "0xffff" << "'c'" << "65535" << "@asra2";
    for(const auto& ident : badArgs) {
        QTest::addRow("%s", QString("Bad argument for .ASCII: %1")
                      .arg(ident).toStdString().c_str())
                << QString("defSym: .ASCII %1\n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << MacroAssembler::badAsciiArgument
                << false;
    }
}

void AssemblerTest::case_badAsciiArg()
{
    execute();
}

void AssemblerTest::case_badBlockArg_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");


    // Check that .BLOCK assembles with a numeric literal argument.
    QStringList goodArgs;
    goodArgs << "0" << "1" << "67" << "65535"
             << "0x0" << "0x100" << "0xFFFF";
    for(const auto& ident : goodArgs) {
        QTest::addRow("%s", QString("Good argument for .BLOCK: %1")
                      .arg(ident).toStdString().c_str())
                << QString(".BLOCK %1\n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << ""
                << true;
    }

    // Check that .BLOCK fails to assemble when given a non-numeric argument.
    QStringList nonIntArgs;
    nonIntArgs << "asra" << "udefsym"
               << "\"hi\"" << "'c'" << "@asra2";
    for(const auto& ident : nonIntArgs) {
        QTest::addRow("%s", QString("Bad argument for .BLOCK: %1")
                      .arg(ident).toStdString().c_str())
                << QString(".BLOCK %1\n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << MacroAssembler::badBlockArgument
                << false;
    }
}

void AssemblerTest::case_badBlockArg()
{
    execute();
}

void AssemblerTest::case_badBurnArg_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");


    // Check that .BURN assembles with a hex argument
    QStringList hexArgs;
    hexArgs << "0x0" << "0x100" << "0xFFFF";
    for(const auto& ident : hexArgs) {
        QTest::addRow("%s", QString("Good argument for .BURN: %1")
                      .arg(ident).toStdString().c_str())
                << QString(".BURN %1\n.END").arg(ident)
                << ModuleType::OPERATING_SYSTEM
                << ""
                << true;
    }

    // Check that .BURN fails to assemble when given a non-hex argument.
    QStringList nonHexArgs;
    nonHexArgs << "asra" << "udefsym"
               << "\"hi\"" << "'c'"
               << "0" << "1" << "65535" << "@asra2";
    for(const auto& ident : nonHexArgs) {
        QTest::addRow("%s", QString("Bad argument for .BURN: %1")
                      .arg(ident).toStdString().c_str())
                << QString(".BURN %1\n.END").arg(ident)
                << ModuleType::OPERATING_SYSTEM
                << MacroAssembler::badBurnArgument
                << false;
    }

    // Tests in linker will assert that only an OS may contain a .BURN,
    // and that exactly one .BURN is present in an OS.
}

void AssemblerTest::case_badBurnArg()
{
    execute();
}

void AssemblerTest::case_badByteArg_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");


    // Check that .BYTE assembles with a numeric, character, string argument.
    QStringList validArgs;
    validArgs << "0x0" << "0xFF" << R"("\x00")"<< R"('\x00')"
              << "1" << "-128" << "255";
    for(const auto& ident : validArgs) {
        QTest::addRow("%s", QString("Good argument for .BYTE: %1")
                      .arg(ident).toStdString().c_str())
                << QString(".BYTE %1\n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << ""
                << true;
    }

    // Check that .BYTE fails when given a non-numeric argument.
    QStringList invalidArgs;
    invalidArgs << "asra" << "udefsym" << "defSym" << "@asra2";
    for(const auto& ident : invalidArgs) {
        QTest::addRow("%s", QString("Bad argument for .BYTE: %1")
                      .arg(ident).toStdString().c_str())
                << QString("defSym:.BYTE %1\n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << MacroAssembler::badByteArgument
                << false;
    }

    // Out of range arguments are check with case_byteOutOfRange;
}

void AssemblerTest::case_badByteArg()
{
    execute();
}

void AssemblerTest::case_badEnd_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");


    // Check that a symbol before .END fails to assemble.
    QTest::addRow("No symbol before .END")
            << QString("sym:.END")
            << ModuleType::USER_PROGRAM
            << MacroAssembler::endForbidsSymbol
            << false;

    // Check that .END fails when followed a token other than a comment or newline.
    QStringList nonCommentArgs;
    nonCommentArgs <<"0x0" << "asra" <<"undefsym"
               << "\"hi\"" << "'c'" << "0xFFFF"
               << "1" << "@asra2";
    for(const auto& ident : nonCommentArgs) {
        QTest::addRow("%s", QString("Bad token following .END: %1")
                      .arg(ident).toStdString().c_str())
                << QString(".END %1").arg(ident)
                << ModuleType::USER_PROGRAM
                << MacroAssembler::endOnlyComment
                << false;
    }
}

void AssemblerTest::case_badEnd()
{
    execute();
}

void AssemblerTest::case_noSymbolEquate_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");


    // Check that .EQUATE requires a symbol to assemble.
    QTest::addRow("Missing symbol before .EQUATE")
            // Line with EQUATE must have a symbol for it to be well formed.
            << QString(".EQUATE 5\n.END")
            << ModuleType::USER_PROGRAM
            << MacroAssembler::equateRequiresSymbol
            << false;
}

void AssemblerTest::case_noSymbolEquate()
{
    execute();
}

void AssemblerTest::case_badEquateArg_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");


    // Check that .EQUATE assembles with a numeric, character, string argument.
    QStringList validArgs;
    validArgs << "0x0" << "0xFFff" << R"("\x01\x02")"<< R"('\x00')"
              << "1" << "-128" << "65535";
    for(const auto& ident : validArgs) {
        QTest::addRow("%s", QString("Good argument for .EQUATE: %1")
                      .arg(ident).toStdString().c_str())
                // Line with EQUATE must have a symbol for it to be well formed.
                << QString("sy:.EQUATE %1\n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << ""
                << true;
    }

    // Check that .EQUATE fails when given an identiifer, macro argument.
    QStringList invalidArgs;
    invalidArgs << "asra" << "udefsym" << "defSym" << "@asra2";
    for(const auto& ident : invalidArgs) {
        QTest::addRow("%s", QString("Bad argument for .EQUATE: %1")
                      .arg(ident).toStdString().c_str())
                << QString("defSym:.EQUATE %1\n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << MacroAssembler::badEquateArgument
                << false;
    }

    // Out of range arguments are check with case_wordOutOfRange.
}

void AssemblerTest::case_badEquateArg()
{
    execute();
}

void AssemblerTest::case_symbolExport_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");


    // Check that .EXPORT must not define a symbol.
    QTest::addRow(".EXPORT defines symbol")
            << QString("sy:.EXPORT sy\n.END")
            << ModuleType::OPERATING_SYSTEM
            << MacroAssembler::exportForbidsSymbol
            << false;
}

void AssemblerTest::case_symbolExport()
{
   execute();
}

void AssemblerTest::case_badExportArg_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");


    // Check that .EXPORT works with various (valid) long symbols.
    QStringList validSymbols;
    validSymbols << "abcdef" << "abcdefgh";
    for(const auto& ident : validSymbols) {
        QTest::addRow("%s", QString("Good argument for .EXPORT: %1")
                      .arg(ident).toStdString().c_str())
                // Operating system must be well formed to prevent other errors.
                << QString(".BURN 0xFFFF\n %1:.BYTE 0\n .EXPORT %1\n .END").arg(ident)
                << ModuleType::OPERATING_SYSTEM
                << ""
                << true;
    }

    // Check that .EXPORT fails symbols that are too long.
    QStringList tooLongSymbols;
    tooLongSymbols << "abcdefghi";
    for(const auto& ident : tooLongSymbols) {
        QTest::addRow("%s", QString("Bad argument for .EXPORT: %1")
                      .arg(ident).toStdString().c_str())
                // Operating system must be well formed to prevent other errors.
                << QString(".BURN 0xFFFF\n %1:.BYTE 0\n .EXPORT %1\n .END").arg(ident)
                << ModuleType::OPERATING_SYSTEM
                << MacroAssembler::longSymbol.arg(ident)
                << false;
    }

    // Check that .EXPORT fails to assemble when given a non-symbolic argument.
    QStringList nonSymbolArgs;
    nonSymbolArgs <<"0x0"
               << "\"hi\"" << "'c'" << "0xFFFF"
               << "0" << "1" << "65535" << "@asra2";
    for(const auto& ident : nonSymbolArgs) {
        QTest::addRow("%s", QString("Bad argument for .EXPORT: %1")
                      .arg(ident).toStdString().c_str())
                // Operating system must be well formed to prevent other errors.
                << QString(".BURN 0xFFFF\n .BYTE 0\n .EXPORT %1\n .END").arg(ident)
                << ModuleType::OPERATING_SYSTEM
                << MacroAssembler::exportRequiresSymbol
                << false;
    }

    // Undefined symbol check are performed in the linker test suite, so no checks
    // for bad symbolic arguments may be performed in the assembler.

}

void AssemblerTest::case_badExportArg()
{
    execute();
}

void AssemblerTest::case_noSymbolScall_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");


    // Check that .SCALL may not define a symbol.
    QTest::addRow(".SCALL defines symbol")
            << QString("sy:.SCALL sy\n.END")
            << ModuleType::OPERATING_SYSTEM
            << MacroAssembler::scallForbidsSymbol
            << false;
}

void AssemblerTest::case_noSymbolScall()
{
    execute();
}

void AssemblerTest::case_badScallArg_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");


    // Purge any system calls added by other test cases.
    registry->clearSystemCalls();

    // Check that .SCALL works with various symbols.
    QStringList validSymbols;
    validSymbols << "abcdef" << "abcdefgh";
    for(const auto& ident : validSymbols) {
        QTest::addRow("%s", QString("Good argument for .SCALL: %1")
                      .arg(ident).toStdString().c_str())
                // Operating system must be well formed to prevent other errors.
                << QString(".BURN 0xFFFF\n %1:.BYTE 0\n .SCALL %1\n .END").arg(ident)
                << ModuleType::OPERATING_SYSTEM
                << ""
                << true;
    }

    // Check that .SCALL fails symbols that are too long.
    QStringList tooLongSymbols;
    tooLongSymbols << "abcdefghi";
    for(const auto& ident : tooLongSymbols) {
        QTest::addRow("%s", QString("Bad argument for .SCALL: %1")
                      .arg(ident).toStdString().c_str())
                // Operating system must be well formed to prevent other errors.
                << QString(".BURN 0xFFFF\n %1:.BYTE 0\n .SCALL %1\n .END").arg(ident)
                << ModuleType::OPERATING_SYSTEM
                << MacroAssembler::longSymbol.arg(ident)
                << false;
    }

    // Check that .SCALL fails to assemble when given a non-symbolic argument.
    QStringList nonSymbolArgs;
    nonSymbolArgs <<"0x0"
               << "\"hi\"" << "'c'" << "0xFFFF"
               << "0" << "1" << "65535" << "@asra2";
    for(const auto& ident : nonSymbolArgs) {
        QTest::addRow("%s", QString("Bad argument for .SCALL: %1")
                      .arg(ident).toStdString().c_str())
                // Operating system must be well formed to prevent other errors.
                << QString(".BURN 0xFFFF\n .BYTE 0\n .SCALL %1\n .END").arg(ident)
                << ModuleType::OPERATING_SYSTEM
                << MacroAssembler::scallRequiresSymbol
                << false;
    }

    // Check that redefinition of a system call fails
    QTest::addRow("Multiple .SCALL for a single symbol")
            << QString(".BURN 0xFFFF\n .SCALL sy\n sy:.BYTE 0\n .SCALL sy\n .END")
            << ModuleType::OPERATING_SYSTEM
            << MacroAssembler::failedToRegisterMacro.arg("sy")
            << false;

    // Undefined symbol check are performed in the linker test suite, so no checks
    // for bad symbolic arguments may be performed in the assembler.
}

void AssemblerTest::case_badScallArg()
{
    execute();
}

void AssemblerTest::case_noSymbolUscall_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");


    // Check that .USCALL may not define a symbol.
    QTest::addRow(".USCALL defines symbol")
            << QString("sy:.USCALL sy\n.END")
            << ModuleType::OPERATING_SYSTEM
            << MacroAssembler::uscallForbidsSymbol
            << false;
}

void AssemblerTest::case_noSymbolUscall()
{
    execute();
}

void AssemblerTest::case_badUscallArg_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");


    // Purge any system calls added by other test cases.
    registry->clearSystemCalls();

    // Check that .USCALL works with various symbols.
    QStringList validSymbols;
    validSymbols << "abcdef" << "abcdefgh";
    for(const auto& ident : validSymbols) {
        QTest::addRow("%s", QString("Good argument for .USCALL: %1")
                      .arg(ident).toStdString().c_str())
                // Operating system must be well formed to prevent other errors.
                << QString(".BURN 0xFFFF\n %1:.BYTE 0\n .USCALL %1\n .END").arg(ident)
                << ModuleType::OPERATING_SYSTEM
                << ""
                << true;
    }

    // Check that .USCALL fails symbols that are too long.
    QStringList tooLongSymbols;
    tooLongSymbols << "abcdefghi";
    for(const auto& ident : tooLongSymbols) {
        QTest::addRow("%s", QString("Bad argument for .USCALL: %1")
                      .arg(ident).toStdString().c_str())
                // Operating system must be well formed to prevent other errors.
                << QString(".BURN 0xFFFF\n %1:.BYTE 0\n .USCALL %1\n .END").arg(ident)
                << ModuleType::OPERATING_SYSTEM
                << MacroAssembler::longSymbol.arg(ident)
                << false;
    }

    // Check that .USCALL fails to assemble when given a non-symbolic argument.
    QStringList nonSymbolArgs;
    nonSymbolArgs <<"0x0"
               << "\"hi\"" << "'c'" << "0xFFFF"
               << "0" << "1" << "65535" << "@asra2";
    for(const auto& ident : nonSymbolArgs) {
        QTest::addRow("%s", QString("Bad argument for .USCALL: %1")
                      .arg(ident).toStdString().c_str())
                // Operating system must be well formed to prevent other errors.
                << QString(".BURN 0xFFFF\n .BYTE 0\n .USCALL %1\n .END").arg(ident)
                << ModuleType::OPERATING_SYSTEM
                << MacroAssembler::uscallRequiresSymbol
                << false;
    }

    // Check that .USCALL may not define a symbol.
    QTest::addRow("Multiple .USCALL for a single symbol")
            << QString(".BURN 0xFFFF\n .USCALL sy\n sy:.BYTE 0\n .USCALL sy\n .END")
            << ModuleType::OPERATING_SYSTEM
            << MacroAssembler::failedToRegisterMacro.arg("sy")
            << false;

    // Undefined symbol check are performed in the linker test suite, so no checks
    // for bad symbolic arguments may be performed in the assembler.
}

void AssemblerTest::case_badUscallArg()
{
    execute();
}

void AssemblerTest::case_badWordArg_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");


    // Check that .WORD assembles with a numeric, character, string argument.
    QStringList validArgs;
    validArgs << "0x0" << "0xFF" << "0xffff" << R"("\x00")" << R"('\x00')"
              << "1" << "-128" << "255" << R"("\x00\x00")" << "65535"
              << "-32768";
    for(const auto& ident : validArgs) {
        QTest::addRow("%s", QString("Good argument for .WORD: %1")
                      .arg(ident).toStdString().c_str())
                << QString(".WORD %1\n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << ""
                << true;
    }

    // Check that .WORD fails when given a non-numeric argument.
    QStringList invalidArgs;
    invalidArgs << "asra" << "udefsym" << "defSym" << "@asra2";
    for(const auto& ident : invalidArgs) {
        QTest::addRow("%s", QString("Bad argument for .WORD: %1")
                      .arg(ident).toStdString().c_str())
                << QString("defSym:.WORD %1\n.END").arg(ident)
                << ModuleType::USER_PROGRAM
                << MacroAssembler::badWordArgument
                << false;
    }
}

void AssemblerTest::case_badWordArg()
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
