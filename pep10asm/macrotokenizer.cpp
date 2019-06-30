#include "macrotokenizer.h"
// Regular expressions for lexical analysis
const QRegularExpression MacroTokenizerHelper::rxAddrMode("^((,)(\\s*)(i|d|x|n|s(?![fx])|sx(?![f])|sf|sfx){1}){1}");
const QRegularExpression MacroTokenizerHelper::rxCharConst("^((\')(?![\'])(([^\'\\\\]){1}|((\\\\)([\'|b|f|n|r|t|v|\"|\\\\]))|((\\\\)(([x|X])([0-9|A-F|a-f]{2}))))(\'))");
const QRegularExpression MacroTokenizerHelper::rxComment("^((;{1})(.)*)");
const QRegularExpression MacroTokenizerHelper::rxDecConst("^((([+|-]{0,1})([0-9]+))|^(([1-9])([0-9]*)))");
const QRegularExpression MacroTokenizerHelper::rxDotCommand("^((.)(([A-Z|a-z]{1})(\\w)*))");
const QRegularExpression MacroTokenizerHelper::rxHexConst("^((0(?![x|X]))|((0)([x|X])([0-9|A-F|a-f])+)|((0)([0-9]+)))");
const QRegularExpression MacroTokenizerHelper::rxIdentifier("(([A-Z|a-z|_]{1})(\\w*))(:){0,1}");
const QRegularExpression MacroTokenizerHelper::rxStringConst("^((\")((([^\"\\\\])|((\\\\)([\'|b|f|n|r|t|v|\"|\\\\]))|((\\\\)(([x|X])([0-9|A-F|a-f]{2}))))*)(\"))");

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

MacroTokenizer::MacroTokenizer()
{

}
