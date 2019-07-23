#ifndef MACROASSEMBLER_H
#define MACROASSEMBLER_H

#include "macromodules.h"
#include "asmargument.h"
#include "macrotokenizer.h"
#include "asmcode.h"
#include "macroregistry.h"
struct AssemblerResult
{
    bool success;
    ErrorInfo error;
};

class TokenizerBuffer;
class MacroAssembler
{
public:
    MacroAssembler(const MacroRegistry* registry);
    ~MacroAssembler();


    AssemblerResult assemble(ModuleAssemblyGraph& graph);
private:
    struct ModuleResult
    {
        bool success;
        ErrorInfo errInfo;
    };
    struct LineResult
    {
        bool success = false;
        AsmCode* codeLine = nullptr;
    };
    ModuleResult assembleModule(ModuleAssemblyGraph &graph, ModuleInstance& instance);
    // Pre: errorMessage is an empty string.
    LineResult assembleLine(ModuleAssemblyGraph &graph, ModuleInstance& instance,
                            QString& errorMessage, bool &dotEndDetected);
    // Check if the name fits our requirements / length.
    bool validateSymbolName(const QStringRef& name, QString& errorMessage);

    NonUnaryInstruction* parseNonUnaryInstruction(Enu::EMnemonic mnemonic, std::optional<QSharedPointer<SymbolEntry>> symbol,
                                                  ModuleInstance& instance, QString& errorMessage);
    QSharedPointer<AsmArgument> parseOperandSpecifier(ModuleInstance &instance, QString& errorMessage);
    Enu::EAddrMode stringToAddrMode(QString str) const;

    DotAddrss* parseADDRSS(std::optional<QSharedPointer<SymbolEntry>> symbol,
                           ModuleInstance& instance, QString& errorMessage);
    DotAscii* parseASCII(std::optional<QSharedPointer<SymbolEntry>> symbol,
                         ModuleInstance& instance, QString& errorMessage);
    DotAlign* parseALIGN(std::optional<QSharedPointer<SymbolEntry>> symbol,
                         ModuleInstance& instance, QString& errorMessage);
    DotBlock* parseBLOCK(std::optional<QSharedPointer<SymbolEntry>> symbol,
                         ModuleInstance& instance, QString& errorMessage);
    DotBurn* parseBURN(std::optional<QSharedPointer<SymbolEntry>> symbol,
                       ModuleInstance& instance, QString& errorMessage);
    DotByte* parseBYTE(std::optional<QSharedPointer<SymbolEntry>> symbol,
                       ModuleInstance& instance, QString& errorMessage);
    DotEnd* parseEND(std::optional<QSharedPointer<SymbolEntry>> symbol,
                     ModuleInstance& instance, QString& errorMessage);
    DotEquate* parseEQUATE(std::optional<QSharedPointer<SymbolEntry>> symbol, ModuleInstance& instance, QString& errorMessage);
    DotExport* parseEXPORT(std::optional<QSharedPointer<SymbolEntry> > symbol,
                     ModuleInstance &instance, QString &errorMessage);
    bool parseSYCALL();
    bool parseUSYCALL();
    DotWord* parseWORD(std::optional<QSharedPointer<SymbolEntry>> symbol,
                       ModuleInstance& instance, QString& errorMessage);

    MacroInvoke* parseMacroInstruction(const ModuleAssemblyGraph& graph, const QString& macroName,
                                       std::optional<QSharedPointer<SymbolEntry>> symbol,
                                       ModuleInstance& instance, QString& errorMessage);

    const MacroRegistry* registry;
    TokenizerBuffer* tokenBuffer;
};

#endif // MACROASSEMBLER_H
