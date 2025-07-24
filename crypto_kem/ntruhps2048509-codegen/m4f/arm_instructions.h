#ifndef ARM_INSTRUCTIONS_H
#define ARM_INSTRUCTIONS_H

#define R(IDX) (IDX)
#define SP R(13)
#define LR R(14)
#define PC R(15)

#define S(IDX) (IDX)

// All section numbers below refer to the ARMv7-M architecture reference manual (DDI0403E_e), and all instructions are
// encoded in 32 bits, unless indicated otherwise

// Section A.7.7.14 (BFI), encoding T1
#define BFI_MSB(LSB, WIDTH) (((LSB) + (WIDTH)-1) << 16)
#define BFI_LSB(LSB) ((((LSB) >> 2) << 28) | (((LSB)&3) << 22))

#define BFI(RD, RN, LSB, WIDTH) (0x0000f360 | ((RD) << 24) | (RN) | (BFI_MSB(LSB, WIDTH)) | (BFI_LSB(LSB)))

#define BFI_BYTE(RD, RN, BYTEIDX) (0x000f360 | ((RD) << 24) | (RN) | ((BYTEIDX) << 19) | (7 << 16) | ((BYTEIDX) << 29))

// Section A.7.7.20 (BX), encoding T1 + Section A.7.7.88 (NOP), encoding T1
#define BX_NOP(RN) (0xbf004700 | ((RN) << 3))

// Section A.7.7.41 (LDM, LDMIA, LDMFD), encoding T2
#define LDM(WB, RN, REGLIST) (0x0000e890 | (WB) | (RN) | ((REGLIST) << 16))
#define LDMIA(RN, REGLIST) LDM(0, RN, REGLIST)
#define LDMFD(RN, REGLIST) LDMIA(RN, REGLIST)
#define LDMIA_BANG(RN, REGLIST) LDM(1 << 5, RN, REGLIST)
#define LDMFD_BANG(RN, REGLIST) LDMIA_BANG(RN, REGLIST)

#define LDM_REG(REG) (1 << (REG))
#define LDM_REGRANGE(FIRSTREG, LASTREG) ((1 << (LASTREG + 1)) - (1 << FIRSTREG))

// Section A.7.7.76 (MOV (immediate)), encoding T3
// IMM is a 16-bit immediate value (if larger, must be masked)
#define MOVW(RD, IMM)            \
    (0x0000f240 | ((RD) << 24) | \
     (((IMM) >> 12) | ((((IMM) >> 11) & 0x01) << 10) | ((((IMM) >> 8) & 0x07) << 28) | (((IMM)&0xff) << 16)))

// Section A.7.7.101 (PUSH), encoding T2
#define PUSH(REGLIST) (0x0000e92d | ((REGLIST) << 16))

#define PUSH_REG(REG) (1 << (REG))
#define PUSH_REGRANGE(FIRSTREG, LASTREG) ((1 << (LASTREG + 1)) - (1 << FIRSTREG))

// Section A.7.7.99 (POP), encoding T2
#define POP(REGLIST) (0x0000e8bd | ((REGLIST) << 16))

#define POP_REG(REG) (1 << (REG))
#define POP_REGRANGE(FIRSTREG, LASTREG) ((1 << (LASTREG + 1)) - (1 << FIRSTREG))

// Section A.7.7.159 (STM, STMIA, STMEA), encoding T2
#define STM(WB, RN, REGLIST) (0x0000e880 | (WB) | (RN) | ((REGLIST) << 16))
#define STMIA(RN, REGLIST) STM(0, RN, REGLIST)
#define STMEA(RN, REGLIST) STMIA(RN, REGLIST)
#define STMIA_BANG(RN, REGLIST) STM(1 << 5, RN, REGLIST)
#define STMEA_BANG(RN, REGLIST) STMIA_BANG(RN, REGLIST)

#define STM_REG(REG) (1 << (REG))
#define STM_REGRANGE(FIRSTREG, LASTREG) ((1 << (LASTREG + 1)) - (1 << FIRSTREG))

// Section A.7.7.163 (STRB (immediate)), encoding T2
#define STRB(RT, RN, IMM) ((0x0000f880) | ((RT) << 28) | (RN) | ((IMM) << 16))

// Section A.7.7.193 (UBFX), encoding T1
#define UBFX_WIDTHM1(WIDTH) (((WIDTH)-1) << 16)
#define UBFX_LSB(LSB) ((((LSB) >> 2) << 28) | (((LSB)&3) << 22))

#define UBFX(RD, RN, LSB, WIDTH) (0x0000f3c0 | ((RD) << 24) | (RN) | (UBFX_WIDTHM1(WIDTH)) | (UBFX_LSB(LSB)))

#define UBFX_BYTE(RD, RN, BYTEIDX) (0x0000f3c0 | ((RD) << 24) | (RN) | (7 << 16) | ((BYTEIDX) << 29))

// Section A7.7.235 (VLDM), encoding T2
#define VLDM(INC_DEC_WB, RN, FIRSTREG, LASTREG)                                               \
    (0x0a00ec10 | (INC_DEC_WB) | (RN) | ((((FIRSTREG) >> 1) << 28) | (((FIRSTREG)&1) << 6)) | \
     (((LASTREG) - (FIRSTREG) + 1) << 16))

#define VLDMIA(RN, FIRSTREG, LASTREG) VLDM(1 << 7, RN, FIRSTREG, LASTREG)
#define VLDMIA_BANG(RN, FIRSTREG, LASTREG) VLDM(((1 << 7) | (1 << 5)), RN, FIRSTREG, LASTREG)
#define VLDMDB_BANG(RN, FIRSTREG, LASTREG) VLDM(((1 << 8) | (1 << 5)), RN, FIRSTREG, LASTREG)

// Section A7.7.243 (VMOV (between Arm core register and single-precision register)), encoding T1
#define VMOV_S(SN) (((SN) >> 1) | (((SN)&1) << 23))

#define VMOV_R_S_OR_S_R(OP, SN, RT) (0x0a10ee00 | (OP << 4) | VMOV_S(SN) | ((RT) << 28))

#define VMOV_OP_S_R 0
#define VMOV_OP_R_S 1

#define VMOV_S_R(SN, RT) VMOV_R_S_OR_S_R(VMOV_OP_S_R, SN, RT)
#define VMOV_R_S(RT, SN) VMOV_R_S_OR_S_R(VMOV_OP_R_S, SN, RT)

// Section A7.7.252 (VPUSH), encoding T2
#define VPUSH(FIRSTREG, LASTREG) \
    (0x0a00ed2d | ((((FIRSTREG) >> 1) << 28) | (((FIRSTREG)&1) << 6)) | (((LASTREG) - (FIRSTREG) + 1) << 16))

// Section A7.7.251 (VPOP), encoding T2
#define VPOP(FIRSTREG, LASTREG) \
    (0x0a00ecbd | ((((FIRSTREG) >> 1) << 28) | (((FIRSTREG)&1) << 6)) | (((LASTREG) - (FIRSTREG) + 1) << 16))

// Section A7.7.235 (VSTM), encoding T2
#define VSTM(INC_DEC_WB, RN, FIRSTREG, LASTREG)                                               \
    (0x0a00ec00 | (INC_DEC_WB) | (RN) | ((((FIRSTREG) >> 1) << 28) | (((FIRSTREG)&1) << 6)) | \
     (((LASTREG) - (FIRSTREG) + 1) << 16))

#define VSTMIA(RN, FIRSTREG, LASTREG) VSTM(1 << 7, RN, FIRSTREG, LASTREG)
#define VSTMIA_BANG(RN, FIRSTREG, LASTREG) VSTM(((1 << 7) | (1 << 5)), RN, FIRSTREG, LASTREG)
#define VSTMDB_BANG(RN, FIRSTREG, LASTREG) VSTM(((1 << 8) | (1 << 5)), RN, FIRSTREG, LASTREG)

#endif  // ARM_INSTRUCTIONS_H
