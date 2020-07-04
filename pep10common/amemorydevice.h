// File: amemorydevice.h
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
#ifndef AMEMORYDEVICE_H
#define AMEMORYDEVICE_H

#include <QObject>
#include <QSet>

/*
 * This class provides a unified interface for memory devices (like RAM, or a cache).
 * It provides concrete methods for singaling errors & error messages,
 * reporting on addresses whose contents changed over a period of time,
 * and reading / writing to memory.
 *
 * One matter of note is the differentiation between read/get and write/set.
 * Read and write are able to trigger side effects (such as creating output in memory-mapped io,
 * or loading a newpage due to a cache miss). Write might also trigger errors,
 * like trying to write to the read-only storage for the operating system.
 *
 * Get and set do not trigger side effects. Get simply reports the value at an address,
 * and set will modify the value. Some chips (like the ConstChip) do not support any modification, even
 * through set, but some chips like the ROMChip will error if written to, but will succede if set.
 *
 * Both set / write will trigger the changed(...) signal
 *
 * Therefore, programmers should use get / set when interacting with the memory model from the UI,
 * and the logical model operating on memory should use get / set.
 *
 */
class AMemoryDevice : public QObject
{
    Q_OBJECT
protected:
    QSet<quint16> bytesWritten, bytesSet;
    mutable QString errorMessage;
    mutable bool error;
public:
    explicit AMemoryDevice(QObject *parent = nullptr) noexcept;

    // Returns true if a fatal error affected memory.
    bool hadError() const noexcept;
    // If there was an error, return the diagnostic message.
    QString getErrorMessage() const noexcept;
    // Return the number of bytes of memory this unit has access to.
    // The Pep/9 memory model should at most be 2^16 bytes, but provide
    // for potential expansion in the future.
    virtual quint32 maxAddress() const noexcept = 0;

    // Remove any pending errors in the memory device.
    void clearErrors();

    // Returns the set of bytes the have been written / set.
    // since the last clear.
    const QSet<quint16> getBytesWritten() const noexcept;
    const QSet<quint16> getBytesSet() const noexcept;
    // Call after all components have (synchronously) had a chance
    // to access these fields. The set of written / set bytes will
    // continue to grow until explicitly reset.
    void clearBytesWritten() noexcept;
    void clearBytesSet() noexcept;

public slots:
    // Clear the contents of memory. All addresses from 0 to size will be set to 0.
    virtual void clearMemory() = 0;
    // Methods (currently unused) that are useful for implementing a cache
    // replacmenet policy using dynamic aging.
    virtual void onCycleStarted() = 0;
    virtual void onCycleFinished() = 0;

    // Read / Write functions that may trap for IO or generate errors from writing to readonly storage.
    virtual bool readByte(quint16 offsetFromBase, quint8& output) const = 0;
    virtual bool writeByte(quint16 offsetFromBase, quint8 value) = 0;
    // Read / Write of words as two read / write byte operations and bitmath.
    virtual bool readWord(quint16 offsetFromBase, quint16& output) const;
    virtual bool writeWord(quint16 offsetFromBase, quint16 value);

    // Get / Set functions that are guarenteed not to trap for IO and will not error.
    virtual bool getByte(quint16 offsetFromBase, quint8& output) const = 0;
    virtual bool setByte(quint16 offsetFromBase, quint8 value) = 0;
    // Get / Set of words as two get / set byte operations and bitmath.
    virtual bool getWord(quint16 offsetFromBase, quint16& output) const;
    virtual bool setWord(quint16 offsetFromBase, quint16 value);

signals:
    // Signal that a memory address has been written / set.
    void changed(quint16 address, quint8 newValue);


};

#endif // AMEMORYDEVICE_H
