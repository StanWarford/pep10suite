#include "macropreprocessor.h"
#include "macrotokenizer.h"
#include "ngraph_prune.h"
#include "ngraph_path.h"

static const QString tooManyMacros = "ERROR: Only one macro may be referenced per line.";
static const QString noIdentifier = "ERROR: A % must be followed by a string identifier.";
static const QString noSuchMaro = "ERROR: Referenced macro does not exist.";
static const QString badArgCount = "ERROR: Macro supplied wrong number of arguments.";
static const QString noDollarInMacro = "ERROR: Cannot use $ as part of a macro identifier.";
static const QString invalidArg = "ERROR: Bad argument: %1. Cannot use $ in macro argument.";
static const QString circularInclude = "ERROR: Circular macro inclusion detected.";
static const QString selfRefence = "ERROR: Macro definition invokes itself.";

MacroPreprocessor::MacroPreprocessor(const MacroRegistry *registry): registry(registry), moduleIndex(0)
{

}

MacroPreprocessor::~MacroPreprocessor()
{
    // We do not own the registry or target, so do no delete them.
    registry = nullptr;
    target = nullptr;
}

void MacroPreprocessor::setTarget(ModuleAssemblyGraph *target)
{
    this->target = target;
}

PreprocessorResult MacroPreprocessor::preprocess()
{
    PreprocessorResult result;
    result.succes = true;
    moduleIndex = target->moduleGraph.num_vertices();
    for(quint16 graphIndex = target->rootModule; graphIndex < moduleIndex; graphIndex++) {
        ModulePrototype& module = *target->prototypeMap[graphIndex];
        auto extract = extractMacroDefinitions(module);
        // If there were parsing errors, abort preprocessing now.
        if(extract.syntaxError) {
            // Need the root module to find how it include the module that failed.
            auto instance = target->instanceMap[target->rootModule][0];
            auto prototype = target->prototypeMap[target->rootModule];
            // Failing path gives the shortest path from root to the bad module,
            // where the first item in the path is the first step towards the bad module.
            auto failingPath =  path<quint16>(target->moduleGraph, target->rootModule, module.index);
            //qDebug() << failingPath;
            int wrongModule = failingPath.front();
            // Take the error message from extraction and place it on the line in root that
            // started the include chain that failed.
            ErrorInfo err = {target->getLineFromIndex(*prototype, wrongModule), std::get<1>(extract.error)};
            instance->errorList.append(err);
            result.error = err;
            result.succes = false;
            return result;
        }
        // At this point, we have only checked syntax errors like %*($&$#.
        // We will still need to process semantic errors %deco $1,d.
        auto link = addModuleLinksToPrototypes(module, extract.moduleDefinitionList);
        qDebug().noquote() << graphIndex << ": "<< target->prototypeMap[graphIndex].get()->name;
        if(link.semanticsError) {
            // Need the root module to find how it include the module that failed.
            auto instance = target->instanceMap[target->rootModule][0];
            auto prototype = target->prototypeMap[target->rootModule];
            // Failing path gives the shortest path from root to the bad module,
            // where the first item in the path is the first step towards the bad module.
            auto failingPath =  path<quint16>(target->moduleGraph, target->rootModule, module.index);
            //qDebug() << failingPath;
            int wrongModule = failingPath.front();
            // Take the error message from linking and place it on the line in root that
            // started the include chain that failed.
            ErrorInfo err = {target->getLineFromIndex(*prototype, wrongModule), std::get<1>(link.error)};
            instance->errorList.append(err);
            result.error = err;
            result.succes = false;
            return result;
        }
    }
    // We have succesfully constructed a macro tree, and
    // all macros invoked were valid. However, the macros
    // might yet still form a circular include.

    auto cycleCheck = checkForCycles();
    if(!std::get<0>(cycleCheck)) {
        // Need the root module to find how it include the module that failed.
        auto instance = target->instanceMap[target->rootModule][0];
        auto prototype = target->prototypeMap[target->rootModule];
        // Failing path points to the first module that participates in cycle
        // that was included by main.
        auto failingPath =  std::get<1>(cycleCheck);
        int wrongModule = failingPath.front();
        result.succes = false;
        // Take the error message from the linker and place it on the line in root that
        // started the include chain that failed.
        result.error = {target->getLineFromIndex(*prototype, wrongModule), circularInclude};
    }

    return result;
}

MacroPreprocessor::ExtractResult MacroPreprocessor::extractMacroDefinitions(ModulePrototype &module)
{
    // Macro invocations always start with a @.
    static QRegularExpression macroInvoke("@");
    ExtractResult retVal;
    MacroDefinition extract;
    if(module.moduleType == ModuleType::MACRO) {
        //++lineIt;
        //++lineNumber;
        // Remove the first line of text from the module.
        module.textLines.pop_front();
        module.text = module.text.mid(module.text.indexOf("\n") + 1);
    }
    quint16 lineNumber = 0;
    auto lineIt = module.textLines.begin();
    // Macro files start with the name of the macro and number of arguments
    // on the first line. Since this line does not reference an external macro
    // we may skip over it.
    for( ; lineIt != module.textLines.end(); lineNumber++, ++lineIt ) {
        // Store the text pointer to by list iterator to reduce dereferencing.
        QString lineText = *lineIt;
        // Check if the line has any potential macros (@).
        auto hasMacroDirective = macroInvoke.globalMatch(lineText);
        // If the line has at least one @, get the index of
        // it so that we can extract the macro name.
        int location = 0;
        if(hasMacroDirective.hasNext()) {
            location = hasMacroDirective.peekNext().capturedStart();
        }

        // Check if the @ is occuring in a comment. If so, skip to the next line.
        auto commentMatch = MacroTokenizerHelper::comment.match(lineText);
        int commentLoc = -1;
        if(commentMatch.hasMatch() && commentMatch.capturedStart() < location) {
            continue;
        }
        // If there was a comment, but it did not overlap with the macro definition,
        // keep track of its location for help with macro argument parsing.
        else if (commentMatch.hasMatch()) {
            commentLoc = commentMatch.capturedStart();
        }

        // Find the number of @ outside of a comment on this line.
        int count = 0;
        for( ; hasMacroDirective.hasNext(); count++) hasMacroDirective.next();
        // Line has no macro definitions, start next line.
        if(count == 0) continue;
        // Otherwise we have multiple potential macros - this line is a
        // mess, so just flag it as an error.
        else if (count > 1) {
            retVal.syntaxError = true;
            retVal.error = {lineNumber, tooManyMacros};
            return retVal;
        }

        // Find the name associated with the macro.
        auto macroNameMatch = MacroTokenizerHelper::identifier.match(lineText, location);
        // The macro didn't have a valid name, error and return.
        if(!macroNameMatch.hasMatch()) {
            retVal.syntaxError = true;
            retVal.error = {lineNumber, noIdentifier};
            return retVal;
        }

        // Check if the macro being request actually exists
        QString macroName = macroNameMatch.captured().trimmed();
        // If it does not, preprocessing can't continue, abort.
        if(!registry->hasMacro(macroName)) {
            // Tried to link to a macro that does not exist.
            retVal.syntaxError = true;
            retVal.error = {lineNumber, noSuchMaro};
            return retVal;
        }

        auto macroObject = registry->getMacro(macroName);
        auto endMacroName = macroNameMatch.capturedEnd();
        //QString argSubstr = lineText.mid(endMacroName, commentLoc);
        //QString trimmedSubstr = argSubstr.trimmed();
        //QStringList splitList = trimmedSubstr.split(",", QString::SplitBehavior::SkipEmptyParts);

        // Get the number of "things" seperated by commas AND outside of comments.
        QString argText = lineText.mid(endMacroName, commentLoc - endMacroName).trimmed();
        QStringList argList = argText.split(",", QString::SplitBehavior::SkipEmptyParts);
        if(argText.size() == 0) {
            // If there is no argument text, we have 0 arguments.
            count = 0;
        }
        else {
            // Otherwise we have text, so we have at least 1 element,
            // and one additional argument per comma in the string.
            count = argText.count(",") + 1;
        }



        // Check that we supply the right number of args to a macro.
        // If we do not, raise an error.
        if(macroObject->argCount != count) {
            // Tried to link to a macro with the wrong number of arguments.
            retVal.syntaxError = true;
            retVal.error = {lineNumber, badArgCount};
            return retVal;
        }

        // All these check show we have a syntactically correct
        // macro invocation, so we may create a MacroDefinition.
        extract.macroName = macroName;
        extract.macroArgs = argList;
        extract.lineNumber = lineNumber;
        retVal.moduleDefinitionList.append(extract);
    }
    retVal.syntaxError = false;
    return retVal;
}

MacroPreprocessor::LinkResult MacroPreprocessor::addModuleLinksToPrototypes(ModulePrototype& module,
                                                                     QList<MacroDefinition> definitions)
{
    LinkResult result;
    result.semanticsError = false;
    for(auto macro : definitions) {
        // Before creating Modules for a macro definition,
        // validate the semantics of the macro name and args.
        // This will pick up on errors like @deco$1 and
        // $myFunc 9,$1,s.
        auto nameValid = validateMacroName(macro.macroName);
        if(!std::get<0>(nameValid)) {
            // If the name was not valid, signal linking failure.
            result.semanticsError = true;
            result.error = {macro.lineNumber, std::get<1>(nameValid)};
            break;
        }
        // If the module has the same name as the macro being called, we have an
        // infinite include loop. While this will be deteced by the cycle checker,
        // we can give a more precise error message if we check here.
        else if(macro.macroName.compare(module.name) == 0) {
            result.semanticsError = true;
            result.error = {macro.lineNumber, selfRefence};
            break;
        }
        auto argsValid = validateMacroArgs(macro.macroArgs);
        if(!std::get<0>(argsValid)) {
            // If the arguments were not valid, signal linking failure.
            result.semanticsError = true;
            result.error = {macro.lineNumber, std::get<1>(argsValid)};
            break;
        }

        // Now that that we are assured the macro is valid, create associated module objects.
        quint16 includedModule = maybeCreatePrototype(macro.macroName, ModuleType::MACRO);
        target->moduleGraph.insert_edge(module.index, includedModule);

        auto instance = maybeCreateInstance(includedModule, macro.macroArgs);
        module.lineToInstance.insert(macro.lineNumber, instance.get());
    }
    return result;
}

std::tuple<bool, std::list<quint16> > MacroPreprocessor::checkForCycles()
{
    // Recursively remove any leaf nodes in the graph,
    // until no more nodes can be removed.
    // If not all nodes can be removed, then there is a
    // cycle in the graph, and compilation must fail.
    auto outGraph = reduce(target->moduleGraph);
    //std::stringstream str;
    //str << outGraph;
    //qDebug() << QString::fromStdString(str.str()).split("\n");
    if(outGraph.num_nodes() == 0) {
        // Graph has no circular includes, it is sane.
        return {true, {}};
    }
    // The graph had a cycle, we need to signal that compilation must be aborted.
    else {
        // Must crawl tree to figure out which lines in the root instance
        // need error messages appended.
        std::list<quint16> pathToCycle;
        for(auto module : target->prototypeMap[target->rootModule]->lineToInstance) {
            if( outGraph.find(module->prototype->index) != outGraph.end()) {
                pathToCycle = path<quint16>(outGraph, target->rootModule, module->prototype->index);
                break;
            }
        }
        return {false, pathToCycle};
    }
}

std::tuple<bool, QString> MacroPreprocessor::validateMacroName(QString macroName)
{
    // The only ivalid symbol in a macro name is a $, as a $ signifies
    // the argument of a previously defined macro
    static const QRegularExpression macroMatch("\\$");
    if(macroMatch.match(macroName).hasMatch()) {
        return {false, noDollarInMacro};
    }
    return {true, ""};
}

std::tuple<bool, QString> MacroPreprocessor::validateMacroArgs(QStringList macroArgs)
{
    /*
     * Macro arguments may not have a $ in them, as this
     * would ballon the complexity of tacking ModuleInstances.
     * I do not see a compelling use case for this level of complexity,
     * so I have opted to outright ban it.
     */
    static const QRegularExpression macroPattern("\\s*$\\d+");
    for(auto arg : macroArgs) {
        auto pattern = macroPattern.match(arg);
        if(pattern.hasMatch()) {
            return {false, invalidArg.arg(pattern.captured())};
        }
    }
    return {true, ""};
}

quint16 MacroPreprocessor::maybeCreatePrototype(QString macroName, ModuleType type)
{
    quint16 index = target->getIndexFromName(macroName);
    // If the index is not -1, then we succeded in finding the module
    // so return the index to that module.
    if(index != 0xFFFF) return index;

    // Otherwise we need to construct a new Module for the macro;
    auto macroObject = registry->getMacro(macroName);
    QSharedPointer<ModulePrototype> prototype = QSharedPointer<ModulePrototype>::create();
    prototype->name = macroObject->macroName;
    prototype->text = macroObject->macroText;
    prototype->index = moduleIndex++;
    prototype->textLines = macroObject->macroText.split("\n");
    prototype->moduleType = type;

    // Track prototype in assembly graph.
    target->prototypeMap.insert(prototype->index, prototype);
    target->moduleGraph.insert_vertex(prototype->index);
    return prototype->index;
}

QSharedPointer<ModuleInstance> MacroPreprocessor::maybeCreateInstance(quint16 moduleIndex, QStringList args)
{
    // If there is not yet a list of instances, create one
    // so that we may iterate over it safely.
    if(!target->instanceMap.contains(moduleIndex)) {
        target->instanceMap.insert(moduleIndex, QList<QSharedPointer<ModuleInstance>>());
    }

    // If two macro invocations use the same arguments,
    // then they are the same instance of a macro.
    for(auto it : target->instanceMap[moduleIndex]) {
        // Don't simply compare lists using operator==, since
        // we need compare wit CaseInsensitive.
        bool match = true;
        for(int inner = 0; inner < args.size(); inner++) {
            if(it->macroArgs[inner].compare(args[inner], Qt::CaseSensitivity::CaseInsensitive) != 0) {
                match = false;
                break;
            }
        }
        if(match) {
            return it;
        }
    }

    // We did not find a macro invocation with the same args, must create a new instance.
    auto instance = QSharedPointer<ModuleInstance>::create();
    instance->prototype = target->prototypeMap[moduleIndex];
    instance->macroArgs = args;
    target->instanceMap[moduleIndex].append(instance);
    return instance;
}
