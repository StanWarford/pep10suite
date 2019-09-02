#include "macromodules.h"
#include "ngraph_path.h"
void ModuleAssemblyGraph::addError(QSharedPointer<AErrorMessage> error)
{
    errors.addError(error);
    if(error->getPrototypeIndex() == rootModule) {
        // Determine which line in the source program caused this error to occur.
        // Need the root module to find how it include the module that failed.
        auto instance = instanceMap[rootModule][0];
        auto prototype = prototypeMap[rootModule];
        // Failing path gives the shortest path from root to the bad module,
        // where the first item in the path is the first step towards the bad module.
        auto failingPath =  path<quint16>(moduleGraph, rootModule, error->getPrototypeIndex());

        int wrongModule = failingPath.front();
        // Take the error message and place it on the line in root that
        // started the include chain that failed.
        int wrongLineInRoot =  getLineFromIndex(*prototype, wrongModule);
        if(!errors.sourceMapped.contains(wrongLineInRoot)) {
            errors.sourceMapped[wrongLineInRoot] = {};
        }
        errors.sourceMapped[wrongLineInRoot].append(error);
    }
    else {
        int wrongLineInRoot =  error->getSourceLineNumber();
        if(!errors.sourceMapped.contains(wrongLineInRoot)) {
            errors.sourceMapped[wrongLineInRoot] = {};
        }
        errors.sourceMapped[wrongLineInRoot].append(error);
    }
}

quint16 ModuleAssemblyGraph::getNextInstanceID()
{
    return nextInstanceID++;
}

quint16 ModuleAssemblyGraph::getIndexFromName(QString macroName) const
{
    for(auto module : prototypeMap) {
        // Check if the the current module is the module being looked for
        if(module->name.compare(macroName, Qt::CaseSensitivity::CaseInsensitive) == 0) {
            return module->index;
        }
    }
    return 0xFFFF;
}

quint16 ModuleAssemblyGraph::getLineFromIndex(ModulePrototype& module, quint16 index) const
{
    for(auto kvPair = module.lineToInstance.keyValueBegin();
        kvPair != module.lineToInstance.keyValueEnd();
        ++kvPair) {
        auto includedModule = (*kvPair).second;
        if(includedModule->prototype->index == index) {
            return (*kvPair).first;
        }
    }
    return 0;
}

std::optional<QSharedPointer<ModuleInstance> > ModuleAssemblyGraph::getInstanceFromArgs(quint16 index, QStringList args) const
{
    if(!instanceMap.contains(index)) return nullopt;

    for(auto item : instanceMap[index]) {
        bool equal = true;
        if(args.size() != item->macroArgs.size()) return nullopt;
        for(int it = 0; it < args.size(); it++) {
            equal &= (item->macroArgs[it].compare(args[it], Qt::CaseInsensitive) == 0);
        }
        if(equal) return std::optional<QSharedPointer<ModuleInstance>>(item);
    }
    return nullopt;
}

std::tuple<QSharedPointer<ModulePrototype>, QSharedPointer<ModuleInstance> > ModuleAssemblyGraph::createRoot(QString text, ModuleType type)
{
    // If there's already an element in the graph, the node has been constructed. Abort
    if(moduleGraph.num_vertices() != 0) {
        return {nullptr, nullptr};
    }
    QSharedPointer<ModulePrototype> rootPrototype = QSharedPointer<ModulePrototype>::create();
    switch(type) {
    case ModuleType::USER_PROGRAM:
        rootPrototype->name = "@@main@@";
        break;
    case ModuleType::OPERATING_SYSTEM:
        rootPrototype->name = "@@op_sys@@";
        break;
    // Root module most be OS or a user program.
    default:
        return {nullptr, nullptr};

    }
    rootPrototype->text = text;
    rootPrototype->index = ModuleAssemblyGraph::defaultRootIndex;
    rootPrototype->textLines = text.split("\n");
    rootPrototype->moduleType = type;
    prototypeMap.insert(rootPrototype->index, rootPrototype);
    moduleGraph.insert_vertex(rootPrototype->index);

    QSharedPointer<ModuleInstance> rootInstance = QSharedPointer<ModuleInstance>::create();
    rootInstance->prototype = rootPrototype;
    rootInstance->macroArgs = QStringList();
    rootInstance->setInstanceIndex(getNextInstanceID());
    instanceMap.insert(rootPrototype->index, {rootInstance});
    return {rootPrototype, rootInstance};
}

ModuleInstance::ModuleInstance(const ModuleInstance &other)
{
    this->burnInfo = other.burnInfo;
    // Must duplicate entire code list
    for(auto line : other.codeList) {
        this->codeList.append(QSharedPointer<AsmCode>(line->cloneAsmCode()));
    }
    // WARNING: Multiple instances now have the same instance index. This is very bad.
    this->instanceIndex = other.instanceIndex;
    this->macroArgs = other.macroArgs;
    this->prototype = other.prototype;
    this->traceInfo = other.traceInfo;
    this->symbolTable = other.symbolTable;
    this->alreadyLinked = other.alreadyLinked;
    this->alreadyAssembled = other.alreadyAssembled;
}

ModuleInstance::~ModuleInstance()
{
    // We don't own prototype. Do not delete.
    this->prototype = nullptr;
}

ModuleInstance &ModuleInstance::operator=(ModuleInstance rhs)
{
    swap(*this, rhs);
    return *this;
}

void ModuleInstance::setInstanceIndex(quint16 lowerPart)
{
    instanceIndex = static_cast<quint32>(prototype->index << 16);
    instanceIndex |= lowerPart;
}
