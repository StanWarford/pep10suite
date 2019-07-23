#ifndef MACROREGISTRY_H
#define MACROREGISTRY_H

#include <QMap>
#include <QSharedPointer>
#include <QString>
#include <tuple>
#include <QFile>
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
    bool registerSystemCall(QString macroName, QString macroText);
    bool registerCustomMacro(QString macroName, QString macroText);
    //Given the text of a macro, get the macro's name & argument count.
    static std::tuple<bool, QString, quint16> macroDefinition(QString macroText);
private:
    QMap<QString, QSharedPointer<Macro>> macroList;
    // Helper function that discovers all builtin macros.
    bool registerCoreMacros(QDir dir);
    bool registerMacro(QString macroName, QString macroText, MacroType type);
    // Core macros are auto-deteced, not added by users.
    bool registerCoreMacro(QString macroName, QString macroText);

};

#endif // MACROREGISTRY_H
