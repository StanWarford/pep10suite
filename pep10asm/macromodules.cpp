#include "macromodules.h"

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
    for(auto kvPair : module.lineToInstance) {
        auto includedModule = std::get<1>(kvPair);
        if(includedModule->prototype->index == index) {
            return std::get<0>(kvPair);
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
    rootPrototype->name = "@@main@@";
    rootPrototype->text = text;
    rootPrototype->index = ModuleAssemblyGraph::defaultRootIndex;
    rootPrototype->textLines = text.split("\n");
    rootPrototype->moduleType = type;
    prototypeMap.insert(rootPrototype->index, rootPrototype);
    moduleGraph.insert_vertex(rootPrototype->index);

    QSharedPointer<ModuleInstance> rootInstance = QSharedPointer<ModuleInstance>::create();
    rootInstance->prototype = rootPrototype.get();
    rootInstance->macroArgs = QStringList();
    instanceMap.insert(rootPrototype->index, {rootInstance});
    return {rootPrototype, rootInstance};
}
