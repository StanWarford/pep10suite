#include "macroassemblerdriver.h"

#include "macroregistry.h"
#include "macrotokenizer.h"
#include "macroassembler.h"
#include "macrolinker.h"

MacroAssemblerDriver::MacroAssemblerDriver(const  MacroRegistry *registry) :  registry(registry),
    processor(new MacroPreprocessor(registry)), assembler(new MacroAssembler(registry)), linker(new MacroLinker)
{

}

MacroAssemblerDriver::~MacroAssemblerDriver()
{
    delete processor;
    delete assembler;
    delete linker;
    // We do not own the registry, so do not delete it.
    registry = nullptr;
}

QSharedPointer<AsmProgram> MacroAssemblerDriver::assembleUserProgram(QString input,
                                                                     QSharedPointer<const SymbolTable> osSymbol)
{
    // Create a clean assembly graph to prevent accidental re-compiling of old code.
    this->graph = ModuleAssemblyGraph();
    auto [rootPrototype, startRootInstance] = graph.createRoot(input, ModuleType::USER_PROGRAM);

    if(!preprocess()) {
        // Cleanup any allocated memory
        return nullptr;
    }
    // Handle any preprocessor errors.
    if(!assembleProgram()) {
        // Cleanup any allocated memory
        return nullptr;
    }
    this->linker->setOSSymbolTable(osSymbol);
    if(!link()) {
        // Cleanup any allocated memory
        return nullptr;
    }
    auto rootInstance = graph.instanceMap[graph.rootModule].first();


    annotate(*rootInstance.get());
    validate(*rootInstance.get());

    return QSharedPointer<AsmProgram>::create(rootInstance->codeList, rootInstance->symbolTable,
                                              nullptr);
}

QSharedPointer<AsmProgram> MacroAssemblerDriver::assembleOperatingSystem(QString input)
{
    // Create a clean assembly graph to prevent accidental re-compiling of old code.
    this->graph = ModuleAssemblyGraph();
    auto [rootPrototype, rootInstance] = graph.createRoot(input, ModuleType::OPERATING_SYSTEM);

    if(!preprocess()) {
        // Cleanup any allocated memory
        return nullptr;
    }
    // Handle any preprocessor errors.
    if(!assembleProgram()) {
        // Cleanup any allocated memory
        return nullptr;
    }
    this->linker->clearOSSymbolTable();
    if(!link()) {
        // Cleanup any allocated memory
        return nullptr;
    }


    annotate(*rootInstance.get());
    validate(*rootInstance.get());
    auto os = QSharedPointer<AsmProgram>::create(rootInstance->codeList,
                                                 rootInstance->symbolTable,
                                                 nullptr,
                                                 rootInstance->burnInfo.startROMAddress,
                                                 rootInstance->burnInfo.burnArgument);
    return os;
}

bool MacroAssemblerDriver::preprocess()
{
    processor->setTarget(&graph);
    auto preprocessResult = processor->preprocess();
    bool retVal = false;
    if(!preprocessResult.succes) {
        qDebug().noquote() << "[PREERR]"
                           << std::get<0>(preprocessResult.error)
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
        qDebug().noquote() << "[ASMERR]"
                           << std::get<0>(assemblyResult.error)
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

void MacroAssemblerDriver::annotate(ModuleInstance &module)
{

}

void MacroAssemblerDriver::validate(ModuleInstance &module)
{

}
