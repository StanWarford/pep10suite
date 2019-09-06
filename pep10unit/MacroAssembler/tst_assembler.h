#ifndef TST_ASSEMBLER_H
#define TST_ASSEMBLER_H

#include <QTest>
#include "macromodules.h"

class MacroRegistry;
class MacroPreprocessor;
class MacroAssembler;
struct ModuleAssemblyGraph;
class AssemblerTest : public QObject
{
    Q_OBJECT
public:
    AssemblerTest();
    ~AssemblerTest();

private slots:
    void initTestCase();
    void cleanupTestCase();

    // Test that true syntax errors are propogated to the end user.
    void case_syntaxError_data();
    void case_syntaxError();

    // Test cases for tokens generated by syntax errors.
    void case_unexpectedToken_data();
    void case_unexpectedToken();

    // Test cases where an EOL is encountered too early.
    //void case_unexpectedEOL_data();
    //void case_unexpectedEOL();

    // Test cases where something other than a comment is detected after a newline.
    //void case_expectNewlineCommnet_data();
    //void case_expectNewlineCommnet();

    // Test cases where a symbol is declared unexpctedly.
    //void case_unexpectedSymbolDeclaration_data();
    //void case_unexpectedSymbolDeclaration();

    // Test cases with non-existant mnemonics e.g. "YETI"
    //void case_invalidMnemonic_data();
    //void case_invalidMnemonic();

    // Test cases for dot commands that may only occur in operating system.
    //void case_onlyInOS_data();
    //void case_onlyInOS();

    // Test cases for invalid dot commands e.g. "YETI"
    //void case_invalidDotCommand_data();
    //void case_invalidDotCommand();

    // Test cases where a symbol is declared to be too long
    //void case_symbolTooLong_data();
    //void case_symbolTooLong();

private:
    QSharedPointer<MacroRegistry> registry;
    QSharedPointer<MacroPreprocessor> preprocessor;
    QSharedPointer<MacroAssembler> assembler;
    void preprocess(ModuleAssemblyGraph& graph, ModuleType programType);
    void execute();
};

#endif // TST_ASSEMBLER_H
