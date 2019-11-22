#ifndef MACROLINKER_H
#define MACROLINKER_H
#include "macromodules.h"
struct LinkResult
{
    bool success;
    QList<ErrorInfo> errorList;
};

class MacroLinker
{
public:
    MacroLinker();
    ~MacroLinker();
    // If LinkResult.success == false, then errorList contains
    // a list of lines in the rootModule that failed to link. This
    // could occur because of multiple defined symbols, undefined symbol
    // references, or failures to link in external macros.
    //
    // If the result is successful, then every line of code in every module instance
    // has had its symbols checked and is prepared for code generation.
    LinkResult link(ModuleAssemblyGraph& graph);

    void setOSSymbolTable(QSharedPointer<const SymbolTable> OSSymbolTable);
    void setForceBurnAt0xFFFF(bool forceBurn0xFFFF);
    void clearOSSymbolTable();
private:
    // Pull in modules from operating system.
    LinkResult pullInExports(ModuleAssemblyGraph &graph);
    // Iteratre through the code in instance and assign address & symbol values.
    // When a macro is hit, the macro is copied and emplace in newMap, the module's
    // prototype's line-to-instance mapping is updated, and the macro line's instance is updated.
    // This copy is performed so that multiple macro invocations (with different addresses)
    // may share the same (unadjusted) code list.
    LinkResult linkModule(ModuleAssemblyGraph graph, ModuleAssemblyGraph::InstanceMap& newMap,
                          ModuleInstance& instance);
    // Pre: instance contains a valid program code listing.
    // Post: The address of every line of code in codeList is increased by addressDelta
    void relocateCode(ModuleInstance& instance, quint16 addressDelta);
    // Adjust the addresses of all code lines to allow for burning in of operating system.
    bool shiftForBURN(ModuleAssemblyGraph& graph);

    bool overflowedMemory;
    quint32 nextSourceLine;
    quint32 nextAddress;
    bool forceBurn0xFFFF;
    std::optional<QSharedPointer<const SymbolTable>> osSymbolTable;

public:
    static const inline QString multidefinedSymbol = ";ERROR: Symbol \"%1\" was previously defined.";
    static const inline QString redefineExportSymbol = ";ERROR: Attempted to redefine symbol\"%1\", which was exported from the operating system.";
    static const inline QString undefinedSymbol = ";ERROR: Symbol \"%1\" is undefined.";
    static const inline QString noBURN = ";ERROR: Only operating systems may contain a .BURN.";
    static const inline QString oneBURN = ";ERROR: Operating systems must contain exactly 1 .BURN.";
    static const inline QString BURNat0xFFFF = ";ERROR: .BURN must have an argument of 0xFFFF.";
    static const inline QString noOperatingSystem = ";ERROR: Attempted to pull in symbols for operating system, but no OS was defined.";
    static const inline QString exceededMemory = ";ERROR: Program requires more than 64k bytes of memory.";
};

#endif // MACROLINKER_H
