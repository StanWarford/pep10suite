#include "macroinstancer.h"

MacroInstancer::MacroInstancer()
{

}

InstanceResult MacroInstancer::instance(ModuleAssemblyGraph& graph)
{

    // Instancing converts a many-to-one mapping of module instances to macro invocations
    // into a one-to-one mapping. This mapping is destructive, and doing it in place in
    // the graph would be nearly impossible. Instead, we will build the new mapping
    // and cut over after all instancing has compeleted.
    ModuleAssemblyGraph::InstanceMap newMap;
    auto rootModuleInstance = graph.getRootInstance();
    newMap[graph.rootModule] = {rootModuleInstance};
    // Begin depth-first linking starting from the root.
    auto linkResult = instanceModule(graph, newMap, *rootModuleInstance);
        graph.instanceMap = newMap;
    if(!linkResult.success) {
        return linkResult;
    }
}

InstanceResult MacroInstancer::instanceModule(ModuleAssemblyGraph graph, ModuleAssemblyGraph::InstanceMap &newInstanceMap, ModuleInstance &instance)
{
    InstanceResult retVal;
    retVal.success = true;

    for(int lineNum = 0 ; lineNum < instance.codeList.size(); lineNum++) {
        auto line = instance.codeList[lineNum];
        if(dynamic_cast<MacroInvoke*>(line.get()) != nullptr) {
            auto macroLine = static_cast<MacroInvoke*>(line.get());
            // Copy and swap the moduleInstance. Now we can adjust the code list for the
            // child module without affecting every instance of the macro in the application.
            auto copiedInstance = QSharedPointer<ModuleInstance>::create(*macroLine->getMacroInstance());
            // As we have a new macro instance, we MUST update it's instance ID to be unique
            copiedInstance->setInstanceIndex(graph.getNextInstanceID());
            // However, for linking updates to be reflected, the macro line must be updated too.
            macroLine->setMacroInstance(copiedInstance);
            // Ensure that new instance map won't error when we append an item.
            if(!newInstanceMap.contains(copiedInstance->prototype->index)) {
                newInstanceMap.insert(copiedInstance->prototype->index, {});
            }
            // Add the new instance to the (in-progress) instance lookup map.
            newInstanceMap[copiedInstance->prototype->index].append(copiedInstance);


            // Sanity check that we are replacing an existing macro instance.
            // If somehow this throws, a macro invocation was added or moved within
            // the code listing for the current module.
            assert(instance.prototype->lineToInstance.contains(lineNum));
            // Adjust link in prototype to point to the new instance copy
            // that has correct address bindings, instead of the generic instance
            // that has no address bindings.
            instance.prototype->lineToInstance[lineNum] = &*copiedInstance;
            // Must assign addresses to children macros before we know
            // the address of the next code line in the current module.
            auto childRetVal = instanceModule(graph, newInstanceMap, *copiedInstance);
            if(childRetVal.success == false) {
                retVal.success = false;
                // Only take the first linking error from child, otherwise a line might
                // have multiple error messages written to it.
                // retVal.errorList.append({lineNum, std::get<1>(childRetVal.errorList[0])});
                continue;
            }
        }
    }
    return retVal;
}
