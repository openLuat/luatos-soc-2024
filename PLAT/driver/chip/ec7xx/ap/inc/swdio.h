
#ifndef SWDIO_H
#define SWDIO_H

typedef struct
{
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
    uint32_t r12;
    uint32_t r13;
    uint32_t r14;
    uint32_t r15;
    uint32_t xpsr;
    uint32_t msp;
    uint32_t psp;
    uint8_t control;
    uint8_t faultmask;
    uint8_t basepri;
    uint8_t primask;
}swdCPRegs_t;

void swdCPCoreRegsDump(swdCPRegs_t *cpDumpRegs);



#endif
