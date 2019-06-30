#ifndef MACROTOKENIZER_H
#define MACROTOKENIZER_H

#include <QString>
#include <QtCore>
#include "enu.h"

namespace MacroTokenizerHelper
{
    // Lexical tokens
    enum ELexicalToken
    {
        LT_ADDRESSING_MODE, LT_CHAR_CONSTANT, LT_COMMENT, LT_DEC_CONSTANT, LT_DOT_COMMAND,
        LT_EMPTY, LT_HEX_CONSTANT, LT_IDENTIFIER, LT_STRING_CONSTANT, LT_SYMBOL_DEF
    };

    // Parser state for the assembler FSM
    enum ParseState
    {
        PS_ADDRESSING_MODE, PS_CLOSE, PS_COMMENT, PS_DOT_ADDRSS, PS_DOT_ALIGN, PS_DOT_ASCII,
        PS_DOT_BLOCK, PS_DOT_BURN, PS_DOT_BYTE, PS_DOT_END, PS_DOT_EQUATE, PS_DOT_WORD,
        PS_FINISH, PS_INSTRUCTION, PS_START, PS_STRING, PS_SYMBOL_DEF
    };

    // Regular expressions for lexical analysis
    extern const QRegularExpression rxAddrMode;
    extern const QRegularExpression rxCharConst;
    extern const QRegularExpression rxComment;
    extern const QRegularExpression rxDecConst;
    extern const QRegularExpression rxDotCommand;
    extern const QRegularExpression rxHexConst;
    extern const QRegularExpression rxIdentifier;
    extern const QRegularExpression rxStringConst;

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
    bool getToken(QString &sourceLine, int &token, QString &tokenString);
    // Replace preprocessor tokens ($1 $2 $3 etc) before evaluating through tokenizer.
    void setMacroArgs(QStringList macroSubstitution);
};

#endif // MACROTOKENIZER_H
