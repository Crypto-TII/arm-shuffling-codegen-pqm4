#ifndef PERK_GENERATOR_H
#define PERK_GENERATOR_H

#include <arm_acle.h>
#include <stddef.h>
#include <stdint.h>

#include "arm_instructions.h"
#include "cmsis_gcc.h"

#define INT_REGS 12

#define APPEND_INSN(INSN)             \
    do {                              \
        asm_word[idx_asm++] = (INSN); \
    }                                 \
    while (0);

#define SAMPLE_ALT_SWAP_BYTE_FP_REGS(POS_I, IDX_VMOV_R)                                                               \
    do {                                                                                                              \
        idx_si = *si >> 2;                                                                                            \
        byte_si = *si-- & 3;                                                                                          \
                                                                                                                      \
        /* Detection of the case where the two elements to be swapped are in the same register is performed */        \
        /* identically as before, although repeated for each of the four elements handled per iteration.    */        \
                                                                                                                      \
        idx_reg = i - idx_si;                                                                                         \
        idx_reg = ((size_t)(idx_reg | -idx_reg)) >> (sizeof(size_t) * 8 - 1);                                         \
                                                                                                                      \
        /* Copy S[idx_si] to R[IDX_VMOV_R] */                                                                         \
        APPEND_INSN(VMOV_R_S(R(IDX_VMOV_R), S(idx_si)));                                                              \
                                                                                                                      \
        /* Copy the POS_I-th byte of R2 to the least significant byte of R12 */                                       \
        APPEND_INSN(UBFX_BYTE(R(12), R(2), POS_I));                                                                   \
        /* Copy the byte_si-th byte of R[2 + idx_reg] to the least significant byte of R1 */                          \
        APPEND_INSN(UBFX_BYTE(R(1), R(2 + idx_reg), byte_si));                                                        \
        /* Copy the least significant byte of R12 to the byte_si-th byte of R[2 + idx_reg] */                         \
        APPEND_INSN(BFI_BYTE(R(2 + idx_reg), R(12), byte_si));                                                        \
        /* Copy the least significant byte of R1 to the POS_I-th byte of R2 */                                        \
        APPEND_INSN(BFI_BYTE(R(2), R(1), POS_I));                                                                     \
                                                                                                                      \
        /* Copy R[2 + idx_reg] to S[idx_si]*/                                                                         \
        APPEND_INSN(VMOV_S_R(S(idx_si), R(2 + idx_reg)));                                                             \
                                                                                                                      \
        /* Note that R2 is not necessarily copied back to the FP registers here -- it may indeed be copied if */      \
        /* idx_reg == 0, but this is only to ensure constant-time execution; it could otherwise be omitted without */ \
        /* affecting correctness. This comment similarly applies to the next elements that are handled in the */      \
        /* current iteration of the loop, although of course R2 must be copied back to the corresponding FP */        \
        /* register after the last element of this iteration is handled. */                                           \
    }                                                                                                                 \
    while (0)

#define SAMPLE_ALT_SWAP_BYTE_INT_REGS(POS_I)                                             \
    do {                                                                                 \
        idx_si = *si >> 2;                                                               \
        byte_si = *si-- & 3;                                                             \
                                                                                         \
        /* Copy the POS_I-th byte of R[i] to the least significant byte of LR */         \
        APPEND_INSN(UBFX_BYTE(LR, R(i), POS_I));                                         \
        /* Copy the byte_si-th byte of R[idx_si] to the least significant byte of R12 */ \
        APPEND_INSN(UBFX_BYTE(R(12), R(idx_si), byte_si));                               \
                                                                                         \
        /* Copy the least significant byte of LR to the byte_si-th byte of R[idx_si] */  \
        APPEND_INSN(BFI_BYTE(R(idx_si), LR, byte_si));                                   \
        /* Copy the least significant byte of R12 to the POS_I-th byte of R[i]*/         \
        APPEND_INSN(BFI_BYTE(R(i), R(12), POS_I));                                       \
    }                                                                                    \
    while (0)

#define SAMPLE_ALT_SWAP_BYTE_INT_REGS_FINAL(POS_I)                                \
    do {                                                                          \
        byte_si = *si-- & 3;                                                      \
                                                                                  \
        /* Copy the POS_I-th byte of R0 to the least significant byte of LR */    \
        APPEND_INSN(UBFX_BYTE(LR, R(0), POS_I));                                  \
        /* Copy the byte_si-th byte of R0 to the least significant byte of R12 */ \
        APPEND_INSN(UBFX_BYTE(R(12), R(0), byte_si));                             \
                                                                                  \
        /* Copy the least significant byte of LR to the byte_si-th byte of R0 */  \
        APPEND_INSN(BFI_BYTE(R(0), LR, byte_si));                                 \
        /* Copy the least significant byte of R12 to the POS_I-th byte of R0 */   \
        APPEND_INSN(BFI_BYTE(R(0), R(12), POS_I));                                \
    }                                                                             \
    while (0)

static inline void sig_perk_perm_gen_given_random_input_gen_alt(const uint16_t si[], size_t n, uint32_t asm_word[]) {
    // Generated function prototype: void (*)(uint8_t v[]);

    // This routine assumes that n >= 4 * INT_REGS. Otherwise, loads and stores to/from the shuffling array will access
    // invalid memory. It shouldn't be too hard to modify the code to handle smaller n; wthe changes needed are to
    // the registers that are pushed and popped at the beginning, as well as the loads and stores in the second stage
    // of shuffling (the one that uses integer registers). As the PUSH/POP/LDM/STM instructions are used, and they use
    // bitmaps to represent that registers that are loaded/stored, it should be possible to get the correct bitmap from
    // the value of n.
    size_t i, idx_i, idx_si, byte_i, byte_si, idx_reg, idx_asm = 0;

    si += n - 1;

    // Push callee-saved registers (integer and, if necessary, FP)
    APPEND_INSN(PUSH(PUSH_REGRANGE(R(4), R(12)) | PUSH_REG(LR)));

    // Per AAPCS32, only FP registers S16-S31 need to be saved. If n is not large enough, none are pushed, otherwise the
    // exact amount needed is pushed
    if ((n - 1) >> 2 >= 16) {
        APPEND_INSN(VPUSH(S(16), S((n - 1) >> 2)));
    }

    // If n > 4 * INT_REGS, the shuffling array does not fit in integer registers alone, so we start by loading it in
    // the FP registers
    if (n > 4 * INT_REGS) {
        // Load shuffling array to FP registers
        APPEND_INSN(VLDMIA(R(0), S(0), S((n - 1) >> 2)));

        // An initial short loop consumes n % 4 elements one by one, so that the main loop only needs to work on a
        // number of elements which is a multiple of 4
        for (i = n - 1; i >= (n & ~3); i--) {
            // Each iteration of the loop swaps the element of index t with the element of index si[n - 1 - t]. Thus,
            // for t = n - 1, n - 2, ..., the elements of si are accessed in the sequence 0, 1, ... Rather than using
            // array-based indexing, the code dereferences the pointer si and increments it at each iteration.
            //
            // Since elements of the shuffling array are stored as bytes, it is possible to fit 4 elements in each
            // register (integer or FP). Therefore, it is necessary to split the index in two parts: one for the
            // word/register index, and one to select between the 4 bytes of a word/register. idx_i and idx_si are the
            // word/register indices corresponding to t and si[n - 1 - t], respectively, whereas byte_i and byte_si are
            // the byte positions of the elements in the corresponding registers.
            //
            // The general strategy of each iteration is as follows:
            //
            // 1. Copy the FP registers containing the elements (bytes) to be swapped to integer registers
            // 2. For each of these two elements, extract the element from its byte position to the least significant
            //    byte of a temporary register, using the UBFX instruction
            // 3. For each of these two elements, copy the least significant byte of the temporary register to the byte
            //    position of the other element, using the BFI instruction
            // 4. Copy the integer registers back to the FP registers
            //
            // However, care must be taken when the two elements to be swapped are in the same register. If the code
            // proceeded identically as in the case where the elements are in different registers, then step 4 would
            // overwrite one of the swaps, resulting in incorrect code. The handling of this case is explained below.

            idx_i = i >> 2;
            idx_si = *si >> 2;

            byte_i = i & 3;
            byte_si = *si-- & 3;

            // Detect and handle the case where the two elements to be swapped are in the same register.
            //
            // To ensure constant-time execution of both the generator and the generated code, there are restrictions on
            // how to handle this case, in the sense that the same instructions must be emitted for the generated code,
            // and the generator must use branchless code.
            //
            // The trick used here is to reserve two registers (R2 and R3) to hold the two elements to be swapped. When
            // the two elements are in different registers, both R2 and R3 are used, but when they are in the same
            // register, only R2 is used. This is achieved by computing the variable idx_reg below (in constant-time) to
            // contain 0 when both elements are in the same register, and 1 when they are in different registers. The
            // first element is always stored in register R2, whereas the second element is stored in R[2 + ind_reg],
            // i.e. R2 if both elements are in the same register, and R3 if they are in different registers.
            //
            // While it is easy to see that the generated code below works correctly when the two elements are in
            // different registers, and thus both R2 and R3 are used, it is worth reviewing why it still works when both
            // elements are in the same register.
            //
            // The first step consists of copying both FP registers containing the target elements to integer registers.
            // In this case, since idx_i = idx_si and idx_reg = 0, there will be two redundant copies of an FP register
            // to the same integer register (R2). While unnecessary and inefficient, it ensures constant-time execution.
            //
            // The second step exatrcts bytes from R2 to different temporary registers (R1 and R12) using the UBFX
            // instruction. Clearly this is not affected by using the same register (R2) as the source for both
            // instructions.
            //
            // The third step copies the least significant bytes of the temporary registers R1 and R12 to the target
            // byte position of the destination registers, which is the same for both instructions in this case (R2).
            // Since the BFI instruction only updates the indicated byte of the destination register, the code works
            // correctly even in the case that both BFI instructions are executed on the same register. Indeed, even if
            // the swap is an identity operation (i.e. swapping an element with itself, rathen than with a different
            // element of the same register), this degrades to a redundant operation as in the first step, which while
            // wasteful of performance, is necessary to ensure constant-time execution.
            //
            // The fourth step consists of copying both integer registers back to their corresponding FP registers, but
            // in this case both integer registers, as well as both FP registers, are the same. As with the first step,
            // this results in two redundant copies of the same registers, which is functionally correct and ensures
            // constant-time execution.

            idx_reg = idx_i - idx_si;
            idx_reg = ((size_t)(idx_reg | -idx_reg)) >> (sizeof(size_t) * 8 - 1);

            // Copy S[idx_i] to R2
            APPEND_INSN(VMOV_R_S(R(2), S(idx_i)));
            // Copy S[idx_si] to R[2 + idx_reg]
            APPEND_INSN(VMOV_R_S(R(2 + idx_reg), S(idx_si)));

            // Copy the byte_i-th byte of R2 to the least significant byte of R12
            APPEND_INSN(UBFX_BYTE(R(12), R(2), byte_i));
            // Copy the byte_si-th byte of R2 or R3 to the least significant byte of R1
            APPEND_INSN(UBFX_BYTE(R(1), R(2 + idx_reg), byte_si));
            // Copy the least significant byte of R12 to the byte_si-th byte of R[2 + idx_reg]
            APPEND_INSN(BFI_BYTE(R(2 + idx_reg), R(12), byte_si));
            // Copy the least significant byte of R1 to the byte_i-th byte of R2
            APPEND_INSN(BFI_BYTE(R(2), R(1), byte_i));

            // Copy R2 to S[idx_i]
            APPEND_INSN(VMOV_S_R(S(idx_i), R(2)));
            // Copy R[2 + idx_reg] to S[idx_si]
            APPEND_INSN(VMOV_S_R(S(idx_si), R(2 + idx_reg)));
        }

        // The main loop handles four elements at a time. This is done so that only a single copy from the FP register
        // to integer registers is required for the elements of index i, i - 1, i - 2 and i - 3. Given that the
        // remainder of i modulo 4 is known and fixed for each element, the code starts by setting i = i / 4 and then
        // decrementing i by 1 rather than 4. This saves some computations in the generator.
        for (i = i >> 2; i >= INT_REGS; i--) {
            // The general setup and strategy is similar to the block above. However, if four elements are in the same
            // register, it is possible to take advantage of this to save some instructions in the generated code, as
            // well as the cost of generating them. The basic idea is that R2 (the register storing the elements of
            // index 4 * i, 4 * i - 1, 4 * i - 2 and 4 * i - 3) need not be saved at each iteration, but only after a
            // block of four iterations which handles all of these values (4 * i, ..., 4 * i - 3). As before, the case
            // where the two elements to be swapped are in the same register must be carefully handled.

            // Copy S[idx_i] to R2
            APPEND_INSN(VMOV_R_S(R(2), S(i)));

            SAMPLE_ALT_SWAP_BYTE_FP_REGS(3, 2 + idx_reg);

            // In the next block, the index of the integer register is changed from 2 + idx_reg to 4 - idx_reg, in
            // comparison to the element-by-element version of the code above. The reason for this is that, in the
            // element-by-element version, R2 is copied back to FP registers at every iteration of the code (as well as
            // R3, if used in that iteration). Therefore, when copying from the corresponding FP registers to R2 in the
            // next iteration, its value is always up-to-date.
            //
            // However, for this version that handles four elements, the following situation may happen:
            // - for the previous element within this iteration, R2 was not copied back to the FP registers (because the
            //   two elements to be swapped were in different registers).
            // - for the current element, the two elements to be swapped are in the same register, so the corresponding
            //   FP register would be copied to R2. Note, however, that this value would be out-of-date, and thus the
            //   algorithm would execute incorrectly.
            //
            // To prevent this situation, the calculation of the index is changed from 2 + idx_reg to 4 - idx_reg. If
            // idx_reg == 1, the value is still written to the correct register (R3), while if idx_reg == 0, the value
            // is written to R4, but is otherwise ignored. This guarantees that the code still executes in constant
            // time, while not updating R2 with an out-of-date value.
            //
            // When writing back to the FP registers, the register index is still calculated as 2 + idx_reg, not
            // 4 - idx_reg as above. This does not pose a problem, as the correct value is written back to the
            // corresponding FP register. This choice is actually arbitrary, as it would be equally correct to use
            // 4 - idx_reg here; this is because the up-to-date value of R2 is always copied to the corresponding FP
            // register after the fourth operation, so at the end of the current iteration, the values in the FP
            // registers will be correct.
            //
            // This is also done when handling the next two elements, so this comment will not be repeated below.

            SAMPLE_ALT_SWAP_BYTE_FP_REGS(2, 4 - idx_reg);
            SAMPLE_ALT_SWAP_BYTE_FP_REGS(1, 4 - idx_reg);

            idx_si = *si >> 2;
            byte_si = *si-- & 3;

            idx_reg = i - idx_si;
            idx_reg = ((size_t)(idx_reg | -idx_reg)) >> (sizeof(size_t) * 8 - 1);

            // Copy S[idx_si] to R[4 - idx_reg]
            APPEND_INSN(VMOV_R_S(R(4 - idx_reg), S(idx_si)));

            // At this point, the desired byte of the array element indexed by i is already at the least significant
            // byte of R2, so the UBFX instruction is not necessary. The corresponding BFI instruction should however be
            // updated to read from R2 rather than R12 below.

            // Copy the byte_si-th byte of R[2 + idx_reg] to the least significant byte of R1
            APPEND_INSN(UBFX_BYTE(R(1), R(2 + idx_reg), byte_si));
            // Copy the least significant byte of R2 to the byte_si-th byte of R[2 + idx_reg]
            APPEND_INSN(BFI_BYTE(R(2 + idx_reg), R(2), byte_si));
            // Copy the least significant byte of R1 to the byte 0 of R2
            APPEND_INSN(BFI_BYTE(R(2), R(1), 0));

            // Copy R[2 + idx_reg] to S[idx_si]
            APPEND_INSN(VMOV_S_R(S(idx_si), R(2 + idx_reg)));
            // Copy R2 to S[idx_i]
            APPEND_INSN(VMOV_S_R(S(i), R(2)));
        }

        // Store FP registers back to shuffling array
        APPEND_INSN(VSTMIA(R(0), S(0), S((n - 1) >> 2)));
    }

    // From now on, only 4 * INT_REGS = 48 elements remain to be shuffled, so they fit in integer registers; we work
    // exclusively there from now on, as this saves the cost of copying from FP registers to integer registers and back
    // in the generated code, as well as the cost of generating these instructions. Even though this requires storing
    // the contents of the FP register file only to immediately reload these to the integer registers, the gains are
    // still worthwhile.

    // Spill R0 to S0, to have an extra register available for the loop below
    APPEND_INSN(VMOV_S_R(S(0), R(0)));

    // Load initial part of shuffling array to integer registers
    APPEND_INSN(LDMIA(R(0), LDM_REGRANGE(R(0), R(11))));

    // Loop over remaining elements of shuffling array, except for the last 3 (which are handled separately after this
    // loop), now working in integer registers.
    for (; i > 0; i--) {
        // The same general strategy as the version with FP registers above is used here, with the simplification
        // afforded by storing the shuffling array directly in integer registers: there is no need to copy the values
        // to/from FP registers.

        SAMPLE_ALT_SWAP_BYTE_INT_REGS(3);
        SAMPLE_ALT_SWAP_BYTE_INT_REGS(2);
        SAMPLE_ALT_SWAP_BYTE_INT_REGS(1);

        // This is unchanged from the corresponding loop of the element-by-element version above.
        idx_si = *si >> 2;
        byte_si = *si-- & 3;

        // At this point, the desired byte of the array element indexed by i is already at the least significant
        // byte of R[idx_i], so the UBFX instruction is not necessary. The corresponding BFI instruction should however
        // be updated to read from R[i] rather than LR below.

        // Copy the byte_si-th byte of R[idx_si] to the least significant byte of R12
        APPEND_INSN(UBFX_BYTE(R(12), R(idx_si), byte_si));

        // Copy the least significant byte of R[i] to the byte_si-th byte of R[idx_si]
        APPEND_INSN(BFI_BYTE(R(idx_si), R(i), byte_si));
        // Copy the least significant byte of R12 to byte 0 of R[i]
        APPEND_INSN(BFI_BYTE(R(i), R(12), 0));
    }

    // In these final three iterations, *si >> 2 is always equal to 0, so it is not necessary to compute idx_si, and
    // register R0 is hardcoded where idx_si was previously used.

    SAMPLE_ALT_SWAP_BYTE_INT_REGS_FINAL(3);
    SAMPLE_ALT_SWAP_BYTE_INT_REGS_FINAL(2);
    SAMPLE_ALT_SWAP_BYTE_INT_REGS_FINAL(1);

    // Reload R12 from S0
    APPEND_INSN(VMOV_R_S(R(12), S(0)));

    // Store integer registers back to initial part of shuffling array
    APPEND_INSN(STMIA(R(12), STM_REGRANGE(R(0), R(11))));

    // Pop callee-saved registers (FP, if necessary, and integer)
    if ((n - 1) >> 2 >= 16) {
        APPEND_INSN(VPOP(S(16), S((n - 1) >> 2)));
    }

    APPEND_INSN(POP(POP_REGRANGE(R(4), R(12)) | POP_REG(PC)));
}

#define SAMPLE_SWAP_BYTE_INT_REGS(BYTE_I)                                                \
    do {                                                                                 \
        idx_si = *si >> 2;                                                               \
        byte_si = *si++ & 3;                                                             \
                                                                                         \
        /* Load (i << 2) + BYTE_I to LR */                                               \
        APPEND_INSN(MOVW(LR, (i << 2) + BYTE_I));                                        \
        /* Copy the byte_si-th byte of R[idx_si] to the least significant byte of R12 */ \
        APPEND_INSN(UBFX_BYTE(R(12), R(idx_si), byte_si));                               \
                                                                                         \
        /* Copy the BYTE_I-th byte of R12 to byte 3 of R[i] */                           \
        APPEND_INSN(BFI_BYTE(R(i), R(12), BYTE_I));                                      \
        /* Copy the least significant byte of LR to the byte_si-th byte of R[idx_si] */  \
        APPEND_INSN(BFI_BYTE(R(idx_si), LR, byte_si));                                   \
    }                                                                                    \
    while (0)

#define SAMPLE_SWAP_BYTE_FP_REGS(BYTE_I, IDX_VMOV_R)                                         \
    do {                                                                                     \
        idx_si = *si >> 2;                                                                   \
        byte_si = *si++ & 3;                                                                 \
                                                                                             \
        idx_reg = i - idx_si;                                                                \
        idx_reg = ((size_t)(idx_reg | -idx_reg)) >> (sizeof(size_t) * 8 - 1);                \
                                                                                             \
        /* Copy S[idx_si] to R[IDX_VMOV_R] */                                                \
        APPEND_INSN(VMOV_R_S(R(IDX_VMOV_R), S(idx_si)));                                     \
                                                                                             \
        /* Load (i << 2) + BYTE_I to R0 */                                                   \
        APPEND_INSN(MOVW(R(0), (i << 2) + BYTE_I));                                          \
        /* Copy the byte_si-th byte of R[2 + idx_reg] to the least significant byte of R1 */ \
        APPEND_INSN(UBFX_BYTE(R(1), R(2 + idx_reg), byte_si));                               \
        /* Copy the BYTE_I-th byte of R1 to byte 0 of R2 */                                  \
        APPEND_INSN(BFI_BYTE(R(2), R(1), BYTE_I));                                           \
        /* Copy the least significant byte of R0 to the byte_si-th byte of R[2 + idx_reg] */ \
        APPEND_INSN(BFI_BYTE(R(2 + idx_reg), R(0), byte_si));                                \
                                                                                             \
        /* Copy R[2 + idx_reg] to S[idx_si] */                                               \
        APPEND_INSN(VMOV_S_R(S(idx_si), R(2 + idx_reg)));                                    \
    }                                                                                        \
    while (0)

static inline void sig_perk_perm_gen_given_random_input_gen(const uint16_t si[], size_t n, uint32_t asm_word[]) {
    // Generated function prototype: void (*)(uint8_t v[]);

    size_t i, idx_i, idx_si, byte_i, byte_si, idx_reg, idx_asm = 0;

    // Push callee-saved registers (integer and, if necessary, FP)
    APPEND_INSN(PUSH(PUSH_REGRANGE(R(4), R(12)) | PUSH_REG(LR)));

    // Per AAPCS32, only FP registers S16-S31 need to be saved. If n is not large enough, none are pushed, otherwise the
    // exact amount needed is pushed
    if ((n - 1) >> 2 >= 16) {
        APPEND_INSN(VPUSH(S(16), S((n - 1) >> 2)));
    }

    // Spill R0 to S0, to have an extra register available for the loop below
    APPEND_INSN(VMOV_S_R(S(0), R(0)));

    // The array doesn't need to be loaded yet, as it's uninitialized at this point.

    // Loop over remaining elements of shuffling array, except for the last 3 (which are handled separately after this
    // loop), now working in integer registers.
    for (i = 0; i < INT_REGS; i++) {
        // The same general strategy as the version with FP registers is used here, with the simplification afforded by
        // by storing the shuffling array directly in integer registers: there is no need to copy the values to/from
        // FP registers.
        SAMPLE_SWAP_BYTE_INT_REGS(0);
        SAMPLE_SWAP_BYTE_INT_REGS(1);
        SAMPLE_SWAP_BYTE_INT_REGS(2);
        SAMPLE_SWAP_BYTE_INT_REGS(3);
    }

    // Reload R12 from S0
    APPEND_INSN(VMOV_R_S(R(12), S(0)));

    // Store integer registers back to initial part of shuffling array
    APPEND_INSN(STMIA(R(12), STM_REGRANGE(R(0), R(11))));

    // Load only the elements of the shuffling array which were written during the previous block to FP registers.
    // The rest remains uninitialized, and does not need to be loaded.
    APPEND_INSN(VLDMIA(R(12), S(0), S(11)));

    for (; i < n >> 2; i++) {
        // Copy S[idx_i] to R2
        APPEND_INSN(VMOV_R_S(R(2), S(i)));

        SAMPLE_SWAP_BYTE_FP_REGS(0, 2 + idx_reg);
        SAMPLE_SWAP_BYTE_FP_REGS(1, 4 - idx_reg);
        SAMPLE_SWAP_BYTE_FP_REGS(2, 4 - idx_reg);
        SAMPLE_SWAP_BYTE_FP_REGS(3, 4 - idx_reg);

        // Copy R2 to S[idx_i]
        APPEND_INSN(VMOV_S_R(S(i), R(2)));
    }

    for (i = 4 * i; i < n; i++) {
        idx_i = i >> 2;
        idx_si = *si >> 2;

        byte_i = i & 3;
        byte_si = *si++ & 3;

        idx_reg = idx_i - idx_si;
        idx_reg = ((size_t)(idx_reg | -idx_reg)) >> (sizeof(size_t) * 8 - 1);

        // Copy S[idx_i] to R2
        APPEND_INSN(VMOV_R_S(R(2), S(idx_i)));
        // Copy S[idx_si] to R[2 + idx_reg]
        APPEND_INSN(VMOV_R_S(R(2 + idx_reg), S(idx_si)));

        // Load i to R0
        APPEND_INSN(MOVW(R(0), i));
        // Copy the byte_si-th byte of R2 or R3 to the least significant byte of R1
        APPEND_INSN(UBFX_BYTE(R(1), R(2 + idx_reg), byte_si));
        // Copy the least significant byte of R1 to the byte_i-th byte of R2
        APPEND_INSN(BFI_BYTE(R(2), R(1), byte_i));
        // Copy the least significant byte of R0 to the byte_si-th byte of R[2 + idx_reg]
        APPEND_INSN(BFI_BYTE(R(2 + idx_reg), R(0), byte_si));

        // Copy R2 to S[idx_i]
        APPEND_INSN(VMOV_S_R(S(idx_i), R(2)));
        // Copy R[2 + idx_reg] to S[idx_si]
        APPEND_INSN(VMOV_S_R(S(idx_si), R(2 + idx_reg)));
    }

    // Store FP registers back to shuffling array
    APPEND_INSN(VSTMIA(R(12), S(0), S((n - 1) >> 2)));

    // Pop callee-saved registers (FP, if necessary, and integer)
    if ((n - 1) >> 2 >= 16) {
        APPEND_INSN(VPOP(S(16), S((n - 1) >> 2)));
    }

    APPEND_INSN(POP(POP_REGRANGE(R(4), R(12)) | POP_REG(PC)));
}

static inline void sig_perk_perm_inverse_gen(const uint8_t p[], size_t n, uint32_t asm_word[]) {
    // Generated function prototype: void (*)(uint8_t o[]);

    size_t i, idx_p, byte_p, idx_asm = 0;

    // Per AAPCS32, only FP registers S16-S31 need to be saved. If n is not large enough, none are pushed, otherwise the
    // exact amount needed is pushed
    if ((n - 1) >> 2 >= 16) {
        APPEND_INSN(VPUSH(S(16), S((n - 1) >> 2)));
    }

    // Output array does not need to be loaded, as it starts out unitialized, and is fully populated by the generated
    // code

    for (i = 0; i < n; i++) {
        idx_p = *p >> 2;
        byte_p = *p++ & 3;

        // Copy S[idx_p] to R1
        APPEND_INSN(VMOV_R_S(R(1), S(idx_p)));

        // Load i to R2
        APPEND_INSN(MOVW(R(2), i));
        // Copy the least significant byte of R2 to the byte_p-th byte of R1
        APPEND_INSN(BFI_BYTE(R(1), R(2), byte_p));

        // Copy R1 to S[idx_p]
        APPEND_INSN(VMOV_S_R(S(idx_p), R(1)));
    }

    // Store FP registers back to shuffling array
    APPEND_INSN(VSTMIA(R(0), S(0), S((n - 1) >> 2)));

    // Pop callee-saved FP registers if necessary
    if ((n - 1) >> 2 >= 16) {
        APPEND_INSN(VPOP(S(16), S((n - 1) >> 2)));
    }

    // Return
    APPEND_INSN(BX_NOP(LR));
}

static inline void sig_perk_perm_compose_inv_gen(const uint8_t p1[], const uint8_t p2[], size_t n,
                                                 uint32_t asm_word[]) {
    // Generated function prototype: void (*)(uint8_t o[]);

    size_t i, idx_p2, byte_p2, idx_asm = 0;

    // Per AAPCS32, only FP registers S16-S31 need to be saved. If n is not large enough, none are pushed, otherwise the
    // exact amount needed is pushed
    if ((n - 1) >> 2 >= 16) {
        APPEND_INSN(VPUSH(S(16), S((n - 1) >> 2)));
    }

    for (i = 0; i < n; i++) {
        idx_p2 = *p2 >> 2;
        byte_p2 = *p2++ & 3;

        // Copy S[idx_p2] to R1
        APPEND_INSN(VMOV_R_S(R(1), S(idx_p2)));

        // Load p1[i] to R1
        APPEND_INSN(MOVW(R(2), p1[i]));
        // Copy the least significant byte of R2 to the byte_p2-th byte of R1
        APPEND_INSN(BFI_BYTE(R(1), R(2), byte_p2));

        // Copy R1 to S[idx_p2]
        APPEND_INSN(VMOV_S_R(S(idx_p2), R(1)));
    }

    // Store FP registers back to shuffling array
    APPEND_INSN(VSTMIA(R(0), S(0), S((n - 1) >> 2)));

    // Pop callee-saved FP registers if necessary
    if ((n - 1) >> 2 >= 16) {
        APPEND_INSN(VPOP(S(16), S((n - 1) >> 2)));
    }

    // Return
    APPEND_INSN(BX_NOP(LR));
}

static inline void sig_perk_perm_compose_gen(const uint8_t p2[], size_t n, uint32_t asm_word[]) {
    // Generated function prototype: void (*)(uint8_t o[], uint8_t p1[]);

    size_t i, idx_p2, byte_p2, idx_asm = 0;

    // Per AAPCS32, only FP registers S16-S31 need to be saved. If n is not large enough, none are pushed, otherwise the
    // exact amount needed is pushed
    if ((n - 1) >> 2 >= 16) {
        APPEND_INSN(VPUSH(S(16), S((n - 1) >> 2)));
    }

    // Load shuffling array to FP registers
    APPEND_INSN(VLDMIA(R(1), S(0), S((n - 1) >> 2)));

    for (i = 0; i < n; i++) {
        idx_p2 = *p2 >> 2;
        byte_p2 = *p2++ & 3;

        // Copy S[idx_p2] to R2
        APPEND_INSN(VMOV_R_S(R(2), S(idx_p2)));

        // Move the byte_p2-th byte of R2 to its least significant byte
        APPEND_INSN(UBFX_BYTE(R(2), R(2), byte_p2));

        // Store
        APPEND_INSN(STRB(R(2), R(0), i));
    }

    // Pop callee-saved FP registers if necessary
    if ((n - 1) >> 2 >= 16) {
        APPEND_INSN(VPOP(S(16), S((n - 1) >> 2)));
    }

    // Return
    APPEND_INSN(BX_NOP(LR));
}

static inline void sig_perk_perm_vect_permute_deinterleave_16bit(uint8_t deinterleaved[], const uint16_t interleaved[],
                                                                 size_t n) {
    size_t i, n_aligned = (n + 3) & ~0x3;
    const uint32_t *interleaved32 = (const uint32_t *)interleaved;
    uint32_t *deinterleaved32 = (uint32_t *)deinterleaved;
    uint32_t *deinterleaved32_high = (uint32_t *)(deinterleaved + n_aligned);

    // Set GE bits for sel instructions later
    uint32_t t;
    asm volatile("usub8 %0, %1, %2\n" : "=r"(t) : "r"(0xFF0000FF), "r"(0x00FFFF00));

    for (i = 0; i < n_aligned >> 2; i++) {
        uint32_t i01 = interleaved32[2 * i];
        uint32_t i23 = interleaved32[2 * i + 1];

        // Let i01 = a || b || c || d and i23 = e || f || g || h

        t = __builtin_bswap32(i01);  // t   = d || c || b || a
        i01 = __sel(i01, t);         // i01 = a || c || b || d

        t = __builtin_bswap32(i23);  // t   = h || g || f || e
        i23 = __sel(i23, t);         // i23 = e || g || f || h

        // deinterleaved32[i]      = a || c || e || g
        deinterleaved32[i] = __PKHBT(i01, i23, 16);
        // deinterleaved32_high[i] = b || d || f || h
        deinterleaved32_high[i] = __PKHTB(i23, i01, 16);
    }

    for (i = i << 2; i < n; i++) {
        deinterleaved[i] = interleaved[i] & 0xFF;
        deinterleaved[i + n_aligned] = (interleaved[i] >> 8) & 0xFF;
    }
}

static inline void sig_perk_perm_vect_permute_interleave_16bit(uint16_t interleaved[], const uint8_t deinterleaved[],
                                                               size_t n) {
    size_t i, n_aligned = (n + 3) & ~0x3;
    uint32_t *interleaved32 = (uint32_t *)interleaved;
    const uint32_t *deinterleaved32 = (const uint32_t *)deinterleaved;
    const uint32_t *deinterleaved32_high = (const uint32_t *)(deinterleaved + n_aligned);

    // Set GE bits for sel instructions later
    uint32_t t0, t1;
    asm volatile("usub8 %0, %1, %2\n" : "=r"(t0) : "r"(0xFF0000FF), "r"(0x00FFFF00));

    for (i = 0; i < n_aligned >> 2; i++) {
        uint32_t low = deinterleaved32[i];
        uint32_t high = deinterleaved32_high[i];

        // Let low = a || c || e || g and high = b || d || f || h

        t0 = __PKHBT(low, high, 16);           // t0                   = a || c || b || d
        t1 = __builtin_bswap32(t0);            // t1                   = d || b || c || a
        interleaved32[2 * i] = __sel(t0, t1);  // interleaved32[i]     = a || b || c || d

        t0 = __PKHTB(high, low, 16);               // t0                   = e || g || f || h
        t1 = __builtin_bswap32(t0);                // t1                   = h || f || g || e
        interleaved32[2 * i + 1] = __sel(t0, t1);  // interleaved32[i + 1] = e || f || g || h
    }

    for (i = i << 2; i < n; i++) {
        interleaved[i] = deinterleaved[i] | (deinterleaved[i + n_aligned] << 8);
    }
}

static inline void sig_perk_perm_vect_permute_gen(const uint8_t p[], size_t n, uint32_t asm_word[]) {
    // Generated function prototype: void (*)(uint8_t output_deinterleaved[], uint8_t input_deinterleaved[]);
    // Note that the generated code only does one pass over the shuffling array, and thus needs to be called twice,
    // passing the address of each half of the output and input buffers, to apply the full permutation.

    size_t i, idx_p, byte_p, idx_asm = 0;

    // Per AAPCS32, only FP registers S16-S31 need to be saved. If n is not large enough, none are pushed, otherwise the
    // exact amount needed is pushed
    if ((n - 1) >> 2 >= 16) {
        APPEND_INSN(VPUSH(S(16), S((n - 1) >> 2)));
    }

    for (i = 0; i < n; i++) {
        idx_p = *p >> 2;
        byte_p = *p++ & 3;

        // Copy S[idx_p2] to R3
        APPEND_INSN(VMOV_R_S(R(3), S(idx_p)));

        // Load input_deinterleaved[i] to R1
        APPEND_INSN(LDRB(R(2), R(1), i))
        // Copy the least significant byte of R1 to the byte_p2-th byte of R2
        APPEND_INSN(BFI_BYTE(R(3), R(2), byte_p));

        // Copy R2 to S[idx_p2]
        APPEND_INSN(VMOV_S_R(S(idx_p), R(3)));
    }

    // Store FP registers back to shuffling array (deinterleaved least-significant bytes)
    APPEND_INSN(VSTMIA(R(0), S(0), S((n - 1) >> 2)));

    // Pop callee-saved FP registers if necessary
    if ((n - 1) >> 2 >= 16) {
        APPEND_INSN(VPOP(S(16), S((n - 1) >> 2)));
    }

    // Return
    APPEND_INSN(BX_NOP(LR));
}

#endif  // PERK_GENERATOR_H
