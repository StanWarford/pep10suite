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
private:
    // Pull in modules from operating system.
    LinkResult pullInExports(ModuleAssemblyGraph &graph);
    // Iteratre through the code in instance and assign address & symbol values.
    // When a macro is hit, the macro is copied and emplace in newMap, the module's
    // prototype's line-to-instance mapping is updated, and the macro line's instance is updated.
    // This copy is performed so that multiple macro invocations (with different addresses)
    // may share the same (unadjusted) code list.
    LinkResult linkModule(ModuleAssemblyGraph::InstanceMap& newMap, ModuleInstance& instance);
    // Adjust the addresses of all code lines to allow for burning in of operating system.
    bool shiftForBURN(ModuleAssemblyGraph& graph);

    quint32 nextAddress;

};

#endif // MACROLINKER_H
