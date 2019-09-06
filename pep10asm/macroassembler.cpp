#include "macroassembler.h"
#include "macrotokenizer.h"
#include <list>
#include "pep.h"
#include "asmcode.h"
#include "symboltable.h"
#include "optional_helper.h"

static QList<MacroTokenizerHelper::ELexicalToken> nonunaryOperandTypes =
    {MacroTokenizerHelper::ELexicalToken::LT_IDENTIFIER,
     MacroTokenizerHelper::ELexicalToken::LT_STRING_CONSTANT,
     MacroTokenizerHelper::ELexicalToken::LT_HEX_CONSTANT,
     MacroTokenizerHelper::ELexicalToken::LT_DEC_CONSTANT,
     MacroTokenizerHelper::ELexicalToken::LT_CHAR_CONSTANT};

MacroAssembler::MacroAssembler(MacroRegistry* registry): registry(registry),
    tokenBuffer(new TokenizerBuffer())
{

}

MacroAssembler::~MacroAssembler()
{
    delete tokenBuffer;
}

AssemblerResult MacroAssembler::assemble(ModuleAssemblyGraph &graph)
{
    AssemblerResult retVal;
    std::list<ModuleInstance*> toAssemble;
    toAssemble.emplace_back(graph.instanceMap[graph.rootModule][0].get());


    // All modules in a single compilation will share the same symbol table
    QSharedPointer<SymbolTable> symbolTable = QSharedPointer<SymbolTable>::create();

    while(!toAssemble.empty()) {
        auto currentModule = toAssemble.front();
        toAssemble.pop_front();

        // If an item has already been assembled, no need to assemble it or
        // discover its dependencies again.
        if(currentModule->alreadyAssembled == true) continue;

        currentModule->symbolTable = symbolTable;
        for(auto childInstance : currentModule->prototype->lineToInstance) {
            if(childInstance->alreadyAssembled == false) {
                toAssemble.emplace_back(childInstance);
            }
        }
        qDebug().noquote() << "Assembling module: " << currentModule->prototype->name;
        auto result = assembleModule(graph, *currentModule);
        //qDebug().noquote() << "";
        if(!result.success) {
            retVal.success = false;
            #pragma message("Must map map line number from child to root")
            retVal.error = result.errInfo;
            return retVal;
        }
        else {
            currentModule->alreadyAssembled = true;
        }
    }
    return retVal;
}

MacroAssembler::ModuleResult MacroAssembler::assembleModule(ModuleAssemblyGraph &graph,
                                                            ModuleInstance &instance)
{
    ModuleResult result;
    result.success = true;
    quint16 lineNumber = 0;
    // Make sure tokenizer is prepared to handle this modules macro arguments.
    tokenBuffer->setMacroSubstitutions(instance.macroArgs);
    tokenBuffer->setTokenizerInput(instance.prototype->textLines);

    QList<QSharedPointer<AsmCode>> codeList;
    QString errorMessage;
    bool dotEndDetected = false;
    while(tokenBuffer->inputRemains()) {
        auto retVal = assembleLine(graph, instance, errorMessage, dotEndDetected);

        // If there was an error,
        if(retVal.success == false) {
            result.success = false;
            // Track the error message as in the return value
            // and in the ModuleInstance;
            auto error = QSharedPointer<FrontEndError>::create(instance.instanceIndex,
                                                               Severity::ERROR,
                                                               errorMessage,
                                                               lineNumber);
            graph.addError(error);
            result.errInfo = error;
            break;
        }
        else {
            //qDebug().noquote() << retVal.codeLine->getAssemblerSource();
        }
        codeList.append(retVal.codeLine);
        ++lineNumber;
        if(dotEndDetected) break;
    }
    // Only emit error about dotEnd when the program assembled successfully.
    // Otherwise, error messages might be surpressed by dotEnd error.
    if(!dotEndDetected && result.success) {
        result.success = false;
        // Track the error message as in the return value
        // and in the ModuleInstance;
        auto error = QSharedPointer<FrontEndError>::create(instance.instanceIndex,
                                                           Severity::ERROR,
                                                           missingEND,
                                                           instance.prototype->textLines.size());
        graph.addError(error);
        result.errInfo = error;
    }
    if(result.success) {
        instance.codeList = codeList;
    }
    return result;
}

MacroAssembler::LineResult MacroAssembler::assembleLine(ModuleAssemblyGraph &graph,
                                                        ModuleInstance &instance,
                                                        QString& errorMessage, bool &dotEndDetected)
{
    LineResult retVal;
    std::optional<QStringRef> symbolDeclaration;
    std::optional<QSharedPointer<SymbolEntry>> symbolPointer;
    QStringRef tokenString;
    Enu::EMnemonic mnemonic;

    // If a new line has an error on it, the error is the only
    // thing that will be reported. This means we don't have to search for errors
    // on every match.
    if(tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LTE_ERROR)) {
        retVal.success = false;
        errorMessage = tokenBuffer->takeLastMatch().second.toString();
        return retVal;
    }
    // Check for non-code lines: comments and empty lines.
    else if(tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_COMMENT)) {
        if(!tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_EMPTY)) {
            errorMessage = expectNewlineAfterComment;
            retVal.success = false;
            return retVal;
        }
        // Match a comment only line.
        auto commentLine =  QSharedPointer<CommentOnly>::create();
        commentLine->setComment(tokenBuffer->takeLastMatch().second.toString());
        retVal.success = true;
        retVal.codeLine = commentLine;
        // Take the \n so that it does not clog token buffer.
        tokenBuffer->takeLastMatch();
        return retVal;
    }
    else if(tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_EMPTY)) {
        // Take the \n so that it does not clog token buffer.
        tokenBuffer->takeLastMatch();
        retVal.success = true;
        retVal.codeLine =  QSharedPointer<BlankLine>::create();
        return retVal;
    }
    // Check for the presence of an (optional) symbol.
    else if(tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_SYMBOL_DEF)) {
        // If the symbol is not followed by a statement needing a symbol, we have an error.
        if(!(tokenBuffer->lookahead(MacroTokenizerHelper::ELexicalToken::LT_IDENTIFIER)
             || tokenBuffer->lookahead(MacroTokenizerHelper::ELexicalToken::LT_DOT_COMMAND)
             || tokenBuffer->lookahead(MacroTokenizerHelper::ELexicalToken::LTE_MACRO_INVOKE))) {
            errorMessage = unexpectedSymbolDecl;
            retVal.success = false;
            return retVal;
        }
        // Remove the : from the symbol name.
        symbolDeclaration = tokenBuffer->takeLastMatch().second.chopped(1);
        if(!validateSymbolName(optional_helper(symbolDeclaration), errorMessage)) {
            retVal.success = false;
            // Error message was set by validateSymbolName(...).
            return retVal;
        }
        symbolPointer = instance.symbolTable->define(symbolDeclaration->toString());
    }

    // Begin parsing mnemonics, pseudo-mnemonics, and macro-mnemonics.
    if(tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_IDENTIFIER)) {
        // Must decide if it is a unary or nonunary instruction.
        tokenString =  tokenBuffer->takeLastMatch().second;
        // See if the token string is in the mnemonic map, ignoring case.
        auto compare = [tokenString](QString mapKey) {
            return tokenString.compare(mapKey, Qt::CaseInsensitive) == 0;
        };
        auto iterator = std::find_if(Pep::mnemonToEnumMap.keyBegin(), Pep::mnemonToEnumMap.keyEnd(), compare);

        // If the iterator is the end of the map, then tokenString was not in the mnemonic map.
        if (iterator == Pep::mnemonToEnumMap.keyEnd()) {
            errorMessage = invalidMnemonic.arg(tokenString);
            retVal.success = false;
            return retVal;
        }

        mnemonic = Pep::mnemonToEnumMap[*iterator];

        // Process a unary instruction
        if(Pep::isUnaryMap.value(mnemonic)) {
            auto unaryInstruction =  QSharedPointer<UnaryInstruction>::create();
            unaryInstruction->setMnemonic(mnemonic);
            if(symbolPointer.has_value()) {
                unaryInstruction->setSymbolEntry(optional_helper(symbolPointer));
            }
            retVal.codeLine = unaryInstruction;
        }
        // Process a non-unary instruction
        else {
            // Unliked unary parsing, use helper method since the code is long.
            retVal.codeLine = parseNonUnaryInstruction(mnemonic, symbolPointer, instance, errorMessage);

            // If some part of parsing the instruction failed, propogate the error upwards.
            if(!errorMessage.isEmpty()) {
                retVal.success = false;
                return retVal;
            }
        }

    }
    else if(tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_DOT_COMMAND)) {
        tokenString =  tokenBuffer->takeLastMatch().second;
        tokenString = tokenString.mid(1);
        // Some instructions (like BURN) may only occur in the operating system,
        // and we need access to the root instance to determine what is being compiled.
        auto rootInstance = graph.instanceMap[graph.rootModule].first();

        // Use compare(...) instead of == because tokenizer does not
        // automatically capitalize dot command strings.
        if (QString("ADDRSS").compare(tokenString, Qt::CaseInsensitive) == 0) {
            retVal.codeLine = parseADDRSS(symbolPointer, instance, errorMessage);
        }
        else if (QString("ALIGN").compare(tokenString, Qt::CaseInsensitive) == 0) {
            retVal.codeLine = parseALIGN(symbolPointer, instance, errorMessage);
        }
        else if (QString("ASCII").compare(tokenString, Qt::CaseInsensitive) == 0) {
            retVal.codeLine = parseASCII(symbolPointer, instance, errorMessage);
        }
        else if (QString("BLOCK").compare(tokenString, Qt::CaseInsensitive) == 0) {
            retVal.codeLine = parseBLOCK(symbolPointer, instance, errorMessage);
        }
        else if (QString("BURN").compare(tokenString, Qt::CaseInsensitive) == 0) {
            if(rootInstance->prototype->moduleType != ModuleType::OPERATING_SYSTEM) {
                // Presence of error message will cause early return after if block.
                errorMessage = onlyInOperatingSystem.arg(".BURN");
            }
            else {
                retVal.codeLine = parseBURN(symbolPointer, instance, errorMessage);
            }

        }
        else if (QString("BYTE").compare(tokenString, Qt::CaseInsensitive) == 0) {
            retVal.codeLine = parseBYTE(symbolPointer, instance, errorMessage);
        }
        else if (QString("END").compare(tokenString, Qt::CaseInsensitive) == 0) {
            dotEndDetected = true;
            retVal.codeLine = parseEND(symbolPointer, instance, errorMessage);
        }
        else if (QString("EQUATE").compare(tokenString, Qt::CaseInsensitive) == 0) {
            retVal.codeLine = parseEQUATE(symbolPointer, instance, errorMessage);
        }
        else if (QString("EXPORT").compare(tokenString, Qt::CaseInsensitive) == 0) {
            if(rootInstance->prototype->moduleType != ModuleType::OPERATING_SYSTEM) {
                // Presence of error message will cause early return after if block.
                errorMessage = onlyInOperatingSystem.arg(".EXPORT");
            }
            else {
                retVal.codeLine = parseEXPORT(symbolPointer, instance, errorMessage);
            }
        }
        else if (QString("SCALL").compare(tokenString, Qt::CaseInsensitive) == 0) {
            if(rootInstance->prototype->moduleType != ModuleType::OPERATING_SYSTEM) {
                // Presence of error message will cause early return after if block.
                errorMessage = onlyInOperatingSystem.arg(".SCALL");
            }
            else {
                retVal.codeLine = parseSCALL(symbolPointer, instance, errorMessage);
            }
        }
        else if (QString("USCALL").compare(tokenString, Qt::CaseInsensitive) == 0) {
            if(rootInstance->prototype->moduleType != ModuleType::OPERATING_SYSTEM) {
                // Presence of error message will cause early return after if block.
                errorMessage = onlyInOperatingSystem.arg(".USCALL");
            }
            else {
                retVal.codeLine = parseUSCALL(symbolPointer, instance, errorMessage);
            }
        }
        else if (QString("WORD").compare(tokenString, Qt::CaseInsensitive) == 0) {
            retVal.codeLine = parseWORD(symbolPointer, instance, errorMessage);
        }
        else {
            retVal.success = false;
            errorMessage = invalidDotCommand.arg(tokenString);
            return retVal;
        }

        if(!errorMessage.isEmpty()) {
            retVal.success = false;
            return retVal;
        }

    }
    else if(tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LTE_MACRO_INVOKE)) {
        // Must handle all macro arguments.
        QString macroName = tokenBuffer->takeLastMatch().second.toString();
        retVal.codeLine = parseMacroInstruction(graph, macroName, symbolPointer, instance, errorMessage);
        if(!errorMessage.isEmpty()) {
            retVal.success = false;
            return retVal;
        }
    }
    // If we didn't match some form of instruction, and we didn't hit empty or comment line,
    // then the current line has major parsing issues and an error must be returned.
    else {
        // Check if we had a symbol followed by nonsense, like
        //  f: "hi" \sjd
        retVal.success = false;
        if(symbolDeclaration.has_value()) {
            errorMessage = unexpectedSymbolDecl;
        }
        // Check if there was a non-empty token that caused a syntax error.
        // If-with-assignment to limit scope of match.
        tokenBuffer->matchOneOf(nonunaryOperandTypes);
        if(auto match = tokenBuffer->takeLastMatch();
                match.first != MacroTokenizerHelper::ELexicalToken::LT_EMPTY) {
            errorMessage =  unexpectedToken.arg(match.second);

        }
        // Otherwise we hit an unexpected end of line.
        else {
           errorMessage = unxpectedEOL;
        }
        return retVal;
        // Otherwise, indicate the token that caused the error,

    }

    // Any line may end in a comment, and prior path that does not generate a codeLine
    // would have returned an error message by now, so no needed to compare codeLine to nullptr.
    if(tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_COMMENT)) {
        assert(retVal.codeLine);
        // Match a comment at the end of a line.
        retVal.codeLine->setComment(tokenBuffer->takeLastMatch().second.toString());
    }
    if(tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_EMPTY)) {
        retVal.success = true;
        tokenBuffer->takeLastMatch();
        return retVal;
    }
    // Something went wrong parsing the line, and all we know it that last
    // token to be visibile caused it, so propogate that token as the error.
    else {
        retVal.success = false;
        auto match = tokenBuffer->takeLastMatch();
        if(match.first == MacroTokenizerHelper::ELexicalToken::LT_EMPTY) {
            errorMessage = unxpectedEOL;
        }
        else {
            errorMessage =  unexpectedToken.arg(match.second);
        }
        return retVal;
    }
}

bool MacroAssembler::validateSymbolName(const QStringRef &name, QString &errorMessage)
{
    // The only additional restriction on symbols after tokenization
    // is that they may not have more than 8 characters.
    if (name.length() > 8) {
        errorMessage = longSymbol.arg(name);
        return false;
    }
    return true;
}

QSharedPointer<NonUnaryInstruction>
        MacroAssembler::parseNonUnaryInstruction(Enu::EMnemonic mnemonic,
                                                 std::optional<QSharedPointer<SymbolEntry>> symbol,
                                                 ModuleInstance &instance,QString &errorMessage)
{
    Enu::EAddrMode addrMode;
    auto nonUnaryInstruction = QSharedPointer<NonUnaryInstruction>::create();
    nonUnaryInstruction->setMnemonic(mnemonic);
    if(symbol.has_value()) {
        nonUnaryInstruction->setSymbolEntry(optional_helper(symbol));
    }

    auto argument = parseOperandSpecifier(instance, errorMessage);

    nonUnaryInstruction->setArgument(argument);

    // If the next token is not an addressing mode, then we need to check if this
    // is a valid instruction to omit the addressing mode for.
    if(!tokenBuffer->lookahead(MacroTokenizerHelper::ELexicalToken::LT_ADDRESSING_MODE)) {
        if (Pep::addrModeRequiredMap.value(mnemonic)) {
            errorMessage = reqAddrMode;
            // No need to delete argument, this will be deleted by nonUnaryInstruction.
            return nullptr;
        }
        // For instructions that do not require an explicit addressing mode, we assume I is implied.
        addrMode = Enu::EAddrMode::I;

    }
    // Otherwise an addressing mode was present.
    else {
        tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_ADDRESSING_MODE);
        addrMode = stringToAddrMode(tokenBuffer->takeLastMatch().second.toString());
        // Nested parens required.
        if ((static_cast<int>(addrMode) & Pep::addrModesMap.value(mnemonic)) == 0) {
            errorMessage = illegalAddrMode;
            // No need to delete argument, this will be deleted by nonUnaryInstruction.
            return nullptr;
        }
    }
    nonUnaryInstruction->setAddressingMode(addrMode);
    return nonUnaryInstruction;
}

QSharedPointer<MacroInvoke>
        MacroAssembler::parseMacroInstruction(const ModuleAssemblyGraph& graph,
                                              const QString &macroName,
                                              std::optional<QSharedPointer<SymbolEntry> > symbol,
                                              ModuleInstance &/*instance*/, QString &errorMessage)
{
    // Require that the registry already contains a definition for the passed macro.
    if(!registry->hasMacro(macroName)) {
        errorMessage = macroDoesNotExist.arg(macroName);
        return nullptr;
    }
    auto macro = registry->getMacro(macroName);
    // Collect all arguments for macro.
    QStringList argumentList;
    while(tokenBuffer->matchOneOf(nonunaryOperandTypes)) {
        auto [token, tokenString] = tokenBuffer->takeLastMatch();
        argumentList << tokenString.toString();
    }

    // Validate that macro was placed in assembly graph by the preprocessor.
    quint16 index = graph.getIndexFromName(macroName);
    if(index == 0xFFFF) {
        errorMessage = macroDoesNotExist.arg(macroName);
        return nullptr;
    }

    // Validate that the correct number of macro arguments were passed.
    if(macro->argCount != argumentList.size()) {
        errorMessage = macroWrongArgCount.arg(macroName);
        return nullptr;
    }

    // Check that a macro with the same argument list was
    // found by the preprocessor.
    auto modulePrototype = graph.prototypeMap[index];
    auto moduleInstance = graph.getInstanceFromArgs(index, argumentList);
    if(!moduleInstance.has_value()) {
        errorMessage = macroDoesNotExist.arg(macroName);
        return nullptr;
    }
    QSharedPointer<MacroInvoke> macroInstruction = QSharedPointer<MacroInvoke>::create();
    if(symbol.has_value()) {
        macroInstruction->setSymbolEntry(optional_helper(symbol));
    }
    macroInstruction->setArgumentList(argumentList);
    macroInstruction->setMacroInstance(optional_helper(moduleInstance));
    return macroInstruction;
}

QSharedPointer<AsmArgument>
        MacroAssembler::parseOperandSpecifier(ModuleInstance &instance, QString &errorMessage)
{
    if(!tokenBuffer->matchOneOf(nonunaryOperandTypes)) {
        errorMessage = opsecAfterMnemonic;
        return nullptr;
    }
    auto pair = tokenBuffer->takeLastMatch();

    MacroTokenizerHelper::ELexicalToken token = pair.first;
    QString tokenString = pair.second.toString();
    // Symbolic argument.
    if (token == MacroTokenizerHelper::ELexicalToken::LT_IDENTIFIER) {
        // Symbols may be no longer than 8 characters
        if (tokenString.length() > 8) {
            errorMessage = longSymbol.arg(tokenString);
            return nullptr;
        }
        return QSharedPointer<SymbolRefArgument>::create(instance.symbolTable->reference(tokenString));
    }
    // String constant of at most two bytes long.
    else if (token == MacroTokenizerHelper::ELexicalToken::LT_STRING_CONSTANT) {
        if (MacroTokenizerHelper::byteStringLength(tokenString) > 2) {
            errorMessage = wordStringOutOfRange;
            return nullptr;
        }
        return QSharedPointer<StringArgument>::create(tokenString);
    }
    // Hex constant in range [0000, FFFF]
    else if (token == MacroTokenizerHelper::ELexicalToken::LT_HEX_CONSTANT) {
        tokenString.remove(0, 2); // Remove "0x" prefix.
        bool ok;
        int value = tokenString.toInt(&ok, 16);
        // If the value is in range for a 16 bit int.
        if (value < 65536) {
            return QSharedPointer<HexArgument>::create(value);
        }
        else {
            errorMessage = wordHexOutOfRange;
            return nullptr;
        }
    }
    // Decimal constant in range [-32768, 65535]
    else if (token == MacroTokenizerHelper::ELexicalToken::LT_DEC_CONSTANT) {
        bool ok;
        int value = tokenString.toInt(&ok, 10);
        if ((-32768 <= value) && (value <= 65535)) {
            // Negative, so store as signed.
            if (value < 0) {
                value += 65536; // Stored as two-byte unsigned.
                return QSharedPointer<DecArgument>::create(value);
            }
            // Otherwise store as unsigned.
            else {
                return QSharedPointer<UnsignedDecArgument>::create(value);
            }
        }
        else {
            errorMessage = wordSignDecOutOfRange;
            return nullptr;
        }
    }
    // Character constant.
    else if (token == MacroTokenizerHelper::ELexicalToken::LT_CHAR_CONSTANT) {
        return QSharedPointer<CharArgument>::create(tokenString);
    }
    else {
        errorMessage = opsecAfterMnemonic;
        return nullptr;
    }
}

Enu::EAddrMode MacroAssembler::stringToAddrMode(QString str) const
{
    str = str.trimmed().toUpper();
    if (str.compare("I", Qt::CaseInsensitive) == 0) return Enu::EAddrMode::I;
    if (str.compare("D", Qt::CaseInsensitive) == 0) return Enu::EAddrMode::D;
    if (str.compare("N", Qt::CaseInsensitive) == 0) return Enu::EAddrMode::N;
    if (str.compare("S", Qt::CaseInsensitive) == 0) return Enu::EAddrMode::S;
    if (str.compare("SF", Qt::CaseInsensitive) == 0) return Enu::EAddrMode::SF;
    if (str.compare("X", Qt::CaseInsensitive) == 0) return Enu::EAddrMode::X;
    if (str.compare("SX", Qt::CaseInsensitive) == 0) return Enu::EAddrMode::SX;
    if (str.compare("SFX", Qt::CaseInsensitive) == 0) return Enu::EAddrMode::SFX;
    return Enu::EAddrMode::NONE;
}

QSharedPointer<DotAddrss>
        MacroAssembler::parseADDRSS(std::optional<QSharedPointer<SymbolEntry> > symbol,
                                    ModuleInstance &instance, QString &errorMessage)
{
    // .ADDRSS requires a symbolic argument
    if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_IDENTIFIER)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        if (tokenString.length() > 8) {
            errorMessage = longSymbol.arg(tokenString);
            return nullptr;
        }
        QSharedPointer<DotAddrss> dotAddrss = QSharedPointer<DotAddrss>::create();
        if(symbol.has_value()) {
            dotAddrss->setSymbolEntry(optional_helper(symbol));
        }
        dotAddrss->setArgument(QSharedPointer<SymbolRefArgument>::create(instance.symbolTable->reference(tokenString)));
        return dotAddrss;
    }
    else {
        errorMessage = addrssSymbolicArg;
        return nullptr;
    }
}

QSharedPointer<DotAscii>
        MacroAssembler::parseASCII(std::optional<QSharedPointer<SymbolEntry> > symbol,
                                   ModuleInstance&, QString &errorMessage)
{

    if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_STRING_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        QSharedPointer<DotAscii> dotAscii = QSharedPointer<DotAscii>::create();
        if(symbol.has_value()) {
            dotAscii->setSymbolEntry(optional_helper(symbol));
        }
        dotAscii->setArgument(QSharedPointer<StringArgument>::create(tokenString));
        return dotAscii;
    }
    else {
        errorMessage = badAsciiArgument;
        return nullptr;
    }
}

QSharedPointer<DotAlign>
        MacroAssembler::parseALIGN(std::optional<QSharedPointer<SymbolEntry> > symbol,
                                   ModuleInstance&, QString &errorMessage)
{
    // ALIGN directives may only take integer arguments in (2, 4, 8).
    if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_DEC_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        QSharedPointer<DotAlign> dotAlign = QSharedPointer<DotAlign>::create();
        if(symbol.has_value()) {
            dotAlign->setSymbolEntry(optional_helper(symbol));
        }
        bool ok;
        int value = tokenString.toInt(&ok, 10);
        if (value == 2 || value == 4 || value == 8) {
            dotAlign->setArgument(QSharedPointer<UnsignedDecArgument>::create(value));
            // Number of bytes generated is now calculated by linker.
            return dotAlign;
        }
        else {
            errorMessage = decConst248;
            return nullptr;
        }
    }
    else {
        errorMessage = badAlignArgument;
        return nullptr;
    }
}

QSharedPointer<DotBlock>
        MacroAssembler::parseBLOCK(std::optional<QSharedPointer<SymbolEntry> > symbol,
                                   ModuleInstance&, QString &errorMessage)
{
    if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_DEC_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        bool ok;
        int value = tokenString.toInt(&ok, 10);
        if ((0 <= value) && (value <= 65535)) {
            QSharedPointer<DotBlock> dotBlock = QSharedPointer<DotBlock>::create();
            if(symbol.has_value()) {
                dotBlock->setSymbolEntry(optional_helper(symbol));
            }
            // Argument may only be unsigned.
            dotBlock->setArgument(QSharedPointer<UnsignedDecArgument>::create(value));
            return dotBlock;
        }
        else {
            errorMessage = wordUnsignDecOutOfRange;
            return nullptr;
        }
    }
    else if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_HEX_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        tokenString.remove(0, 2); // Remove "0x" prefix.
        bool ok;
        int value = tokenString.toInt(&ok, 16);
        if (value < 65536) {
            QSharedPointer<DotBlock> dotBlock = QSharedPointer<DotBlock>::create();
            if(symbol.has_value()) {
                dotBlock->setSymbolEntry(optional_helper(symbol));
            }
            dotBlock->setArgument(QSharedPointer<HexArgument>::create(value));
            return dotBlock;
        }
        else {
            errorMessage = wordHexOutOfRange;
            return nullptr;
        }
    }
    else {
        errorMessage = badBlockArgument;
        return nullptr;
    }
}

QSharedPointer<DotBurn>
        MacroAssembler::parseBURN(std::optional<QSharedPointer<SymbolEntry> > symbol,
                                  ModuleInstance &instance, QString &errorMessage)
{
    if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_HEX_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        tokenString.remove(0, 2); // Remove "0x" prefix.
        bool ok;
        int value = tokenString.toInt(&ok, 16);
        if (value < 65536) {
            QSharedPointer<DotBurn> dotBurn = QSharedPointer<DotBurn>::create();
            if(symbol.has_value()) {
                dotBurn->setSymbolEntry(optional_helper(symbol));
            }
            dotBurn->setArgument(QSharedPointer<HexArgument>::create(value));
            // Multiple BURN directives will be detected in linker.
            instance.burnInfo.burnCount++;
            instance.burnInfo.burnArgument = value;
            return dotBurn;
        }
        else {
            errorMessage = wordHexOutOfRange;
            return nullptr;
        }
    }
    else {
        errorMessage = badBurnArgument;
        return nullptr;
    }
}

QSharedPointer<DotByte>
        MacroAssembler::parseBYTE(std::optional<QSharedPointer<SymbolEntry> > symbol,
                                  ModuleInstance&, QString &errorMessage)
{
    bool ok;
    QSharedPointer<DotByte> dotByte = QSharedPointer<DotByte>::create();
    if(symbol.has_value()) {
        dotByte->setSymbolEntry(optional_helper(symbol));
    }

    if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_CHAR_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        dotByte->setArgument(QSharedPointer<CharArgument>::create(tokenString));
    }
    else if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_DEC_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        int value = tokenString.toInt(&ok, 10);
        if ((-128 <= value) && (value <= 255)) {
            if (value < 0) {
                value += 256; // value stored as one-byte unsigned.
            }
            dotByte->setArgument(QSharedPointer<DecArgument>::create(value));
        }
        else {
            errorMessage = byteSignDecOutOfRange;
            return nullptr;
        }
    }
    else if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_HEX_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        tokenString.remove(0, 2); // Remove "0x" prefix.
        int value = tokenString.toInt(&ok, 16);
        if (value < 256) {
            dotByte->setArgument(QSharedPointer<HexArgument>::create(value));
        }
        else {
            errorMessage = byteHexOutOfRange;
            return nullptr;
        }
    }
    else if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_STRING_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        if (IsaParserHelper::byteStringLength(tokenString) > 1) {
            errorMessage = byteStringOutOfRange;
            return nullptr;
        }
        dotByte->setArgument(QSharedPointer<StringArgument>::create(tokenString));
    }
    else {
        errorMessage = badByteArgument;
        return nullptr;
    }
    return dotByte;
}

QSharedPointer<DotEnd>
        MacroAssembler::parseEND(std::optional<QSharedPointer<SymbolEntry> > symbol,
                                 ModuleInstance&, QString &errorMessage)
{
    if(symbol.has_value()) {
        errorMessage = ";ERROR: .END directive may not define a symbol.";
        return nullptr;
    }
    if (!(tokenBuffer->lookahead(MacroTokenizerHelper::ELexicalToken::LT_COMMENT)
          || tokenBuffer->lookahead(MacroTokenizerHelper::ELexicalToken::LT_EMPTY))) {
        errorMessage = ";ERROR: Only a comment can follow .END.";
        return nullptr;
    }

    return QSharedPointer<DotEnd>::create();
}

QSharedPointer<DotEquate>
        MacroAssembler::parseEQUATE(std::optional<QSharedPointer<SymbolEntry> > symbol,
                                    ModuleInstance&, QString &errorMessage)
{
    if (!symbol.has_value()) {
        errorMessage = ";ERROR: .EQUATE must have a symbol definition.";
        return nullptr;
    }

    bool ok;
    QSharedPointer<DotEquate> dotEquate = QSharedPointer<DotEquate>::create();
    if(symbol.has_value()) {
        dotEquate->setSymbolEntry(optional_helper(symbol));
    }
    if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_DEC_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        int value = tokenString.toInt(&ok, 10);
        if ((-32768 <= value) && (value <= 65535)) {

            if (value < 0) {
                value += 65536; // Stored as two-byte unsigned.
                dotEquate->setArgument(QSharedPointer<DecArgument>::create(value));
            }
            else {
                dotEquate->setArgument(QSharedPointer<UnsignedDecArgument>::create(value));
            }
            dotEquate->getSymbolEntry()->setValue(QSharedPointer<SymbolValueNumeric>::create(value));
        }
        else {
            errorMessage = wordSignDecOutOfRange;
            return nullptr;
        }
    }
    else if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_HEX_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        tokenString.remove(0, 2); // Remove "0x" prefix.
        int value = tokenString.toInt(&ok, 16);
        if (value < 65536) {
            dotEquate->setArgument(QSharedPointer<HexArgument>::create(value));
            dotEquate->getSymbolEntry()->setValue(QSharedPointer<SymbolValueNumeric>::create(value));
        }
        else {
            errorMessage = wordHexOutOfRange;
            return nullptr;
        }
    }
    else if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_STRING_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        if (IsaParserHelper::byteStringLength(tokenString) > 2) {
            errorMessage = wordStringOutOfRange;
            return nullptr;
        }
        dotEquate->setArgument(QSharedPointer<StringArgument>::create(tokenString));
        dotEquate->getSymbolEntry()->setValue(QSharedPointer<SymbolValueNumeric>::create(IsaParserHelper::string2ArgumentToInt(tokenString)));
    }
    else if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_CHAR_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        dotEquate->setArgument(QSharedPointer<CharArgument>::create(tokenString));
        dotEquate->getSymbolEntry()->setValue(QSharedPointer<SymbolValueNumeric>::create(IsaParserHelper::charStringToInt(tokenString)));
    }
    else {
        errorMessage = ";ERROR: .EQUATE requires a dec, hex, or string constant argument.";
        return nullptr;
    }
    return dotEquate;
}

QSharedPointer<DotExport>
        MacroAssembler::parseEXPORT(std::optional<QSharedPointer<SymbolEntry> > symbol,
                                 ModuleInstance &instance, QString &errorMessage)
{
    if (symbol.has_value()) {
        errorMessage = ";ERROR: .EXPORT must not have a symbol definition.";
        return nullptr;
    }

    if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_IDENTIFIER)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        if (tokenString.length() > 8) {
            errorMessage = longSymbol.arg(tokenString);
            return nullptr;
        }
        QSharedPointer<DotExport> dotExport = QSharedPointer<DotExport>::create();
        dotExport->setArgument(QSharedPointer<SymbolRefArgument>::create(instance.symbolTable->reference(tokenString)));
        // Export declares a symbol from the operating system to be visible in user code.
        instance.symbolTable->declareExternal(tokenString);
        return dotExport;
    }
    else {
        errorMessage = ";ERROR: .EXPORT requires a symbol argument.";
        return nullptr;
    }
}

QSharedPointer<DotSycall> MacroAssembler::parseSCALL(std::optional<QSharedPointer<SymbolEntry> > symbol, ModuleInstance &instance, QString &errorMessage)
{
    if (symbol.has_value()) {
        errorMessage = ";ERROR: .SYCALL must not have a symbol definition.";
        return nullptr;
    }
    if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_IDENTIFIER)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        if (tokenString.length() > 8) {
            errorMessage = longSymbol.arg(tokenString);
            return nullptr;
        }
        QSharedPointer<DotSycall> dotSycall = QSharedPointer<DotSycall>::create();
        dotSycall->setArgument(QSharedPointer<SymbolRefArgument>::create(instance.symbolTable->reference(tokenString)));
        // Export declares a symbol from the operating system to be visible in user code.
        bool success = registry->registerNonunarySystemCall(tokenString);
        if(!success) {
            errorMessage = ";ERROR: Failed to register system call.";
            return nullptr;
        }
        return dotSycall;
    }
    else {
        errorMessage = ";ERROR: .SYCALL requires a symbol argument.";
        return nullptr;
    }
}

QSharedPointer<DotUSycall> MacroAssembler::parseUSCALL(std::optional<QSharedPointer<SymbolEntry> > symbol, ModuleInstance &instance, QString &errorMessage)
{
    if (symbol.has_value()) {
        errorMessage = ";ERROR: .USYCALL must not have a symbol definition.";
        return nullptr;
    }
    if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_IDENTIFIER)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        if (tokenString.length() > 8) {
            errorMessage = longSymbol.arg(tokenString);
            return nullptr;
        }
        QSharedPointer<DotUSycall> dotUSycall = QSharedPointer<DotUSycall>::create();
        dotUSycall->setArgument(QSharedPointer<SymbolRefArgument>::create(instance.symbolTable->reference(tokenString)));
        // Export declares a symbol from the operating system to be visible in user code.
        bool success = registry->registerUnarySystemCall(tokenString);
        if(!success) {
            errorMessage = ";ERROR: Failed to register system call.";
            return nullptr;
        }
        return dotUSycall;
    }
    else {
        errorMessage = ";ERROR: .USYCALL requires a symbol argument.";
        return nullptr;
    }
}

QSharedPointer<DotWord>
        MacroAssembler::parseWORD(std::optional<QSharedPointer<SymbolEntry> > symbol,
                                  ModuleInstance&, QString &errorMessage)
{
    bool ok;
    QSharedPointer<DotWord> dotWord = QSharedPointer<DotWord>::create();
    if(symbol.has_value()) {
        dotWord->setSymbolEntry(optional_helper(symbol));
    }

    if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_CHAR_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        dotWord->setArgument(QSharedPointer<CharArgument>::create(tokenString));
    }
    else if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_DEC_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        int value = tokenString.toInt(&ok, 10);
        if ((-32768 <= value) && (value < 65536)) {
            if (value < 0) {
                value += 65536; // Stored as two-byte unsigned.
                dotWord->setArgument(QSharedPointer<DecArgument>::create(value));
            }
            else {
                dotWord->setArgument(QSharedPointer<UnsignedDecArgument>::create(value));
            }
        }
        else {
            errorMessage = wordSignDecOutOfRange;
            return nullptr;
        }
    }
    else if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_HEX_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        tokenString.remove(0, 2); // Remove "0x" prefix.
        int value = tokenString.toInt(&ok, 16);
        if (value < 65536) {
            dotWord->setArgument(QSharedPointer<HexArgument>::create(value));
        }
        else {
            errorMessage = wordHexOutOfRange;
            return nullptr;
        }
    }
    else if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_STRING_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        if (IsaParserHelper::byteStringLength(tokenString) > 2) {
            errorMessage = wordStringOutOfRange;
            return nullptr;
        }
        dotWord->setArgument(QSharedPointer<StringArgument>::create(tokenString));
    }
    else {
        errorMessage = ";ERROR: .WORD requires a char, dec, hex, or string constant argument.";
        return nullptr;
    }
    return dotWord;
}
