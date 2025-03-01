/*
 * Copyright (C) 2011-2019 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


#include "enclave_entry.h"
.data
g_clean_ymm:
.long 0
.data
g_tcs:
.word 0x0
.word 0x0
.word 0x0
.word 0x0
.data
g_aep_pointer:
.word 0x0
.word 0x0
.word 0x0
.word 0x0

.text
.global sgx_reporter_enter
.global sgx_reporter_enter2
sgx_reporter_enter:
EENTER_PROLOG

mov %rdi, (%rdi)
mov $0x2, %rax
mov %rsi, %rbx
enclu

lea_pic sgx_async_exit, %rcx
sgx_async_exit:
//    .byte 0x0f, 0x01, 0xd7


EENTER_EPILOG
retq


sgx_reporter_enter2:
EENTER_PROLOG
mov frame_arg3, %rdi                       /* output */
mov %r8, %r10                               /*size*/

lea exit_inst(%rip),%r11                   /*return*/
//clean the upper bits of YMM registers
lea_pic  g_clean_ymm, %rbx
mov (%rbx), %rcx
cmp $0, %rcx
je   1f
vzeroupper
1:
mov         frame_arg0, %rbx                /* tcs addr */
lea_pic     g_tcs, %rax
mov         %rbx, (%rax)
/* fetch AEP; init when NULL */
lea_pic     g_aep_pointer, %rax
mov         (%rax), %rcx                    /* aep addr */
cmp         $0x0, %rcx
jnz 1f
lea_pic     __default_async_exit_pointer, %rcx
mov         %rcx, (%rax)
1:
mov         $0x2, %rax                /* EENTER leaf */

__default_async_exit_pointer:
ENCLU

exit_inst:
EENTER_EPILOG
retq

