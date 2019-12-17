// File: boundedexecmicrocpu.cpp
/*
    Pep9Term is a  command line tool utility for assembling Pep/9 programs to
    object code and executing object code programs.

    Copyright (C) 2019  J. Stanley Warford & Matthew McRaven, Pepperdine University

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
#include "boundexecmicrocpu.h"

#include <QCoreApplication>
#include <QTimer>

#include "amemorydevice.h"
#include "cpudata.h"
BoundExecMicroCpu::BoundExecMicroCpu(quint64 maxCycles, const AsmProgramManager *manager,
                                   QSharedPointer<AMemoryDevice> memDevice, QObject *parent):
    FullMicrocodedCPU(manager, memDevice, parent), maxCycles(maxCycles)

{
    // This version of the CPU does not respond to breakpoints, and as such
    // does not register any handlers with the InterruptHandler.
}

// All of our memory is owned by sharedpointers, so we
// should not attempt to delete anything ourselves.
BoundExecMicroCpu::~BoundExecMicroCpu() = default;

quint64 BoundExecMicroCpu::getDefaultMaxCycles()
{
    return defaultMaxCycles;
}

bool BoundExecMicroCpu::onRun()
{
    // Execute instructions until an error occurs, the simulation finished,
    // or we exceed our step count.
    std::function<bool(void)> cond = [this](){ if(maxCycles <= microCycleCounter) {
            controlError = true;
            errorMessage = "Possible endless loop detected.";
            // Make sure to explicitly terminate simulation, else will be stuck in infinite loop.
            emit simulationFinished();
            return false;
        }
        return !hadErrorOnStep() && !executionFinished;
    };
    doMCStepWhile(cond);

    //If there was an error on the control flow
    if(hadErrorOnStep()) {
        if(BoundExecMicroCpu::memory->hadError()) {
            qDebug() << "Memory section reporting an error";
            return false;
        }
        else if (BoundExecMicroCpu::getDataSection()){
            qDebug() << "Data section reporting an error";
            return false;
        }
        else {
            qDebug() << "Control section reporting an error";
            return false;
        }
    }

    return true;
}
