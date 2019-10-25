#ifndef TST_LINKER_H
#define TST_LINKER_H

#include <QTest>
#include "macromodules.h"

class AsmProgram;
class MacroAssembler;
class MacroLinker;
class MacroPreprocessor;
class MacroRegistry;

class LinkerTest: public QObject
{
       Q_OBJECT
public:
    LinkerTest();
    ~LinkerTest() override;
private slots:
    void initTestCase();
    void cleanupTestCase();

    // Test cases where the operating system is not defined.
    void case_noOS_data();
    void case_noOS();

    // Test cases where the program is longer than 64k.
    void case_programTooLong_data();
    void case_programTooLong();

    // Test cases where a program references a non-existant symbol.
    void case_noSuchSymbol_data();
    void case_noSuchSymbol();

    // Test cases where the operating system has no .BURN statement.
    void case_osNoBurn_data();
    void case_osNoBurn();

    // Test cases where the operating system has multiple .BURN statements.
    void case_osManyBurn_data();
    void case_osManyBurn();

    // Explicitly check that an EXPORT statement with an undefined symbol raises an error.
    //void case_osExportUndefinedOperand_data();
    //void case_osExportUndefinedOperand();

    // Check that ADDRSS, EXPORT, SCALL, USCALL fail when given an undefined symbol.

private:
    QSharedPointer<MacroRegistry> registry;
    QSharedPointer<MacroPreprocessor> preprocessor;
    QSharedPointer<MacroAssembler> assembler;
    QSharedPointer<MacroLinker> linker;
    void execute();
    void preprocess(ModuleAssemblyGraph& graph, ModuleType programType);
    void assemble(ModuleAssemblyGraph &graph);
};

#endif // TST_LINKER_H
