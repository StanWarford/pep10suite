#include "tst_assembleos.h"
#include "pep.h"
#include "asmprogram.h"
#include "macroassemblerdriver.h"
#include "macroregistry.h"
AssembleOS::AssembleOS(): registry(new MacroRegistry())
{

}

AssembleOS::~AssembleOS()
= default;

void AssembleOS::initTestCase()
{
    osText = Pep::resToString(":/help-asm/figures/pep10os.pep", false);
    QVERIFY2(!osText.isEmpty(), "Default operating system was empty.");
}

void AssembleOS::assembleOS()
{
    QSharedPointer<AsmProgram> prog;
    MacroAssemblerDriver assembler(registry);
    auto asmResult = assembler.assembleOperatingSystem(osText);
    QVERIFY2(asmResult.success, "Assembly of operating system did not succede");
    QVERIFY2(!asmResult.program.isNull(), "Operating system program does not exist");
    QVERIFY2(asmResult.program->getProgram().length() != 0,
             "Operating system program contains no code.");

}
