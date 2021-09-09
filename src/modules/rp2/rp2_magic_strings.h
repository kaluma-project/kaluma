/* Copyright (c) 2017 Kaluma
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __PIO_MAGIC_STRINGS_H
#define __PIO_MAGIC_STRINGS_H

#define MSTR_RP2_PIO_PIO "PIO"
#define MSTR_RP2_PIO_FIFO_JOIN_NONE "FIFO_JOIN_NONE"
#define MSTR_RP2_PIO_FIFO_JOIN_TX "FIFO_JOIN_TX"
#define MSTR_RP2_PIO_FIFO_JOIN_RX "FIFO_JOIN_RX"
#define MSTR_RP2_PIO_SHIFT_RIGHT "SHIFT_RIGHT"
#define MSTR_RP2_PIO_SHIFT_LEFT "SHIFT_LEFT"
#define MSTR_RP2_PIO_TX_LESSTHAN "TX_LESSTHAN"
#define MSTR_RP2_PIO_RX_LESSTHAN "RX_LESSTHAN"

#define MSTR_RP2_ASM "ASM"
#define MSTR_RP2_ASM_JMP "jmp"
#define MSTR_RP2_ASM_WAIT "wait"
#define MSTR_RP2_ASM_IN "in"
#define MSTR_RP2_ASM_OUT "out"
#define MSTR_RP2_ASM_PUSH "push"
#define MSTR_RP2_ASM_PULL "pull"
#define MSTR_RP2_ASM_MOV "mov"
#define MSTR_RP2_ASM_IRQ "irq"
#define MSTR_RP2_ASM_SET "set"
#define MSTR_RP2_ASM_NOP "nop"
#define MSTR_RP2_ASM_WRAP_TARGET "wrap_target"
#define MSTR_RP2_ASM_WRAP "wrap"
#define MSTR_RP2_ASM_SIDE "side"
#define MSTR_RP2_ASM_DELAY "delay"
#define MSTR_RP2_ASM_TO_BINARY "toBinary"

#define MSTR_RP2_STATE_MACHINE "StateMachine"
#define MSTR_RP2_STATE_MACHINE_ACTIVE "active"
#define MSTR_RP2_STATE_MACHINE_RESTART "restart"
#define MSTR_RP2_STATE_MACHINE_EXEC "exec"
#define MSTR_RP2_STATE_MACHINE_GET "get"
#define MSTR_RP2_STATE_MACHINE_PUT "put"
#define MSTR_RP2_STATE_MACHINE_IRQ "irq"

#define MSTR_RP2_PIO_ADD_PROGRAM "pio_add_program"
#define MSTR_RP2_PIO_SM_INIT "pio_sm_init"
#define MSTR_RP2_PIO_SM_IN_BASE "inBase"
#define MSTR_RP2_PIO_SM_OUT_BASE "outBase"
#define MSTR_RP2_PIO_SM_OUT_COUNT "outCount"
#define MSTR_RP2_PIO_SM_SET_BASE "setBase"
#define MSTR_RP2_PIO_SM_SET_COUNT "setCount"
#define MSTR_RP2_PIO_SM_WRAP_TARGET "wrapTarget"
#define MSTR_RP2_PIO_SM_WRAP "wrap"
#define MSTR_RP2_PIO_SM_INSHIFT_DIR "inShiftDir"
#define MSTR_RP2_PIO_SM_AUTOPUSH "autopush"
#define MSTR_RP2_PIO_SM_PUSH_THRESHOLD "pushThreshold"
#define MSTR_RP2_PIO_SM_OUTSHIFT_DIR "outShiftDir"
#define MSTR_RP2_PIO_SM_AUTOPULL "autopull"
#define MSTR_RP2_PIO_SM_PULL_THRESHOLD "pullThreshold"
#define MSTR_RP2_PIO_SM_FIFO_JOIN "fifoJoin"
#define MSTR_RP2_PIO_SM_SET_ENABLED "pio_sm_set_enabled"
#define MSTR_RP2_PIO_SM_PUT "pio_sm_put"
#define MSTR_RP2_PIO_SM_GET "pio_sm_get"

#endif /* __PIO_MAGIC_STRINGS_H */
