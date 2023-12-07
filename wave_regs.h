// QE IP Library Registers
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: Xilinx XUP Blackboard

// Hardware configuration:
// 
// AXI4-Lite interface:
//  Mapped to offset of 0x10000
// 
// QE 0 and 1 interface:
//   GPIO[11-10] are used for QE 0 inputs
//   GPIO[9-8] are used for QE 1 inputs

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#ifndef WAVE_REGS_H_
#define WAVE_REGS_H_

#define OFS_MODE          0
#define OFS_RUN           1
#define OFS_FREQ_A        2
#define OFS_FREQ_B        3
#define OFS_OFFSET        4
#define OFS_AMPLITUDE     5
#define OFS_DTYCYC        6
#define OFS_CYCLES        7

#define WAVE_SPAN_IN_BYTES 32

#endif

