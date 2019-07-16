// File: asmcode.cpp
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

#include <QRegExp>
#include <QSharedPointer>

#include "asmcode.h"
#include "asmargument.h"
#include "isaasm.h"
#include "macromodules.h"
#include "pep.h"
#include "symbolvalue.h"
#include "symbolentry.h"
#include "symboltable.h"

AsmCode::AsmCode(): sourceCodeLine(0), memAddress(0), symbolEntry(QSharedPointer<SymbolEntry>()), comment(),
    emitObjectCode(true), hasCom(false)
{

}

AsmCode::AsmCode(const AsmCode &other)
{
    this->sourceCodeLine = other.sourceCodeLine;
    this->memAddress = other.memAddress;
    this->symbolEntry = other.symbolEntry;
    this->comment = other.comment;
    this->emitObjectCode = other.emitObjectCode;
    this->hasCom = other.hasCom;
}

UnaryInstruction::UnaryInstruction(const UnaryInstruction &other) : AsmCode(other)
{
    this->breakpoint = other.breakpoint;
    this->mnemonic = other.mnemonic;
}

NonUnaryInstruction::NonUnaryInstruction(const NonUnaryInstruction &other) : AsmCode(other)
{
    this->mnemonic = other.mnemonic;
    this->addressingMode = other.addressingMode;
    this->argument = other.argument;
    this->breakpoint = other.breakpoint;
}
DotAddrss::DotAddrss(const DotAddrss &other) : AsmCode(other)
{
    this->argument = other.argument;
}

DotAlign::DotAlign(const DotAlign &other) : AsmCode(other)
{
    this->argument = other.argument;
    this->numBytesGenerated = other.numBytesGenerated;
}

DotAscii::DotAscii(const DotAscii &other) : AsmCode(other)
{
    this->argument = other.argument;
}

DotBlock::DotBlock(const DotBlock &other) : AsmCode(other)
{
    this->argument = other.argument;
}

DotBurn::DotBurn(const DotBurn &other) : AsmCode(other)
{
    this->argument = other.argument;
}
DotByte::DotByte(const DotByte &other) : AsmCode(other)
{
    this->argument = other.argument;
}
DotEnd::DotEnd(const DotEnd &other) : AsmCode(other)
{

}

DotEquate::DotEquate(const DotEquate &other) : AsmCode(other)
{
    this->argument = other.argument;
}

DotWord::DotWord(const DotWord &other) : AsmCode(other)
{
    this->argument = other.argument;
}
CommentOnly::CommentOnly(const CommentOnly &other)  : AsmCode(other)
{

}
BlankLine::BlankLine(const BlankLine &other)  : AsmCode(other)
{

}

MacroInvoke::MacroInvoke(const MacroInvoke &other) : AsmCode(other)
{
   this->argumentList = other.argumentList;
   this->macroInstance = other.macroInstance;
}

UnaryInstruction &UnaryInstruction::operator=(UnaryInstruction other)
{
    swap(*this, other);
    return *this;
}

NonUnaryInstruction &NonUnaryInstruction::operator=(NonUnaryInstruction other)
{
    swap(*this, other);
    return *this;
}

DotAddrss &DotAddrss::operator=(DotAddrss other)
{
    swap(*this, other);
    return *this;
}

DotAlign &DotAlign::operator=(DotAlign other)
{
    swap(*this, other);
    return *this;
}

DotAscii &DotAscii::operator=(DotAscii other)
{
    swap(*this, other);
    return *this;
}

DotBlock &DotBlock::operator=(DotBlock other)
{
    swap(*this, other);
    return *this;
}

DotBurn &DotBurn::operator=(DotBurn other)
{
    swap(*this, other);
    return *this;
}

DotByte &DotByte::operator=(DotByte other)
{
    swap(*this, other);
    return *this;
}

DotEnd &DotEnd::operator=(DotEnd other)
{
    swap(*this, other);
    return *this;
}

DotEquate &DotEquate::operator=(DotEquate other)
{
    swap(*this, other);
    return *this;
}

DotWord &DotWord::operator=(DotWord other)
{
    swap(*this, other);
    return *this;
}

CommentOnly &CommentOnly::operator=(CommentOnly other)
{
    swap(*this, other);
    return *this;
}

BlankLine &BlankLine::operator=(BlankLine other)
{
    swap(*this, other);
    return *this;
}

MacroInvoke &MacroInvoke::operator=(MacroInvoke other)
{
    swap(*this, other);
    return *this;
}

AsmCode::~AsmCode()
{

}

void AsmCode::setEmitObjectCode(bool emitObject)
{
    emitObjectCode = emitObject;
}

bool AsmCode::getEmitObjectCode() const
{
    return emitObjectCode;
}

bool AsmCode::hasComment() const
{
    return hasCom;
}

QString AsmCode::getComment() const
{
    return comment;
}

void AsmCode::setComment(QString comment)
{
    this->comment = comment;
}

int AsmCode::getMemoryAddress() const
{
    return memAddress;
}

void AsmCode::setMemoryAddress(quint16 address)
{
    this->memAddress = address;
}

void AsmCode::adjustMemAddress(int addressDelta)
{
    // Memory addresses less than 0 are invalid or don't
    // have a real address (like comments), so they can't
    // be relocated.
    if(memAddress >=0) memAddress += addressDelta;
}



AsmCode *UnaryInstruction::cloneAsmCode() const
{
    return new UnaryInstruction(*this);
}

AsmCode *NonUnaryInstruction::cloneAsmCode() const
{
    return new NonUnaryInstruction(*this);
}

AsmCode *DotAddrss::cloneAsmCode() const
{
    return new DotAddrss(*this);
}

AsmCode *DotAlign::cloneAsmCode() const
{
    return new DotAlign(*this);
}

AsmCode *DotAscii::cloneAsmCode() const
{
    return new DotAscii(*this);
}

AsmCode *DotBlock::cloneAsmCode() const
{
    return new DotBlock(*this);
}

AsmCode *DotBurn::cloneAsmCode() const
{
    return new DotBurn(*this);
}

AsmCode *DotByte::cloneAsmCode() const
{
    return new DotByte(*this);
}

AsmCode *DotEnd::cloneAsmCode() const
{
    return new DotEnd(*this);
}

AsmCode *DotEquate::cloneAsmCode() const
{
    return new DotEquate(*this);
}

AsmCode *DotWord::cloneAsmCode() const
{
    return new DotWord(*this);
}

AsmCode *CommentOnly::cloneAsmCode() const
{
    return new CommentOnly(*this);
}

AsmCode *BlankLine::cloneAsmCode() const
{
    return new BlankLine(*this);
}

AsmCode *MacroInvoke::cloneAsmCode() const
{
    return new MacroInvoke(*this);
}

void UnaryInstruction::appendObjectCode(QList<int> &objectCode) const
{
    if(!emitObjectCode) return;
    objectCode.append(Pep::opCodeMap.value(mnemonic));
}

void NonUnaryInstruction::appendObjectCode(QList<int> &objectCode) const
{
    if(!emitObjectCode) return;
    int instructionSpecifier = Pep::opCodeMap.value(mnemonic);
    if (Pep::addrModeRequiredMap.value(mnemonic)) {
        instructionSpecifier += Pep::aaaAddressField(addressingMode);
    }
    else {
        instructionSpecifier += Pep::aAddressField(addressingMode);
    }
    objectCode.append(instructionSpecifier);
    int operandSpecifier = argument->getArgumentValue();
    objectCode.append(operandSpecifier / 256);
    objectCode.append(operandSpecifier % 256);
}

void DotAddrss::appendObjectCode(QList<int> &objectCode) const
{
    if(!emitObjectCode) return;
    int symbolValue = this->argument->getArgumentValue();
    objectCode.append(symbolValue / 256);
    objectCode.append(symbolValue % 256);

}

void DotAlign::appendObjectCode(QList<int> &objectCode) const
{
    if(!emitObjectCode) return;
    for (int i = 0; i < numBytesGenerated->getArgumentValue(); i++) {
        objectCode.append(0);
    }
}

void DotAscii::appendObjectCode(QList<int> &objectCode) const
{
    if(!emitObjectCode) return;
        int value;
        QString str = argument->getArgumentString();
        str.remove(0, 1); // Remove the leftmost double quote.
        str.chop(1); // Remove the rightmost double quote.
        while (str.length() > 0) {
            IsaParserHelper::unquotedStringToInt(str, value);
            objectCode.append(value);
        }
}

void DotBlock::appendObjectCode(QList<int> &objectCode) const
{
    if(!emitObjectCode) return;
    for (int i = 0; i < argument->getArgumentValue(); i++) {
        objectCode.append(0);
    }
}

void DotByte::appendObjectCode(QList<int> &objectCode) const
{
    if(!emitObjectCode) return;
    objectCode.append(argument->getArgumentValue());
}

void DotWord::appendObjectCode(QList<int> &objectCode) const
{
    if(!emitObjectCode) return;
    int value = argument->getArgumentValue();
    objectCode.append(value / 256);
    objectCode.append(value % 256);
}

bool UnaryInstruction::hasBreakpoint() const
{
    return breakpoint;
}

void UnaryInstruction::setBreakpoint(bool b)
{
    breakpoint = b;
}

Enu::EMnemonic UnaryInstruction::getMnemonic() const
{
    return this->mnemonic;
}

void UnaryInstruction::setMnemonic(Enu::EMnemonic mnemonic)
{
    this->mnemonic = mnemonic;
}

bool NonUnaryInstruction::hasBreakpoint() const
{
    return breakpoint;
}

void NonUnaryInstruction::setBreakpoint(bool b)
{
    breakpoint = b;
}

bool NonUnaryInstruction::hasSymbolicOperand() const
{
    return dynamic_cast<SymbolRefArgument*>(argument) != nullptr;
}

QSharedPointer<const SymbolEntry> NonUnaryInstruction::getSymbolicOperand() const
{
    return dynamic_cast<SymbolRefArgument*>(argument)->getSymbolValue();
}

AsmArgument *NonUnaryInstruction::getArgument() const
{
    return argument;
}

void NonUnaryInstruction::setArgument(AsmArgument * argument)
{
    this->argument = argument;
}

Enu::EMnemonic NonUnaryInstruction::getMnemonic() const
{
    return mnemonic;
}

void NonUnaryInstruction::setMnemonic(Enu::EMnemonic mnemonic)
{
    this->mnemonic = mnemonic;
}

Enu::EAddrMode NonUnaryInstruction::getAddressingMode() const
{
    return addressingMode;
}

void NonUnaryInstruction::setAddressingMode(Enu::EAddrMode addressingMode)
{
    this->addressingMode = addressingMode;
}

QString UnaryInstruction::getAssemblerListing() const
{
    QString memStr = QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper();
    // Potentially skip codegen
    QString codeStr;
    if(emitObjectCode) {
        codeStr = QString("%1").arg(Pep::opCodeMap.value(mnemonic), 2, 16, QLatin1Char('0')).toUpper();
    }
    else {
        codeStr = "  ";
    }

    QString lineStr = QString("%1%2%3")
                      .arg(memStr, -6, QLatin1Char(' '))
                      .arg(codeStr, -7, QLatin1Char(' '))
                      .arg(getAssemblerSource());
    return lineStr;
}

QString UnaryInstruction::getAssemblerSource() const
{
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString mnemonStr = Pep::enumToMnemonMap.value(mnemonic);
    QString lineStr = QString("%1%2%3")
                      .arg(symbolStr, -9, QLatin1Char(' '))
                      .arg(mnemonStr, -8, QLatin1Char(' '))
                      .arg("            " + comment);
    return lineStr;
}

QString NonUnaryInstruction::getAssemblerListing() const
{
    QString memStr = QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper();
    int temp = Pep::opCodeMap.value(mnemonic);
    temp += Pep::addrModeRequiredMap.value(mnemonic) ? Pep::aaaAddressField(addressingMode) : Pep::aAddressField(addressingMode);
    // Potentially skip codegen
    QString codeStr;
    QString oprndNumStr;
    if(emitObjectCode) {
        codeStr = QString("%1").arg(temp, 2, 16, QLatin1Char('0')).toUpper();
        oprndNumStr = QString("%1").arg(argument->getArgumentValue(), 4, 16, QLatin1Char('0')).toUpper();
    }
    else {
        codeStr = "  ";
        oprndNumStr = " ";
    }

    QString lineStr = QString("%1%2%3%4")
                      .arg(memStr, -6, QLatin1Char(' '))
                      .arg(codeStr, -2)
                      .arg(oprndNumStr, -5, QLatin1Char(' '))
                      .arg(getAssemblerSource(), -29);
    return lineStr;
}

QString NonUnaryInstruction::getAssemblerSource() const
{
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString mnemonStr = Pep::enumToMnemonMap.value(mnemonic);
    QString oprndStr = argument->getArgumentString();
    if (Pep::addrModeRequiredMap.value(mnemonic)) {
        oprndStr.append("," + Pep::intToAddrMode(addressingMode));
    }
    else if (addressingMode == Enu::EAddrMode::X) {
        oprndStr.append("," + Pep::intToAddrMode(addressingMode));
    }
    QString lineStr = QString("%1%2%3%4")
                      .arg(symbolStr, -9, QLatin1Char(' '))
                      .arg(mnemonStr, -8, QLatin1Char(' '))
                      .arg(oprndStr, -12)
                      .arg(comment);
    return lineStr;
}

QString DotAddrss::getAssemblerListing() const
{
    QString memStr = QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper();
    int symbolValue = this->argument->getArgumentValue();

    // Potentially skip codegen
    QString codeStr;
    if(emitObjectCode) {
        codeStr = QString("%1").arg(symbolValue, 4, 16, QLatin1Char('0')).toUpper();
    }
    else {
        codeStr = "";
    }

    QString lineStr = QString("%1%2%3")
                      .arg(memStr, -6, QLatin1Char(' '))
                      .arg(codeStr, -7, QLatin1Char(' '))
                      .arg(getAssemblerSource());
    return lineStr;
}

QString DotAddrss::getAssemblerSource() const
{
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = ".ADDRSS";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("%1%2%3%4")
                      .arg(symbolStr, -9, QLatin1Char(' '))
                      .arg(dotStr, -8, QLatin1Char(' '))
                      .arg(oprndStr, -12)
                      .arg(comment);
    return lineStr;
}

QString DotAlign::getAssemblerListing() const
{
    int numBytes = numBytesGenerated->getArgumentValue();
    QString memStr = numBytes == 0 ? "      " : QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper();
    // Potentially skip codegen
    QString codeStr = "";
    while (emitObjectCode && (numBytes > 0) && (codeStr.length() < 6)) {
        codeStr.append("00");
        numBytes--;
    }
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = ".ALIGN";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("%1%2%3%4%5%6")
            .arg(memStr, -6, QLatin1Char(' '))
            .arg(codeStr, -7, QLatin1Char(' '))
            .arg(symbolStr, -9, QLatin1Char(' '))
            .arg(dotStr, -8, QLatin1Char(' '))
            .arg(oprndStr, -12)
            .arg(comment);
    while (numBytes > 0) {
        codeStr = "";
        while ((numBytes > 0) && (codeStr.length() < 6)) {
            codeStr.append("00");
            numBytes--;
        }
        lineStr.append(QString("\n      %1").arg(codeStr, -7, QLatin1Char(' ')));
    }
    return lineStr;
}

QString DotAlign::getAssemblerSource() const
{
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = ".ALIGN";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("%1%2%3%4")
            .arg(symbolStr, -9, QLatin1Char(' '))
            .arg(dotStr, -8, QLatin1Char(' '))
            .arg(oprndStr, -12)
            .arg(comment);
    return lineStr;
}

QString DotAscii::getAssemblerListing() const
{
    QString memStr = QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper();
    QString str = argument->getArgumentString();
    str.remove(0, 1); // Remove the leftmost double quote.
    str.chop(1); // Remove the rightmost double quote.
    int value;
    // Potentially skip codegen
    QString codeStr = "";
    while (emitObjectCode && (str.length() > 0) && (codeStr.length() < 6)) {
        IsaParserHelper::unquotedStringToInt(str, value);
        codeStr.append(QString("%1").arg(value, 2, 16, QLatin1Char('0')).toUpper());
    }

    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = ".ASCII";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("%1%2%3%4%5%6\n")
            .arg(memStr, -6, QLatin1Char(' '))
            .arg(codeStr, -7, QLatin1Char(' '))
            .arg(symbolStr, -9, QLatin1Char(' '))
            .arg(dotStr, -8, QLatin1Char(' '))
            .arg(oprndStr, -12)
            .arg(comment);
    while (str.length() > 0) {
        codeStr = "";
        while ((str.length() > 0) && (codeStr.length() < 6)) {
            IsaParserHelper::unquotedStringToInt(str, value);
            codeStr.append(QString("%1").arg(value, 2, 16, QLatin1Char('0')).toUpper());
        }
        lineStr.append(QString("      %1").arg(codeStr, -7, QLatin1Char(' '))%"\n");
    }
    return lineStr;
}

QString DotAscii::getAssemblerSource() const
{
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = ".ASCII";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("%1%2%3%4\n")
            .arg(symbolStr, -9, QLatin1Char(' '))
            .arg(dotStr, -8, QLatin1Char(' '))
            .arg(oprndStr, -12)
            .arg(comment);
    return lineStr;
}

QString DotBlock::getAssemblerListing() const
{
    QString memStr = QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper();
    int numBytes = argument->getArgumentValue();
    // Potentially skip codegen
    QString codeStr = "";
    while (emitObjectCode && (numBytes > 0) && (codeStr.length() < 6)) {
        codeStr.append("00");
        numBytes--;
    }
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = ".BLOCK";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("%1%2%3%4%5%6")
            .arg(memStr, -6, QLatin1Char(' '))
            .arg(codeStr, -7, QLatin1Char(' '))
            .arg(symbolStr, -9, QLatin1Char(' '))
            .arg(dotStr, -8, QLatin1Char(' '))
            .arg(oprndStr, -12)
            .arg(comment);
    while (emitObjectCode && numBytes > 0) {
        codeStr = "";
        while ((numBytes > 0) && (codeStr.length() < 6)) {
            codeStr.append("00");
            numBytes--;
        }
        lineStr.append(QString("\n      %1").arg(codeStr, -7, QLatin1Char(' ')));
    }
    return lineStr;
}

QString DotBlock::getAssemblerSource() const
{
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = ".BLOCK";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("%1%2%3%4")
            .arg(symbolStr, -9, QLatin1Char(' '))
            .arg(dotStr, -8, QLatin1Char(' '))
            .arg(oprndStr, -12)
            .arg(comment);
    return lineStr;
}

QString DotBurn::getAssemblerListing() const
{
    QString memStr = QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper();
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = ".BURN";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("%1       %2%3%4%5")
            .arg(memStr, -6, QLatin1Char(' '))
            .arg(symbolStr, -9, QLatin1Char(' '))
            .arg(dotStr, -8, QLatin1Char(' '))
            .arg(oprndStr, -12)
            .arg(comment);
    return lineStr;
}

QString DotBurn::getAssemblerSource() const
{
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = ".BURN";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("%1%2%3%4")
            .arg(symbolStr, -9, QLatin1Char(' '))
            .arg(dotStr, -8, QLatin1Char(' '))
            .arg(oprndStr, -12)
            .arg(comment);
    return lineStr;
}

AsmArgument *DotBurn::getArgument() const
{
    return this->argument;
}

void DotBurn::setArgument(AsmArgument *argument)
{
    this->argument = argument;
}

QString DotByte::getAssemblerListing() const
{
    QString memStr = QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper();
    // Potentially skip codegen
    QString codeStr;
    if(emitObjectCode) {
        codeStr = QString("%1").arg(argument->getArgumentValue(), 2, 16, QLatin1Char('0')).toUpper();
    }
    else {
        codeStr = "";
    }
    QString lineStr = QString("%1%2%3")
            .arg(memStr, -6, QLatin1Char(' '))
            .arg(codeStr, -7, QLatin1Char(' '))
            .arg(getAssemblerSource());
    return lineStr;
}

QString DotByte::getAssemblerSource() const
{
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = ".BYTE";
    QString oprndStr = argument->getArgumentString();
    if (oprndStr.startsWith("0x")) {
        oprndStr.remove(2, 2); // Display only the last two hex characters
    }
    QString lineStr = QString("%1%2%3%4")
            .arg(symbolStr, -9, QLatin1Char(' '))
            .arg(dotStr, -8, QLatin1Char(' '))
            .arg(oprndStr, -12)
            .arg(comment);
    return lineStr;
}

QString DotEnd::getAssemblerListing() const
{
    QString memStr = QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper();
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = ".END";
    QString lineStr = QString("%1       %2")
            .arg(memStr, -6, QLatin1Char(' '))
            .arg(getAssemblerSource());
    return lineStr;
}

QString DotEnd::getAssemblerSource() const
{
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = ".END";
    QString lineStr = QString("%1%2              %3")
            .arg(symbolStr, -9, QLatin1Char(' '))
            .arg(dotStr, -8, QLatin1Char(' '))
            .arg(comment);
    return lineStr;
}

QString DotEquate::getAssemblerListing() const
{
    return "             " + getAssemblerSource();
}

QString DotEquate::getAssemblerSource() const
{
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = ".EQUATE";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("%1%2%3%4")
            .arg(symbolStr, -9, QLatin1Char(' '))
            .arg(dotStr, -8, QLatin1Char(' '))
            .arg(oprndStr, -12)
            .arg(comment);
    return lineStr;
}

AsmArgument *DotEquate::getArgument() const
{
    return this->argument;
}

void DotEquate::setArgument(AsmArgument *argument)
{
    this->argument = argument;
}

QString DotWord::getAssemblerListing() const
{
    QString memStr = QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper();
    // Potentially skip codegen
    QString codeStr;
    if(emitObjectCode) {
        codeStr = QString("%1").arg(argument->getArgumentValue(), 4, 16, QLatin1Char('0')).toUpper();
    }
    else {
        codeStr = "";
    }

    QString dotStr = ".WORD";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("%1%2%3")
            .arg(memStr, -6, QLatin1Char(' '))
            .arg(codeStr, -7, QLatin1Char(' '))
            .arg(getAssemblerSource());
    return lineStr;
}

QString DotWord::getAssemblerSource() const
{
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = ".WORD";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("%1%2%3%4")
            .arg(symbolStr, -9, QLatin1Char(' '))
            .arg(dotStr, -8, QLatin1Char(' '))
            .arg(oprndStr, -12)
            .arg(comment);
    return lineStr;
}

QString CommentOnly::getAssemblerListing() const
{
    return "             " + getAssemblerSource();
}

QString CommentOnly::getAssemblerSource() const
{
    return comment;
}

QString BlankLine::getAssemblerListing() const
{
    return "";
}

QString BlankLine::getAssemblerSource() const
{
    return "";
}

quint16 UnaryInstruction::objectCodeLength() const
{
    return 1;
}

quint16 NonUnaryInstruction::objectCodeLength() const
{
    return 3;
}

quint16 DotAddrss::objectCodeLength() const
{
    return 2;
}

bool DotAddrss::hasSymbolicOperand() const
{
    return true;
}

QSharedPointer<const SymbolEntry> DotAddrss::getSymbolicOperand() const
{
    // The value of a .addrss instruction is always the value of another symbol.
    return static_cast<SymbolRefArgument*>(argument)->getSymbolValue();
}

AsmArgument *DotAddrss::getArgument() const
{
    return this->argument;
}

void DotAddrss::setArgument(AsmArgument *argument)
{
    this->argument = argument;
}

quint16 DotAlign::objectCodeLength() const
{
    if(emitObjectCode) {
        return static_cast<quint16>(numBytesGenerated->getArgumentValue());
    }
    else {
        return 0;
    }
}

AsmArgument *DotAlign::getArgument() const
{
    return this->argument;
}

void DotAlign::setArgument(AsmArgument *argument)
{
    this->argument = argument;
}

quint16 DotAlign::getNumBytesGenerated() const
{
    return numBytesGenerated->getArgumentValue();
}

quint16 DotAscii::objectCodeLength() const
{
    QList<int> num;
    appendObjectCode(num);
    return static_cast<quint16>(num.length());
}

AsmArgument *DotAscii::getArgument() const
{
    return this->argument;
}

void DotAscii::setArgument(AsmArgument * argument)
{
    this->argument = argument;
}

quint16 DotBlock::objectCodeLength() const
{
    if(emitObjectCode) {
        return static_cast<quint16>(argument->getArgumentValue());
    }
    else {
        return 0;
    }
}

AsmArgument *DotBlock::getArgument() const
{
    return this->argument;
}

void DotBlock::setArgument(AsmArgument* argument)
{
    this->argument = argument;
}

quint16 DotByte::objectCodeLength() const
{
    if(emitObjectCode) {
        return 1;
    }
    else {
        return 0;
    }
}

AsmArgument *DotByte::getArgument() const
{
    return this->argument;
}

void DotByte::setArgument(AsmArgument *argument)
{
    this->argument = argument;
}

quint16 DotWord::objectCodeLength() const
{
    if(emitObjectCode) {
        return 2;
    }
    else {
        return 0;
    }
}

AsmArgument *DotWord::getArgument() const
{
    return this->argument;
}

void DotWord::setArgument(AsmArgument *argument)
{
    this->argument = argument;
}

void MacroInvoke::appendObjectCode(QList<int> &objectCode) const
{

}

QString MacroInvoke::getAssemblerListing() const
{
    QStringList list;
    for(auto code : this->macroInstance->codeList) {
        // Don't generate listing for anything after and including .END
        if(dynamic_cast<DotEnd*>(code) != nullptr) break;
        list << code->getAssemblerListing();
    }
    return list.join("\n");
}

QString MacroInvoke::getAssemblerSource() const
{
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = "%"+macroInstance->prototype->name;
    QString oprndStr = argumentList.join(',');
    // Split generation at dotStr, as it may contain a valid replace sequence (e.g. %L1).
    QString lineStr = QString("%1%2")
            .arg(symbolStr, -9, QLatin1Char(' '))
            .arg(dotStr, -8, QLatin1Char(' '))
            % QString("%1%2")
            .arg(oprndStr, -12)
            .arg(comment);
    return lineStr;
}

quint16 MacroInvoke::objectCodeLength() const
{
    quint16 total = 0;
    for(auto line :this->macroInstance->codeList) {
        total += line->objectCodeLength();
    }
    return total;
}

QStringList MacroInvoke::getArgumentList() const
{
    return argumentList;
}

void MacroInvoke::setArgumentList(QStringList argumentList)
{
    this->argumentList = argumentList;
}

QSharedPointer<ModuleInstance> MacroInvoke::getMacroInstance() const
{
    return macroInstance;
}

void MacroInvoke::setMacroInstance(QSharedPointer<ModuleInstance> macroInstance)
{
    this->macroInstance = macroInstance;
}
