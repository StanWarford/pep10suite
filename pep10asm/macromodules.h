#ifndef MACROMODULES_H
#define MACROMODULES_H

#include <QtCore>
#include <optional>
#include "ngraph.h"
#include "symboltable.h"
#include "asmcode.h"

// Track the line number an contents of an error message.
using ErrorInfo = std::tuple<int /*line*/, QString /*errMessage*/> ;

/*
 * Different file types allow for differnt compiling options,
 * so a module must track what type of file it is.
 *
 * For example, a user program or macro shall not contain a .BURN,
 * but an operating system requires it.
 */
enum ModuleType {
    MACRO,
    OPERATING_SYSTEM,
    USER_PROGRAM,
};

// Information collected during assembly stage
// about the number and value of a .BURN directive.
struct MacroBurnInfo
{
    /*
     * Information filled in during assembly.
     */
    // The number of burn statements that occured in the program
    int burnCount = 0;
    // If burnCount == 1, stores the argument passed to the .BURN
    int burnArgument;

    /*
     * Information filled in during linking.
     */
    // What should the first byte of the ROM be?
    int startROMAddress;
    // Store the byte offset from the start of the program
    // to the .BURN directive.
    int burnAddress;
};

/*
 * A module represents a single Pep10 assembler file.
 *
 * In Pep9, only one program would ever be evaluated or assembled
 * at a time. With Pep10's macro facility, a single user program may
 * include many macros, which in turn include many macros.
 *
 * A single translation unit (a file) is a module, and an executable
 * program is composed of several modules.
 *
 * Modules are delineated between prototypes and instances. A Prototype
 * is the macro as defined by the assembly file on the hard disk, macro
 * substitutions ($1, $2, etc.) included. An Instance is generated when
 * a macro is invoked (e.g. %DECO num,d), and tracks the arguments that
 * must be substitued into the Prototype.
 *
 * As the Instance (after substitution occurs) represents a real assembler file,
 * all assembly are performed on the Instance.
 *
 */
struct ModuleInstance;
/*
 * Module prototype MUST NOT have an owning smart pointer to child instances,
 * as the instances having an owning pointer to the prototype.
 * Adding an owning pointer would cause memory to not be freed, causing a memory leak.
 */
struct ModulePrototype {
    /*
     * Information that must be filled in before preprocessing.
     */
    // The index in the lookup graph.
    quint16 index = 0xffff;
    // Name by which the module can be looked up. For macros,
    // this is the name as reproted by the MacroRegistry. For user
    // programs and uperating systems, it may be anything starting and
    // ending with @@ (@@main@@ is valid, main@@ is not).
    QString name;
    // What is the program text (macro substitutions included) of the moudle?
    QString text;
    ModuleType moduleType;
    // Break the program text at newlines (keeping empty lines)
    // to save from calling text.split("\n") multiple times.
    QStringList textLines;

    /*
     * Information filled in by preprocessor.
     */
    QMap<quint16, ModuleInstance*> lineToInstance;
};


struct ModuleInstance {
    ModuleInstance() = default;
    // Perform a deep copy instead of default shallow copy.
    ModuleInstance(const ModuleInstance& other);
    ~ModuleInstance();
    ModuleInstance& operator=(ModuleInstance rhs);

    // Errors may be raised on any line at any step.
    QList<ErrorInfo> errorList;

    /*
     * Information that must be filled in before preprocessing.
     */
    // Keep parent prototype alive for the lifetime of all child instances.
    QSharedPointer<ModulePrototype> prototype;

    /*
     * Information filled in by preprocessor.
     */
    // May be empty if the module is not a macro type.
    QStringList macroArgs;

    /*
     * Information filled in during assembly.
     */
    bool alreadyAssembled;
    QList<QSharedPointer<AsmCode>> codeList;
    // Information about the presence & value of a .BURN directive.
    MacroBurnInfo burnInfo;
    // All module instances share the same symbol table.
    QSharedPointer<SymbolTable> symbolTable;

    /*
     * Information filled in during linking.
     */
    bool alreadyLinked;

    /*
     * Information filled in during annotation.
     */
    int traceInfo;
    // Information filled in during validation.

    // Necessary for reducing code duplication on copy assignment.
    friend void swap(ModuleInstance& first, ModuleInstance&second)
    {
        using std::swap;
        swap(first.burnInfo, second.burnInfo);
        swap(first.codeList, second.codeList);
        swap(first.errorList, second.errorList);
        swap(first.macroArgs, second.macroArgs);
        swap(first.prototype, second.prototype);
        swap(first.traceInfo, second.traceInfo);
        swap(first.symbolTable, second.symbolTable);
        swap(first.alreadyLinked, second.alreadyLinked);
        swap(first.alreadyAssembled, second.alreadyAssembled);
    }
};

/*
 * The ModuleAssemblyGraph tracks which modules are included
 * by other modules using a DiGraph.
 */
struct ModuleAssemblyGraph
{
    // Map which macros use other macros.
    NGraph::tGraph<quint16> moduleGraph;
    // Map the ID of a vertex to its prototype.
    QMap<quint16, QSharedPointer<ModulePrototype>> prototypeMap;
    // For a given ID, log every separate instance of the prototype.
    using InstanceMap = QMap<quint16, QList<QSharedPointer<ModuleInstance>>>;
    InstanceMap instanceMap;
    // By convention the root module is 0, but it would be best to have
    // it be an explicit field.
    quint16 rootModule = defaultRootIndex;
    static const quint16 defaultRootIndex = 0;

    // Attempt to find a module prototype with the same name as macroName.
    // It will ignore case, and return a valid index in the module graph if
    // the name is found.
    // If the name is not found, 0xFFFF will be returned instead.
    quint16 getIndexFromName(QString macroName) const;
    // Given a module index, from the line of code in module which
    // references that index. Will not recurse through children.
    // Returns 0 if index was not referenced in module.
    quint16 getLineFromIndex(ModulePrototype& module, quint16 index) const;
    // Check the instance map for a instance with equivilant macro arguments (ignoring case).
    std::optional<QSharedPointer<ModuleInstance>> getInstanceFromArgs(quint16 index, QStringList args) const;
    // Allow easy creation of root, since the root module has no dependencies.
    // If the root has already been created, then the function will return nullptr's.
    std::tuple<QSharedPointer<ModulePrototype>, QSharedPointer<ModuleInstance>> createRoot(QString text, ModuleType type);
    // Helper functions to build macro modules are not included, since the macro
    // modules may suffer from complex errors during construction.
    // Construction of other modules should be handled by preprocessor.
};

#endif // MACROMODULES_H
