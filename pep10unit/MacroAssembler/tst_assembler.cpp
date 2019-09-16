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

void AssemblerTest::case_missingEnd_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");

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

void AssemblerTest::case_invalidMnemonic_data()
{
    QTest::addColumn<QString>("ProgramText");
    QTest::addColumn<ModuleType>("MainModuleType");
    QTest::addColumn<QString>("ExpectedError");
    QTest::addColumn<bool>("ExpectPass");

    // Tests expected to fail.
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

    // Passing unary instructions
    QTest::addRow("Parse unary instruction ASRA.")
            << "asra\n.END\n"
            << ModuleType::USER_PROGRAM
            << ""
            << true;

    // Passing nonunary instructions
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

    using element = typename std::tuple<QString,QString, ModuleType, bool, QString>;
    QList<element> identList;
    // For each token type, construct a test case with a symbol before it.
    // For tokens that identify multiple code lines (identifiers, dot commands)
    // must test multiple permutations of these symbols.
    identList  << element("User .SCALL", ".SCALL ld\n", ModuleType::USER_PROGRAM,
                          false, MacroAssembler::onlyInOperatingSystem.arg(".SCALL"))
               << element("OS .SCALL", ".SCALL ld\n", ModuleType::OPERATING_SYSTEM,
                          false, MacroAssembler::missingEND)
               << element("User .USCALL", ".USCALL ld\n", ModuleType::USER_PROGRAM,
                          false, MacroAssembler::onlyInOperatingSystem.arg(".USCALL"))
               << element("OS .USCALL", ".USCALL 0\n", ModuleType::OPERATING_SYSTEM,
                          false, MacroAssembler::missingEND)
               << element("User .BURN", ".BURN 0x9f\n", ModuleType::USER_PROGRAM,
                          false, MacroAssembler::onlyInOperatingSystem.arg(".BURN"))
               << element("OS .BURN", ".BURN 0x9f\n", ModuleType::OPERATING_SYSTEM,
                          false, MacroAssembler::missingEND)
               << element("User .EXPORT", ".EXPORT ld\n", ModuleType::USER_PROGRAM,
                          false, MacroAssembler::onlyInOperatingSystem.arg(".EXPORT"))
               << element("OS .EXPORT", ".EXPORT ld\n", ModuleType::OPERATING_SYSTEM,
                          false, MacroAssembler::missingEND);
    for(auto ident : identList) {
        registry->clearCustomMacros();
        QTest::addRow("%s", QString("Testing %1.")
                      .arg(std::get<0>(ident)).toStdString().c_str())
                << QString("%1").arg(std::get<1>(ident))
                << std::get<2>(ident)
                << std::get<4>(ident)
                << std::get<3>(ident);
        registry->registerCustomMacro("test",
                                      QString("@test 0\n %1\n.END").arg(std::get<1>(ident)));
        QTest::addRow("%s", QString("Testing %1 as an embedded macro.")
                      .arg(std::get<0>(ident)).toStdString().c_str())
                << "@test\n"
                << std::get<2>(ident)
                << std::get<4>(ident)
                << std::get<3>(ident);
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


    // Invalid dot commands
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
    // Validate all remaining dot commands
    #pragma message("TODO: Move validation to helper functions")
    #pragma message("TODO: Finish purmuting dot commands / operands")
    QTest::addRow("Validate .END.")
            << ".END\n"
            << ModuleType::USER_PROGRAM
            << ""
            << true;

    // Validate other arguments in handler specifically for ADDRSS.
    QTest::addRow("Validate .ADDRSS ld.")
            << "ld:nop\n.ADDRSS ld\n.END\n"
            << ModuleType::USER_PROGRAM
            << ""
            << true;

    // Validate non {2,4,8} args in .ALIGN handler.
    for(auto string : {"2","4","8"}) {
        QTest::addRow("%s", QString("Validate .ALIGN %1.")
                      .arg(string).toStdString().c_str())
                << QString(".ALIGN %1\n.END\n").arg(string)
                << ModuleType::USER_PROGRAM
                << ""
                << true;
    }

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

    // Failing tests that omit addressing modes on non-unary instructions
    QSet<QString> mnemonics;
    QMetaObject meta = Enu::staticMetaObject;
    QMetaEnum metaEnum = meta.enumerator(meta.indexOfEnumerator("EMnemonic"));
    QMetaEnum addrEnum = meta.enumerator(meta.indexOfEnumerator("EAddrMode"));
    QString tempqs;
    for(int it = 0; it < metaEnum.keyCount(); it++)
    {
        auto mnemonic = static_cast<Enu::EMnemonic>(metaEnum.value(it));
        // If the instruction requires an addressing mode, construct
        // a test where it it provided no addressing modes.
        if(Pep::addrModeRequiredMap[mnemonic]) {
            QTest::newRow(QString("Require addressing mode for %1.")
                          .arg(QString(metaEnum.key(it)).toUpper()).toStdString().c_str())
                    << QString(metaEnum.key(it)).toUpper() + " k \n.END\n"
                    << ModuleType::USER_PROGRAM
                    << MacroAssembler::reqAddrMode
                    << false;

            // If the instruction doesn't allow all addressing modes,
            // then construct a test case where it is provided the illegal
            // addressing mode.
            if(auto allowed = Pep::addrModesMap[mnemonic];
                    allowed != static_cast<int>(Enu::EAddrMode::ALL)) {
                for(int addrIt = 0; addrIt < addrEnum.keyCount(); addrIt++) {
                    if(allowed & addrEnum.value(addrIt)) {
                        continue;
                    }
                    else if(static_cast<Enu::EAddrMode>(addrEnum.value(addrIt))
                            == Enu::EAddrMode::NONE) {
                        continue;
                    }
                    QTest::newRow(QString("Illegal addressing mode for %1 k,%2.")
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
        // Handle true unary instructions where any addressing mode is illegal.
        else if(Pep::isUnaryMap[mnemonic]){
            #pragma message("TODO: Create better error message for unary instruction treated like nonunary")
            QTest::newRow(QString("Forbid addressing mode for unary instruction %1.")
                          .arg(QString(metaEnum.key(it)).toUpper()).toStdString().c_str())
                    << QString(metaEnum.key(it)).toUpper() + " k,n \n.END\n"
                    << ModuleType::USER_PROGRAM
                    << MacroAssembler::unxpectedEOL
                    << false;
        }
        // Handle BR / CALL mnemonics where the addressing mode is optional.
        else{
            #pragma message("TODO: Create better error message for unary instruction treated like nonunary")
            for(int addrIt = 0; addrIt < addrEnum.keyCount(); addrIt++) {
                if(Pep::addrModesMap[mnemonic] & addrEnum.value(addrIt)) {
                    continue;
                }
                else if(static_cast<Enu::EAddrMode>(addrEnum.value(addrIt))
                        == Enu::EAddrMode::NONE) {
                    continue;
                }
                QTest::newRow(QString("Illegal addressing mode for %1 k,%2.")
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


    // Test that br will fail to assemble with any argument
    // pattern that are similar to correct invocations.
    // We have already tested missing addressing modes,
    // so even if an instruction were missing both,
    // two compile - edit passes would catch both errors.
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
    // We have previous tests that check for presence of addressing modes,
    // so no need to rpeeat ourselves here.
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
