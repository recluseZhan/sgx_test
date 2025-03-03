/*
 * Copyright (C) 2011-2021 Intel Corporation. All rights reserved.
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

#include "Enclave.h"
#include "Enclave_t.h" /* print_string */
#include <stdarg.h>
#include <stdio.h> /* vsnprintf */
#include <string.h>
#include <sgx_thread.h>
#include <sgx_trts_exception.h>

/* 
 * printf: 
 *   Invokes OCALL to display the enclave buffer to the terminal.
 */
int printf(const char* fmt, ...)
{
    char buf[BUFSIZ] = { '\0' };
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_print_string(buf);
    return (int)strnlen(buf, BUFSIZ - 1) + 1;
}

//
void *get_saved_reg(unsigned long offset) {
    void *value;
    asm volatile(
        "mov %%gs:0x8,%%rax\n\t"
        "add %1,%%rax\n\t"     
        "mov (%%rax),%0 \n\t"
        : "=r"(value)
        : "r"(offset)
        : "rax"
    );
    return value;
}

unsigned long source[10]={0,1,2,3,4,5,6,7,8,9};
unsigned long target[10]={0};
static unsigned long data = 0x123;
void ecall_read(unsigned long *va){
    //unsigned long data_va = (unsigned long)&data;
    unsigned long data_va = (unsigned long)source;
    *va = data_va;
    printf("va:0x%lx,&data:0x%lx,data:0x%lx\n",*va,source,source[0]);
}

static sgx_thread_t self_tcs;
void ecall_tcs(unsigned long *tcs){
    self_tcs = sgx_thread_self();
    *tcs = (unsigned long)self_tcs;
    printf("tcs:0x%lx\n",*tcs);
    //__asm__ volatile("mov %%gs:0, %0" : "=r"(tcs));
}

static unsigned long self_aep;
void ecall_aep(unsigned long *aep){
    asm volatile(
        "mov %%gs:0x8,%%rax\n\t"
	"mov 0x10(%%rax),%0\n\t"
	:"=r"(self_aep)
	::"rax"
    );
    *aep = self_aep;
    printf("aep:0x%lx\n",*aep);
}

void ecall_reg(void){
    unsigned long *app_rdi = (unsigned long *)get_saved_reg(0x68);
    printf("app_rdi:0x%lx,app_rdi_value:0x%lx\n",app_rdi,app_rdi);
}

void ecall_write(void){
    unsigned long va;
    asm volatile(
        "mov %%rdi,%0\n\t"
	:"=r"(va)::
    );
    //*va=0x234;
    printf("test2:0x%lx\n",va);
}

void ecall_write2(unsigned long *va){
    printf("t3.1:0x%lx,0x%lx\n",va,*va);
    *va = 0x234;
    printf("t3.2:0x%lx,0x%lx\n",va,*va);
}
//#include <sgx_trts.h>
//#include <sgx_urts.h>
//unsigned long source[10]={0,1,2,3,4,5,6,7,8,9};
//unsigned long target[10]={0};
void ecall_write3(void){
    int i = 0;
    for(i=0;i<10;i++){
        printf("%lx ",target[i]);
    }
    printf("\n");
    
    //sgx_cpu_context_t *ctx;
    //unsigned long ctx_rdi = ctx->rdi;
    //printf("ctx_rdi:0x%lx\n",ctx_rdi);
    
    unsigned long tcs_base;
    unsigned long ssa_base;
    unsigned long *ssa_a;
    unsigned long rdi_base;
    unsigned long *rdi_a;
    
    asm volatile(
        "mov %%gs:0x0,%0\n\t":"=r"(tcs_base)		    
    );
    ssa_base = tcs_base+0x10;
    ssa_a = (unsigned long*)ssa_base;
    ssa_base = *ssa_a;
    rdi_base = ssa_base+0x38;
    rdi_a = (unsigned long*)rdi_base;

    printf("tcs_base:0x%lx, ssa_base:0x%lx, rdi_base:0x%lx\n",tcs_base,ssa_base,*rdi_a);
    /*
    asm volatile(
        "mov %%gs:0x0,%0\n\t"
	"mov 0x10(%0),%1\n\t"
	"mov 0x38(%1),%2\n\t"
        :"=r"(tcs_base),"=r"(ssa_base),"=r"(rdi_base)
    );
    //tcs_t *tcs = (tcs_t*)tcs_base;
    //ssa_base = tcs_base + tcs->ossa;
    //ssa_base = tcs_base + 0x10;
    printf("tcs_base:0x%lx, ssa_base:0x%lx, rdi_base:0x%x\n",tcs_base,ssa_base,rdi_base);*/
    /*
    unsigned long *ssa_rdi;
    asm volatile(
        "mov %%gs:0x8,%%rax\n\t"
        "lea 0x56(%%rax),%0\n\t"
	:"=r"(ssa_rdi)::
    );
    printf("ssa_rdi:0x%lx\n",ssa_rdi);
    */
    unsigned long s,t;
    s = (unsigned long)source;
    t = (unsigned long)target;
    asm volatile(
        "mov %0,%%rdi\n\t"
        "mov $20,%%rcx\n\t"
        "mov %1,%%rsi\n\t"
	"rep movsd"
        ::"r"(t),"r"(s):	
    );

    for(i=0;i<10;i++){
        printf("%lx ",target[i]);
    }
    printf("\n");
}
//
