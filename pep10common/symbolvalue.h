// File: symbolvalue.h
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
#ifndef SYMBOLVALUE_H
#define SYMBOLVALUE_H

#include <QtCore>
#include <QSharedPointer>

#include "enu.h"

class SymbolEntry;

/*
 * A symbol's value type can be EMPTY (i.e the symbol is undefined), or an address (i.e. the symbol is singly defined, so it describes a ADDRESS in a program).
 * A symbol's value type is not meaninful in the case of a multiply defined symbol, so no special type is required for it.
 */
enum class SymbolType
{
    EMPTY, ADDRESS, NUMERIC_CONSTANT, EXTERNAL
};

/*
 * Abstract base class defining the properties of a symbolic value.
 */
class AbstractSymbolValue
{
private:
public:
    AbstractSymbolValue();
	virtual ~AbstractSymbolValue();
	virtual qint32 getValue() const = 0;
    virtual SymbolType getSymbolType() const = 0;
    virtual bool canRelocate() const {return false;}
};

/*
 * A symbol value representing the value contained by an undefined symbol.
 */
class SymbolValueEmpty :
public AbstractSymbolValue
{
public:
    SymbolValueEmpty();
    ~SymbolValueEmpty() override;
    
    // Inherited via AbstractSymbolValue
    qint32 getValue() const override;
    SymbolType getSymbolType() const override;
};

/*
 * A symbol value representing an constant numeric value.
 */
class SymbolValueNumeric :
public AbstractSymbolValue
{
public:
    explicit SymbolValueNumeric(quint16 value);
    ~SymbolValueNumeric() override;
    void setValue(quint16 value);
    // Inherited via AbstractSymbolValue
    qint32 getValue() const override;
    SymbolType getSymbolType() const override;
private:
    quint16 value;
};

/*
 * A symbol value representing an address of a line of code.
 * The effective address (and thus the value) is base + offset.
 *
 * Having a seperate offset parameter allows for easy relocation of programs,
 * which is necessary when compiling the Pep9OS.
 */
class SymbolValueLocation :
public AbstractSymbolValue
{
public:
    explicit SymbolValueLocation(quint16 value);
    ~SymbolValueLocation() override;
    void setBase(quint16 value);
    void setOffset(quint16 value);
    // Inherited via AbstractSymbolValue
    qint32 getValue() const override;
    SymbolType getSymbolType() const override;
    bool canRelocate() const override;
    quint16 getOffset() const;
    quint16 getBase() const;
private:
    quint16 base, offset;
};

/*
 * A symbol value pointing to a symbol entry in a different table.
 *
 * This "pointer" value is necessary to implement .EXPORT statements in
 * the PEP10 operating system.
 */
class SymbolValueExternal :
public AbstractSymbolValue
{
public:
    explicit SymbolValueExternal(QSharedPointer<const SymbolEntry>);
    ~SymbolValueExternal() override;
    // Inherited via AbstractSymbolValue
    qint32 getValue() const override;
    SymbolType getSymbolType() const override;
    bool canRelocate() const override;
    // Since we are pointing to a symbol in another table,
    // don't allow the symbol to be modified here.
    QSharedPointer<const SymbolEntry> getSymbolValue();
private:
        QSharedPointer<const SymbolEntry> symbol;

};

#endif // SYMBOLVALUE_H
