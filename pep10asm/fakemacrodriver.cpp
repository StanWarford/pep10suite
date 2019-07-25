#include "fakemacrodriver.h"

FakeMacroDriver::FakeMacroDriver(): registry(QSharedPointer<MacroRegistry>::create()), assembler(new MacroAssemblerDriver(registry))
{
    registry->registerCustomMacro("asla6", "@asla6 0\n@asla5\nasla\n.END\n");
    registry->registerCustomMacro("asla7", "@asla7 0\n@asla6\nasla\n.END\n");
    registry->registerCustomMacro("DIVA", "@DIVA 2\nLDWX $1,$2\nCPWX 0,i\n@asla2\n.END\n");
    // Trigger loop detection
    registry->registerCustomMacro("LOOPA", "@LOOPA 0\n@LOOPB\n.END\n");
    registry->registerCustomMacro("LOOPB", "@LOOPB 0\n@LOOPA\n.END\n");

    // Angry diamond
    registry->registerCustomMacro("L1A", "@L1A 0\n@L1B\n@L2\n.END\n");
    registry->registerCustomMacro("L1B", "@L1B 0\n@L2\n.END\n");
    registry->registerCustomMacro("L2",  "@L2 0\n@asla7\n@L3\n.END\n");
    // Angry diamond
    registry->registerCustomMacro("L3", "@L3 0\n@L4A\n@L4B\n@L4C\n.END\n");
    registry->registerCustomMacro("L4A", "@L4A 0\n@L4B\n@L4C\n.END\n");
    registry->registerCustomMacro("L4B", "@L4B 0\n@L4C\n@DIVA l,sfx\n.END\n");
    // Causes a loop in the bottom of a recursive tree.
    //registry->registerCustomMacro("L4C", "@L4C 0\n@DIVA k,d\n@LOOPA\n.END\n");
    //Causes a syntax error at the bottom of the tree: too many args.
    //registry->registerCustomMacro("L4C", "@L4C 0\n@DIVA k,l,d\n.END\n");
    //Causes a syntax error at the bottom of the tree: too many args.
    //registry->registerCustomMacro("L4C", "@L4C 0\n@DIVA k,,d\n.END\n");
    //Causes a syntax error at the bottom of the tree: recursive!.
    //registry->registerCustomMacro("L4C", "@L4C 0\n@L4C\n.END\n");
    // Works normally.
    registry->registerCustomMacro("L4C", "@L4C 0\n@DIVA k,d\n.END\n");


}

FakeMacroDriver::~FakeMacroDriver()
{
    delete assembler;
}

void FakeMacroDriver::run()
{
    QString input =
    //"ADDA 5,d\n.END\n";
    "@asla5\nldwa charIn,d\nl:@DIVA k,d\nk:@L1A\n.END\n";
    //"@DIVA k,sfx\n.END\n";
    //"@LOOPA\n.END\n";
    QString osText = Pep::resToString(":/help-asm/figures/pep10os.pep", false);
    auto osResult = assembler->assembleOperatingSystem(osText);
    auto useResult = assembler->assembleUserProgram(input, osResult->getSymbolTable());

}
