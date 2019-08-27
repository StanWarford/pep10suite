#include "macrostackannotater.h"
#include "optional_helper.h"
#include "symbolentry.h"


static const QString noHint = ";ERROR: A .%1 may not contain a stack hint.";
static const QString noSymbol = ";WARNING: An .%1 must define a symbol to have format tags.";
static const QString neSymbol = ";WARNING: Looked up a symbol that does not exist: %1";
static const QString noEquate = ";WARNING: Looked for existing symbol not defined in .EQUATE: %1";
static const QString badHint = ";ERROR: Only ADDSP, SUBSP may define a stack hint";
static const QString badStruct = ";Warning: invalid struct definition.";
static const QString bytesAllocMismatch = ";WARNING: Number of bytes allocated (%1) not equal to number of bytes listed in trace tag (%2).";
static const QString noSymbolTag = ";WARNING: .%1 may not specify a symbol trace tag.";
static const QString noArrayTag = ";WARNING: .%1 may not specify an array trace tag.";
MacroStackAnnotater::MacroStackAnnotater()
{

}

MacroStackAnnotater::~MacroStackAnnotater()
{

}

StackAnnotationResult MacroStackAnnotater::annotateStack(ModuleAssemblyGraph &graph)
{
    // Clean any state from previous runs.
    globalLines.clear();
    codeLines.clear();
    equateLines.clear();
    dynamicSymbols.clear();
    staticSymbols.clear();
    resultCache = {};

    // Begin extracting symbol data recursively from root module.
    auto rootInstance = graph.instanceMap[graph.rootModule].first();
    symbolTable = rootInstance->symbolTable;
    discoverLines(*rootInstance);
    parseEquateLines();
    resolveGlobals();
    resolveCodeLines();
    return {};
}

bool MacroStackAnnotater::containsStackHint(QString comment)
{
    return localsHint.match(comment).hasMatch() ||
           paramsHint.match(comment).hasMatch();
}

bool MacroStackAnnotater::containsFormatTag(QString comment)
{
    if(comment.isEmpty()) {
        return false;
    }
    return containsArrayType(comment) || containsPrimitiveType(comment);
}

bool MacroStackAnnotater::containsSymbolTag(QString comment)
{
    return symbolTag.match(comment).hasMatch();
}

bool MacroStackAnnotater::containsArrayType(QString comment)
{
    return arrayTag.match(comment).hasMatch();
}

bool MacroStackAnnotater::containsPrimitiveType(QString comment)
{
    // A primitive type matches any integral type, but not arrays.
    return formatTag.match(comment).hasMatch();
}

QString MacroStackAnnotater::extractTypeTags(QString comment)
{
    // Even though array tags are less common, check array tags first.
    // Normal (primitive) format tags look almost identical to array tags,
    // so checking arrays first prevents for a false positive on the primitive tag.
    if(auto arrayMatch = arrayTag.match(comment);
            arrayMatch.hasMatch()) {
        return arrayMatch.captured();
    }
    else if(auto formatMatch = formatTag.match(comment);
            formatMatch.hasMatch()) {
               return formatMatch.captured();
           }
    return "";
}

Enu::ESymbolFormat MacroStackAnnotater::primitiveType(QString formatTag)
{
    if (formatTag.startsWith("#1c", Qt::CaseInsensitive)) return Enu::ESymbolFormat::F_1C;
    if (formatTag.startsWith("#1d", Qt::CaseInsensitive)) return Enu::ESymbolFormat::F_1D;
    if (formatTag.startsWith("#2d", Qt::CaseInsensitive)) return Enu::ESymbolFormat::F_2D;
    if (formatTag.startsWith("#1h", Qt::CaseInsensitive)) return Enu::ESymbolFormat::F_1H;
    if (formatTag.startsWith("#2h", Qt::CaseInsensitive)) return Enu::ESymbolFormat::F_2H;
    return Enu::ESymbolFormat::F_NONE;
}

QPair<Enu::ESymbolFormat, quint8> MacroStackAnnotater::arrayType(QString formatTag)
{
    auto type = primitiveType(formatTag);
    auto match = arrayMultiplier.match(formatTag);
    QString text = match.captured(0);
    text.chop(1); // Drop the a at the end.
    int size = text.toInt();
    return {type, static_cast<quint8>(size)};
}

QStringList MacroStackAnnotater::extractTagList(QString comment)
{
    auto items = symbolTag.globalMatch(comment);
    QStringList out;
    while(items.hasNext()) {
        QString match = items.next().captured(0);
        match = match.mid(1);
        out.append(match);
    }
    return out;
}

void MacroStackAnnotater::discoverLines(ModuleInstance &instance)
{
    for(auto line : instance.codeList) {

        // Recursively discover submodules.
        if(MacroInvoke* asMacro = dynamic_cast<MacroInvoke*>(line.get());
           asMacro != nullptr) {
            discoverLines(*asMacro->getMacroInstance());
        }

        // .BLOCK may be a(n) 1) integral type, 2) array of integral types, or 3) a struct, or 4) nothing.
        // Tags are always optional.
        else if(DotBlock* asBlock = dynamic_cast<DotBlock*>(line.get());
                asBlock != nullptr) {
            if(containsFormatTag(asBlock->getComment()) ||
               containsSymbolTag(asBlock->getComment()) ||
               containsStackHint(asBlock->getComment())) {
                // Trace tags on any line make them required throughout the rest of the program.
                hadAnyTraceTags = true;
                globalLines.append(asBlock);
            }
        }
        // .BYTE may be a(n) 1) integral type.
        // Tags are always optional.
        else if(DotByte* asByte = dynamic_cast<DotByte*>(line.get());
                asByte != nullptr) {
            if(containsFormatTag(asByte->getComment()) ||
               containsSymbolTag(asByte->getComment()) ||
               containsStackHint(asByte->getComment())) {
                // Trace tags on any line make them required throughout the rest of the program.
                hadAnyTraceTags = true;
                globalLines.append(asByte);
            }
        }
        // .EQUATE may be a(n) 1) integral type 2) array of integral types, 3) a struct, or 4) nothing.
        // Tags are always optional.
        else if(DotEquate* asEquate = dynamic_cast<DotEquate*>(line.get());
                asEquate != nullptr) {
            if(containsFormatTag(asEquate->getComment()) ||
               containsSymbolTag(asEquate->getComment()) ||
               containsStackHint(asEquate->getComment())) {
                hadAnyTraceTags = true;
                equateLines.append(asEquate);
            }
        }
        // .WORD may be a(n) 1) integral type, 2) array of 2 characters.
        // Tags are always optional.
        else if(DotWord* asWord = dynamic_cast<DotWord*>(line.get());
                asWord != nullptr) {
            if(containsFormatTag(asWord->getComment()) ||
               containsSymbolTag(asWord->getComment()) ||
               containsStackHint(asWord->getComment())) {
                // Trace tags on any line make them required throughout the rest of the program.
                hadAnyTraceTags = true;
                globalLines.append(asWord);
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
                codeLines.append(asUnary);
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
                codeLines.append(asNonunary);
            }
        }
    }
}

void MacroStackAnnotater::resolveGlobals()
{
    QString instrText;
    // No need to set hadTraceTag flag, as this was handled in discoverLines(...).
    for(auto line : this->globalLines) {
        // .BLOCK may be a(n) 1) integral type, 2) array of integral types, or 3) a struct, or 4) nothing.
        // Tags are always optional.
        // Since the structs declared by .BLOCK cannot be used as part of other stucts
        // the recursive checks are no longer needed.
        if(DotBlock* asBlock = dynamic_cast<DotBlock*>(line);
                asBlock != nullptr) {
            instrText = "BLOCK";
        }
        // .BYTE may be a(n) 1) integral type or 2) nothing.
        // It is also allowed to be a struct of size 1 or of type #1c1a, but
        // these types are rather useless.
        // Tags are always optional.
        else if(DotByte* asByte = dynamic_cast<DotByte*>(line);
                asByte != nullptr) {
            instrText = "BYTE";
        }
        // .WORD may be a(n) 1) integral type, 2) array of two 1 byte integrals or 3) nothing.
        // Tags are always optional.
        else if(DotWord* asWord = dynamic_cast<DotWord*>(line);
                asWord != nullptr) {
            instrText = "WORD";
        }

        QString comment = line->getComment();
        int bytesAllocated = line->objectCodeLength();
        // If line has a @params @locals hint, it is malformed.
        if(containsStackHint(comment)) {
            resultCache.errorList.append({line->getListingLineNumber(), badHint});
        }
        // Lines with a trace tag but no symbol definition are malformed.
        else if((containsFormatTag(comment) || containsSymbolTag(comment)) &&
                !line->hasSymbolEntry()) {
            resultCache.warningList.append({line->getListingLineNumber(), noSymbol.arg(instrText)});
        }
        // Primitive integral size must match number of bytes allocated.
        else if(containsPrimitiveType(comment)) {
            QSharedPointer<SymbolEntry> symbol = line->getSymbolEntry();
            Enu::ESymbolFormat type = primitiveType(extractTypeTags(comment));
            int tagWidth = Enu::tagNumBytes(type);
            // Verify that the size of the trace tag matches the size number of bytes allocated by the BLOCK.
            if(tagWidth != bytesAllocated) {
                resultCache.warningList.append({line->getListingLineNumber(),
                                                bytesAllocMismatch.arg(bytesAllocated).arg(tagWidth)});
            }
            else {
                // Create format trace tag for the line.
                auto formatTag = QSharedPointer<PrimitiveType>::create(line->getSymbolEntry(), type);
                staticSymbols.insert(symbol->getName(), {symbol, formatTag});

                // Create frame entry in global memory trace.
                pushGlobalHelper(line, {formatTag});
            }
        }
        // Number of bytes allocated must exactly match the product of the
        // array lenght times the size of the primitive type.
        else if(containsArrayType(comment)) {
            QSharedPointer<SymbolEntry> symbol = line->getSymbolEntry();
            auto [type, count] = arrayType(extractTypeTags(comment));
            int tagWidth = count * Enu::tagNumBytes(type);
            // Verify that the size of the trace tag matches the size number of bytes allocated.
            if(tagWidth != bytesAllocated) {
                resultCache.warningList.append({line->getListingLineNumber(),
                                                bytesAllocMismatch.arg(bytesAllocated).arg(tagWidth)});
            }
            else {
                // Create format trace tag for the line.
                auto formatTag = QSharedPointer<ArrayType>::create(line->getSymbolEntry(), type, count);
                staticSymbols.insert(symbol->getName(), {symbol, formatTag});

                // Create frame entry in global memory trace.
                pushGlobalHelper(line, {formatTag});
            }
        }
        // BLOCK may also be a struct of size N.
        else if(containsSymbolTag(comment)) {
            QSharedPointer<SymbolEntry> symbol = line->getSymbolEntry();
            // Create symbol trace tag (sturct) for the line.
            auto symbolTraceList = this->extractTagList(comment);
            auto [structPtr, errMessage] = parseStruct(symbol->getName(), symbolTraceList);

            // Validate parsing of struct, if there is an errory, we must stop.
            if(!errMessage.isEmpty() || structPtr.isNull()) {
                resultCache.warningList.append({line->getListingLineNumber(), errMessage});
            }
            // If sizeof(struct) exceeds the number of bytes allocated, there is an allocation error.
            else if(structPtr->size() != bytesAllocated) {
                resultCache.warningList.append({line->getListingLineNumber(),
                                                bytesAllocMismatch.arg(bytesAllocated).arg(structPtr->size())});
            }
            // Otherwise, struct was succesfully allocated, and we may push it onto the globals stack.
            else {
                staticSymbols.insert(symbol->getName(), {symbol, structPtr});

                pushGlobalHelper(line, {structPtr});
            }

        }
    }
}

void MacroStackAnnotater::resolveCodeLines()
{
    for(auto line : this->codeLines) {
        // Unary instructions do not require tags.
        if(UnaryInstruction* asUnary = dynamic_cast<UnaryInstruction*>(line);
                asUnary != nullptr) {
            // TODO
        }
        // Non-unary instructions may require tags.
        // If any ADDSP and SUBSP has a tag, then all must have tags.
        // CALL may have a tag if it is calling MALLOC.
        else if(NonUnaryInstruction* asNonunary = dynamic_cast<NonUnaryInstruction*>(line);
                asNonunary != nullptr) {
            // TODO
        }
    }
}

void MacroStackAnnotater::parseEquateLines()
{
    // EQUATE's that participate in struct lookups.
    QList<DotEquate*> structLines;
    // For each EQUATE line, parse any primitive, array of primitive, or empty types.
    for(auto dotEquate : equateLines) {
        // Trace tags are optional for EQUATE, so absent comment means succesful parse.
        if(dotEquate->getComment().isEmpty()) {
            continue;
        }
        // EQUATE must not define a stack direction hint.
        else if(containsStackHint(dotEquate->getComment())) {
            this->resultCache.warningList.append({dotEquate->getListingLineNumber(), noHint});
        }
        // EQUATE may define a symbol tag.
        else if(containsSymbolTag(dotEquate->getComment())) {
            // Flag this equate as needing extra parsing work.
            structLines.append(dotEquate);
        }
        // EQUATE may contain a format tag.
        else if(containsFormatTag(dotEquate->getComment())) {
            // If a format tag is present, the line must declare a symbol.
            if(!dotEquate->hasSymbolEntry()) {
                this->resultCache.warningList.append({dotEquate->getListingLineNumber(),
                                                      noSymbol.arg("EQUATE")});
            }
            auto symbol = dotEquate->getSymbolEntry();
            QSharedPointer<AType> formatTag;

            // If there is an array, create a type tag representing it.
            if(containsArrayType(dotEquate->getComment())) {
                hadAnyTraceTags = true;
                auto [type, size] = arrayType(extractTypeTags(dotEquate->getComment()));
                formatTag = QSharedPointer<ArrayType>::create(dotEquate->getSymbolEntry(), type, size);
            }
            //Otherwise if there is an integral primitve, create a type tag.
            else if(containsPrimitiveType(dotEquate->getComment())) {
                hadAnyTraceTags = true;
                Enu::ESymbolFormat type = primitiveType(extractTypeTags(dotEquate->getComment()));
                formatTag = QSharedPointer<PrimitiveType>::create(dotEquate->getSymbolEntry(), type);
            }
            // Should be impossible, but compiler warns on unitialized formatTag.
            else {
                assert(false && "Contradictory format tags on EQUATE.");
            }
            dynamicSymbols.insert(symbol->getName(), {symbol, formatTag});
        }
        // Otherwise there is a non-empty comment that does not declare any debugging information.
    }

    /*
     *  Handle struct declarations.
     */
    quint16 oldSize = 0, newSize = structLines.size();
    // Iterate over the list of structs, attempting to parse their definitions
    // until the list is empty or until errors prevent further parsing.
    // Since structs can recurse(eg struct A can contain a struct B),
    // more complicated handling is needed.
    do {
        for(QMutableListIterator<DotEquate*> it(structLines); it.hasNext();) {
            auto line = it.next();
            QString name = line->getSymbolEntry()->getName();
            QStringList symbols = extractTagList(line->getComment());
            auto [ptr, errMessage] = parseStruct(name, symbols);
            // If there was an error, give up on parsing the struct this iteration.
            if(!errMessage.isEmpty()) {
                continue;
            }
            // If parsing was sucessful, then add it to the list of defined structs,
            // and renove it from the list of structs to parse.
            it.remove();
            dynamicSymbols.insert(name, {line->getSymbolEntry(), ptr});
        }
        // Update loop variables with new sizes.
        oldSize = newSize;
        newSize = structLines.size();
    } while(oldSize != newSize);

    // Propogate any remaining error messages.
    if(newSize != 0) {
        // Any remaining lines could not be parsed, and therefor are warnings.
        for(auto badLine : structLines) {
            QString name = badLine->getSymbolEntry()->getName();
            QStringList symbols = extractTagList(badLine->getComment());
            QString errMessage = parseStruct(name, symbols).second;
            // Propogate struct parsing error to the error list.
            resultCache.warningList.append({badLine->getListingLineNumber(), errMessage});
        }
    }
}

void MacroStackAnnotater::pushGlobalHelper(AsmCode * globalLine, QList<QSharedPointer<AType> > items)
{
    // Push the format tag onto the globals "stack", and create a new stack frame.
    QList<TraceCommand> actions;
    for(auto formatTag : items) {
        actions << TraceCommand(TraceAction::PUSH, FrameTarget::NEXT,
                                TraceTarget::GLOBALS, formatTag);
    }
    actions << TraceCommand(TraceAction::SETFRAME, FrameTarget::NEXT,
                            TraceTarget::GLOBALS);
    globalLine->setTraceData(actions);
}

QPair<QSharedPointer<StructType>, QString> MacroStackAnnotater::parseStruct(QString name,
                                               QStringList symbols)
{
    // If there is no symbol associated with the struct-to-be, then assembly should fail.
    if(!symbolTable->exists(name)) return {nullptr, neSymbol.arg(name)};
    else {
        auto namePtr = symbolTable->getValue(name);
        QList<QSharedPointer<AType>> structList;
        // For every symbol tag in the tag list:
        for(auto string: symbols) {
            // If there is no symbol by that name, error.
            if(!symbolTable->exists(string)) {
                this->resultCache.success = AnnotationSucces::SUCCESS_WITH_WARNINGS;
                return {nullptr, neSymbol.arg(string)};
            }
            // If there is no non-static storage space associated with that symbol
            // then it cannot appear in a struct. So error.
            else if(!dynamicSymbols.contains(string)) {
                this->resultCache.success = AnnotationSucces::SUCCESS_WITH_WARNINGS;
                return {nullptr, noEquate.arg(string)};
            }
            else structList.append(dynamicSymbols[string].second);
        }
        // Otherwise create the struct, and return no error message.
        return {QSharedPointer<StructType>::create(namePtr, structList),""};
    }
}
