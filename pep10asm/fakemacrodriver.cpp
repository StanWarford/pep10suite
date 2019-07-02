#include "fakemacrodriver.h"

FakeMacroDriver::FakeMacroDriver(): registry(new MacroRegistry()), assembler(new MacroAssemblerDriver(registry))
{
    registry->registerCoreMacro("asl2", "%asl2 0\nasl\nasl\n.END\n", 0);
    registry->registerCoreMacro("asl3", "%asl3 0\n%asl2\nasl\n.END\n", 0);
    registry->registerCoreMacro("asl4", "%asl4 0\n%asl3\nasl\n.END\n", 0);
    registry->registerCoreMacro("asl5", "%asl5 0\n%asl4\nasl\n.END\n", 0);
    registry->registerCoreMacro("asl6", "%asl6 0\n%asl5\nasl\n.END\n", 0);
    registry->registerCoreMacro("asl7", "%asl7 0\n%asl6\nasl\n.END\n", 0);
    registry->registerCoreMacro("DIVA", "%DIVA 2\nLDWX $1,$2\nCPWX 0,i\n%asl4\n.END\n", 2);
    // Trigger loop detection
    registry->registerCoreMacro("LOOPA", "%LOOPA 0\n%LOOPB\n.END\n", 0);
    registry->registerCoreMacro("LOOPB", "%LOOPB 0\n%LOOPA\n.END\n", 0);

    // Angry diamond
    registry->registerCoreMacro("L1A", "%L1A 0\n%L1B\n%L2\n.END\n", 0);
    registry->registerCoreMacro("L1B", "%L1B 0\n%L2\n.END\n", 0);
    registry->registerCoreMacro("L2", "%L2 0\n%asl7\n%L3\n.END\n", 0);
    // Angry diamond
    registry->registerCoreMacro("L3", "%L3 0\n%L4A\n%L4B\n%L4C\n.END\n", 0);
    registry->registerCoreMacro("L4A", "%L4A 0\n%L4B\n%L4C\n.END\n", 0);
    registry->registerCoreMacro("L4B", "%L4B 0\n%L4C\n%DIVA l,sfx\n.END\n", 0);
    // Causes a loop in the bottom of a recursive tree.
    //registry->registerCoreMacro("L4C", "%L4C 0\n%DIVA k,d\n%LOOPA\n.END\n", 0);
    //Causes a syntax error at the bottom of the tree: too many args.
    //registry->registerCoreMacro("L4C", "%L4C 0\n%DIVA k,l,d\n.END\n", 0);
    //Causes a syntax error at the bottom of the tree: too many args.
    //registry->registerCoreMacro("L4C", "%L4C 0\n%DIVA k,,d\n.END\n", 0);
    //Causes a syntax error at the bottom of the tree: recursive!.
    //registry->registerCoreMacro("L4C", "%L4C 0\n%L4C\n.END\n", 0);
    // Works normally.
    registry->registerCoreMacro("L4C", "%L4C 0\n%DIVA k,d\n.END\n", 0);


}

FakeMacroDriver::~FakeMacroDriver()
{
    delete registry;
    delete assembler;
}

void FakeMacroDriver::run()
{
    QString input =
    "ADDA 5,d\n.END\n";
    //"%asl5\n%DIVA k,d\n%L1A\n.END\n";
    //"%LOOPA\n.END\n";
    assembler->assembleUserProgram(input);

}
