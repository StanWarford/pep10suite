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
    QSharedPointer<FrontEndError> error;
};

class TokenizerBuffer;
class MacroAssembler
{
public:
    MacroAssembler(MacroRegistry* registry);
    ~MacroAssembler();


    AssemblerResult assemble(ModuleAssemblyGraph& graph);
private:
    struct ModuleResult
    {
        bool success;
        QSharedPointer<FrontEndError> errInfo;
    };
    struct LineResult
    {
        bool success = false;
        QSharedPointer<AsmCode> codeLine = nullptr;
    };
    ModuleResult assembleModule(ModuleAssemblyGraph &graph, ModuleInstance& instance);
    // Pre: errorMessage is an empty string.
    LineResult assembleLine(ModuleAssemblyGraph &graph, ModuleInstance& instance,
                            QString& errorMessage, bool &dotEndDetected);
    // Check if the name fits our requirements / length.
    bool validateSymbolName(const QStringRef& name, QString& errorMessage);

    QSharedPointer<NonUnaryInstruction> parseNonUnaryInstruction(Enu::EMnemonic mnemonic,
                                                                 std::optional<QSharedPointer<SymbolEntry>> symbol,
                                                                 ModuleInstance& instance,
                                                                 QString& errorMessage);
    QSharedPointer<AsmArgument> parseOperandSpecifier(ModuleInstance &instance, QString& errorMessage);
    Enu::EAddrMode stringToAddrMode(QString str) const;

    QSharedPointer<DotAddrss> parseADDRSS(std::optional<QSharedPointer<SymbolEntry>> symbol,
                                          ModuleInstance& instance,
                                          QString& errorMessage);
    QSharedPointer<DotAscii> parseASCII(std::optional<QSharedPointer<SymbolEntry>> symbol,
                                        ModuleInstance& instance,
                                        QString& errorMessage);
    QSharedPointer<DotAlign> parseALIGN(std::optional<QSharedPointer<SymbolEntry>> symbol,
                                        ModuleInstance& instance,
                                        QString& errorMessage);
    QSharedPointer<DotBlock> parseBLOCK(std::optional<QSharedPointer<SymbolEntry>> symbol,
                                        ModuleInstance& instance,
                                        QString& errorMessage);
    QSharedPointer<DotBurn> parseBURN(std::optional<QSharedPointer<SymbolEntry>> symbol,
                                      ModuleInstance& instance,
                                      QString& errorMessage);
    QSharedPointer<DotByte> parseBYTE(std::optional<QSharedPointer<SymbolEntry>> symbol,
                                      ModuleInstance& instance,
                                      QString& errorMessage);
    QSharedPointer<DotEnd> parseEND(std::optional<QSharedPointer<SymbolEntry>> symbol,
                                    ModuleInstance& instance,
                                    QString& errorMessage);
    QSharedPointer<DotEquate> parseEQUATE(std::optional<QSharedPointer<SymbolEntry>> symbol,
                                          ModuleInstance& instance,
                                          QString& errorMessage);
    QSharedPointer<DotExport> parseEXPORT(std::optional<QSharedPointer<SymbolEntry> > symbol,
                                          ModuleInstance &instance,
                                          QString &errorMessage);
    QSharedPointer<DotSycall> parseSCALL(std::optional<QSharedPointer<SymbolEntry> > symbol,
                                          ModuleInstance &instance,
                                          QString &errorMessage);
    QSharedPointer<DotUSycall> parseUSCALL(std::optional<QSharedPointer<SymbolEntry> > symbol,
                                            ModuleInstance &instance,
                                            QString &errorMessage);
    QSharedPointer<DotWord> parseWORD(std::optional<QSharedPointer<SymbolEntry>> symbol,
                       ModuleInstance& instance, QString& errorMessage);

    QSharedPointer<MacroInvoke> parseMacroInstruction(const ModuleAssemblyGraph& graph,
                                                      const QString& macroName,
                                                      std::optional<QSharedPointer<SymbolEntry>> symbol,
                                                      ModuleInstance& instance, QString& errorMessage);

    MacroRegistry* registry;
    TokenizerBuffer* tokenBuffer;
};

#endif // MACROASSEMBLER_H
