#ifndef MACROPREPROCESSOR_H
#define MACROPREPROCESSOR_H

#include <QtCore>
#include "macromodules.h"
#include "macroregistry.h"

class FrontEndError;
// Report whether the entire preprocessing stage
// was successful, or report what line in the main
// module caused (or included) a macro error.
struct PreprocessorResult
{
    bool succes;
    QSharedPointer<FrontEndError> error;
};

/*
 * The Macro preprocessor is responsible for discovering any macros
 * referenced in the root module of a ModuleAssemblyGraph, and
 * recursively discovering all referenced macros.
 *
 * It validates that all macro names/arguments are syntatically correct
 * (e.g. they are valid tokens) and semantically correct (macro invocations
 * do not use a macro substitution as part of their identifier or argument list).
 *
 * It treats macros as case insensitive, e.g. @AsRa4 will match against @ASRA4,
 * which is a core macro.
 */
class MacroPreprocessor
{
public:
    MacroPreprocessor(const MacroRegistry* registry);
    virtual ~MacroPreprocessor();
    // Give the preprocessor a graph to operate on.
    // This must be called before preprocess, or the
    // behavior is undefined.
    void setTarget(ModuleAssemblyGraph* target);
    // Given an already set target, recursively discover and
    // validate macros.
    virtual PreprocessorResult preprocess();

protected:
    // Keep track of a parsed macro invocation and the
    // source line from which it was invoked.
    struct MacroDefinition
    {
        quint16 lineNumber;
        QString macroName;
        QStringList macroArgs;
    };

    // A list of all macro invocations or
    // an error regarding macro invocation syntax.
    struct ExtractResult
    {
        bool syntaxError;
        ErrorInfo error;
        QList<MacroDefinition> moduleDefinitionList;
    };

    // Report correctness of macro invocation semantics.
    struct LinkResult
    {
        bool semanticsError;
        ErrorInfo error;
    };

    // Find all macro invocations within a module and validate invocation syntax.
    ExtractResult extractMacroDefinitions(ModulePrototype& module);
    // Add module references to ModuleAssemblyGraph and validate macro semantics.
    LinkResult addModuleLinksToPrototypes(ModulePrototype& module, QList<MacroDefinition>);

    // Return true if the target graph has a cycle in it.
    std::tuple<bool, std::list<quint16>> checkForCycles();

    // Is the macro name valid?
    std::tuple<bool, QString> validateMacroName(QString macroName);
    // If any of the macro args are macro substitutions ($1, $2, etc)
    // then the argument list is invalid.
    std::tuple<bool, QString> validateMacroArgs(QStringList macroArgs);

private:
    // We do not own registry or target, do not delete.
    const MacroRegistry* registry;
    ModuleAssemblyGraph* target;
    // Keep track of the next available vertex.
    quint16 moduleIndex;
    // Create a module or return a pointer to an identical one.
    quint16 maybeCreatePrototype(QString macroName, ModuleType type);
    QSharedPointer<ModuleInstance> maybeCreateInstance(quint16 moduleIndex, QStringList args);
    QSharedPointer<FrontEndError> mapError(quint16 badModule, QString message);
};

static const QString tooManyMacros = ";ERROR: Only one macro may be referenced per line.";
static const QString noIdentifier = ";ERROR: A @ must be followed by a string identifier.";
static const QString noSuchMaro = ";ERROR: Referenced macro does not exist.";
static const QString badArgCount = ";ERROR: Macro supplied wrong number of arguments.";
static const QString noDollarInMacro = ";ERROR: Cannot use $ as part of a macro identifier.";
static const QString invalidArg = ";ERROR: Bad argument: %1. Cannot use $ in macro argument.";
static const QString circularInclude = ";ERROR: Circular macro inclusion detected.";
static const QString selfRefence = ";ERROR: Macro definition invokes itself.";
#endif // MACROPREPROCESSOR_H
