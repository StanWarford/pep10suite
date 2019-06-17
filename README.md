# What is Pep/10?

The Pep/10 computer is a 16-bit complex instruction set computer (CISC). It is designed to teach computer architecture, assembly language programming, and computer organization principles. It is the successor to Pep/9 as described in the text [_Computer Systems_, J. Stanley Warford, 5th edition](http://computersystemsbook.com/5th-edition/). Pep/10 instructions are based on an expanding opcode and are either unary (one byte) or nonunary (three bytes). The eight addressing modes and eight dot commands are designed for straightforward translation from C to assembly language.

# Pep10Suite
Pep10Suite is a suite of software for the Pep/10 virtual machine.
It consists of four applications:
* [Pep10](#pep10)
* [Pep10CPU](#pep10cpu)
* [Pep10Micro](#pep10micro)
* [Pep10Term](#pep10term)

## Pep10
Pep10 is a simulator allowing users to interact with the Pep/10 virtual machine at the assembly, operating system, and ISA levels.

The Pep10 assembler features an integrated text editor, error messages in red type that are inserted within the source code at the place where the error is detected, student-friendly machine language object code in hexadecimal format, the ability to code directly in machine language, bypassing the assembler, and the ability to redefine the mnemonics for the unimplemented opcodes that trigger synchronous traps.

The simulator features simulated ROM that is not altered by store instructions, a small operating system burned into simulated ROM that includes a loader and a trap handler system, an integrated debugger that allows for break points, single and multi step execution, CPU tracing, and memory tracing, the option to trace an application, the loader, or the operating system, the ability to recover from endless loops, and the ability to modify the operating system by designing new trap handlers for the unimplemented opcodes.

## Pep10CPU
Pep10CPU is a simulator allowing users to interact with the data section of the Pep/10 CPU.

It contains two versions of the Pep/10 CPU data section &ndash; one with a one-byte wide data bus and another with a two-byte wide data bus. Using a GUI, students are able to set the control signals to direct the flow of data and change the state of the CPU. Alternatively, the Microcode IDE allows students to write microprogram code fragments to perform useful computations. An integrated unit test facility allows users to write pre- and post-conditions to verify correct behavior of arbitrary microprograms.

While debugging a microprogram fragment, the CPU simulator performs graphical tracing of data paths through the CPU. Using breakpoints, students may skip over previously debugged microstatments and resume debugging at a later point in the program.

## Pep10Micro
Pep10Micro is a fully microcoded implementation of the Pep/9 virtual machine.
It adds a control section, missing in Pep10CPU, and extends the microcode language to allow conditional microcode branches.
It integrates all the programming features of Pep10 and the graphical CPU interaction of Pep10CPU to simulate the complete execution of assembly language programs.

The Pep10Micro IDE:

* Provides the assembler from Pep10 and the CPU simulator from Pep10CPU so that complete assembly language programs can be executed at the microcode level spanning four levels of system abstraction &ndash; the assembly level, the operating system level, the ISA level, and the microcode level.
* Runs both memory aligned and nonaligned programs. Assembly language programs that do not use optimal .ALIGN directives still execute correctly but slower.
* Provides performance statistics in the form of statement execution counts at the microcode level and the ISA level. Students can measure the performance differences between aligned and nonaligned programs.
* Retains the unit tests of the original Pep/9 CPU IDE so that students can write microcode fragments with the extended microinstruction format.
* Supports new debugging features like step-into, step-out, and step-over so students can trace assembly programs more efficiently.

## Pep10Term
Pep10Term is a command-line version of the Pep/10 virtual machine.
It uses the assembler from the Pep10 application to create a .pepo file, and the simulator to execute the .pepo file.
Teachers can script Pep10Term to batch test assembly language homework submissions.

# Building from Sources
To sucessfully build from the sources, you must have Qt Creator and the Qt libraries installed on your machine, including the WebEngine components for the integrated Help systems. Qt can be downloaded from [the Qt website](https://www.qt.io/download).

If you want to package the application with an installer, you must also install the Qt Installer Framework (QtIFW) 3.0 or higher.

# Help Documentation
The programs come packaged with help documentation to describe the nature and function of the Pep/10 virtual machine including walkthroughs on Pep/10 assembly language programming and debugging tools/tips. They also have collections of sample assembly programs from the text [_Computer Systems_, J. Stanley Warford, 5th edition](http://computersystemsbook.com/5th-edition/), on which Pep/9 is based.

# Executable Downloads
For analytic reasons, we prefer that you download executables from the above book web site instead of GitHub.
