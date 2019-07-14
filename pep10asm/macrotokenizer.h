#ifndef MACROTOKENIZER_H
#define MACROTOKENIZER_H

#include <QString>
#include <QtCore>
#include "enu.h"
#include <optional>
#include <QObject>

namespace MacroTokenizerHelper
{
    Q_NAMESPACE;
    // Lexical tokens
    enum class ELexicalToken
    {
        LT_ADDRESSING_MODE, LT_CHAR_CONSTANT, LT_COMMENT, LT_DEC_CONSTANT, LT_DOT_COMMAND,
        LT_EMPTY, LT_HEX_CONSTANT, LT_IDENTIFIER, LT_STRING_CONSTANT, LT_SYMBOL_DEF,
        LTE_MACRO_INVOKE, LTE_MACRO_SUBSTITUTION, LTE_ERROR
    };
    Q_ENUM_NS(ELexicalToken);
    // Parser state for the assembler FSM
    enum class ParseState
    {
        PS_ADDRESSING_MODE, PS_CLOSE, PS_COMMENT, PS_DOT_ADDRSS, PS_DOT_ALIGN, PS_DOT_ASCII,
        PS_DOT_BLOCK, PS_DOT_BURN, PS_DOT_BYTE, PS_DOT_END, PS_DOT_EQUATE, PS_DOT_WORD,
        PS_FINISH, PS_INSTRUCTION, PS_START, PS_STRING, PS_SYMBOL_DEF
    };

    // Regular expressions for lexical analysis
    extern const QRegularExpression addrMode;
    extern const QRegularExpression charConst;
    extern const QRegularExpression comment;
    extern const QRegularExpression decConst;
    extern const QRegularExpression dotCommand;
    extern const QRegularExpression hexConst;
    extern const QRegularExpression identifier;
    extern const QRegularExpression stringConst;
    extern const QRegularExpression macroInvocation;
    extern const QRegularExpression macroSubstitution;

    // Regular expressions for trace tag analysis
    extern const QRegularExpression rxFormatTag;
    extern const QRegularExpression rxSymbolTag;
    extern const QRegularExpression rxArrayMultiplier;
    extern const QRegularExpression rxArrayTag;

    bool startsWithHexPrefix(QString str);
    // Post: Returns true if str starts with the characters 0x or 0X. Otherwise returns false.

    Enu::EAddrMode stringToAddrMode(QString str);
    // Post: Returns the addressing mode integer defined in Pep from its string representation.

    int charStringToInt(QString str);
    // Pre: str is enclosed in single quotes.
    // Post: Returns the ASCII integer value of the character accounting for \ quoted characters.

    int string2ArgumentToInt(QString str);
    // Pre: str is enclosed in double quotes and contains at most two possibly quoted characters.
    // Post: Returns the two-byte ASCII integer value for the string.

    void unquotedStringToInt(QString &str, int &value);
    // Pre: str is a character or string stripped of its single or double quotes.
    // Post: The sequence of characters representing the first possibly \ quoted character
    // is stripped from the beginning of str.
    // Post: value is the ASCII integer value of the first possibly \ quoted character.

    int byteStringLength(QString str);
    // Pre: str is a double quoted string.
    // Post: Returns the byte length of str accounting for possibly \ quoted characters.

}

/*
 * A tokenizer that mostly mimics the behavior of Pep9, but if the macro substitutions are set,
 * it will replace any macro substitutions in a source line before tokenizing it.
 * Choosing to handle macro substitutions here allows the assembler to be entirely unaware of
 * the macro framework.
 */
class MacroTokenizer
{
public:
    MacroTokenizer();
    ~MacroTokenizer();
    bool getToken(QString &sourceLine, int& offset, MacroTokenizerHelper::ELexicalToken &token, QStringRef &tokenString, QString& errorString);
    // Replace preprocessor tokens ($1 $2 $3 etc) before evaluating through tokenizer.
    void setMacroSubstitutions(QStringList macroSubstitution);
    void performMacroSubstitutions(QString& sourceLine);

private:
    QStringList macroSubstitutions;

};

class TokenizerBuffer
{
    MacroTokenizer* tokenizer;
    QVector<QString> tokenizerInput;
    int inputIterator;
    QString errorMessage;
    QList<QPair<MacroTokenizerHelper::ELexicalToken, QStringRef>> matches;
    QList<QPair<MacroTokenizerHelper::ELexicalToken, QStringRef>> backedUpInput;

public:
    TokenizerBuffer();
    ~TokenizerBuffer();

    void setMacroSubstitutions(QStringList args);
    void clearMacroSubstitutions();
    void setTokenizerInput(QStringList lines);
    bool inputRemains();

    bool match(MacroTokenizerHelper::ELexicalToken);
    bool matchOneOf(QList<MacroTokenizerHelper::ELexicalToken>);
    bool lookahead(MacroTokenizerHelper::ELexicalToken);
    QPair<MacroTokenizerHelper::ELexicalToken, QStringRef> takeLastMatch();
    QList<QPair<MacroTokenizerHelper::ELexicalToken, QStringRef>> getMatches();
    bool skipNextLine();
    // Clear the match list.
    void clearMatchBuffer();
private:
    // Grab the next token and place it in backed up input.
    void fetchNextLine();

};

#endif // MACROTOKENIZER_H
