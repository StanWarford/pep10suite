#include "macroisaassembler.h"

#include "macroregistry.h"
#include "macrotokenizer.h"

MacroISAAssembler::MacroISAAssembler(const  MacroRegistry *registry) :  registry(registry), processor(new MacroPreprocessor(registry))
{

}

MacroISAAssembler::~MacroISAAssembler()
{
    delete processor;
    // We do not own the registry, so do not delete it.
    registry = nullptr;
}

void *MacroISAAssembler::assembleUserProgram(QString input)
{


    auto [rootPrototype, rootInstance] = graph.createRoot(input, ModuleType::USER_PROGRAM);


    auto preprocessResult = preprocess();
    if(!preprocessResult.succes) {
        qDebug().noquote() << std::get<0>(preprocessResult.error)
                           <<": "
                           << std::get<1>(preprocessResult.error);
    }
    // Handle any preprocessor errors.
    assembleProgram(*rootInstance.get());
    link(*rootInstance.get());
    annotate(*rootInstance.get());
    validate(*rootInstance.get());
    return nullptr;
}

PreprocessorResult MacroISAAssembler::preprocess()
{
        processor->setTarget(&graph);
        return processor->preprocess();
}

void MacroISAAssembler::assembleProgram(ModuleInstance &module)
{

}

void MacroISAAssembler::link(ModuleInstance &module)
{

}

void MacroISAAssembler::annotate(ModuleInstance &module)
{

}

void MacroISAAssembler::validate(ModuleInstance &module)
{

}
