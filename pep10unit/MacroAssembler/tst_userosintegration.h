#ifndef TST_USEROSINTEGRATION_H
#define TST_USEROSINTEGRATION_H

#include <QTest>

#include "macromodules.h"

class AsmProgram;
class MacroAssembler;
class MacroLinker;
class MacroPreprocessor;
class MacroRegistry;

/*
 * Test cases for the interaction between user programs and the operating system
 *
 * One such interaction is EXPORT'ed symbols from the operating system appear in user
 * programs. If this mechanism were to fail, errors would be entirely opaque to the user
 * so extensive tests are needed to ensure that the only thing breaking user code is
 * user code.
 *
 * Other monitored behaviors include checking that .SCALL, .USCALL do not accidentally EXPORT
 * the associated symbol, as well as check that the system call macros generate a macro
 * usable in a user program.
 */
class UserOSIntegrationTest : public QObject
{
    Q_OBJECT

public:
    UserOSIntegrationTest();
    ~UserOSIntegrationTest() override;

private slots:
    void initTestCase();
    void cleanupTestCase();

    // Test that symbols exported from the operating system reach the user program.
    void case_testExport_data();
    void case_testExport();

    // Test that system call macros generate correct object code in user program.
    void case_testScall_data();
    void case_testScall();

    void case_testUScall_data();
    void case_testUScall();

    // Check that an operating system can't use its own system calls.
    void case_syscallInOS_data();
    void case_syscallInOS();

private:
    QSharedPointer<MacroRegistry> registry;
    QSharedPointer<MacroPreprocessor> preprocessor;
    QSharedPointer<MacroAssembler> assembler;
    QSharedPointer<MacroLinker> linker;
    void execute(ModuleAssemblyGraph& os, ModuleAssemblyGraph& user);
    void preprocess(ModuleAssemblyGraph& graph);
};
#endif // TST_USEROSINTEGRATION_H
