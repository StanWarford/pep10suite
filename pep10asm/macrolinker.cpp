#include "macrolinker.h"
#include "ngraph_prune.h"
#include "symbolentry.h"
#include "symbolvalue.h"
#include "optional_helper.h"
#include "asmargument.h"

MacroLinker::MacroLinker(): nextAddress(0), forceBurn0xFFFF(false)
{

}

MacroLinker::~MacroLinker()
{

}

LinkResult MacroLinker::link(ModuleAssemblyGraph &graph)
{
    // Reset address counter, as a linker may be reused
    overflowedMemory = false;
    nextAddress = 0;
    nextSourceLine = 0;
    LinkResult result;
    result.success = true;

    auto rootModuleInstance = graph.getRootInstance();
    // Pull in any symbols declared in operating system if we are not assembling an operating system.
    if(rootModuleInstance->prototype->moduleType != ModuleType::OPERATING_SYSTEM) {
        auto exportsResults = pullInExports(graph);
        if(!exportsResults.success) {
            return exportsResults;
        }
    }

    // Begin depth-first linking starting from the root.
    auto linkResult = linkModule(graph, *rootModuleInstance);
    if(!linkResult.success) {
        return linkResult;
    }

    if(overflowedMemory) {
        result.errorList.append({0, exceededMemory});
        result.success = false;
    }

    // Operating system needs to have its code list adjusted to be burned in to high memory.
    if(rootModuleInstance->prototype->moduleType == ModuleType::OPERATING_SYSTEM) {
        result.success = shiftForBURN(graph);
        for(auto errorList : graph.errors.sourceMapped) {
            for(auto error : errorList) {
               result.errorList.append({error->getSourceLineNumber(),
                                        error->getErrorMessage()});
            }
        }
    }
    // If a user program has a burn, return an error.
    else if(rootModuleInstance->burnInfo.burnCount != 0) {
        result.errorList.append({0, noBURN});
        result.success = false;
    }
    return result;
}

void MacroLinker::setOSSymbolTable(QSharedPointer<const SymbolTable> OSSymbolTable)
{
    osSymbolTable = OSSymbolTable;
}

void MacroLinker::setForceBurnAt0xFFFF(bool forceBurn0xFFFF)
{
    this->forceBurn0xFFFF = forceBurn0xFFFF;
}

void MacroLinker::clearOSSymbolTable()
{
    osSymbolTable.reset();
}

LinkResult MacroLinker::pullInExports(ModuleAssemblyGraph &graph)
{

    if(!osSymbolTable.has_value()) {
        return {false, {{0, noOperatingSystem}} };
    }
    auto symList = optional_helper(osSymbolTable)->getExternalSymbols();
    auto rootInstance = graph.getRootInstance();
    for(auto symbol : symList) {
        if(rootInstance->symbolTable->exists(symbol->getName())) {
            rootInstance->symbolTable->define(symbol->getName());
            auto value = QSharedPointer<SymbolValueExternal>::create(symbol);
            rootInstance->symbolTable->setValue(symbol->getName(), value);
        }
    }
    return {true, {}};
}

LinkResult MacroLinker::linkModule(ModuleAssemblyGraph graph,
                                   ModuleInstance& instance)
{
    LinkResult retVal;
    // Unless an error occurs, assume process is successful.
    retVal.success = true;

    // Check each line of code in instance & assign memory addresses.
    for(int lineNum = 0 ; lineNum < instance.codeList.size(); lineNum++) {
        auto line = instance.codeList[lineNum];
        // Assign the line numbers to code lines.
        line->setSourceLineNumber(lineNum);
        line->setListingLineNumber(nextSourceLine++);
        // Now that we are assigning addresses, we can properly deduce the number
        // of padding bytes that need to be generated.
        if(auto asAlign = dynamic_cast<DotAlign*>(line.get()); asAlign != nullptr) {
            int alignment = asAlign->getArgument()->getArgumentValue();
            int padding =  (alignment - nextAddress % alignment) % alignment;
            asAlign->setNumBytesGenerated(padding);
        }
        // Must detect object code "address" of .BURN to prevent code generation above
        // the .BURN statement.
        else if(auto asBurn = dynamic_cast<DotBurn*>(line.get()); asBurn != nullptr) {
            instance.burnInfo.burnAddress = nextAddress;

        }
        // Check for symbol declarations.
        if(line->hasSymbolEntry()) {
            auto symbolPtr = line->getSymbolEntry();
            // Check if line has a multiply defined symbol declaration.
            if(symbolPtr->isMultiplyDefined()) {
                if(auto asExtern = dynamic_cast<SymbolValueExternal*>(symbolPtr->getRawValue().get());
                        asExtern != nullptr){
                    QString symbolName = line->getSymbolEntry()->getName();
                    retVal.success = false;
                    retVal.errorList.append({lineNum, redefineExportSymbol.arg(symbolName)});
                    continue;
                }
                else {
                    QString symbolName = line->getSymbolEntry()->getName();
                    retVal.success = false;
                    retVal.errorList.append({lineNum, multidefinedSymbol.arg(symbolName)});
                    continue;
                }

            }
            else if(dynamic_cast<DotEquate*>(line.get()) != nullptr)
            {
                // The value of a .EQUATE is handled in the assembler,
                // as it is not tied the address of a the current line of code.
            }
            // Otherwise the symbol was defined once, and its value needs to
            // be set to the current address.
            else if (symbolPtr->isDefined()) {
                auto valuePtr = QSharedPointer<SymbolValueLocation>::create(nextAddress);
                instance.symbolTable->setValue(symbolPtr->getSymbolID(), valuePtr);
            }

        }

        // Check if line has a symbolic operand that is undefined.
        if(line->hasSymbolicOperand()  && line->getSymbolicOperand()->isUndefined()) {
            QString symbolName = line->getSymbolicOperand()->getName();
            retVal.success = false;
            retVal.errorList.append({lineNum, undefinedSymbol.arg(symbolName)});
            continue;
        }

        // Handle macro invocations in a depth-first manner.
        if(dynamic_cast<MacroInvoke*>(line.get()) != nullptr) {
            // While a macro line does not have a logical address,
            // storing the current address may help diagnose problems
            // in future steps of the macro assembler.
            line->setMemoryAddress(nextAddress);
            // We don't increment the address, this will be done by children of module.
            auto macroLine = static_cast<MacroInvoke*>(line.get());
            // Copy and swap the moduleInstance. Now we can adjust the code list for the
            // child module without affecting every instance of the macro in the application.
            auto child = *macroLine->getMacroInstance();
            auto childRetVal = linkModule(graph, child);
            if(childRetVal.success == false) {
                retVal.success = false;
                // Only take the first linking error from child, otherwise a line might
                // have multiple error messages written to it.
                retVal.errorList.append({lineNum, std::get<1>(childRetVal.errorList[0])});
                continue;
            }
        }
        // Otherwise, we don't have a macro invocation, and we can assign addresses normally.
        else {
            line->setMemoryAddress(nextAddress);
            if(0xFFFF - line->objectCodeLength() < nextAddress) {
                overflowedMemory = true;
            }
            nextAddress += line->objectCodeLength();
        }
    }
    //qDebug().noquote() << "Linked: "<< instance.prototype->name << instance.macroArgs;
    // Whether successfully or not, the module has finished the linking process.
    instance.alreadyLinked = true;
    return retVal;
}

void MacroLinker::relocateCode(ModuleInstance &instance, quint16 addressDelta)
{
    for (int i = 0; i < instance.codeList.length(); i++) {
        instance.codeList[i]->adjustMemAddress(addressDelta);
    }
}

bool MacroLinker::shiftForBURN(ModuleAssemblyGraph& graph)
{
    /*
     * I doubt .ALIGN after a burn work as anticipated.
     * In order to properly align, one would need to begin with the last line of code,
     * and assign address back-to-front. This wouldn't be a massive change,
     * but it would completely divorce the OS linker implementation from
     * the user linker.
     */

    auto rootInstance = graph.getRootInstance();
    bool success = true;
    QString errorString;
    if (rootInstance->burnInfo.burnCount != 1) {
        success = false;
        auto error = QSharedPointer<BackEndError>::create(rootInstance->instanceIndex, Severity::ERROR,
                                                          oneBURN, rootInstance->codeList.first());
        graph.addError(error);
    }
    else if(forceBurn0xFFFF && rootInstance->burnInfo.burnArgument != 0xFFFF) {
        success = false;
        #pragma message("TODO: Insert error on correct line")
        auto error = QSharedPointer<BackEndError>::create(rootInstance->instanceIndex, Severity::ERROR,
                                                          BURNat0xFFFF, rootInstance->codeList.first());
        graph.addError(error);
    }
    if(!success) return false;
    auto &codeList = rootInstance->codeList;
    auto &burnInfo = rootInstance->burnInfo;
    for(int it = 0; it < codeList.size(); it++) {
        if(codeList[it]->getMemoryAddress() < burnInfo.burnAddress) {
            codeList[it]->setEmitObjectCode(false);
        }
    }

    // Adjust for .BURN.
    // The number of  bytes that need to be added to each instruction is the value of the BURN (0xFFFF)
    // minus the number of bytes in the program (nextAddress + 1)
    quint16 addressDelta = static_cast<quint16>(burnInfo.burnArgument - nextAddress + 1);
    // The first byte of ROM is the object code address of the BURN shifted by addressDelta.
    burnInfo.startROMAddress = addressDelta + burnInfo.burnAddress;
    relocateCode(*rootInstance.get(), addressDelta);
    rootInstance->symbolTable->setOffset(addressDelta);

    /*
     * Adjust for .ALIGNs before .BURN
     *
     * For any .ALIGNs before a .BURN, they should align in the opposite direction.
     * That is, instead of spanning from an arbitray starting address to the byte
     * before the desired alignment, they should end at the address of the next
     * instruction, and continue upwards in memory until the first byte of the
     * .ALIGN is on the proper byte boundary.
     *
     * This feature is needed so that arbitrary changes may be made to the
     * operating system without the user having to manually align the IO ports.
     *
     */
    // Find the .BURN directive
    int indexOfBurn = 0;
    for(int it = 0; it < codeList.size(); it++) {
        if(dynamic_cast<DotBurn*>(codeList[it].get()) != nullptr) {
            indexOfBurn = it;
            break;
        }
    }

    // Total number of bytes that each instruction needs to be shifted.
    int rollingOffset = 0;

    // Begin aligning every instruction from the .BURN upwards.
    for(int it = indexOfBurn; it > 0; it--) {
        // Every line of assembly code before a .BURN must be moved upward by the size of all preceding .ALIGN
        // directives, starting from the .BURN and moving upward.
        codeList[it]->adjustMemAddress(rollingOffset);
        // Symbols that represent memory locations must also be adjusted.
        if(codeList[it]->hasSymbolEntry()) {
            auto sym = rootInstance->symbolTable->getValue(codeList[it]->getSymbolEntry()->getSymbolID());
            // Don't allow symbols with an .EQUATE, .ADDRSS to be re-adjusted.
            if(sym->getRawValue()->canRelocate()) {
                sym->setValue(QSharedPointer<SymbolValueLocation>::create(codeList[it]->getMemoryAddress()));
            }
        }

        // If the instruction is a .ALIGN, then we must re-calculate the rolling offset.
        if(dynamic_cast<DotAlign*>(codeList[it].get()) != nullptr) {
            // The instruction is known to be an ALIGN directive, so just cast it.
            DotAlign* asAlign = static_cast<DotAlign*>(codeList[it].get());
            // The address of the .ALIGN.
            int startAddr = asAlign->getMemoryAddress();
            // The address of the last byte of the .ALIGN.
            int endAddr = startAddr + asAlign->getNumBytesGenerated();
            // Based on the ending byte, calculate where the first byte needs to be for proper alignment.
            int blockStart = endAddr - (endAddr % asAlign->getArgument()->getArgumentValue());
            // We can't change an AsmArgument in place, so we must construct a new one.
            //delete asAlign->numBytesGenerated;
            // The align must still reach down to endAddr, but now must span up to blockStart.
            asAlign->setNumBytesGenerated(endAddr - blockStart);
            // Other instructions will be shifter by the change in starting address.
            rollingOffset += startAddr - blockStart ;
            asAlign->setMemoryAddress(blockStart);
        }
    }
    return true;
}
