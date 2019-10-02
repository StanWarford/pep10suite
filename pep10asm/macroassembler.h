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
public:
    static const inline QString unexpectedToken = ";ERROR: Unexpected token %1 encountered.";
    static const inline QString unxpectedEOL = ";ERROR: Found unexpected end of line.";
    static const inline QString expectNewlineAfterComment = ";ERROR: \n expected after a comment";
    static const inline QString unexpectedSymbolDecl = ";ERROR: symbol definition must be followed by an identifier, dot command, or macro.";
    static const inline QString invalidMnemonic = ";ERROR: Invalid mnemonic \"%1\".";
    static const inline QString onlyInOperatingSystem = ";ERROR: Only operating systems may contain a %1.";
    static const inline QString invalidDotCommand = ";ERROR: Invalid dot command \"%1\"";
    static const inline QString longSymbol = ";ERROR: Symbol %1 cannot have more than eight characters.";
    static const inline QString missingEND = ";ERROR: Missing .END sentinel.";
    static const inline QString reqAddrMode = ";ERROR: Addressing mode required for this instruction.";
    static const inline QString illegalAddrMode = ";ERROR: Illegal addressing mode for this instruction.";
    static const inline QString macroDoesNotExist = ";ERROR: Macro %1 does not exist.";
    static const inline QString macroWrongArgCount = ";ERROR: Macro %1 has wrong number of arguments.";
    static const inline QString opsecAfterMnemonic = ";ERROR: Operand specifier expected after mnemonic.";

    static const inline QString byteDecOutOfRange = ";ERROR: Decimal constant is out of byte range (-128..255).";
    static const inline QString byteHexOutOfRange = ";ERROR: Hex constant is out of byte range (0x00..0xFF).";
    static const inline QString byteStringOutOfRange = ";ERROR: string operands must have length one.";
    static const inline QString wordStringOutOfRange = ";ERROR: String operands must have length at most two.";
    static const inline QString wordHexOutOfRange = ";ERROR: Hexidecimal constant is out of range (0x0000..0xFFFF).";
    static const inline QString wordSignDecOutOfRange = ";ERROR: Decimal constant is out of range (-32768..65535).";
    static const inline QString wordUnsignDecOutOfRange = ";ERROR: Decimal constant is out of range (0..65535).";

    static const inline QString badAddrssArgument = ";ERROR: .ADDRSS requires a symbolic argument.";
    static const inline QString decConst248 = ";ERROR: Decimal constant is out of range (2, 4, 8).";
    static const inline QString badAlignArgument = ";ERROR: .ALIGN requires a decimal constant 2, 4, or 8.";
    static const inline QString badAsciiArgument = ";ERROR: .ASCII requires a string constant argument.";

    static const inline QString badBlockArgument = ";ERROR: .BLOCK requires a decimal or hex constant argument.";
    static const inline QString badBurnArgument = ";ERROR: .BURN requires a hex constant argument.";
    static const inline QString badByteArgument = ";ERROR: .BYTE requires a char, dec, hex, or string constant argument.";
};

#endif // MACROASSEMBLER_H
