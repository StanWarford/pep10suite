// File: enu.h
/*
    The Pep/9 suite of applications (Pep9, Pep9CPU, Pep9Micro) are
    simulators for the Pep/9 virtual machine, and allow users to
    create, simulate, and debug across various levels of abstraction.

    Copyright (C) 2010  J. Stanley Warford, Pepperdine University

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
#ifndef ENU_H
#define ENU_H

#include <QtCore>

namespace Enu {
    Q_NAMESPACE

    /*
     * Enumerations for all applications
     */
    static const quint8 maxRegisterNumber = 31;
    static const quint8 signalDisabled = 255;
    enum class CPURegisters: quint8
    {
        // Two byte registers
        // Present in any version of Pep/9
        A = 0, X = 2, SP = 4, PC = 6, OS = 11,
        TR = 8,
        // Present in any derivative of Pep9CPU
        T2 = 14, T3 = 16, T4 = 18, T5 = 20,
        M1 = 22, M2 = 24, M3 = 26, M4 = 28, M5 = 30,
        // "Fictitious" registers for Pep9micro
        MicroProgramCounter = 128,

        // One byte registers
        // Present in any version of Pep/9
        IS=10,
        // Present in any derivative of Pep9CPU
        T1=13
    };

    enum class BreakpointTypes: int
    {
        MICROCODE = 1<<0, ASSEMBLER = 1<<1,
    };

    // Bit masks that signal which editing actions should be available through context menus
    enum EditButton: int
    {
        COPY = 1<<0, CUT = 1<<1, PASTE = 1<<2, UNDO = 1<<3, REDO = 1<<4
    };

    enum class EPane
    {
        ESource,
        EObject,
        EListing,
        EListingTrace,
        EMemoryTrace,
        EBatchIO,
        ETerminal,
        EMicrocode,
        EDataSection,
    };
    Q_ENUM_NS(EPane);


    /*
     * Enumerations for Pep9
     */
    /*enum class EMnemonic: int
    {
        ADDA, ADDX, ADDSP, ANDA, ANDX, ASLA, ASLX, ASRA, ASRX,
        BR, BRC, BREQ, BRGE, BRGT, BRLE, BRLT, BRNE, BRV,
        CALL, CPBA, CPBX, CPWA, CPWX,
        DECI, DECO,
        HEXO,
        LDBA, LDBX, LDWA, LDWX,
        MOVAFLG, MOVFLGA, MOVSPA,
        NEGA, NEGX, NOP, NOP0, NOP1, NOTA, NOTX,
        ORA, ORX,
        RET, RETTR, ROLA, ROLX, RORA, RORX,
        STBA, STBX, STWA, STWX, STOP, STRO, SUBA, SUBX, SUBSP

    };*/
    /*
     * Enumerations for Pep10
     */
    enum class EMnemonic: int
        {
            RET, SRET,
            MOVSPA, MOVASP, MOVFLGA, MOVAFLG, MOVTA,
            NOP,
            USCALL,
            // FAULTS
            NOTA, NOTX, NEGA, NEGX,
            ASLA, ASLX, ASRA, ASRX,
            ROLA, ROLX, RORA, RORX,
            //STOP,
            BR, BRLE, BRLT, BREQ, BRNE, BRGE, BRGT, BRV, BRC,
            CALL, SCALL,
            LDWT, LDWA, LDWX, LDBA, LDBX,
            STWA, STWX, STBA, STBX,
            CPWA, CPWX, CPBA, CPBX,
            ADDA, ADDX, SUBA, SUBX,
            ANDA, ANDX, ORA, ORX, XORA, XORX,
            ADDSP, SUBSP
        };
    Q_ENUM_NS(EMnemonic);

    // Addressing modes for instructions
    enum class EAddrMode: int
    {
        NONE = 0,
        I = 1,
        D = 2,
        N = 4,
        S = 8,
        SF = 16,
        X = 32,
        SX = 64,
        SFX = 128,
        ALL = 255
    };
    Q_ENUM_NS(EAddrMode);

    // Format for symbols
    enum class ESymbolFormat: int
    {
        F_NONE, F_1C, F_1D, F_2D, F_1H, F_2H
    };
    Q_ENUM_NS(ESymbolFormat);

    quint16 tagNumBytes(Enu::ESymbolFormat symbolFormat);
    // Pre: symbolFormat is a valid format trace tag type.
    // Post: Returns the corresponding integer number of bytes.

    enum class EExecState
    {
        EStart,
        ERun, ERunAwaitIO,
        EDebugAwaitIO, EDebugAwaitClick, EDebugRunToBP, EDebugSingleStep
    };

    /*
     * Enumerations for Pep9CPU
     */
    // For ALU function 15, and used to mask status bits |'ed
    // together in a single integer.
    enum EMask
    {
        SMask = 0x10,
        NMask = 0x08,
        ZMask = 0x04,
        VMask = 0x02,
        CMask = 0x01,
    };

    enum MainBusState {
        None,
        MemReadFirstWait,
        MemReadSecondWait,
        MemReadReady,
        MemWriteFirstWait,
        MemWriteSecondWait,
        MemWriteReady,
    };
    Q_ENUM_NS(MainBusState);

    // Instruction mnemonics
    enum EControlSignals
    {
        MemRead=0, MemWrite,
        A,B,EOMux, MARMux,MARA, MARB, AMux, ALU,CSMux, AndZ,
        CMux, C,
        MDRMux, MDROMux, MDREMux,MDR, MDRE, MDRO,
        PValid,
    };
    Q_ENUM_NS(EControlSignals);

    enum EClockSignals{
        NCk=0,
        ZCk,VCk,CCk,SCk,MARCk,LoadCk,MDRCk, MDROCk, MDRECk,
        PValidCk,
    };
    Q_ENUM_NS(EClockSignals);

    enum EMemoryRegisters
    {
        MEM_MARA,MEM_MARB,MEM_MDR,MEM_MDRO,MEM_MDRE
    };
    Q_ENUM_NS(EMemoryRegisters);

    enum EStatusBit
    {
        STATUS_N,STATUS_Z,STATUS_V,STATUS_C,STATUS_S
    };
    Q_ENUM_NS(EStatusBit);

    enum EALUFunc
    {
        A_func=0,ApB_func=1,ApBpCin_func=2,ApnBp1_func=3,
        ApnBpCin_func=4,AandB_func=5,nAandB_func=6,AorB_func=7,
        nAorB_func=8,AxorB_func=9,nA_func=10,ASLA_func=11,
        ROLA_func=12,ASRA_func=13,RORA_func=14,NZVCA_func=15,
        UNDEFINED_func=255,
    };

    enum ECPUKeywords {
        Pre, Post,
        Mem, Acc, X, SP, PC, IR, Trap,
        T1, T2, T3, T4, T5,
        N, Z, V,Cbit, S,
        MARAREG, MARBREG,
        MDRREG, MDREREG, MDROREG
    };

    enum CPUType {
        OneByteDataBus,
        TwoByteDataBus,
    };

    // For our drawing/shapes classes:
    enum Direction {
        Up,
        Down,
        Left,
        Right,
    };


    /*
     * Enumerations for Pep9Micro
     */
    enum EBranchFunctions{
        Unconditional = 0,
        uBRGT = 1, uBRGE = 2, uBREQ = 3, uBRLE = 4, uBRLT = 5,
        uBRNE = 6, uBRV = 7, uBRC = 8, uBRS = 9,
        IsPrefetchValid = 10,
        IsUnary = 11,
        IsPCEven = 12,
        AddressingModeDecoder = 13, // Adressing modes jump table
        InstructionSpecifierDecoder = 14, // Instruction jump table
        Stop = 15,
        Assembler_Assigned = 16
    };
    Q_ENUM_NS(EBranchFunctions);
}
#endif
