#include "macroregistry.h"

MacroRegistry::MacroRegistry(QString registryName) : macroList()
{

}

MacroRegistry::~MacroRegistry()
{

}

bool MacroRegistry::hasMacro(QString macroName) const
{
    auto item = macroList.find(macroName);
    return item != macroList.end();
}

QSharedPointer<const Macro> MacroRegistry::getMacro(QString macroName) const
{
    if(auto item = macroList.find(macroName); item != macroList.end()) {
        return item.value();
    }
    return nullptr;
}

bool MacroRegistry::registerCoreMacro(QString macroName, QString macroText, quint8 argCount)
{
    return registerMacro(macroName, macroText, argCount, MacroType::CoreMacro);
}

bool MacroRegistry::registerSystemCall(QString macroName, QString macroText, quint8 argCount)
{
    return registerMacro(macroName, macroText, argCount, MacroType::SystemMacro);
}

bool MacroRegistry::registerCustomMacro(QString macroName, QString macroText, quint8 argCount)
{
    return registerMacro(macroName, macroText, argCount, MacroType::UserMacro);
}

bool MacroRegistry::registerMacro(QString macroName, QString macroText, quint8 argCount, MacroType type)
{
    if(macroList.contains(macroName)) {
        return false;
    }
    QSharedPointer<Macro> newMacro = QSharedPointer<Macro>::create();
    newMacro->macroName = macroName;
    newMacro->macroText = macroText;
    newMacro->argCount = argCount;
    newMacro->type = type;
    macroList.insert(macroName, newMacro);
    return true;
}
