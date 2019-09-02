#include "macroassemblerdriver.h"

#include "macroregistry.h"
#include "macrotokenizer.h"
#include "macroassembler.h"
#include "macrolinker.h"
#include "macrostackannotater.h"

MacroAssemblerDriver::MacroAssemblerDriver(QSharedPointer<MacroRegistry> registry) :  registry(registry),
    processor(new MacroPreprocessor(registry.get())), assembler(new MacroAssembler(registry.get())),
    linker(new MacroLinker), annotater(new MacroStackAnnotater)
{

}

MacroAssemblerDriver::~MacroAssemblerDriver()
{
    delete processor;
    delete assembler;
    delete linker;
    delete annotater;
    // We do not own the registry, so do not delete it.
    registry = nullptr;
}

ProgramOutput MacroAssemblerDriver::assembleUserProgram(QString input,
                                                                     QSharedPointer<const SymbolTable> osSymbol)
{
    ProgramOutput output;
    // Create a clean assembly graph to prevent accidental re-compiling of old code.
    this->graph = ModuleAssemblyGraph();
    auto [rootPrototype, startRootInstance] = graph.createRoot(input, ModuleType::USER_PROGRAM);

    if(!preprocess()) {
        output.success = false;
        // Report the error list of the root instance.
        output.errors = graph.errors;
        return output;
    }
    // Handle any preprocessor errors.
    if(!assembleProgram()) {
        output.success = false;
        output.errors = graph.errors;
        return output;
    }
    this->linker->setOSSymbolTable(osSymbol);
    if(!link()) {
        output.success = false;
        output.errors = graph.errors;
        return output;
    }
    auto rootInstance = graph.instanceMap[graph.rootModule].first();

    if(!annotate()) {
        output.success = false;
        output.errors = graph.errors;
        return output;
    }
    validate(*rootInstance.get());

    output.program = QSharedPointer<AsmProgram>::create(rootInstance->codeList,
                                                        rootInstance->symbolTable,
                                                        nullptr);
    output.success = true;
    return output;
}

ProgramOutput MacroAssemblerDriver::assembleOperatingSystem(QString input)
{
    ProgramOutput output;
    // Clear out old system calls to make way for new definitions.
    registry->clearSystemCalls();
    // Create a clean assembly graph to prevent accidental re-compiling of old code.
    this->graph = ModuleAssemblyGraph();
    auto [rootPrototype, rootInstance] = graph.createRoot(input, ModuleType::OPERATING_SYSTEM);

    if(!preprocess()) {
        output.success = false;
        // Report the error list of the root instance.
        output.errors = graph.errors;
        return output;
    }
    // Handle any preprocessor errors.
    if(!assembleProgram()) {
        output.success = false;
        output.errors = graph.errors;
        return output;
    }
    this->linker->clearOSSymbolTable();
    if(!link()) {
        output.success = false;
        output.errors = graph.errors;
        return output;
    }

    if(!annotate()) {
        output.success = false;
        output.errors = graph.errors;
        return output;
    }

    validate(*rootInstance.get());
    output.program = QSharedPointer<AsmProgram>::create(rootInstance->codeList,
                                                        rootInstance->symbolTable,
                                                        nullptr,
                                                        rootInstance->burnInfo.startROMAddress,
                                                        rootInstance->burnInfo.burnArgument);
    output.success = true;
    return output;
}

bool MacroAssemblerDriver::preprocess()
{
    processor->setTarget(&graph);
    auto preprocessResult = processor->preprocess();
    bool retVal = false;
    if(!preprocessResult.succes) {
        qDebug().noquote() << "[PREERR]"
                           << preprocessResult.error->getSourceLineNumber()
                           <<": "
                           << preprocessResult.error->getErrorMessage();
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
        qDebug().noquote() << "[ASMERR]"
                           << assemblyResult.error->getSourceLineNumber()
                           <<": "
                           << assemblyResult.error->getErrorMessage();
        retVal = false;
    }
    else {
        qDebug() << "Assembly was successful.";
        retVal = true;
    }
    return retVal;
}

bool MacroAssemblerDriver::link()
{
    auto linkerResult = linker->link(graph);
    bool retVal = false;
    if(!linkerResult.success) {
        for(auto error : linkerResult.errorList) {
            qDebug().noquote() << "[LNKERR]"
                               << std::get<0>(error)
                               <<": "
                               << std::get<1>(error);
            retVal = false;
        }

    }
    else {
        qDebug() << "Linking was successful.";
        retVal = true;
    }
    return retVal;
}

bool MacroAssemblerDriver::annotate()
{
    auto annotateResult = annotater->annotateStack(graph);
    bool retVal = false;
    if(annotateResult.success != AnnotationSucces::SUCCESS) {
        for(auto error : annotateResult.warningList) {
            qDebug().noquote() << "[STACKWARN]"
                               << std::get<0>(error)
                               <<": "
                               << std::get<1>(error);
            retVal = false;
        }
        for(auto error : annotateResult.errorList) {
            qDebug().noquote() << "[STACKERR]"
                               << std::get<0>(error)
                               <<": "
                               << std::get<1>(error);
            retVal = false;
        }

    }
    else {
        qDebug() << "Stack annotation was successful.";
        retVal = true;
    }
    return retVal;
}

void MacroAssemblerDriver::validate(ModuleInstance &module)
{

}
