#include "tracedata.h"

TraceData::TraceData(): modifySP(false), hint(StackHint::EMPTY_HINT),
    operandType(nullptr), declareType(nullptr)
{

}

TraceData::TraceData(const TraceData &other): modifySP(other.modifySP), hint(other.hint),
    operandType(other.operandType), declareType(other.declareType)
{

}

TraceData &TraceData::operator=(TraceData rhs)
{
    swap(*this, rhs);
    return *this;
}

bool TraceData::getModifySP() const
{
    return modifySP;
}

void TraceData::setModifySP(bool value)
{
    this->modifySP = value;
}

StackHint TraceData::getStackHint() const
{
    return hint;
}

void TraceData::setStackHint(StackHint hint)
{
    this->hint = hint;
}

QSharedPointer<AType> TraceData::getOperandType() const
{
    return operandType;
}

void TraceData::setOperandType(QSharedPointer<AType> data)
{
    this->operandType = data;
}

QSharedPointer<AType> TraceData::getDeclaredType() const
{
    return declareType;
}

void TraceData::setDeclaredType(QSharedPointer<AType> data)
{
    this->declareType = data;
}
