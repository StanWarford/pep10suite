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

MacroAssembler::MacroAssembler(const MacroRegistry* registry): tokenBuffer(new TokenizerBuffer()),
    registry(registry)
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
    // Macro modules declare the name and argument count on the first line.
    // The assembler doesn't know how to parse it, so this line should be skipped.
    //if(instance.prototype->moduleType == ModuleType::MACRO) {
        //tokenBuffer->skipNextLine();
    //}
    while(tokenBuffer->inputRemains()) {
        auto retVal = assembleLine(graph, instance, errorMessage, dotEndDetected);

        // If there was an error,
        if(retVal.success == false) {
            result.success = false;
            // Track the error message as in the return value
            // and in the ModuleInstance.
            result.errInfo = {lineNumber, errorMessage};
            instance.errorList.append(result.errInfo);
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
#pragma message("Validate location of END error.")
        result.errInfo = {instance.prototype->textLines.size(), ";ERROR: Missing .END sentinel."};
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
    // Check the non-code lines: comments and empty lines.
    else if(tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_COMMENT)) {
        if(!tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_EMPTY)) {
            errorMessage = ";ERROR: \n expected after a comment";
            retVal.success = false;
            return retVal;
        }
        // Match a comment only line.
        auto commentLine =  QSharedPointer<CommentOnly>::create();
        commentLine->setComment(tokenBuffer->takeLastMatch().second.toString());
        retVal.success = true;
        retVal.codeLine = commentLine;
        // Take the \n.
        tokenBuffer->takeLastMatch();
        return retVal;
    }
    else if(tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_EMPTY)) {
        // Take the \n.
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
            errorMessage = ";ERROR: symbol definition must be followed by a  identifier, dot command, or macro.";
            retVal.success = false;
            return retVal;
        }
        // Remove the : from the symbol name.
        symbolDeclaration = tokenBuffer->takeLastMatch().second.chopped(1);
        if(!validateSymbolName(optional_helper(symbolDeclaration), errorMessage)) {
            retVal.success = false;
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

        // If the itera is the end of the map, then tokenString was not in the mnemonic map.
        if (iterator == Pep::mnemonToEnumMap.keyEnd()) {
            errorMessage = ";ERROR: Invalid mnemonic.";
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
            // Unliked unary parsing, use helper method since the code is long and involved.
            retVal.codeLine = parseNonUnaryInstruction(mnemonic, symbolPointer, instance, errorMessage);

            // If some part of parsing the instruction failed, raise the error to the next level.
            if(!errorMessage.isEmpty()) {
                retVal.success = false;
                return retVal;
            }
        }

    }
    else if(tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_DOT_COMMAND)) {
        tokenString =  tokenBuffer->takeLastMatch().second;
        tokenString = tokenString.mid(1);
        if (QString("ADDRSS").compare(tokenString, Qt::CaseInsensitive) == 0) {
            retVal.codeLine = parseADDRSS(symbolPointer, instance, errorMessage);
            if(!errorMessage.isEmpty()) {
                retVal.success = false;
                return retVal;
            }
        }
        else if (QString("ALIGN").compare(tokenString, Qt::CaseInsensitive) == 0) {
            retVal.codeLine = parseALIGN(symbolPointer, instance, errorMessage);
            if(!errorMessage.isEmpty()) {
                retVal.success = false;
                return retVal;
            }
        }
        else if (QString("ASCII").compare(tokenString, Qt::CaseInsensitive) == 0) {
            retVal.codeLine = parseASCII(symbolPointer, instance, errorMessage);
            if(!errorMessage.isEmpty()) {
                retVal.success = false;
                return retVal;
            }
        }
        else if (QString("BLOCK").compare(tokenString, Qt::CaseInsensitive) == 0) {
            retVal.codeLine = parseBLOCK(symbolPointer, instance, errorMessage);
            if(!errorMessage.isEmpty()) {
                retVal.success = false;
                return retVal;
            }
        }
        else if (QString("BURN").compare(tokenString, Qt::CaseInsensitive) == 0) {
            retVal.codeLine = parseBURN(symbolPointer, instance, errorMessage);
            if(!errorMessage.isEmpty()) {
                retVal.success = false;
                return retVal;
            }
        }
        else if (QString("BYTE").compare(tokenString, Qt::CaseInsensitive) == 0) {
            retVal.codeLine = parseBYTE(symbolPointer, instance, errorMessage);
            if(!errorMessage.isEmpty()) {
                retVal.success = false;
                return retVal;
            }
        }
        else if (QString("END").compare(tokenString, Qt::CaseInsensitive) == 0) {
            dotEndDetected = true;
            retVal.codeLine = parseEND(symbolPointer, instance, errorMessage);
            if(!errorMessage.isEmpty()) {
                retVal.success = false;
                return retVal;
            }
        }
        else if (QString("EQUATE").compare(tokenString, Qt::CaseInsensitive) == 0) {
            retVal.codeLine = parseEQUATE(symbolPointer, instance, errorMessage);
            if(!errorMessage.isEmpty()) {
                retVal.success = false;
                return retVal;
            }
        }
        else if (QString("EXPORT").compare(tokenString, Qt::CaseInsensitive) == 0) {
            retVal.codeLine = parseEXPORT(symbolPointer, instance, errorMessage);
            if(!errorMessage.isEmpty()) {
                retVal.success = false;
                return retVal;
            }
        }
        else if (QString("SYCALL").compare(tokenString, Qt::CaseInsensitive) == 0) {
            if(!parseSYCALL()){}
        }
        else if (QString("USYCALL").compare(tokenString, Qt::CaseInsensitive) == 0) {
            if(!parseUSYCALL()){}
        }
        else if (QString("WORD").compare(tokenString, Qt::CaseInsensitive) == 0) {
            retVal.codeLine = parseWORD(symbolPointer, instance, errorMessage);
            if(!errorMessage.isEmpty()) {
                retVal.success = false;
                return retVal;
            }
        }
        else {
            retVal.success = false;
            errorMessage = ";ERROR: Invalid dot command";
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

    if(tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_COMMENT)) {
        // Match a comment at the end of a line.
        retVal.codeLine->setComment(tokenBuffer->takeLastMatch().second.toString());
    }
    if(tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_EMPTY)) {
        retVal.success = true;
        tokenBuffer->takeLastMatch();
        return retVal;
    }
    else {
        retVal.success = false;
        return retVal;
    }
}

bool MacroAssembler::validateSymbolName(const QStringRef &name, QString &errorMessage)
{
    if (name.length() > 8) {
        errorMessage = ";ERROR: Symbol " + name + " cannot have more than eight characters.";
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
            errorMessage = ";ERROR: Addressing mode required for this instruction.";
            // No need to delete argument, this will be deleted by nonUnaryInstruction.
            return nullptr;
        }
        addrMode = Enu::EAddrMode::I;

    }
    // Otherwise an addressing mode was present.
    else {
        tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_ADDRESSING_MODE);
        addrMode = stringToAddrMode(tokenBuffer->takeLastMatch().second.toString());
        // Nested parens required.
        if ((static_cast<int>(addrMode) & Pep::addrModesMap.value(mnemonic)) == 0) {
            errorMessage = ";ERROR: Illegal addressing mode for this instruction.";
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
                                              ModuleInstance &instance, QString &errorMessage)
{
    if(!registry->hasMacro(macroName)) {
        errorMessage = ";ERROR: Macro " + macroName + " does not exist.";
        return nullptr;
    }
    auto macro = registry->getMacro(macroName);
    QStringList argumentList;
    while(tokenBuffer->matchOneOf(nonunaryOperandTypes)) {
        auto [token, tokenString] = tokenBuffer->takeLastMatch();
        argumentList << tokenString.toString();
    }

    // Validate that passed macro is a real macro
    quint16 index = graph.getIndexFromName(macroName);
    if(index == 0xFFFF) {
        errorMessage = ";ERROR: Module " + macroName + " is not a previously declared macro.";
        return nullptr;
    }

    // Validate that the correct number of macro arguments were passed.
    if(macro->argCount != argumentList.size()) {
        errorMessage = ";ERROR: Module " + macroName + " is not a previously declared macro.";
        return nullptr;
    }

    auto modulePrototype = graph.prototypeMap[index];
    auto moduleInstance = graph.getInstanceFromArgs(index, argumentList);
    if(!moduleInstance.has_value()) {
        errorMessage = ";ERROR: Module instance for " + macroName + " could not be found.";
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
        errorMessage = ";ERROR: Operand specifier expected after mnemonic.";
        return nullptr;
    }
    auto pair = tokenBuffer->takeLastMatch();

    MacroTokenizerHelper::ELexicalToken token = pair.first;
    QString tokenString = pair.second.toString();
    if (token == MacroTokenizerHelper::ELexicalToken::LT_IDENTIFIER) {
        if (tokenString.length() > 8) {
            errorMessage = ";ERROR: Symbol " + tokenString + " cannot have more than eight characters.";
            return nullptr;
        }
        return QSharedPointer<SymbolRefArgument>::create(instance.symbolTable->reference(tokenString));
    }
    else if (token == MacroTokenizerHelper::ELexicalToken::LT_STRING_CONSTANT) {
        if (MacroTokenizerHelper::byteStringLength(tokenString) > 2) {
            errorMessage = ";ERROR: String operands must have length at most two.";
            return nullptr;
        }
        return QSharedPointer<StringArgument>::create(tokenString);
    }
    else if (token == MacroTokenizerHelper::ELexicalToken::LT_HEX_CONSTANT) {
        tokenString.remove(0, 2); // Remove "0x" prefix.
        bool ok;
        int value = tokenString.toInt(&ok, 16);
        // If the value is in range for a 16 bit int.
        if (value < 65536) {
            return QSharedPointer<HexArgument>::create(value);
        }
        else {
            errorMessage = ";ERROR: Hexidecimal constant is out of range (0x0000..0xFFFF).";
            return nullptr;
        }
    }
    else if (token == MacroTokenizerHelper::ELexicalToken::LT_DEC_CONSTANT) {
        bool ok;
        int value = tokenString.toInt(&ok, 10);
        if ((-32768 <= value) && (value <= 65535)) {
            if (value < 0) {
                value += 65536; // Stored as two-byte unsigned.
                return QSharedPointer<DecArgument>::create(value);
            }
            else {
                return QSharedPointer<UnsignedDecArgument>::create(value);
            }
        }
        else {
            errorMessage = ";ERROR: Decimal constant is out of range (-32768..65535).";
            return nullptr;
        }
    }
    else if (token == MacroTokenizerHelper::ELexicalToken::LT_CHAR_CONSTANT) {
        return QSharedPointer<CharArgument>::create(tokenString);
    }
    else {
        errorMessage = ";ERROR: Operand specifier expected after mnemonic.";
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
    if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_IDENTIFIER)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        if (tokenString.length() > 8) {
            errorMessage = ";ERROR: Symbol " + tokenString + " cannot have more than eight characters.";
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
        errorMessage = ";ERROR: .ADDRSS requires a symbol argument.";
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
        errorMessage = ";ERROR: .ASCII requires a string constant argument.";
        return nullptr;
    }
}

QSharedPointer<DotAlign>
        MacroAssembler::parseALIGN(std::optional<QSharedPointer<SymbolEntry> > symbol,
                                   ModuleInstance&, QString &errorMessage)
{
    bool ok;
    if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_DEC_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        QSharedPointer<DotAlign> dotAlign = QSharedPointer<DotAlign>::create();
        if(symbol.has_value()) {
            dotAlign->setSymbolEntry(optional_helper(symbol));
        }
        int value = tokenString.toInt(&ok, 10);
        if (value == 2 || value == 4 || value == 8) {
            dotAlign->setArgument(QSharedPointer<UnsignedDecArgument>::create(value));
            // Num bytes generated is now deduced by the linker.
            //int numBytes = (value - byteCount % value) % value;
#pragma message("Fix byte count calc.")
            // dotAlign->numBytesGenerated = new UnsignedDecArgument(numBytes);
            return dotAlign;
        }
        else {
            errorMessage = ";ERROR: Decimal constant is out of range (2, 4, 8).";
            return nullptr;
        }
    }
    else {
        errorMessage = ";ERROR: .ALIGN requires a decimal constant 2, 4, or 8.";
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
            if (value < 0) {
                value += 65536; // Stored as two-byte unsigned.
                dotBlock->setArgument(QSharedPointer<DecArgument>::create(value));
            }
            else {
                dotBlock->setArgument(QSharedPointer<UnsignedDecArgument>::create(value));
            }
            return dotBlock;
        }
        else {
            errorMessage = ";ERROR: Decimal constant is out of range (0..65535).";
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
            errorMessage = ";ERROR: Hexidecimal constant is out of range (0x0000..0xFFFF).";
            return nullptr;
        }
    }
    else {
        errorMessage = ";ERROR: .BLOCK requires a decimal or hex constant argument.";
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
            instance.burnInfo.burnCount++;
            instance.burnInfo.burnArgument = value;
            return dotBurn;
        }
        else {
            errorMessage = ";ERROR: Hexidecimal constant is out of range (0x0000..0xFFFF).";
            return nullptr;
        }
    }
    else {
        errorMessage = ";ERROR: .BURN requires a hex constant argument.";
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
        DotByte *dotByte = new DotByte;
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
            errorMessage = ";ERROR: Decimal constant is out of byte range (-128..255).";
            return nullptr;
        }
    }
    else if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_HEX_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        tokenString.remove(0, 2); // Remove "0x" prefix.
        int value = tokenString.toInt(&ok, 16);
        if (value < 256) {
            DotByte *dotByte = new DotByte;
            dotByte->setArgument(QSharedPointer<HexArgument>::create(value));
        }
        else {
            errorMessage = ";ERROR: Hex constant is out of byte range (0x00..0xFF).";
            return nullptr;
        }
    }
    else if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_STRING_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        if (IsaParserHelper::byteStringLength(tokenString) > 1) {
            errorMessage = ";ERROR: .BYTE string operands must have length one.";
            return nullptr;
        }
        dotByte->setArgument(QSharedPointer<StringArgument>::create(tokenString));
    }
    else {
        errorMessage = ";ERROR: .BYTE requires a char, dec, hex, or string constant argument.";
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
            errorMessage = ";ERROR: Decimal constant is out of range (-32768..65535).";
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
            errorMessage = ";ERROR: Hexidecimal constant is out of range (0x0000..0xFFFF).";
            return nullptr;
        }
    }
    else if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_STRING_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        if (IsaParserHelper::byteStringLength(tokenString) > 2) {
            errorMessage = ";ERROR: .EQUATE string operand must have length at most two.";
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
            errorMessage = ";ERROR: Symbol " + tokenString + " cannot have more than eight characters.";
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

bool MacroAssembler::parseSYCALL()
{
    return false;
}

bool MacroAssembler::parseUSYCALL()
{
    return false;
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
            errorMessage = ";ERROR: Decimal constant is out of range (-32768..65535).";
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
            errorMessage = ";ERROR: Hexidecimal constant is out of range (0x0000..0xFFFF).";
            return nullptr;
        }
    }
    else if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_STRING_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        if (IsaParserHelper::byteStringLength(tokenString) > 2) {
            errorMessage = ";ERROR: .WORD string operands must have length at most two.";
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
