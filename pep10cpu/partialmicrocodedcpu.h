#ifndef FULLMICROCODEDCPU_H
#define FULLMICROCODEDCPU_H

#include "interfacemccpu.h"
#include <QElapsedTimer>
class CPUDataSection;
class PartialMicrocodedMemoizer;
class PartialMicrocodedCPU : public ACPUModel, public InterfaceMCCPU
{
    Q_OBJECT
    friend class CPUMemoizer;
    friend class PartialMicrocodedMemoizer;
public:
    PartialMicrocodedCPU(Enu::CPUType type, QSharedPointer<AMemoryDevice>, QObject* parent = nullptr) noexcept;
    ~PartialMicrocodedCPU() override;
    QSharedPointer<CPUDataSection> getDataSection();

    // ACPUModel interface
    bool getStatusBitCurrent(Enu::EStatusBit) const override;
    bool getStatusBitStart(Enu::EStatusBit) const override;
    quint8 getCPURegByteCurrent(Enu::CPURegisters reg) const override;
    quint16 getCPURegWordCurrent(Enu::CPURegisters reg) const override;
    quint8 getCPURegByteStart(Enu::CPURegisters reg) const override;
    quint16 getCPURegWordStart(Enu::CPURegisters reg) const override;
    void initCPU() override;
    bool stoppedForBreakpoint() const noexcept override;
    QString getErrorMessage() const noexcept override;
    bool hadErrorOnStep() const noexcept override;

    // InterfaceMCCPU interface
    void setCPUType(Enu::CPUType type)  override;
    void onMCStep() override;
    void onClock() override;

    // ACPUModel interface
public slots:
    void onSimulationStarted() override;
    void onSimulationFinished() override;
    void enableDebugging() override;
    void forceBreakpoint(Enu::BreakpointTypes) override;
    void onCancelExecution() override;
    bool onRun() override;
    void onResetCPU() override;

private:
    CPUDataSection *data;
    QSharedPointer<CPUDataSection> dataShared;
    PartialMicrocodedMemoizer *memoizer;

    // Callback function to handle InteruptHandler's BREAKPOINT_MICRO.
    void breakpointMicroHandler();
    void branchHandler() override;
};

#endif // FULLMICROCODEDCPU_H
