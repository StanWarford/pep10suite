// File: registerfile.h
/*
    The Pep/9 suite of applications (Pep9, Pep9CPU, Pep9Micro) are
    simulators for the Pep/9 virtual machine, and allow users to
    create, simulate, and debug across various levels of abstraction.

    Copyright (C) 2018 J. Stanley Warford & Matthew McRaven, Pepperdine University

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
#include "registerfile.h"

RegisterFile::RegisterFile(): registersStart(), registersCurrent()
{
    registersStart.fill(0);
    registersCurrent.fill(0);
}

quint8 RegisterFile::readStatusBitsStart() const
{
    return statusBitsStart;
}

quint8 RegisterFile::readStatusBitsCurrent() const
{
    return statusBitsCurrent;
}

bool RegisterFile::readStatusBitCurrent(Enu::EStatusBit bit)
{
    return crackStatusBit(statusBitsCurrent, bit);
}

bool RegisterFile::readStatusBitStart(Enu::EStatusBit bit)
{
    return crackStatusBit(statusBitsStart, bit);
}

void RegisterFile::writeStatusBit(Enu::EStatusBit bit, bool val)
{
    int mask = 0;
    switch(bit)
    {
    case Enu::STATUS_N:
        mask = Enu::NMask;
        break;
    case Enu::STATUS_Z:
        mask = Enu::ZMask;
        break;
    case Enu::STATUS_V:
        mask = Enu::VMask;
        break;
    case Enu::STATUS_C:
        mask = Enu::CMask;
        break;
    case Enu::STATUS_S:
        mask = Enu::SMask;
        break;
    default:
        // Should never occur, but might happen if a bad status bit is passed
        return;
    }
    // Mask out the target bit, then or the new value in to the target position.
    // Explicitly narrow status bit calculation.
    statusBitsCurrent = static_cast<quint8>((statusBitsCurrent & ~mask) | ((val ? 1 : 0) * mask));
}

void RegisterFile::writeStatusBits(quint8 bits)
{
    // Only keep SNZVC positions
    this->statusBitsCurrent = bits & 0b11111;
}

void RegisterFile::clearStatusBits()
{
   statusBitsStart = 0;
   statusBitsCurrent = 0;
}

quint16 RegisterFile::readRegisterWordCurrent(quint8 reg) const
{
    if(reg + 1 <= Enu::maxRegisterNumber) {
        return static_cast<quint16>(registersCurrent.at(reg) << 8 | registersCurrent.at(reg + 1));
    }
    else return 0;
}

quint16 RegisterFile::readRegisterWordCurrent(Enu::CPURegisters reg) const
{
    return readRegisterWordCurrent(convertRegister(reg));
}

quint16 RegisterFile::readRegisterWordStart(quint8 reg) const
{
    if(reg + 1 <= Enu::maxRegisterNumber) {
        return static_cast<quint16>(registersStart.at(reg) << 8 | registersStart.at(reg + 1));
    }
    else return 0;
}

quint16 RegisterFile::readRegisterWordStart(Enu::CPURegisters reg) const
{
    return readRegisterWordStart(convertRegister(reg));
}

quint8 RegisterFile::readRegisterByteCurrent(quint8 reg) const
{
    if(reg <= Enu::maxRegisterNumber) return registersCurrent.at(reg);
    else return 0;
}

quint8 RegisterFile::readRegisterByteCurrent(Enu::CPURegisters reg) const
{
    return readRegisterByteCurrent(convertRegister(reg));
}

quint8 RegisterFile::readRegisterByteStart(quint8 reg) const
{
    if(reg <= Enu::maxRegisterNumber) return registersStart.at(reg);
    else return 0;
}

quint8 RegisterFile::readRegisterByteStart(Enu::CPURegisters reg) const
{
    return readRegisterByteStart(convertRegister(reg));
}

void RegisterFile::writeRegisterWord(quint8 reg, quint16 val)
{
    if(reg + 1 <= Enu::maxRegisterNumber) {
        registersCurrent.at(reg) = static_cast<quint8>(val >> 8);
        registersCurrent.at(reg + 1) = static_cast<quint8>(val & 0xff);
    }
}

void RegisterFile::writeRegisterWord(Enu::CPURegisters reg, quint16 val)
{
    writeRegisterWord(convertRegister(reg), val);
}

void RegisterFile::writeRegisterByte(quint8 reg, quint8 val)
{
    if(reg <= Enu::maxRegisterNumber) {
        registersCurrent.at(reg) = val;
    }
}

void RegisterFile::writeRegisterByte(Enu::CPURegisters reg, quint8 val)
{
    writeRegisterByte(convertRegister(reg), val);
}

void RegisterFile::clearRegisters()
{
    registersStart.fill(0);
    registersCurrent.fill(0);
}

void RegisterFile::writePCStart(quint16 val)
{
    quint8 reg = convertRegister(Enu::CPURegisters::PC);
    if(reg + 1 < Enu::maxRegisterNumber) {
        registersStart.at(reg) = static_cast<quint8>(val >> 8);
        registersStart.at(reg + 1) = static_cast<quint8>(val & 0xff);
    }
}

void RegisterFile::setIRCache(quint8 val)
{
    irCache = val;
}

quint8 RegisterFile::getIRCache() const
{
    return irCache;
}

void RegisterFile::flattenFile()
{
    /*
     * Compilers might complain about this, but this can be optimized more easily to use SIMD
     * or vector instructions, hopefully making the cost of copying registers neglibible.
     *
     * Since the arrays are declared to be the same size using std::array, no need
     * to verify that both arrays are indeed the same length at runtime.
     */
    memcpy(registersStart.data(), registersCurrent.data(), static_cast<std::size_t>(registersStart.size()));
    statusBitsStart = statusBitsCurrent;
}

bool RegisterFile::crackStatusBit(quint8 statusBits, Enu::EStatusBit bit)
{
    int mask = 0;
    switch(bit)
    {
    case Enu::STATUS_N:
        mask = Enu::NMask;
        break;
    case Enu::STATUS_Z:
        mask = Enu::ZMask;
        break;
    case Enu::STATUS_V:
        mask = Enu::VMask;
        break;
    case Enu::STATUS_C:
        mask = Enu::CMask;
        break;
    case Enu::STATUS_S:
        mask = Enu::SMask;
        break;
    default:
        // Should never occur, but might happen if a bad status bit is passed
        return false;
    }
    return statusBits & mask;
}
