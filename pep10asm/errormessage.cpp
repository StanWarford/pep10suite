#include "errormessage.h"
#include "asmcode.h"
AErrorMessage::AErrorMessage(quint32 instanceIndex, Severity severity, Stage stage, QString message):
    instanceIndex(instanceIndex), severity(severity), stage(stage), errorMessage(message)
{

}

AErrorMessage::AErrorMessage(const AErrorMessage &other):
    instanceIndex(other.instanceIndex), severity(other.severity),
    stage(other.stage), errorMessage(other.errorMessage)
{

}

AErrorMessage::AErrorMessage(AErrorMessage &&other):
    instanceIndex(std::move(other.instanceIndex)),severity(std::move(other.severity)),
    stage(std::move(other.stage)), errorMessage(std::move(other.errorMessage))
{

}

quint16 AErrorMessage::getPrototypeIndex() const
{
    return static_cast<quint16>(instanceIndex >> 16);
}

quint32 AErrorMessage::getInstanceIndex() const
{
    return instanceIndex;
}

AErrorMessage::~AErrorMessage() = default;

Severity AErrorMessage::getSeverity() const
{
    return severity;
}

Stage AErrorMessage::getStage() const
{
    return stage;
}

QString AErrorMessage::getErrorMessage() const
{
    return errorMessage;
}

FrontEndError::FrontEndError(quint32 instanceIndex, Severity severity, QString message, int sourceLine):
    AErrorMessage(instanceIndex, severity, Stage::FRONTEND, message), sourceLine(sourceLine)
{

}

FrontEndError::FrontEndError(const FrontEndError &other): AErrorMessage(other),
    sourceLine(other.sourceLine)
{

}

FrontEndError::FrontEndError(FrontEndError &&other):AErrorMessage(other),
    sourceLine(std::move(other.sourceLine))
{

}

FrontEndError &FrontEndError::operator=(FrontEndError rhs)
{
    swap(*this, rhs);
    return *this;
}

FrontEndError::~FrontEndError() = default;

int FrontEndError::getSourceLineNumber() const
{
    return sourceLine;
}

bool FrontEndError::presentInListing() const
{
    return false;
}

int FrontEndError::getListingLineNumber() const
{
    return -1;
}

BackEndError::BackEndError(quint32 instanceIndex, Severity severity, QString message, QSharedPointer<AsmCode> line):
    AErrorMessage(instanceIndex, severity, Stage::BACKEND, message),
    line(std::move(line))
{

}

BackEndError::BackEndError(const BackEndError &other): AErrorMessage(other),
    line(other.line)
{

}

BackEndError::BackEndError(BackEndError &&other): AErrorMessage(other),
    line(std::move(other.line))
{

}

BackEndError &BackEndError::operator=(BackEndError rhs)
{
    swap(*this, rhs);
    return *this;
}

BackEndError::~BackEndError() = default;

int BackEndError::getSourceLineNumber() const
{
    return line->getSourceLineNumber();
}

bool BackEndError::presentInListing() const
{
    return true;
}

int BackEndError::getListingLineNumber() const
{
    return line->getListingLineNumber();
}

void ErrorDictionary::addError(QSharedPointer<AErrorMessage> error)
{
    quint32 instanceIndex = error->getInstanceIndex();
    if(!this->errors.contains(instanceIndex)) {
        errors.insert(instanceIndex, {});
    }
    if(auto instance = this->errors[instanceIndex];
            !instance.contains(error->getSourceLineNumber())) {
        instance.insert(error->getSourceLineNumber(),{});
    }
    this->errors[instanceIndex][error->getSourceLineNumber()].append(error);
}

ErrorDictionary::ModuleErrors ErrorDictionary::getModuleErrors(quint32 instanceIndex)
{
    if(!this->errors.contains(instanceIndex)) {
        return {};
    }
    return errors[instanceIndex];
}
