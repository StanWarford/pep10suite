#ifndef MACROREGISTRY_H
#define MACROREGISTRY_H

#include <QMap>
#include <QSharedPointer>
#include <QString>
#include <tuple>
#include "macro.h"

/*
 * This object keeps track of which macros exist, how many arguments they take, and which
 * level in the macro heirarchy they occupy.
 *
 * Only one macro may exist with the same name - there cannot be multiple macros with the same name
 * and different levels and/or different argument counts.
 */
class MacroRegistry
{
public:
    MacroRegistry(QString macroPath = "default");
    ~MacroRegistry();
    bool hasMacro(QString macroName) const;
    QSharedPointer<const Macro> getMacro(QString macroName) const;

    // Register a macro to the specified level with a given argument count.
    bool registerCoreMacro(QString macroName, QString macroText, quint8 argCount);
    bool registerSystemCall(QString macroName, QString macroText, quint8 argCount);
    bool registerCustomMacro(QString macroName, QString macroText, quint8 argCount);
private:
    QMap<QString, QSharedPointer<Macro>> macroList;
    bool registerMacro(QString macroName, QString macroText, quint8 argCount, MacroType type);

};

#endif // MACROREGISTRY_H
