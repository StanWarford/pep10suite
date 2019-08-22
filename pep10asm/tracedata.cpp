#include "tracedata.h"
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
