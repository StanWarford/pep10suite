#include "macroregistry.h"

MacroRegistry::MacroRegistry(QString registryName) : macroList()
{

}

MacroRegistry::~MacroRegistry()
{

}

bool MacroRegistry::hasMacro(QString macroName) const
{
    auto equalIg = [&macroName](const QSharedPointer<Macro>& other){
        return other->macroName.compare(macroName, Qt::CaseInsensitive) == 0;
    };
    auto item = std::find_if(macroList.cbegin(), macroList.cend(), equalIg);
    return item != macroList.cend();
}

QSharedPointer<const Macro> MacroRegistry::getMacro(QString macroName) const
{
    auto equalIg = [&macroName](const QSharedPointer<Macro>& other){
        return other->macroName.compare(macroName, Qt::CaseInsensitive) == 0;
    };
    auto item = std::find_if(macroList.cbegin(), macroList.cend(), equalIg);
    if(item != macroList.end()) {
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
