// File: asmcode.h
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.
    
    Copyright (C) 2009  J. Stanley Warford, Pepperdine University

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ASMCODE_H
#define ASMCODE_H

#include "pep.h"
#include "tracedata.h"
#include <QSharedPointer>
#include <QSet>
class AsmArgument; // Forward declaration for attributes of code classes.
class SymbolRefArgument; // Forward declare identifier argument.
class AsmArgumentList; //Forward declare a list of arguments.
class SymbolEntry;
struct ModuleInstance;
/*
 * Abstract Code class that represents a single line of assembly code.
 * It contains methods for generating object code, & pretty-printing source code.
 * It assists the memory trace pane's stack frames with trace tag processing.
 * It also provides default implementations for most functions to reduce code reuse in subclasses.
 */
class AsmCode
{
    friend class IsaAsm;
public:
    AsmCode();
    AsmCode(const AsmCode& other);
    virtual ~AsmCode() = 0;
    // Cannot support operator= in AsmCode, it is pure virtual.
    virtual AsmCode* cloneAsmCode() const = 0;

    bool hasSymbolEntry() const {return !symbolEntry.isNull();}
    // Before attempting to use the value return by this function, check if the symbol is null.
    // Dereferencing an empty shared pointer causes memory access violatations that are hard to debug.
    QSharedPointer<SymbolEntry> getSymbolEntry() {return symbolEntry;}
    QSharedPointer<const SymbolEntry> getSymbolEntry() const {return symbolEntry;}
    void setSymbolEntry(QSharedPointer< SymbolEntry> symbol) {symbolEntry = symbol;}

    // Set if object code should be generated for this code line. If an instruction is before
    // a .BURN directive, then this should be set to false. If this is false, object code
    // length should be 0.
    void setEmitObjectCode(bool emitObject);
    bool getEmitObjectCode() const;

    virtual bool hasComment() const;
    QString getComment() const;
    void setComment(QString);

    // Can this line have trace tags?
    virtual bool tracksTraceTags() const {return false;}

    // Detailed information about how the instruction interacts with the memory trace.
    QList<TraceCommand> getTraceData() const;
    void setTraceData(QList<TraceCommand> trace);

    virtual void appendObjectCode(QList<int> &) const { return; }

    virtual int getMemoryAddress() const;
    void setMemoryAddress(quint16 address);
    virtual void adjustMemAddress(int addressDelta);

    // The line number (0 indexed) of the line of code in the source program.
    virtual int getSourceLineNumber() const;
    void setSourceLineNumber(quint32 lineNumber);

    // The line number (0 indexed) of the line of code in the listing.
    virtual int getListingLineNumber() const;
    void setListingLineNumber(quint32 lineNumber);

    // Get the assembler listing, which is memaddress + object code + sourceLine.
    virtual QString getAssemblerListing() const = 0;
    // Returns the properly formatted source line.
    virtual QString getAssemblerSource() const = 0;
    virtual quint16 objectCodeLength() const {return 0;}

    virtual bool hasBreakpoint() const { return false;}
    virtual void setBreakpoint(bool) {}

    virtual bool hasSymbolicOperand() const {return false;}
    virtual QSharedPointer<const SymbolEntry> getSymbolicOperand() const { return nullptr;}
    friend void swap(AsmCode& first, AsmCode& second)
    {
        using std::swap;
        swap(first.emitObjectCode, second.emitObjectCode);
        swap(first.hasCom, second.hasCom);
        swap(first.sourceCodeLine, second.sourceCodeLine);
        swap(first.listingCodeLine, second.listingCodeLine);
        swap(first.memAddress, second.memAddress);
        swap(first.symbolEntry, second.symbolEntry);
        swap(first.comment, second.comment);
        swap(first.trace, second.trace);

    }

protected:
    bool emitObjectCode, hasCom;
    int sourceCodeLine, listingCodeLine, memAddress =-1;
    QSharedPointer<SymbolEntry> symbolEntry;
    QString comment;
    // Information collected during assembly to enable memory tracing features.
    QList<TraceCommand> trace;
};

// Concrete code classes
class UnaryInstruction: public AsmCode
{
    friend class IsaAsm;
private:
    Enu::EMnemonic mnemonic;
    bool breakpoint = false;
public:
    UnaryInstruction() = default;
    ~UnaryInstruction() override = default;
    UnaryInstruction(const UnaryInstruction& other);
    UnaryInstruction& operator=(UnaryInstruction other);
    AsmCode *cloneAsmCode() const override;


    void appendObjectCode(QList<int> &objectCode) const override;
//    NO LONGER WITH PEP/9. FOR RET1, RET2, ..., RET7
//    bool processSymbolTraceTags(int &sourceLine, QString &errorString);

    // AsmCode interface
    QString getAssemblerListing() const override;
    QString getAssemblerSource() const override;
    quint16 objectCodeLength() const override;

    bool hasBreakpoint() const override;
    void setBreakpoint(bool b) override;

    Enu::EMnemonic getMnemonic() const;
    void setMnemonic(Enu::EMnemonic);

    bool tracksTraceTags() const override;

    friend void swap(UnaryInstruction& first, UnaryInstruction& second)
    {
        using std::swap;
        swap(static_cast<AsmCode&>(first), static_cast<AsmCode&>(second));
        swap(first.breakpoint, second.breakpoint);
        swap(first.mnemonic, second.mnemonic);
    }
};

class NonUnaryInstruction: public AsmCode
{
    friend class IsaAsm;
private:
    Enu::EMnemonic mnemonic;
    Enu::EAddrMode addressingMode;
    QSharedPointer<AsmArgument> argument = nullptr;
    bool breakpoint = false;
public:
    NonUnaryInstruction() = default;
     ~NonUnaryInstruction() override = default;
    NonUnaryInstruction(const NonUnaryInstruction& other);
    NonUnaryInstruction& operator=(NonUnaryInstruction other);
    AsmCode *cloneAsmCode() const override;
    // ~NonUnaryInstruction() { delete argument; }
    void appendObjectCode(QList<int> &objectCode) const override;

    // AsmCode interface
    QString getAssemblerListing() const override;
    QString getAssemblerSource() const override;
    quint16 objectCodeLength() const override;

    bool hasBreakpoint() const override;
    void setBreakpoint(bool b) override;

    Enu::EMnemonic getMnemonic() const;
    void setMnemonic(Enu::EMnemonic);

    Enu::EAddrMode getAddressingMode() const;
    void setAddressingMode(Enu::EAddrMode);

    bool hasSymbolicOperand() const override;
    QSharedPointer<const SymbolEntry> getSymbolicOperand() const override;
    QSharedPointer<AsmArgument> getArgument() const;
    void setArgument(QSharedPointer<AsmArgument>);

    bool tracksTraceTags() const override;

    friend void swap(NonUnaryInstruction& first, NonUnaryInstruction& second)
    {
        using std::swap;
        swap(static_cast<AsmCode&>(first), static_cast<AsmCode&>(second));
        swap(first.mnemonic, second.mnemonic);
        swap(first.addressingMode, second.addressingMode);
        swap(first.argument, second.argument);
        swap(first.breakpoint, second.breakpoint);
    }
};

class DotAddrss: public AsmCode
{
    friend class IsaAsm;
private:
    QSharedPointer<AsmArgument> argument = nullptr;
public:
    DotAddrss() = default;
    ~DotAddrss() override = default;
    DotAddrss(const DotAddrss& other);
    DotAddrss& operator=(DotAddrss other);
    AsmCode *cloneAsmCode() const override;
     void appendObjectCode(QList<int> &objectCode) const override;

    // AsmCode interface
    QString getAssemblerListing() const override;
    QString getAssemblerSource() const override;
    quint16 objectCodeLength() const override;

    bool hasSymbolicOperand() const override;
    QSharedPointer<const SymbolEntry> getSymbolicOperand() const override;
    QSharedPointer<AsmArgument> getArgument() const;
    void setArgument(QSharedPointer<AsmArgument>);

    friend void swap(DotAddrss& first, DotAddrss& second)
    {
        using std::swap;
        swap(static_cast<AsmCode&>(first), static_cast<AsmCode&>(second));
        swap(first.argument, second.argument);
    }
};

class DotAlign: public AsmCode
{
    friend class IsaAsm;
private:
    QSharedPointer<AsmArgument> argument = nullptr;
    QSharedPointer<AsmArgument> numBytesGenerated = nullptr;
public:
    DotAlign() = default;
    virtual ~DotAlign() override = default;
    DotAlign(const DotAlign& other);
    DotAlign& operator=(DotAlign other);
    AsmCode *cloneAsmCode() const override;
    void appendObjectCode(QList<int> &objectCode) const override;

    // AsmCode interface
    QString getAssemblerListing() const override;
    QString getAssemblerSource() const override;
    quint16 objectCodeLength() const override;

    QSharedPointer<AsmArgument> getArgument() const;
    void setArgument(QSharedPointer<AsmArgument>);

    quint16 getNumBytesGenerated() const;
    void setNumBytesGenerated(quint16);

    friend void swap(DotAlign& first, DotAlign& second)
    {
        using std::swap;
        swap(static_cast<AsmCode&>(first), static_cast<AsmCode&>(second));
        swap(first.argument, second.argument);
        swap(first.numBytesGenerated, second.numBytesGenerated);
    }

};

class DotAscii: public AsmCode
{
    friend class IsaAsm;
private:
    QSharedPointer<AsmArgument> argument = nullptr;
public:
    DotAscii() = default;
    ~DotAscii() override = default;
    DotAscii(const DotAscii& other);
    DotAscii& operator=(DotAscii other);
    AsmCode *cloneAsmCode() const override;
    void appendObjectCode(QList<int> &objectCode) const override;

    // AsmCode interface
    QString getAssemblerListing() const override;
    QString getAssemblerSource() const override;
    virtual quint16 objectCodeLength() const override;

    QSharedPointer<AsmArgument> getArgument() const;
    void setArgument(QSharedPointer<AsmArgument>);

    friend void swap(DotAscii& first, DotAscii& second)
    {
        using std::swap;
        swap(static_cast<AsmCode&>(first), static_cast<AsmCode&>(second));
        swap(first.argument, second.argument);
    }
};

class DotBlock: public AsmCode
{
    friend class IsaAsm;
private:
    QSharedPointer<AsmArgument> argument = nullptr;
public:
    DotBlock() = default;
    ~DotBlock() override = default;
    DotBlock(const DotBlock& other);
    DotBlock& operator=(DotBlock other);
    AsmCode *cloneAsmCode() const override;

    void appendObjectCode(QList<int> &objectCode) const override;
    // AsmCode interface
    QString getAssemblerListing() const override;
    QString getAssemblerSource() const override;
    quint16 objectCodeLength() const override;

    QSharedPointer<AsmArgument> getArgument() const;
    void setArgument(QSharedPointer<AsmArgument>);

    bool tracksTraceTags() const override;

    friend void swap(DotBlock& first, DotBlock& second)
    {
        using std::swap;
        swap(static_cast<AsmCode&>(first), static_cast<AsmCode&>(second));
        swap(first.argument, second.argument);
    }
};

class DotBurn: public AsmCode
{
    friend class IsaAsm;
private:
    QSharedPointer<AsmArgument> argument = nullptr;

public:
    DotBurn() = default;
    virtual ~DotBurn() override = default;
    DotBurn(const DotBurn& other);
    DotBurn& operator=(DotBurn other);
    AsmCode *cloneAsmCode() const override;

    // AsmCode interface
    QString getAssemblerListing() const override;
    QString getAssemblerSource() const override;

    QSharedPointer<AsmArgument> getArgument() const;
    void setArgument(QSharedPointer<AsmArgument>);

    friend void swap(DotBurn& first, DotBurn& second)
    {
        using std::swap;
        swap(static_cast<AsmCode&>(first), static_cast<AsmCode&>(second));
        swap(first.argument, second.argument);
    }
};

class DotByte: public AsmCode
{
    friend class IsaAsm;
private:
    QSharedPointer<AsmArgument> argument = nullptr;

public:
    DotByte() = default;
    ~DotByte() override = default;
    DotByte(const DotByte& other);
    DotByte& operator=(DotByte other);
    AsmCode *cloneAsmCode() const override;
    void appendObjectCode(QList<int> &objectCode) const override;

    // AsmCode interface
    virtual QString getAssemblerListing() const override;
    QString getAssemblerSource() const override;
    quint16 objectCodeLength() const override;

    QSharedPointer<AsmArgument> getArgument() const;
    void setArgument(QSharedPointer<AsmArgument>);

    bool tracksTraceTags() const override;

    friend void swap(DotByte& first, DotByte& second)
    {
        using std::swap;
        swap(static_cast<AsmCode&>(first), static_cast<AsmCode&>(second));
        swap(first.argument, second.argument);
    }
};

class DotEnd: public AsmCode
{
    friend class IsaAsm;
public:
    DotEnd() = default;
    ~DotEnd() override = default;
    DotEnd(const DotEnd& other);
    DotEnd& operator=(DotEnd other);
    AsmCode *cloneAsmCode() const override;
    // AsmCode interface
    QString getAssemblerListing() const override;
    QString getAssemblerSource() const override;

    friend void swap(DotEnd& first, DotEnd& second)
    {
        using std::swap;
        swap(static_cast<AsmCode&>(first), static_cast<AsmCode&>(second));
    }
};

class DotEquate: public AsmCode
{
    friend class IsaAsm;
private:
    QSharedPointer<AsmArgument> argument = nullptr;
public:
    DotEquate() = default;
    ~DotEquate() override = default;
    DotEquate(const DotEquate& other);
    DotEquate& operator=(DotEquate other);
    AsmCode *cloneAsmCode() const override;

    // AsmCode interface
    QString getAssemblerListing() const override;
    QString getAssemblerSource() const override;

    QSharedPointer<AsmArgument> getArgument() const;
    void setArgument(QSharedPointer<AsmArgument>);

    bool tracksTraceTags() const override;

    friend void swap(DotEquate& first, DotEquate& second)
    {
        using std::swap;
        swap(static_cast<AsmCode&>(first), static_cast<AsmCode&>(second));
        swap(first.argument, second.argument);
    }
};

class DotExport: public AsmCode
{
    QSharedPointer<SymbolRefArgument> argument = nullptr;
public:
    DotExport() = default;
    ~DotExport() override = default;
    DotExport(const DotExport& other);
    DotExport& operator=(DotExport other);
    AsmCode *cloneAsmCode() const override;
    void appendObjectCode(QList<int> &objectCode) const override;

    // AsmCode interface
    QString getAssemblerListing() const override;
    QString getAssemblerSource() const override;
    quint16 objectCodeLength() const override;

    bool hasSymbolicOperand() const override;
    QSharedPointer<const SymbolEntry> getSymbolicOperand() const override;
    QSharedPointer<SymbolRefArgument> getArgument() const;
    void setArgument(QSharedPointer<SymbolRefArgument>);

    friend void swap(DotExport& first, DotExport& second)
    {
        using std::swap;
        swap(static_cast<AsmCode&>(first), static_cast<AsmCode&>(second));
        swap(first.argument, second.argument);
    }
};

class DotSycall: public AsmCode
{
    QSharedPointer<SymbolRefArgument> argument = nullptr;
public:
    DotSycall() = default;
    ~DotSycall() override = default;
    DotSycall(const DotSycall& other);
    DotSycall& operator=(DotSycall other);
    AsmCode *cloneAsmCode() const override;
    void appendObjectCode(QList<int> &objectCode) const override;

    // AsmCode interface
    QString getAssemblerListing() const override;
    QString getAssemblerSource() const override;
    quint16 objectCodeLength() const override;

    bool hasSymbolicOperand() const override;
    QSharedPointer<const SymbolEntry> getSymbolicOperand() const override;
    QSharedPointer<SymbolRefArgument> getArgument() const;
    void setArgument(QSharedPointer<SymbolRefArgument>);

    friend void swap(DotSycall& first, DotSycall& second)
    {
        using std::swap;
        swap(static_cast<AsmCode&>(first), static_cast<AsmCode&>(second));
        swap(first.argument, second.argument);
    }
};

class DotUSycall: public AsmCode
{
    QSharedPointer<SymbolRefArgument> argument = nullptr;
public:
    DotUSycall() = default;
    ~DotUSycall() override = default;
    DotUSycall(const DotUSycall& other);
    DotUSycall& operator=(DotUSycall other);
    AsmCode *cloneAsmCode() const override;
    void appendObjectCode(QList<int> &objectCode) const override;

    // AsmCode interface
    virtual QString getAssemblerListing() const override;
    QString getAssemblerSource() const override;
    quint16 objectCodeLength() const override;

    bool hasSymbolicOperand() const override;
    QSharedPointer<const SymbolEntry> getSymbolicOperand() const override;
    QSharedPointer<SymbolRefArgument> getArgument() const;
    void setArgument(QSharedPointer<SymbolRefArgument>);

    friend void swap(DotUSycall& first, DotUSycall& second)
    {
        using std::swap;
        swap(static_cast<AsmCode&>(first), static_cast<AsmCode&>(second));
        swap(first.argument, second.argument);
    }
};

class DotWord: public AsmCode
{
    friend class IsaAsm;
private:
    QSharedPointer<AsmArgument> argument = nullptr;
public:
    DotWord() = default;
    virtual ~DotWord() override = default;
    DotWord(const DotWord& other);
    DotWord& operator=(DotWord other);
    AsmCode *cloneAsmCode() const override;

    void appendObjectCode(QList<int> &objectCode) const override;

    // AsmCode interface
    virtual QString getAssemblerListing() const override;
    QString getAssemblerSource() const override;
    quint16 objectCodeLength() const override;

    QSharedPointer<AsmArgument> getArgument() const;
    void setArgument(QSharedPointer<AsmArgument>);

    bool tracksTraceTags() const override;

    friend void swap(DotWord& first, DotWord& second)
    {
        using std::swap;
        swap(static_cast<AsmCode&>(first), static_cast<AsmCode&>(second));
        swap(first.argument, second.argument);
    }
};

class CommentOnly: public AsmCode
{
    friend class IsaAsm;
public:
    CommentOnly() = default;
    ~CommentOnly() override = default;
    CommentOnly(const CommentOnly& other);
    CommentOnly& operator=(CommentOnly other);
    AsmCode *cloneAsmCode() const override;

    // AsmCode interface
    QString getAssemblerListing() const override;
    QString getAssemblerSource() const override;

    friend void swap(CommentOnly& first, CommentOnly& second)
    {
        using std::swap;
        swap(static_cast<AsmCode&>(first), static_cast<AsmCode&>(second));
    }
};

class BlankLine: public AsmCode
{
    friend class IsaAsm;
public:
    BlankLine() = default;
    ~BlankLine() override = default;
    BlankLine(const BlankLine& other);
    BlankLine& operator=(BlankLine other);
    AsmCode *cloneAsmCode() const override;

    // AsmCode interface
    QString getAssemblerListing() const override;
    QString getAssemblerSource() const override;

    friend void swap(BlankLine& first, BlankLine& second)
    {
        using std::swap;
        swap(static_cast<AsmCode&>(first), static_cast<AsmCode&>(second));
    }
};

class MacroInvoke: public AsmCode
{
private:
    QStringList argumentList;
    QSharedPointer<ModuleInstance> macroInstance;
public:
    MacroInvoke() = default;
    ~MacroInvoke() override = default;
    MacroInvoke(const MacroInvoke& other);
    MacroInvoke& operator=(MacroInvoke other);
    AsmCode *cloneAsmCode() const override;
    void appendObjectCode(QList<int> &objectCode) const override;
    void adjustMemAddress(int addressDelta) override;

    // AsmCode interface
    QString getAssemblerListing() const override;
    QString getAssemblerSource() const override;
    quint16 objectCodeLength() const override;

    QStringList getArgumentList() const;
    void setArgumentList(QStringList);

    QSharedPointer<ModuleInstance> getMacroInstance() const;
    void setMacroInstance(QSharedPointer<ModuleInstance>);


    friend void swap(MacroInvoke& first, MacroInvoke& second)
    {
        using std::swap;
        swap(static_cast<AsmCode&>(first), static_cast<AsmCode&>(second));
        swap(first.argumentList, second.argumentList);
        swap(first.macroInstance, second.macroInstance);
    }
};
#endif // CODE_H
