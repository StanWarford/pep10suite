#include "macrotokenizer.h"
// Regular expressions for lexical analysis
//const QRegularExpression MacroTokenizerHelper::addrMode("((,)(\\s*)(i|d|x|n|s(?![fx])|sx(?![f])|sf|sfx){1}){1}");
const QRegularExpression init(QString string)
{
    QRegularExpression regEx(string);
    // Ignore case in our regular expressions.
    regEx.setPatternOptions(regEx.patternOptions() | QRegularExpression::CaseInsensitiveOption);
    return regEx;
}
// For addressing modes starting with s, use negative lookaheads to ensure
// that the longest possible addressing is matched.
const QRegularExpression MacroTokenizerHelper::addrMode = init("(i|d|x|n|s(?![fx])|sx(?!f)|sf(?!x)|sfx)\\s*");
const QRegularExpression MacroTokenizerHelper::charConst("((\')(?![\'])(([^\'\\\\]){1}|((\\\\)([\'|b|f|n|r|t|v|\"|\\\\]))|((\\\\)(([x|X])([0-9|A-F|a-f]{2}))))(\'))");
const QRegularExpression MacroTokenizerHelper::comment = init(";.*");
const QRegularExpression MacroTokenizerHelper::decConst = init("[+|-]{0,1}[0-9]+\\s*");
const QRegularExpression MacroTokenizerHelper::dotCommand = init("\\.[a-zA-Z]\\w*\\s*");
const QRegularExpression MacroTokenizerHelper::hexConst = init("0[xX][0-9a-fA-F]+\\s*");
const QRegularExpression MacroTokenizerHelper::identifier = init("[A-Z|a-z|_]\\w*(:){0,1}\\s*");
const QRegularExpression MacroTokenizerHelper::stringConst("((\")((([^\"\\\\])|((\\\\)([\'|b|f|n|r|t|v|\"|\\\\]))|((\\\\)(([x|X])([0-9|A-F|a-f]{2}))))*)(\"))");
const QRegularExpression MacroTokenizerHelper::macroInvocation = init("@[A-Z|a-z|_]{1}(\\w*)\\s*");
const QRegularExpression MacroTokenizerHelper::macroSubstitution = init("\\$\\d+\\s*");
// Regular expressions for trace tag analysis
const QRegularExpression MacroTokenizerHelper::rxFormatTag("(#((1c)|(1d)|(1h)|(2d)|(2h))((\\d)+a)?(\\s|$))");
const QRegularExpression MacroTokenizerHelper::rxArrayTag("(#((1c)|(1d)|(1h)|(2d)|(2h))(\\d)+a)(\\s|$)?");
const QRegularExpression MacroTokenizerHelper::rxSymbolTag("#[a-zA-Z][a-zA-Z0-9]{0,7}");
const QRegularExpression MacroTokenizerHelper::rxArrayMultiplier("((\\d)+)a");

// Formats for trace tag error messages
const QString bytesAllocMismatch = ";WARNING: Number of bytes allocated (%1) not equal to number of bytes listed in trace tag (%2).";
const QString badTag = ";WARNING: %1 not specified in .EQUATE";
const QString neSymbol(";WARNING: Looked up a symbol that does not exist: %1");
const QString noEquate(";WARNING: Looked for existing symbol not defined in .EQUATE: %1");
const QString noSymbol(";WARNING: Trace tag with no symbol declaration");
const QString illegalAddrMode(";WARNING: Stack trace not possible unless immediate addressing is specified.");

bool MacroTokenizerHelper::startsWithHexPrefix(QStringRef str)
{
    if (str.length() < 2) return false;
    if (str[0] != '0') return false;
    if (str[1] == 'x' || str[1] == 'X') return true;
    return false;
}

Enu::EAddrMode MacroTokenizerHelper::stringToAddrMode(QString str)
{
    str.remove(0, 1); // Remove the comma.
    str = str.trimmed().toUpper();
    if (str == "I") return Enu::EAddrMode::I;
    if (str == "D") return Enu::EAddrMode::D;
    if (str == "N") return Enu::EAddrMode::N;
    if (str == "S") return Enu::EAddrMode::S;
    if (str == "SF") return Enu::EAddrMode::SF;
    if (str == "X") return Enu::EAddrMode::X;
    if (str == "SX") return Enu::EAddrMode::SX;
    if (str == "SFX") return Enu::EAddrMode::SFX;
    return Enu::EAddrMode::NONE;
}

void MacroTokenizerHelper::unquotedStringToInt(QString &str, int &value)
{
    QString s;
    if (str.startsWith("\\x") || str.startsWith("\\X")) {
        str.remove(0, 2); // Remove the leading \x or \X
        s = str.left(2);
        str.remove(0, 2); // Get the two-char hex number
        bool ok;
        value = s.toInt(&ok, 16);
    }
    else if (str.startsWith("\\")) {
        str.remove(0, 1); // Remove the leading bash
        s = str.left(1);
        str.remove(0,1);
        if (s == "b") { // backspace
            value = 8;
        }
        else if (s == "f") { // form feed
            value = 12;
        }
        else if (s == "n") { // line feed (new line)
            value = 10;
        }
        else if (s == "r") { // carriage return
            value = 13;
        }
        else if (s == "t") { // horizontal tab
            value = 9;
        }
        else if (s == "v") { // vertical tab
            value = 11;
        }
        else {
            value = QChar(s[0]).toLatin1();
        }
    }
    else {
        s = str.left(1);
        str.remove(0, 1);
        value = QChar(s[0]).toLatin1();
    }
    value += value < 0 ? 256 : 0;
}

int MacroTokenizerHelper::byteStringLength(QString str)
{
    str.remove(0, 1); // Remove the leftmost double quote.
    str.chop(1); // Remove the rightmost double quote.
    int length = 0;
    while (str.length() > 0) {
        if (str.startsWith("\\x") || str.startsWith("\\X")) {
            str.remove(0, 4); // Remove the \xFF
        }
        else if (str.startsWith("\\")) {
            str.remove(0, 2); // Remove the quoted character
        }
        else {
            str.remove(0, 1); // Remove the single character
        }
        length++;
    }
    return length;
}

MacroTokenizer::MacroTokenizer()
{

}

MacroTokenizer::~MacroTokenizer()
{

}

bool MacroTokenizer::getToken(QString &sourceLine, int& offset, MacroTokenizerHelper::ELexicalToken &token,
                              QStringRef &tokenString, QString &errorString)
{
    using namespace MacroTokenizerHelper;
    if (offset >= sourceLine.length()) {
        token = ELexicalToken::LT_EMPTY;
        tokenString = QStringRef();
        return true;
    }

    QChar firstChar = sourceLine[offset];
    if (firstChar == '$') {
        auto match = macroSubstitution.match(sourceLine, offset);
        if (!match.hasMatch()) {
            token = ELexicalToken::LTE_ERROR;
            errorString = ";ERROR: Malformed macro substitution.";
            return false;
        }
    }
    // Works!
    if (firstChar == '@') {
        auto match = macroInvocation.match(sourceLine, offset);
        if (!match.hasMatch()) {
            token = ELexicalToken::LTE_ERROR;
            errorString = ";ERROR: Malformed macro invocation.";
        }
        token = ELexicalToken::LTE_MACRO_INVOKE;
        int startIdx = match.capturedStart();
        int len = match.capturedLength();
        tokenString = QStringRef(&sourceLine, startIdx + 1, len - 1).trimmed();
        offset += len;
        return true;
    }
    // Works!
    if (firstChar == ',') {
        auto match = addrMode.match(sourceLine, offset);
        if (!match.hasMatch()) {
            token = ELexicalToken::LTE_ERROR;
            errorString = ";ERROR: Malformed addressing mode.";
            return false;
        }
        token = ELexicalToken::LT_ADDRESSING_MODE;
        int startIdx = match.capturedStart();
        int len = match.capturedLength();;
        tokenString = QStringRef(&sourceLine, startIdx, len).trimmed();
        // Must move offset forward one extra character to account for ,
        offset += len + 1;
        return true;
    }
    if (firstChar == '\'') {
        auto match = charConst.match(sourceLine, offset);
        if (!match.hasMatch()) {
            token = ELexicalToken::LTE_ERROR;
            errorString = ";ERROR: Malformed character constant.";
            return false;
        }
        token = ELexicalToken::LT_CHAR_CONSTANT;
        int startIdx = match.capturedStart();
        int len = match.capturedLength();;
        tokenString = QStringRef(&sourceLine, startIdx, len).trimmed();
        offset += len;
        return true;
    }
    if (firstChar == ';') {
        auto match = comment.match(sourceLine, offset);
        if (!match.hasMatch()) {
            // This error should not occur, as any characters are allowed in a comment.
            token = ELexicalToken::LTE_ERROR;
            errorString = ";ERROR: Malformed comment";
            return false;
        }
        token = ELexicalToken::LT_COMMENT;
        int startIdx = match.capturedStart();
        int len = match.capturedLength();;
        tokenString = QStringRef(&sourceLine, startIdx, len).trimmed();
        offset += len;
        return true;
    }
    if (startsWithHexPrefix(sourceLine.midRef(offset))) {
        auto match = hexConst.match(sourceLine, offset);
        if (!match.hasMatch()) {
            token = ELexicalToken::LTE_ERROR;
            errorString = ";ERROR: Malformed hex constant.";
            return false;
        }
        token = ELexicalToken::LT_HEX_CONSTANT;
        int startIdx = match.capturedStart();
        int len = match.capturedLength();;
        tokenString = QStringRef(&sourceLine, startIdx, len).trimmed();
        offset += len;
        return true;
    }
    // Works!
    if ((firstChar.isDigit() || firstChar == '+' || firstChar == '-')) {
        auto match = decConst.match(sourceLine, offset);
        if (!match.hasMatch()) {
            token = ELexicalToken::LTE_ERROR;
            errorString = ";ERROR: Malformed decimal constant.";
            return false;
        }
        token = ELexicalToken::LT_DEC_CONSTANT;
        int startIdx = match.capturedStart();
        int len = match.capturedLength();;
        tokenString = QStringRef(&sourceLine, startIdx, len).trimmed();
        offset += len;
        return true;
    }
    if (firstChar == '.') {
        auto match = dotCommand.match(sourceLine, offset);
        if (!match.hasMatch()) {
            token = ELexicalToken::LTE_ERROR;
            errorString = ";ERROR: Malformed dot command.";
            return false;
        }
        token = ELexicalToken::LT_DOT_COMMAND;
        int startIdx = match.capturedStart();
        int len = match.capturedLength();;
        tokenString = QStringRef(&sourceLine, startIdx, len).trimmed();
        offset += len;
        return true;
    }
    // Works!
    if (firstChar.isLetter() || firstChar == '_') {
        auto match = identifier.match(sourceLine, offset);
        if (!match.hasMatch()) {
            token = ELexicalToken::LTE_ERROR;
            // This error should not occur, as one-character identifiers are valid.
            errorString = ";ERROR: Malformed identifier.";
            return false;
        }

        int startIdx = match.capturedStart();
        int len = match.capturedLength();;
        tokenString = QStringRef(&sourceLine, startIdx, len).trimmed();
        token = tokenString.contains(':') ?
                    ELexicalToken::LT_SYMBOL_DEF :
                    ELexicalToken::LT_IDENTIFIER;
        offset += len;
        return true;
    }
    if (firstChar == '\"') {
        auto match = stringConst.match(sourceLine, offset);
        if (!match.hasMatch()) {
            token = ELexicalToken::LTE_ERROR;
            errorString = ";ERROR: Malformed string constant.";
            return false;
        }
        token = ELexicalToken::LT_STRING_CONSTANT;
        int startIdx = match.capturedStart();
        int len = match.capturedLength();;
        tokenString = QStringRef(&sourceLine, startIdx, len).trimmed();
        offset += len;
        return true;
    }
    token = ELexicalToken::LTE_ERROR;
    errorString = ";ERROR: Syntax error.";
    return false;
}

void MacroTokenizer::setMacroSubstitutions(QStringList macroSubstitution)
{
    this->macroSubstitutions = macroSubstitution;
}

void MacroTokenizer::performMacroSubstitutions(QString &sourceLine)
{
    for(int it = 0; it < macroSubstitutions.size(); it++) {
        QString argName = QString("\\$%1").arg(it + 1);
        sourceLine.replace(QRegularExpression(argName), macroSubstitutions[it]);
    }

}


TokenizerBuffer::TokenizerBuffer(): tokenizer(new MacroTokenizer())
{

}

TokenizerBuffer::~TokenizerBuffer()
{
    delete tokenizer;
}

void TokenizerBuffer::setMacroSubstitutions(QStringList args)
{
    tokenizer->setMacroSubstitutions(args);
}

void TokenizerBuffer::clearMacroSubstitutions()
{
    tokenizer->setMacroSubstitutions({});
}

void TokenizerBuffer::setTokenizerInput(QStringList lines)
{
    tokenizerInput.resize(lines.size());
    int index = 0;
    for(auto line : lines) {
        // ALways purge whitespace.
        tokenizerInput[index] = line.trimmed();
        index++;
    }
    inputIterator = 0;
    backedUpInput.clear();
    matches.clear();
}

bool TokenizerBuffer::inputRemains()
{
    return (inputIterator < tokenizerInput.size()) || !backedUpInput.isEmpty();
}

bool TokenizerBuffer::match(MacroTokenizerHelper::ELexicalToken token)
{
    if(backedUpInput.isEmpty()) {
        fetchNextLine();
    }
    if(backedUpInput.front().first == token) {
        matches.append(backedUpInput.front());
        backedUpInput.pop_front();
        return true;
    }
    else {
        return false;
    }
}

bool TokenizerBuffer::matchOneOf(QList<MacroTokenizerHelper::ELexicalToken> tokenList)
{
    if(backedUpInput.isEmpty()) {
        fetchNextLine();
    }
    auto backedUpToken = backedUpInput.front().first;
    for(auto token : tokenList) {
        if(backedUpToken == token) {
            matches.append(backedUpInput.front());
            backedUpInput.pop_front();
            return true;
        }
    }
    return false;
}

bool TokenizerBuffer::lookahead(MacroTokenizerHelper::ELexicalToken token)
{
    if(backedUpInput.isEmpty()) {
        fetchNextLine();
    }
    if(backedUpInput.front().first == token) {
        return true;
    }
    else {
        return false;
    }
}

QPair<MacroTokenizerHelper::ELexicalToken, QStringRef> TokenizerBuffer::takeLastMatch()
{
    if(matches.isEmpty()) {
        return {MacroTokenizerHelper::ELexicalToken::LT_EMPTY, QStringRef()};
    }
    else {
        auto retVal = matches.first();
        matches.pop_front();
        return retVal;
    }

}

QList<QPair<MacroTokenizerHelper::ELexicalToken, QStringRef> > TokenizerBuffer::getMatches()
{
    return matches;
}

bool TokenizerBuffer::skipNextLine()
{
    inputIterator += 1;
    return inputRemains();
}

void TokenizerBuffer::clearMatchBuffer()
{
    matches.clear();
}

void TokenizerBuffer::fetchNextLine()
{
    MacroTokenizerHelper::ELexicalToken token;
    QStringRef tokenString;
    QList<QPair<MacroTokenizerHelper::ELexicalToken, QStringRef>> newTokens;
    int offset = 0;
    bool hadMacroInvoke = false;
    if(inputIterator < tokenizerInput.size()) tokenizerInput[inputIterator].trimmed();
    // Only need to perform macro substitutions once per line.
    tokenizer->performMacroSubstitutions(tokenizerInput[inputIterator]);
    while(token != MacroTokenizerHelper::ELexicalToken::LT_EMPTY) {

        if(!tokenizer->getToken(tokenizerInput[inputIterator], offset, token, tokenString, this->errorMessage)) {
            // If a new line has an error on it, the error is the only
            // thing that will be reported. This means we don't have to search for errors
            // on every match.
            tokenString = QStringRef(&this->errorMessage);
            qDebug().noquote() << token << tokenString;
            backedUpInput.append({token, tokenString});
            break;
        }
        if(token == MacroTokenizerHelper::ELexicalToken::LTE_MACRO_INVOKE) {
            hadMacroInvoke = true;
        }

        if(hadMacroInvoke) {
            if(tokenizerInput[inputIterator][offset] == ",") {
                ++offset;
            }
        }
        // qDebug().noquote() << token << tokenString;
        newTokens.append({token, tokenString});
    }
    backedUpInput.append(newTokens);
    ++inputIterator;
}
