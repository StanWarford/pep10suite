#include "macrostackannotater.h"

MacroStackAnnotater::MacroStackAnnotater()
{

}

MacroStackAnnotater::~MacroStackAnnotater()
{

}

StackAnnotationResult MacroStackAnnotater::annotateStack(ModuleAssemblyGraph &graph)
{
    auto rootInstance = graph.instanceMap[graph.rootModule].first();
    discoverLines(*rootInstance);
    return {};
}

void *MacroStackAnnotater::containsHint(...) const
{
    return nullptr;
}

void *MacroStackAnnotater::containsTypeTag(...) const
{
    return nullptr;
}

void *MacroStackAnnotater::containsSymbolTag(...) const
{
    return nullptr;
}

void MacroStackAnnotater::discoverLines(ModuleInstance &instance)
{
    for(auto line : instance.codeList) {
        StackModifyingLine modifying;
        // Recursively discover submodules.
        if(MacroInvoke* asMacro = dynamic_cast<MacroInvoke*>(line.get());
           asMacro != nullptr) {
            discoverLines(*asMacro->getMacroInstance());
        }

        // .BLOCK may be a(n) 1) integral type, 2) array of integral types, or 3) a struct, or 4) nothing.
        // Tags are always optional.
        else if(DotBlock* asBlock = dynamic_cast<DotBlock*>(line.get());
                asBlock != nullptr) {
            if(containsTypeTag(asBlock) || containsSymbolTag(asBlock)) {
                modifying.line = line;
                // Must calculate root index.
                modifying.rootLineIdx = -1;
                unresolvedModifyingLines.append(modifying);
            }
        }
        // .BYTE may be a(n) 1) integral type.
        // Tags are always optional.
        else if(DotByte* asByte = dynamic_cast<DotByte*>(line.get());
                asByte != nullptr) {
            if(containsTypeTag(asByte) || containsSymbolTag(asByte)) {
                modifying.line = line;
                // Must calculate root index.
                modifying.rootLineIdx = -1;
                unresolvedModifyingLines.append(modifying);
            }
        }
        // .EQUATE may be a(n) 1) integral type.
        // Tags are always optional.
        else if(DotEquate* asEquate = dynamic_cast<DotEquate*>(line.get());
                asEquate != nullptr) {
            if(containsTypeTag(asEquate) || containsSymbolTag(asEquate)) {
                modifying.line = line;
                // Must calculate root index.
                modifying.rootLineIdx = -1;
                unresolvedModifyingLines.append(modifying);
            }
        }
        // .WORD may be a(n) 1) integral type, 2) array of 2 characters.
        // Tags are always optional.
        else if(DotWord* asWord = dynamic_cast<DotWord*>(line.get());
                asWord != nullptr) {
            if(containsTypeTag(asWord) || containsSymbolTag(asWord)) {
                modifying.line = line;
                // Must calculate root index.
                modifying.rootLineIdx = -1;
                unresolvedModifyingLines.append(modifying);
            }
        }
        // Unary instructions do not require tags.
        else if(UnaryInstruction* asUnary = dynamic_cast<UnaryInstruction*>(line.get());
                asUnary != nullptr) {
            if(asUnary->getMnemonic() == Enu::EMnemonic::RET ||
               asUnary->getMnemonic() == Enu::EMnemonic::SRET ||
               asUnary->getMnemonic() == Enu::EMnemonic::USCALL ||
               asUnary->getMnemonic() == Enu::EMnemonic::MOVASP) {
                // RET modifies stack via popping RETADDR.
                // SRET, USCALL modify stack via context switch.
                // MOVASP modifies stack setting SP to an arbitrary value.
                // A new "stack frame" should be created anytime MOVASP is called.
                modifying.line = line;
                // Must calculate root index.
                modifying.rootLineIdx = -1;
                unresolvedModifyingLines.append(modifying);
            }
        }
        // Non-unary instructions may require tags.
        // If any ADDSP and SUBSP has a tag, then all must have tags.
        // CALL may have a tag if it is calling MALLOC.
        else if(NonUnaryInstruction* asNonunary = dynamic_cast<NonUnaryInstruction*>(line.get());
                asNonunary != nullptr) {

            if(asNonunary->getMnemonic() == Enu::EMnemonic::ADDSP ||
               asNonunary->getMnemonic() == Enu::EMnemonic::SUBSP ||
               asNonunary->getMnemonic() == Enu::EMnemonic::CALL ||
               asNonunary->getMnemonic() == Enu::EMnemonic::SCALL) {
                // ADDSP, SUBSP modify stack pointer via arithmatic.
                // CALL modifies stack via pushing RETADDR.
                // SCALL modifies stack via context switch.
                modifying.line = line;
                // Must calculate root index.
                modifying.rootLineIdx = -1;
                unresolvedModifyingLines.append(modifying);
            }
        }
    }
}

void MacroStackAnnotater::resolveLines()
{
    quint16 oldSize = 0;
    quint16 newSize = unresolvedModifyingLines.size();
    do {
        // If there are no elements, give up.
        if(newSize == 0) {
            break;
        }
        // Attempt to parse a given line.
        for(int it=0; it< unresolvedModifyingLines.size(); it++) {
            bool success = parseLine(unresolvedModifyingLines[it]);
            // If the line parsed successfully, move the line from
            // in-progress list to finished list.
            if(success) {
                resolvedModifyingLines.append(unresolvedModifyingLines.takeAt(it));
                it--;
            }
        }
        oldSize = newSize;
        newSize = unresolvedModifyingLines.size();
    } while((oldSize != newSize));

}

bool MacroStackAnnotater::parseLine(StackModifyingLine line)
{
    // .BLOCK may be a(n) 1) integral type, 2) array of integral types, or 3) a struct, or 4) nothing.
    // Tags are always optional.
    if(DotBlock* asBlock = dynamic_cast<DotBlock*>(line.line.get());
            asBlock != nullptr) {
        return parseBlock(asBlock);
    }
    // .BYTE may be a(n) 1) integral type.
    // Tags are always optional.
    else if(DotByte* asByte = dynamic_cast<DotByte*>(line.line.get());
            asByte != nullptr) {
        return parseByte(asByte);
    }
    // .EQUATE may be a(n) 1) integral type.
    // Tags are always optional.
    else if(DotEquate* asEquate = dynamic_cast<DotEquate*>(line.line.get());
            asEquate != nullptr) {
        return parseEquate(asEquate);
    }
    // .WORD may be a(n) 1) integral type, 2) array of 2 characters.
    // Tags are always optional.
    else if(DotWord* asWord = dynamic_cast<DotWord*>(line.line.get());
            asWord != nullptr) {
        return parseWord(asWord);
    }
    // Unary instructions do not require tags.
    else if(UnaryInstruction* asUnary = dynamic_cast<UnaryInstruction*>(line.line.get());
            asUnary != nullptr) {
        return parseUnary(asUnary);
    }
    // Non-unary instructions may require tags.
    // If any ADDSP and SUBSP has a tag, then all must have tags.
    // CALL may have a tag if it is calling MALLOC.
    else if(NonUnaryInstruction* asNonunary = dynamic_cast<NonUnaryInstruction*>(line.line.get());
            asNonunary != nullptr) {
        return parseNonunary(asNonunary);
    }
    return true;
}

bool MacroStackAnnotater::parseBlock(void *)
{
    return true;
}

bool MacroStackAnnotater::parseByte(void *)
{
    return true;
}

bool MacroStackAnnotater::parseEquate(void *)
{
    return true;
}

bool MacroStackAnnotater::parseWord(void *)
{
    return true;
}

bool MacroStackAnnotater::parseUnary(void *)
{
    return true;
}

bool MacroStackAnnotater::parseNonunary(void *)
{
    return true;
}

