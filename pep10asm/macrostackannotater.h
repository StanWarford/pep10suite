#ifndef MACROSTACKANNOTATER_H
#define MACROSTACKANNOTATER_H
#include "macromodules.h"
#include "asmcode.h"
#include "typetags.h"
#include "asmprogram.h"

enum class AnnotationSucces
{
    SUCCESS,
    SUCCESS_WITH_WARNINGS,
    FAILURE
};

static inline QRegularExpression paramsHint =
        QRegularExpression("@params", QRegularExpression::PatternOption::CaseInsensitiveOption);
static inline QRegularExpression localsHint =
        QRegularExpression("@locals", QRegularExpression::PatternOption::CaseInsensitiveOption);
static inline QRegularExpression formatTag =
        QRegularExpression("#((1[cdh])|(2[dh]))\\s*", QRegularExpression::PatternOption::CaseInsensitiveOption);
static inline  QRegularExpression arrayTag =
        QRegularExpression("#((1[cdh])|(2[dh]))\\d+a\\s*", QRegularExpression::PatternOption::CaseInsensitiveOption);
static inline QRegularExpression arrayMultiplier =
        QRegularExpression("\\d+a", QRegularExpression::PatternOption::CaseInsensitiveOption);
static inline QRegularExpression symbolTag =
        QRegularExpression("#[A-Z|a-z|_]{1}\\w*", QRegularExpression::PatternOption::CaseInsensitiveOption);
struct StackAnnotationResult
{
    bool hadTraceTags;
    AnnotationSucces success;
    // Warnings that indicate bad style / possible misbehavior, but not critically wrong
    QList<ErrorInfo> warningList;
    // List of errors that should fatally prevent program assembly.
    QList<ErrorInfo> errorList;
};

class MacroStackAnnotater
{
    typedef QMap<QString, QPair<QSharedPointer<const SymbolEntry>, QSharedPointer<AType>>> TypeMap;
    QSharedPointer<SymbolTable> symbolTable;
    // Global lines are BYTE, BLOCK, or WORD lines.
    // Codelines are any asm instruction that modifies the stack,
    // such as RET, USCALL.
    QList<AsmCode*> globalLines, codeLines;
    QList<DotEquate*> equateLines;
    // Dynamic symbols (specified by .EQUATE) may appear on the heap, stack, or as part of a global struct.
    // they do not refer to physical addresses, but rather offsets from them. They may also be composed
    // of symbol trace tags, forming a struct.
    // Static symbols (specified by .BLOCK, .BYTE, .WORD) may only occur in globals.
    TypeMap dynamicSymbols, staticSymbols;
    StackAnnotationResult resultCache;
    bool hadAnyTraceTags;

public:
    MacroStackAnnotater();
    ~MacroStackAnnotater();
    // If LinkResult.success == false, then errorList contains
    // a list of lines in the rootModule that failed to link. This
    // could occur because of multiple defined symbols, undefined symbol
    // references, or failures to link in external macros.
    //
    // If the result is successful, then every line of code in every module instance
    // has had its symbols checked and is prepared for code generation.
    StackAnnotationResult annotateStack(ModuleAssemblyGraph& graph);
    static bool containsStackHint(QString comment);
    static bool containsFormatTag(QString comment);
    static bool containsSymbolTag(QString comment);
    static bool containsArrayType(QString comment);
    static bool containsPrimitiveType(QString comment);
    // Return the string representation of a present primitive / array type tag.
    static QString extractTypeTags(QString comment);
    static Enu::ESymbolFormat primitiveType(QString formatTag);
    // Pre: formatTag is a valid format trace tag.
    // Post: Returns the enumerated trace tag format type.
    static QPair<Enu::ESymbolFormat, quint8> arrayType(QString formatTag);
private:
    // If the line contains a list of symbol tags, return all present symbol tags
    static QStringList extractTagList(QString comment);
    // Recursively traverse module tree to find all lines of code that
    // interact with the stack, heap, or globals.
    void discoverLines(ModuleInstance& instance);
    void resolveGlobals();
    void resolveCodeLines();
    void parseEquateLines();
    void parseBlock(DotBlock*);
    void parseByte(DotByte*);
    void parseWord(DotWord*);
    // Helper that generates TraceCommands for pushing elements onto the global stack,
    // and then annotates the line of code with the list of commands.
    void pushGlobalHelper(AsmCode* globalLine, QList<QSharedPointer<AType>> items);
    QPair<QSharedPointer<StructType>,QString> parseStruct(QString name, QStringList symbols);
    // Attempt to create a type definition for a struct from a list of symbol tags.
    // Returns a pointer (possibly null) and an error message.
    // In the case of successful parsing, the pointer will be non-non, and the string will be empty.
    // If the pointer is null, then there exists an error message.
};

#endif // MACROSTACKANNOTATER_H
