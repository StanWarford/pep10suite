#include "macroassemblerdriver.h"

#include "macroregistry.h"
#include "macrotokenizer.h"
#include "macroassembler.h"

MacroAssemblerDriver::MacroAssemblerDriver(const  MacroRegistry *registry) :  registry(registry),
    processor(new MacroPreprocessor(registry)), assembler(new MacroAssembler(registry))
{

}

MacroAssemblerDriver::~MacroAssemblerDriver()
{
    delete processor;
    //delete assembler;
    // We do not own the registry, so do not delete it.
    registry = nullptr;
}

void *MacroAssemblerDriver::assembleUserProgram(QString input)
{
    auto [rootPrototype, rootInstance] = graph.createRoot(input, ModuleType::USER_PROGRAM);

    if(!preprocess()) {
        // Cleanup any allocated memory
        return nullptr;
    }
    // Handle any preprocessor errors.
    if(!assembleProgram()) {
        // Cleanup any allocated memory
        return nullptr;
    }
    link(*rootInstance.get());
    annotate(*rootInstance.get());
    validate(*rootInstance.get());
    return nullptr;
}

bool MacroAssemblerDriver::preprocess()
{
    processor->setTarget(&graph);
    auto preprocessResult = processor->preprocess();
    bool retVal = false;
    if(!preprocessResult.succes) {
        qDebug().noquote() << std::get<0>(preprocessResult.error)
                           <<": "
                           << std::get<1>(preprocessResult.error);
        retVal = false;
    }
    else {
        qDebug() << "Preprocessing was successful.";
        retVal = true;
    }
    return retVal;
}

bool MacroAssemblerDriver::assembleProgram()
{
    auto assemblyResult = assembler->assemble(graph);
    bool retVal = false;
    if(!assemblyResult.success) {
        qDebug().noquote() << std::get<0>(assemblyResult.error)
                           <<": "
                           << std::get<1>(assemblyResult.error);
        retVal = false;
    }
    else {
        qDebug() << "Assembly was successful.";
        retVal = true;
    }
    return retVal;
}

void MacroAssemblerDriver::link(ModuleInstance &module)
{

}

void MacroAssemblerDriver::annotate(ModuleInstance &module)
{

}

void MacroAssemblerDriver::validate(ModuleInstance &module)
{

}
