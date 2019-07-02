#include "macroassembler.h"
#include "macrotokenizer.h"
#include <list>
#include "pep.h"
#include "asmcode.h"
#include "symboltable.h"
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
        for(auto childModule : currentModule->prototype->lineToInstance) {
            auto childInstance = std::get<1>(childModule);
            if(childInstance->alreadyAssembled == false) {
                toAssemble.emplace_back(childInstance);
            }
        }
        auto result = assembleModule(graph, *currentModule);
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

MacroAssembler::ModuleResult MacroAssembler::assembleModule(ModuleAssemblyGraph &graph, ModuleInstance &instance)
{
    ModuleResult result;
    quint16 lineNumber = 0;
    // Make sure tokenizer is prepared to handle this modules macro arguments.
    tokenBuffer->setMacroSubstitutions(instance.macroArgs);
    tokenBuffer->setTokenizerInput(instance.prototype->textLines);

    QList<AsmCode*> codeList;
    QString errorMessage;
    bool dotEndDetected = false;
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
            qDebug().noquote() << retVal.codeLine->getAssemblerSource();
        }
        codeList.append(retVal.codeLine);
        ++lineNumber;
        if(dotEndDetected) break;
    }
    if(result.success) {
        instance.codeList = codeList;
    }
    else {
        for(auto line : codeList) {
            delete line;
        }
    }
    return result;
}

MacroAssembler::LineResult MacroAssembler::assembleLine(ModuleAssemblyGraph &graph, ModuleInstance &instance,
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
        auto commentLine = new CommentOnly;
        commentLine->comment = tokenBuffer->takeLastMatch().second.toString();
        retVal.success = true;
        retVal.codeLine = commentLine;
        return retVal;
    }
    else if(tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_EMPTY)) {
        // Match an empty line.
        retVal.success = true;
        retVal.codeLine = new BlankLine;
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
        if(!validateSymbolName(symbolDeclaration.value(), errorMessage)) {
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
            return tokenString.compare(mapKey, Qt::CaseInsensitive) ;
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
            auto unaryInstruction = new UnaryInstruction();
            unaryInstruction->mnemonic = mnemonic;
            if(symbolDeclaration.has_value()) {
                unaryInstruction->symbolEntry = nullptr;
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
        if (tokenString == "ADDRSS") {
            retVal.codeLine = parseADDRSS(symbolPointer, instance, errorMessage);
            if(!errorMessage.isEmpty()) {
                retVal.success = false;
                return retVal;
            }
        }
        else if (tokenString == "ALIGN") {
            retVal.codeLine = parseALIGN(symbolPointer, instance, errorMessage);
            if(!errorMessage.isEmpty()) {
                retVal.success = false;
                return retVal;
            }
        }
        else if (tokenString == "ASCII") {
            retVal.codeLine = parseASCII(symbolPointer, instance, errorMessage);
            if(!errorMessage.isEmpty()) {
                retVal.success = false;
                return retVal;
            }
        }
        else if (tokenString == "BLOCK") {
            retVal.codeLine = parseBLOCK(symbolPointer, instance, errorMessage);
            if(!errorMessage.isEmpty()) {
                retVal.success = false;
                return retVal;
            }
        }
        else if (tokenString == "BURN") {
            retVal.codeLine = parseBURN(symbolPointer, instance, errorMessage);
            if(!errorMessage.isEmpty()) {
                retVal.success = false;
                return retVal;
            }
        }
        else if (tokenString == "BYTE") {
            retVal.codeLine = parseBYTE(symbolPointer, instance, errorMessage);
            if(!errorMessage.isEmpty()) {
                retVal.success = false;
                return retVal;
            }
        }
        else if (tokenString == "END") {
            dotEndDetected = true;
            retVal.codeLine = parseEND(symbolPointer, instance, errorMessage);
            if(!errorMessage.isEmpty()) {
                retVal.success = false;
                return retVal;
            }
        }
        else if (tokenString == "EQUATE") {
            retVal.codeLine = parseEQUATE(symbolPointer, instance, errorMessage);
            if(!errorMessage.isEmpty()) {
                retVal.success = false;
                return retVal;
            }
        }
        else if (tokenString == "EXPORT") {
            if(!parseEXPORT()){}
        }
        else if (tokenString == "SYCALL") {
            if(!parseSYCALL()){}
        }
        else if (tokenString == "USYCALL") {
            if(!parseUSYCALL()){}
        }
        else if (tokenString == "WORD") {
            retVal.codeLine = parseWORD(symbolPointer, instance, errorMessage);
            if(!errorMessage.isEmpty()) {
                retVal.success = false;
                return retVal;
            }
        }

    }
    else if(tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LTE_MACRO_INVOKE)) {
        // This one kind of breaks the rules...
        #pragma message("Must work on macro invocation assembly")
        // Must handle all macro arguments.
        QString macroName = tokenBuffer->takeLastMatch().second.toString();
        retVal.codeLine = parseMacroInstruction(graph, macroName, symbolPointer, instance, errorMessage);
        if(!errorMessage.isEmpty()) {
            retVal.success = false;
            return retVal;
        }
    }

    if(tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_COMMENT)) {
        // Match a comment only line.
        retVal.codeLine->comment = tokenBuffer->takeLastMatch().second.toString();
        QStringList arguments;
        while(tokenBuffer->matchOneOf(nonunaryOperandTypes)) {

        }
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

NonUnaryInstruction *MacroAssembler::parseNonUnaryInstruction(Enu::EMnemonic mnemonic,
                                                              std::optional<QSharedPointer<SymbolEntry>> symbol,
                                                              ModuleInstance &instance, QString &errorMessage)
{
    Enu::EAddrMode addrMode;
    auto nonUnaryInstruction = new NonUnaryInstruction();
    nonUnaryInstruction->mnemonic = mnemonic;
    if(symbol.has_value()) {
        nonUnaryInstruction->symbolEntry = symbol.value();
    }

    auto argument = parseOperandSpecifier(instance, errorMessage);

    // If there was an error parsing the operand, it is already captured in the appropriate argument.
    // we must only clean up memory that we allocated.
    if(!errorMessage.isEmpty()) {
        delete nonUnaryInstruction;
        return nullptr;
    }
    nonUnaryInstruction->argument = argument;

    // If the next token is not an addressing mode, then we need to check if this
    // is a valid instruction to omit the addressing mode for.
    if(!tokenBuffer->lookahead(MacroTokenizerHelper::ELexicalToken::LT_ADDRESSING_MODE)) {
        if (Pep::addrModeRequiredMap.value(mnemonic)) {
            errorMessage = ";ERROR: Addressing mode required for this instruction.";
            delete nonUnaryInstruction;
            // No need to delete argument, this will be deleted by nonUnaryInstruction.
            return nullptr;
        }
        addrMode = Enu::EAddrMode::I;

    }
    // Otherwise an addressing mode was present.
    else {
        tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_ADDRESSING_MODE);
        addrMode = IsaParserHelper::stringToAddrMode(tokenBuffer->takeLastMatch().second.toString());
        if ((static_cast<int>(addrMode) & Pep::addrModesMap.value(mnemonic)) == 0) { // Nested parens required.
            errorMessage = ";ERROR: Illegal addressing mode for this instruction.";
            return nullptr;
        }
    }
    nonUnaryInstruction->addressingMode = addrMode;
    return nonUnaryInstruction;
}

MacroInvoke *MacroAssembler::parseMacroInstruction(const ModuleAssemblyGraph& graph,const QString &macroName,
                                                   std::optional<QSharedPointer<SymbolEntry> > symbol,
                                                   ModuleInstance &instance, QString &errorMessage)
{
    if(!registry->hasMacro(macroName)) {
        errorMessage = ";ERROR: Macro " + macroName + " does not exist.";
        return nullptr;
    }
    auto macro = registry->getMacro(macroName);
    QList<AsmArgument*> argumentList;
    while(tokenBuffer->matchOneOf(nonunaryOperandTypes)) {
        auto [token, tokenString] = tokenBuffer->takeLastMatch();
        auto argument = macroArgParse(instance, token, tokenString.toString(), errorMessage);
        if(!errorMessage.isEmpty()) {
            break;
        }
        argumentList.append(argument);
    }
    if(!errorMessage.isEmpty()) {
        for (auto argument : argumentList) {
            delete argument;
        }
        return nullptr;
    }

    // Validate that passed macro is a real macro
    quint16 index = graph.getIndexFromName(macroName);
    if(index == 0xFFFF) {
        errorMessage = ";ERROR: Module " + macroName + " is not a previously declared macro.";
        for (auto argument : argumentList) {
            delete argument;
        }
        return nullptr;
    }

    // Validate that the correct number of macro arguments were passed.
    if(macro->argCount != argumentList.size()) {
        errorMessage = ";ERROR: Module " + macroName + " is not a previously declared macro.";
        for (auto argument : argumentList) {
            delete argument;
        }
        return nullptr;
    }
    QStringList argumentString;
    for(auto argument : argumentList) {
        argumentString << argument->getArgumentString();
    }
    auto modulePrototype = graph.prototypeMap[index];
    auto moduleInstance = graph.getInstanceFromArgs(index, argumentString);
    if(!moduleInstance.has_value()) {
        errorMessage = ";ERROR: Module instance for " + macroName + " could not be found.";
        for (auto argument : argumentList) {
            delete argument;
        }
        return nullptr;
    }
    MacroInvoke *macroInstruction = new MacroInvoke;
    if(symbol.has_value()) {
        macroInstruction->symbolEntry = symbol.value();
    }
    macroInstruction->argumentList = new AsmArgumentList(argumentList);
    macroInstruction->macroInstance = moduleInstance.value();
    return macroInstruction;
}

AsmArgument *MacroAssembler::macroArgParse(ModuleInstance &instance, MacroTokenizerHelper::ELexicalToken token,
                                           const QString &argumentString, QString &errorMessage)
{
    if (token == MacroTokenizerHelper::ELexicalToken::LT_IDENTIFIER) {
        if (argumentString.length() > 8) {
            errorMessage = ";ERROR: Symbol " + argumentString + " cannot have more than eight characters.";
            return nullptr;
        }
        return new SymbolRefArgument(instance.symbolTable->reference(argumentString));
    }
    else if (token == MacroTokenizerHelper::ELexicalToken::LT_STRING_CONSTANT) {
        if (MacroTokenizerHelper::byteStringLength(argumentString) > 2) {
            errorMessage = ";ERROR: String operands must have length at most two.";
            return nullptr;
        }
        return new StringArgument(argumentString);
    }
    else if (token == MacroTokenizerHelper::ELexicalToken::LT_HEX_CONSTANT) {
        QString temp = argumentString;
        temp.remove(0, 2); // Remove "0x" prefix.
        bool ok;
        int value = temp.toInt(&ok, 16);
        // If the value is in range for a 16 bit int.
        if (value < 65536) {
            return new HexArgument(value);
        }
        else {
            errorMessage = ";ERROR: Hexidecimal constant is out of range (0x0000..0xFFFF).";
            return nullptr;
        }
    }
    else if (token == MacroTokenizerHelper::ELexicalToken::LT_DEC_CONSTANT) {
        bool ok;
        int value = argumentString.toInt(&ok, 10);
        if ((-32768 <= value) && (value <= 65535)) {
            if (value < 0) {
                value += 65536; // Stored as two-byte unsigned.
                return new DecArgument(value);
            }
            else {
                return new UnsignedDecArgument(value);
            }
        }
        else {
            errorMessage = ";ERROR: Decimal constant is out of range (-32768..65535).";
            return nullptr;
        }
    }
    else if (token == MacroTokenizerHelper::ELexicalToken::LT_CHAR_CONSTANT) {
        return new CharArgument(argumentString);
    }
    else {
        errorMessage = ";ERROR: Operand specifier expected after mnemonic.";
        return nullptr;
    }
}

AsmArgument *MacroAssembler::parseOperandSpecifier(ModuleInstance &instance, QString &errorMessage)
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
        return new SymbolRefArgument(instance.symbolTable->reference(tokenString));
    }
    else if (token == MacroTokenizerHelper::ELexicalToken::LT_STRING_CONSTANT) {
        if (MacroTokenizerHelper::byteStringLength(tokenString) > 2) {
            errorMessage = ";ERROR: String operands must have length at most two.";
            return nullptr;
        }
        return new StringArgument(tokenString);
    }
    else if (token == MacroTokenizerHelper::ELexicalToken::LT_HEX_CONSTANT) {
        tokenString.remove(0, 2); // Remove "0x" prefix.
        bool ok;
        int value = tokenString.toInt(&ok, 16);
        // If the value is in range for a 16 bit int.
        if (value < 65536) {
            return new HexArgument(value);
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
                return new DecArgument(value);
            }
            else {
                return new UnsignedDecArgument(value);
            }
        }
        else {
            errorMessage = ";ERROR: Decimal constant is out of range (-32768..65535).";
            return nullptr;
        }
    }
    else if (token == MacroTokenizerHelper::ELexicalToken::LT_CHAR_CONSTANT) {
        return new CharArgument(tokenString);
    }
    else {
        errorMessage = ";ERROR: Operand specifier expected after mnemonic.";
        return nullptr;
    }
}

DotAddrss *MacroAssembler::parseADDRSS(std::optional<QSharedPointer<SymbolEntry> > symbol,
                                       ModuleInstance &instance, QString &errorMessage)
{

    if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_IDENTIFIER)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        if (tokenString.length() > 8) {
            errorMessage = ";ERROR: Symbol " + tokenString + " cannot have more than eight characters.";
            return nullptr;
        }
        DotAddrss *dotAddrss = new DotAddrss;
        if(symbol.has_value()) {
            dotAddrss->symbolEntry = symbol.value();
        }
        dotAddrss->argument = new SymbolRefArgument(instance.symbolTable->reference(tokenString));
        return dotAddrss;
    }
    else {
        errorMessage = ";ERROR: .ADDRSS requires a symbol argument.";
        return nullptr;
    }
}

DotAscii *MacroAssembler::parseASCII(std::optional<QSharedPointer<SymbolEntry> > symbol, ModuleInstance&, QString &errorMessage)
{

    if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_STRING_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        DotAscii *dotAscii = new DotAscii;
        if(symbol.has_value()) {
            dotAscii->symbolEntry = symbol.value();
        }
        dotAscii->argument = new StringArgument(tokenString);
        return dotAscii;
    }
    else {
        errorMessage = ";ERROR: .ASCII requires a string constant argument.";
        return nullptr;
    }
}

DotAlign *MacroAssembler::parseALIGN(std::optional<QSharedPointer<SymbolEntry> > symbol, ModuleInstance&, QString &errorMessage)
{
    bool ok;
    if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_DEC_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        DotAlign *dotAlign = new DotAlign;
        if(symbol.has_value()) {
            dotAlign->symbolEntry = symbol.value();
        }
        int value = tokenString.toInt(&ok, 10);
        if (value == 2 || value == 4 || value == 8) {
            dotAlign->argument = new UnsignedDecArgument(value);
            // Num bytes generated is now deduced by the linker.
            //int numBytes = (value - byteCount % value) % value;
            // dotAlign->numBytesGenerated = new UnsignedDecArgument(numBytes);
            return dotAlign;
        }
        else {
            errorMessage = ";ERROR: Decimal constant is out of range (2, 4, 8).";
            delete dotAlign;
            return nullptr;
        }
    }
    else {
        errorMessage = ";ERROR: .ALIGN requires a decimal constant 2, 4, or 8.";
        return nullptr;
    }
}

DotBlock *MacroAssembler::parseBLOCK(std::optional<QSharedPointer<SymbolEntry> > symbol, ModuleInstance&, QString &errorMessage)
{

    if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_DEC_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        bool ok;
        int value = tokenString.toInt(&ok, 10);
        if ((0 <= value) && (value <= 65535)) {
            DotBlock *dotBlock = new DotBlock;
            if(symbol.has_value()) {
                dotBlock->symbolEntry = symbol.value();
            }
            if (value < 0) {
                value += 65536; // Stored as two-byte unsigned.
                dotBlock->argument = new DecArgument(value);
            }
            else {
                dotBlock->argument = new UnsignedDecArgument(value);
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
            DotBlock *dotBlock = new DotBlock;
            if(symbol.has_value()) {
                dotBlock->symbolEntry = symbol.value();
            }
            dotBlock->argument = new HexArgument(value);
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

DotBurn *MacroAssembler::parseBURN(std::optional<QSharedPointer<SymbolEntry> > symbol, ModuleInstance &instance, QString &errorMessage)
{
    if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_HEX_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        tokenString.remove(0, 2); // Remove "0x" prefix.
        bool ok;
        int value = tokenString.toInt(&ok, 16);
        if (value < 65536) {
            DotBurn *dotBurn = new DotBurn;
            if(symbol.has_value()) {
                dotBurn->symbolEntry = symbol.value();
            }
            dotBurn->argument = new HexArgument(value);
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

DotByte *MacroAssembler::parseBYTE(std::optional<QSharedPointer<SymbolEntry> > symbol, ModuleInstance&, QString &errorMessage)
{
    bool ok;
    // This block of code was repeated in every if() statement,
    // While this might lead to some unecessary deletes, the
    // number of errors that occur on a .BYTE directive should be
    // minimal.
    DotByte *dotByte = new DotByte;
    if(symbol.has_value()) {
        dotByte->symbolEntry = symbol.value();
    }

    if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_CHAR_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        DotByte *dotByte = new DotByte;
        dotByte->argument = new CharArgument(tokenString);
    }
    else if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_DEC_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        int value = tokenString.toInt(&ok, 10);
        if ((-128 <= value) && (value <= 255)) {
            if (value < 0) {
                value += 256; // value stored as one-byte unsigned.
            }
            dotByte->argument = new DecArgument(value);
        }
        else {
            errorMessage = ";ERROR: Decimal constant is out of byte range (-128..255).";
            delete dotByte;
            return nullptr;
        }
    }
    else if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_HEX_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        tokenString.remove(0, 2); // Remove "0x" prefix.
        int value = tokenString.toInt(&ok, 16);
        if (value < 256) {
            DotByte *dotByte = new DotByte;
            dotByte->argument = new HexArgument(value);
        }
        else {
            errorMessage = ";ERROR: Hex constant is out of byte range (0x00..0xFF).";
            delete dotByte;
            return nullptr;
        }
    }
    else if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_STRING_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        if (IsaParserHelper::byteStringLength(tokenString) > 1) {
            errorMessage = ";ERROR: .BYTE string operands must have length one.";
            delete dotByte;
            return nullptr;
        }
        dotByte->argument = new StringArgument(tokenString);
    }
    else {
        errorMessage = ";ERROR: .BYTE requires a char, dec, hex, or string constant argument.";
        delete dotByte;
        return nullptr;
    }
    return dotByte;
}

DotEnd *MacroAssembler::parseEND(std::optional<QSharedPointer<SymbolEntry> > symbol, ModuleInstance&, QString &errorMessage)
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

    return new DotEnd;
}

DotEquate *MacroAssembler::parseEQUATE(std::optional<QSharedPointer<SymbolEntry> > symbol, ModuleInstance&, QString &errorMessage)
{
    if (!symbol.has_value()) {
        errorMessage = ";ERROR: .EQUATE must have a symbol definition.";
        return nullptr;
    }

    bool ok;
    // This block of code was repeated in every if() statement,
    // While this might lead to some unecessary deletes, the
    // number of errors that occur on a .EQUATE directive should be
    // minimal.
    DotEquate *dotEquate = new DotEquate;
    if(symbol.has_value()) {
        dotEquate->symbolEntry = symbol.value();
    }
    if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_DEC_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        int value = tokenString.toInt(&ok, 10);
        if ((-32768 <= value) && (value <= 65535)) {

            if (value < 0) {
                value += 65536; // Stored as two-byte unsigned.
                dotEquate->argument = new DecArgument(value);
            }
            else {
                dotEquate->argument = new UnsignedDecArgument(value);
            }
            dotEquate->symbolEntry->setValue(QSharedPointer<SymbolValueNumeric>::create(value));
        }
        else {
            errorMessage = ";ERROR: Decimal constant is out of range (-32768..65535).";
            delete dotEquate;
            return nullptr;
        }
    }
    else if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_HEX_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        tokenString.remove(0, 2); // Remove "0x" prefix.
        int value = tokenString.toInt(&ok, 16);
        if (value < 65536) {
            dotEquate->argument = new HexArgument(value);
            dotEquate->symbolEntry->setValue(QSharedPointer<SymbolValueNumeric>::create(value));
        }
        else {
            errorMessage = ";ERROR: Hexidecimal constant is out of range (0x0000..0xFFFF).";
            delete dotEquate;
            return nullptr;
        }
    }
    else if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_STRING_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        if (IsaParserHelper::byteStringLength(tokenString) > 2) {
            errorMessage = ";ERROR: .EQUATE string operand must have length at most two.";
            delete dotEquate;
            return nullptr;
        }
        dotEquate->argument = new StringArgument(tokenString);
        dotEquate->symbolEntry->setValue(QSharedPointer<SymbolValueNumeric>::create(IsaParserHelper::string2ArgumentToInt(tokenString)));
    }
    else if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_CHAR_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        dotEquate->argument = new CharArgument(tokenString);
        dotEquate->symbolEntry->setValue(QSharedPointer<SymbolValueNumeric>::create(IsaParserHelper::charStringToInt(tokenString)));
    }
    else {
        errorMessage = ";ERROR: .EQUATE requires a dec, hex, or string constant argument.";
        delete dotEquate;
        return nullptr;
    }
    return dotEquate;
}

bool MacroAssembler::parseEXPORT()
{
    return false;
}

bool MacroAssembler::parseSYCALL()
{
    return false;
}

bool MacroAssembler::parseUSYCALL()
{
    return false;
}

DotWord *MacroAssembler::parseWORD(std::optional<QSharedPointer<SymbolEntry> > symbol, ModuleInstance&, QString &errorMessage)
{
    bool ok;
    // This block of code was repeated in every if() statement,
    // While this might lead to some unecessary deletes, the
    // number of errors that occur on a .WORD directive should be
    // minimal.
    DotWord *dotWord = new DotWord;
    if(symbol.has_value()) {
        dotWord->symbolEntry = symbol.value();
    }

    if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_CHAR_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        dotWord->argument = new CharArgument(tokenString);
    }
    else if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_DEC_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        int value = tokenString.toInt(&ok, 10);
        if ((-32768 <= value) && (value < 65536)) {
            if (value < 0) {
                value += 65536; // Stored as two-byte unsigned.
                dotWord->argument = new DecArgument(value);
            }
            else {
                dotWord->argument = new UnsignedDecArgument(value);
            }
        }
        else {
            errorMessage = ";ERROR: Decimal constant is out of range (-32768..65535).";
            delete dotWord;
            return nullptr;
        }
    }
    else if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_HEX_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        tokenString.remove(0, 2); // Remove "0x" prefix.
        int value = tokenString.toInt(&ok, 16);
        if (value < 65536) {
            dotWord->argument = new HexArgument(value);
        }
        else {
            errorMessage = ";ERROR: Hexidecimal constant is out of range (0x0000..0xFFFF).";
            delete dotWord;
            return nullptr;
        }
    }
    else if (tokenBuffer->match(MacroTokenizerHelper::ELexicalToken::LT_STRING_CONSTANT)) {
        QString tokenString = tokenBuffer->takeLastMatch().second.toString();
        if (IsaParserHelper::byteStringLength(tokenString) > 2) {
            errorMessage = ";ERROR: .WORD string operands must have length at most two.";
            delete dotWord;
            return nullptr;
        }
        dotWord->argument = new StringArgument(tokenString);
    }
    else {
        errorMessage = ";ERROR: .WORD requires a char, dec, hex, or string constant argument.";
        delete dotWord;
        return nullptr;
    }
    return dotWord;
}
