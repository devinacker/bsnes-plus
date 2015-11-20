/*
  libco.amd64_gcc (2015-11-20)
  author: byuu, Alex W. Jackson
  license: public domain
*/

#define LIBCO_C
#include "libco.h"
#include <assert.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

static thread_local long long co_active_buffer[64];
static thread_local cothread_t co_active_handle = 0;

static void crash() {
  assert(0);  /* called only if cothread_t entrypoint returns */
}

cothread_t co_active() {
  if(!co_active_handle) co_active_handle = &co_active_buffer;
  return co_active_handle;
}

cothread_t co_create(unsigned int size, void (*entrypoint)(void)) {
  cothread_t handle;

  if(!co_active_handle) co_active_handle = &co_active_buffer;
  size += 512;  /* allocate additional space for storage */
  size &= ~15;  /* align stack to 16-byte boundary */

  if(handle = (cothread_t)malloc(size)) {
    long long *p = (long long*)((char*)handle + size);  /* seek to top of stack */
    *--p = (long long)crash;                            /* crash if entrypoint returns */
    *--p = (long long)entrypoint;                       /* start of function */
    *(long long*)handle = (long long)p;                 /* stack pointer */
  }

  return handle;
}

void co_delete(cothread_t handle) {
  free(handle);
}

#if defined(__APPLE__)
  #define SYM(x) "_" #x
#else
  #define SYM(x) #x
#endif

__asm__(
  ".globl " SYM(co_switch) "\n\t"
  SYM(co_switch) ":         \n\t"
#ifdef _WIN32
/*
 * Windows ABI:
 * function argument in rcx
 * rbp, rsi, rdi, rbx, r12-r15 and xmm6-xmm15 are non-volatile
 */
  "movq " SYM(co_active_handle) "(%rip),%rdx\n\t" /* from */
  "movq %rcx," SYM(co_active_handle) "(%rip)\n\t" /* to */

  "movq %rsp,(%rdx)       \n\t" /* save old stack pointer */
  "movq (%rcx),%rsp       \n\t" /* load new stack pointer */
  "popq %rax              \n\t" /* pop return address off stack */

  "movq %rbp, 8(%rdx)     \n\t" /* backup non-volatile registers */
  "movq %rsi,16(%rdx)     \n\t"
  "movq %rdi,24(%rdx)     \n\t"
  "movq %rbx,32(%rdx)     \n\t"
  "movq %r12,40(%rdx)     \n\t"
  "movq %r13,48(%rdx)     \n\t"
  "movq %r14,56(%rdx)     \n\t"
  "movq %r15,64(%rdx)     \n\t"

  "movaps %xmm6,  80(%rdx)\n\t"
  "movaps %xmm7,  96(%rdx)\n\t"
  "movaps %xmm8, 112(%rdx)\n\t"
  "addq $112,%rdx         \n\t"
  "movaps %xmm9,  16(%rdx)\n\t"
  "movaps %xmm10, 32(%rdx)\n\t"
  "movaps %xmm11, 48(%rdx)\n\t"
  "movaps %xmm12, 64(%rdx)\n\t"
  "movaps %xmm13, 80(%rdx)\n\t"
  "movaps %xmm14, 96(%rdx)\n\t"
  "movaps %xmm15,112(%rdx)\n\t"

  "movq  8(%rcx),%rbp     \n\t" /* restore non-volatile registers */
  "movq 16(%rcx),%rsi     \n\t"
  "movq 24(%rcx),%rdi     \n\t"
  "movq 32(%rcx),%rbx     \n\t"
  "movq 40(%rcx),%r12     \n\t"
  "movq 48(%rcx),%r13     \n\t"
  "movq 56(%rcx),%r14     \n\t"
  "movq 64(%rcx),%r15     \n\t"

  "movaps  80(%rcx),%xmm6 \n\t"
  "movaps  96(%rcx),%xmm7 \n\t"
  "movaps 112(%rcx),%xmm8 \n\t"
  "addq $112,%rcx         \n\t"
  "movaps  16(%rcx),%xmm9 \n\t"
  "movaps  32(%rcx),%xmm10\n\t"
  "movaps  48(%rcx),%xmm11\n\t"
  "movaps  64(%rcx),%xmm12\n\t"
  "movaps  80(%rcx),%xmm13\n\t"
  "movaps  96(%rcx),%xmm14\n\t"
  "movaps 112(%rcx),%xmm15\n\t"
#else
/*
 * System V ABI:
 * function argument in rdi
 * rbp, rbx, and r12-r15 are non-volatile
 */
  "movq " SYM(co_active_handle) "(%rip),%rdx\n\t" /* from */
  "movq %rdi," SYM(co_active_handle) "(%rip)\n\t" /* to */

  "movq %rsp,(%rdx)       \n\t" /* save old stack pointer */
  "movq (%rdi),%rsp       \n\t" /* load new stack pointer */
  "popq %rax              \n\t" /* pop return address off stack */

  "movq %rbp, 8(%rdx)     \n\t" /* backup non-volatile registers */
  "movq %rbx,16(%rdx)     \n\t"
  "movq %r12,24(%rdx)     \n\t"
  "movq %r13,32(%rdx)     \n\t"
  "movq %r14,40(%rdx)     \n\t"
  "movq %r15,48(%rdx)     \n\t"

  "movq  8(%rdi),%rbp     \n\t" /* restore non-volatile registers */
  "movq 16(%rdi),%rbx     \n\t"
  "movq 24(%rdi),%r12     \n\t"
  "movq 32(%rdi),%r13     \n\t"
  "movq 40(%rdi),%r14     \n\t"
  "movq 48(%rdi),%r15     \n\t"
#endif
  "jmp *%rax              \n\t" /* jump to "to" thread */
);

#undef SYM

#ifdef __cplusplus
}
#endif
