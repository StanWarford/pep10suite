#include "tst_assembleprograms.h"
#include "macroassemblerdriver.h"
AssemblePrograms::AssemblePrograms(): registry(new MacroRegistry())
{

}

AssemblePrograms::~AssemblePrograms() = default;

void AssemblePrograms::initTestCase()
{
    osText = Pep::resToString(":/help-asm/figures/pep10os.pep", false);
    QVERIFY2(!osText.isEmpty(), "Default operating system was empty.");

    QSharedPointer<AsmProgram> prog;
    MacroAssemblerDriver assembler(registry);
    auto asmResult = assembler.assembleOperatingSystem(osText);

    QVERIFY2(asmResult.success, "Assembly of operating system did not succede");
    QVERIFY2(!asmResult.program.isNull(), "Operating system program does not exist");
    QVERIFY2(asmResult.program->getProgram().length() != 0,
             "Operating system program contains no code.");

    operatingSystem = asmResult.program;
}

void AssemblePrograms::assemblePrograms_data()
{
    QTest::addColumn<QString>("ProgramText");
    QDirIterator it = QDirIterator(":/help-asm/figures/", QStringList()<<"fig*.pep");
    while(it.hasNext()) {
        auto file = it.next();
        QFileInfo info(file);
        QString str = "Assemble file "+info.fileName();
        QTest::newRow(str.toStdString().c_str())
                << Pep::resToString(file, false);
    }
}

void AssemblePrograms::assemblePrograms()
{
    QFETCH(QString, ProgramText);

    MacroAssemblerDriver assembler(registry);
    auto osSymTable = operatingSystem->getSymbolTable();
    auto asmResult = assembler.assembleUserProgram(ProgramText, operatingSystem->getSymbolTable());

    QVERIFY2(asmResult.success, "Assembly of sample program did not succede.");
    QVERIFY2(asmResult.errors.errors.empty(), "Sample program had warnings or errors.");
    QVERIFY2(!asmResult.program.isNull(), "Sample program does not exist.");
    QVERIFY2(asmResult.program->getProgram().length() != 0,
             "Sample program contains no code.");

}
