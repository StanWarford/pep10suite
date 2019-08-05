#ifndef MACROSTACKANNOTATER_H
#define MACROSTACKANNOTATER_H
#include "macromodules.h"
#include "asmcode.h"
enum class AnnotationSucces
{
    SUCCESS,
    SUCCESS_WITH_WARNINGS,
    FAILURE
};

struct StackAnnotationResult
{
    AnnotationSucces success;
    // Warnings that indicate bad style / possible misbehavior, but not critically wrong
    QList<ErrorInfo> warningList;
    // List of errors that should fatally prevent program assembly.
    QList<ErrorInfo> errorList;
};
struct StackModifyingLine
{
    int rootLineIdx;
    QSharedPointer<AsmCode> line;
};

class MacroStackAnnotater
{
    QList<StackModifyingLine> unresolvedModifyingLines;
    QList<StackModifyingLine> resolvedModifyingLines;
    QMap<QString, void*> parsedSymbols;
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
    void* containsHint(...) const;
    void* containsTypeTag(...) const;
    void* containsSymbolTag(...) const;
private:
    void discoverLines(ModuleInstance& instance);
    void resolveLines();
    bool parseLine(StackModifyingLine line);
    bool parseBlock(void *);
    bool parseByte(void *);
    bool parseEquate(void *);
    bool parseWord(void *);
    bool parseUnary(void *);
    bool parseNonunary(void *);
};

#endif // MACROSTACKANNOTATER_H
