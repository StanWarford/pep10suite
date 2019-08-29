#ifndef TRACEDATA_H
#define TRACEDATA_H

#include <optional>

#include "optional_helper.h"
#include "typetags.h"
enum class TraceAction
{
    PUSH,      //Add elements to a stack.
    POP,       //Remove elements from a stack.
    SETFRAME,  //Move the current frame forwards or backwards.
    SWAPTRACE, //Exchange the active globals-stack-heap for a new one.
    NOP,       //No operation is performed.
};
enum class FrameTarget
{
    NEXT,     //Perform the operation on the frame above the frame pointer.
    CURRENT,  //Perform the operation on the frame at the frame pointer.
    PREVIOUS, //Perform the operation on the frame below the frame pointer.
    DEDUCED,  //Allow the debugger to "guess" which frame to insert into.
    NONE,     //The action does not modify the frame pointer.
};

enum class TraceTarget
{
    GLOBALS, //Perform the action in the globals memory trace.
    STACK,   //Perform the action in the stack memory trace.
    HEAP,    //Perform the action in the heap memory trace.
    SWAP,    //The action will exhange which set of memory traces are active.
    NONE,    //The action does not effect the memory trace visualization.
};
/*
 * A TraceComman represents a single expression in the memory debugger
 * embedded domain language. It fully encapsulates one step, telling the debugger
 * what to do, which stack frame to modify, in which memory tace, and what
 * symbol is to be modified (if any). It is an improvement over the old Pep9 debugger,
 * since it moves the bulk of the stack debugging checks from runtime to compile time.
 *
 * For example PUSH NEXT STACK retAddr, would instruct the the debugger to push
 * a symbol named retAddr onto the current frame in the stack. For more details, see
 * the design documentation in docs/trace-debugger.
 */
class TraceCommand
{
public:
    // Create a NOP trace command by default.
    TraceCommand();
    // Create a trace command with an empty symbol.
    TraceCommand(TraceAction action, FrameTarget frame, TraceTarget trace);
    // Create a trace command with a present symbol.
    TraceCommand(TraceAction action, FrameTarget frame, TraceTarget trace, QSharedPointer<AType> tag);
    TraceCommand(const TraceCommand& other);
    // Move & dtor are trivial due to RAII.
    TraceCommand(TraceCommand&& other) = default;
    ~TraceCommand() = default;
    // Assign by value to take advantage of copy-swap.
    TraceCommand& operator=(TraceCommand other);

    // Return data
    TraceAction getAction() const;
    FrameTarget getFrame() const;
    TraceTarget getTrace() const;
    std::optional<QSharedPointer<const AType>> getTag() const;

    // Needed to implement copy-swap idiom.
    friend void swap(TraceCommand& first, TraceCommand& second){
        using std::swap;
        swap(first.action, second.action);
        swap(first.frame, second.frame);
        swap(first.trace, second.trace);
        swap(first.tag, second.tag);
    }
    // Helper function to express command as a string.
    QString toString() const;
    // Helper function that truncates each part of the command to two characters,
    // except for the symbol which retains its full length.
    QString toShortString() const;
private:
    TraceAction action;
    FrameTarget frame;
    TraceTarget trace;
    std::optional<QSharedPointer<AType>> tag;
};

// Helper functions needed by TraceCommand's toString methods.
QString toString(TraceAction);
QString toShortString(TraceAction);
QString toString(FrameTarget);
QString toShortString(FrameTarget);
QString toString(TraceTarget);
QString toShortString(TraceTarget);
#endif // TRACEDATA_H
