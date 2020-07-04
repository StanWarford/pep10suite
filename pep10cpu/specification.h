// File: specification.h
/*
    Pep9CPU is a CPU simulator for executing microcode sequences to
    implement instructions in the instruction set of the Pep/9 computer.

    Copyright (C) 2018  J. Stanley Warford, Pepperdine University

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
#ifndef SPECIFICATION_H
#define SPECIFICATION_H

#include <QString>
#include "enu.h"
#include "cpupane.h"

class AMemoryDevice;
class CPUDataSection; //Forward declare CPUDataSection to avoid inclusion loops
class Specification
{
public:
    explicit Specification() noexcept;
    virtual ~Specification();
    virtual void setUnitPre(CPUDataSection *, AMemoryDevice*) noexcept { }
    virtual bool testUnitPost(const CPUDataSection*, const AMemoryDevice*, QString&) const noexcept {return true;}
    virtual QString getSourceCode() const noexcept = 0;
};

class MemSpecification: public Specification {    
public:
    MemSpecification(int memoryAddress, int memoryValue, int numberBytes) noexcept;
    void setUnitPre(CPUDataSection *data, AMemoryDevice* memDevice) noexcept override;
    bool testUnitPost(const CPUDataSection *data, const AMemoryDevice* memDevice,
                      QString &errorString) const noexcept override;
    QString getSourceCode() const noexcept override;
private:
    int memAddress;
    int memValue;
    int numBytes;
};

class RegSpecification: public Specification {
public:
    RegSpecification(Enu::ECPUKeywords registerAddress, int registerValue) noexcept;
    void setUnitPre(CPUDataSection *data, AMemoryDevice* memDevice) noexcept override;
    bool testUnitPost(const CPUDataSection *data, const AMemoryDevice*,
                      QString &errorString) const noexcept override;
    QString getSourceCode() const noexcept override;
private:
    Enu::ECPUKeywords regAddress;
    int regValue;
};

class StatusBitSpecification: public Specification {
public:
    StatusBitSpecification(Enu::ECPUKeywords statusBitAddress, bool statusBitValue) noexcept;
    void setUnitPre(CPUDataSection *data, AMemoryDevice*) noexcept override;
    bool testUnitPost(const CPUDataSection *data, const AMemoryDevice*,
                      QString &errorString) const noexcept override;
    QString getSourceCode() const noexcept override;
private:
    Enu::ECPUKeywords nzvcsAddress;
    bool nzvcsValue;
};

#endif // SPECIFICATION_H
