#include "tracedata.h"
#include "optional_helper.h"
TraceCommand::TraceCommand(): action(TraceAction::NOP), frame(FrameTarget::NONE),
    trace(TraceTarget::NONE), tag(std::nullopt)
{

}

TraceCommand::TraceCommand(TraceAction action, FrameTarget frame, TraceTarget trace):
    action(action), frame(frame), trace(trace), tag(std::nullopt)
{

}

TraceCommand::TraceCommand(TraceAction action, FrameTarget frame, TraceTarget trace, QSharedPointer<AType> tag):
    action(action), frame(frame), trace(trace), tag(tag)
{

}

TraceCommand::TraceCommand(const TraceCommand &other)
{
    this->action = other.action;
    this->frame = other.frame;
    this->trace = other.trace;
    this->tag = other.tag;
}

TraceCommand &TraceCommand::operator=(TraceCommand other)
{
    swap(*this, other);
    return *this;
}

TraceAction TraceCommand::getAction() const
{
    return action;
}

FrameTarget TraceCommand::getFrame() const
{
    return frame;
}

TraceTarget TraceCommand::getTrace() const
{
    return trace;
}

std::optional<QSharedPointer<const AType>> TraceCommand::getTag() const
{
    return tag;
}

QString TraceCommand::toString() const
{
    QString out = QString("%1 %2 %3")
            .arg(::toString(this->getAction()))
            .arg(::toString(this->getTrace()))
            .arg(::toString(this->getFrame()));
    if(tag.has_value()) {
        out.append(QString(" %1").arg(optional_helper(tag)->toString()));
    }
    return out;
}
QString TraceCommand::toShortString() const
{
    QString out = QString("%1 %2 %3")
            .arg(::toShortString(this->getAction()))
            .arg(::toShortString(this->getTrace()))
            .arg(::toShortString(this->getFrame()));
    if(tag.has_value()) {
        out.append(QString(" %1").arg(optional_helper(tag)->toString()));
    }
    return out;
}
QString toString(TraceAction action)
{
    switch(action) {
    case TraceAction::NOP:
        return "NOP";
    case TraceAction::POP:
        return "POP";
    case TraceAction::PUSH:
        return "PUSH";
    case TraceAction::SETFRAME:
        return "SETFRAME";
    case TraceAction::SWAPTRACE:
        return "SWAP";
    }
}

QString toShortString(TraceAction action)
{
    switch(action) {
    case TraceAction::NOP:
        return "NP";
    case TraceAction::POP:
        return "PO";
    case TraceAction::PUSH:
        return "PU";
    case TraceAction::SETFRAME:
        return "ST";
    case TraceAction::SWAPTRACE:
        return "SW";
    }
}

QString toString(FrameTarget target)
{
    switch(target) {
    case FrameTarget::NEXT:
        return "NEXT";
    case FrameTarget::NONE:
        return "NONE";
    case FrameTarget::CURRENT:
        return "CURRENT";
    case FrameTarget::DEDUCED:
        return "DEDUCED";
    case FrameTarget::PREVIOUS:
        return "PREVIOUS";
    }
}

QString toShortString(FrameTarget target)
{
    switch(target) {
    case FrameTarget::NEXT:
        return "NX";
    case FrameTarget::NONE:
        return "NO";
    case FrameTarget::CURRENT:
        return "CR";
    case FrameTarget::DEDUCED:
        return "DD";
    case FrameTarget::PREVIOUS:
        return "PR";
    }
}

QString toString(TraceTarget target)
{
    switch(target) {
    case TraceTarget::HEAP:
        return "HEAP";
    case TraceTarget::GLOBALS:
        return "GLOBAL";
    case TraceTarget::STACK:
        return "STACK";
    case TraceTarget::SWAP:
        return "SWAP";
    case TraceTarget::NONE:
        return "NONE";
    }
}

QString toShortString(TraceTarget target)
{
    switch(target) {
    case TraceTarget::HEAP:
        return "HP";
    case TraceTarget::GLOBALS:
        return "GL";
    case TraceTarget::STACK:
        return "ST";
    case TraceTarget::SWAP:
        return "SW";
    case TraceTarget::NONE:
        return "NO";
    }
}
