#ifndef FAKEMACRODRIVER_H
#define FAKEMACRODRIVER_H

#include "macroisaassembler.h"
#include "macrotokenizer.h"
#include "macroregistry.h"

/*
 * This class "assembles" a fake program, giving insight into
 * how the new macro assembler works via printouts to the console.
 *
 * It registers fake macros, and assembles a simple program.
 */
class FakeMacroDriver
{
public:
    FakeMacroDriver();
    ~FakeMacroDriver();
    void run();
private:
    MacroRegistry* registry;
    MacroISAAssembler* assembler;
};

#endif // FAKEMACRODRIVER_H
