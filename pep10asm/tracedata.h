#ifndef TRACEDATA_H
#define TRACEDATA_H

#include "typetags.h"

enum class StackHint
{
    PARAMS,
    LOCALS,
    EMPTY_HINT,
};

/*
 * Helper class to collect memory trace information at assembly time
 * instead of during the simulation.
 */
class TraceData
{
public:
    TraceData();
    TraceData(const TraceData& other);
    TraceData& operator=(TraceData rhs);

    // Does the code line modify the stack pointer?
    // This is limited to MOVASP, ADDSP, SUBSP,
    // CALL, RET, SCALL, and SRET.
    // If false, no additional stack checks need be performed.
    bool getModifySP() const;
    void setModifySP(bool value);

    // For stack modifying instructions ADDSP, and SUBSP,
    // should values be treated as a locals allocation
    // (allocate in the current stack frame) or a parameters
    // allocation (allocate above the current frame).
    StackHint getStackHint() const;
    void setStackHint(StackHint hint);

    // The data type of an instructions operand.
    // For example in the instruction ADDSP 8,i #2d4a,
    // the operand type is a class representing 2D4A.
    // The instruction .WORD 65 ;#2d has no operand type,
    // since .WORD is a declaration, not an instruction modifying
    // the stack.
    QSharedPointer<AType> getOperandType() const;
    void setOperandType(QSharedPointer<AType> data);

    // The data type of an instructions operand.
    // For example in the instruction ADDSP 8,i #2d4a,
    // the declared type is empty, since the instruction
    // modifes the stack instead of providing a type definition.
    // The instruction .WORD 65 ;#2d has a declaration type of
    // #2d.
    QSharedPointer<AType> getDeclaredType() const;
    void setDeclaredType(QSharedPointer<AType> data);

    friend void swap(TraceData& first, TraceData& second)
    {
        using std::swap;
        swap(first.modifySP, second.modifySP);
        swap(first.hint, second.hint);
        swap(first.operandType, second.operandType);
        swap(first.declareType, second.declareType);
    }

private:
    bool modifySP;
    StackHint hint;
    QSharedPointer<AType> operandType;
    QSharedPointer<AType> declareType;
};

#endif // TRACEDATA_H
