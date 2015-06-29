/*
  libco.amd64_gcc (2015-04-22)
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

/*
  The compiler *must not* create a stack frame for this function!
  Unfortunately, GCC does not support __attribute__((naked)) on x86,
  so we must do the best we can by forcing omit-frame-pointer and
  explicitly specifying a volatile register for the local variable
  (some GCCs have brain damage and may put a local variable in rbp
  even when volatile registers are available)
*/
#ifdef __clang__
  #ifndef __OPTIMIZE__
    #error "libco: please enable optimization or define LIBCO_NO_INLINE_ASM"
  #else
    #define NAKED __attribute__((naked))
  #endif
#else
  #define NAKED __attribute__((optimize("omit-frame-pointer")))
#endif
void NAKED co_switch(cothread_t to) {
  register cothread_t from __asm__("rdx") = co_active_handle;
  co_active_handle = to;

  __asm__ __volatile__(
    "movq %%rsp,(%[from])       \n\t" /* save old stack pointer */
    "movq (%[to]),%%rsp         \n\t" /* load new stack pointer */
    "popq %%rax                 \n\t" /* pop return address off stack */
#ifdef _WIN32
/* Windows ABI: rbp, rsi, rdi, rbx, r12-r15 and xmm6-xmm15 are non-volatile */
    "movq %%rbp, 8(%[from])     \n\t" /* backup non-volatile registers */
    "movq %%rsi,16(%[from])     \n\t"
    "movq %%rdi,24(%[from])     \n\t"
    "movq %%rbx,32(%[from])     \n\t"
    "movq %%r12,40(%[from])     \n\t"
    "movq %%r13,48(%[from])     \n\t"
    "movq %%r14,56(%[from])     \n\t"
    "movq %%r15,64(%[from])     \n\t"

    "movaps %%xmm6,  80(%[from])\n\t"
    "movaps %%xmm7,  96(%[from])\n\t"
    "movaps %%xmm8, 112(%[from])\n\t"
    "addq $112,%[from]          \n\t"
    "movaps %%xmm9,  16(%[from])\n\t"
    "movaps %%xmm10, 32(%[from])\n\t"
    "movaps %%xmm11, 48(%[from])\n\t"
    "movaps %%xmm12, 64(%[from])\n\t"
    "movaps %%xmm13, 80(%[from])\n\t"
    "movaps %%xmm14, 96(%[from])\n\t"
    "movaps %%xmm15,112(%[from])\n\t"

    "movq  8(%[to]),%%rbp       \n\t" /* restore non-volatile registers */
    "movq 16(%[to]),%%rsi       \n\t"
    "movq 24(%[to]),%%rdi       \n\t"
    "movq 32(%[to]),%%rbx       \n\t"
    "movq 40(%[to]),%%r12       \n\t"
    "movq 48(%[to]),%%r13       \n\t"
    "movq 56(%[to]),%%r14       \n\t"
    "movq 64(%[to]),%%r15       \n\t"

    "movaps  80(%[to]),%%xmm6   \n\t"
    "movaps  96(%[to]),%%xmm7   \n\t"
    "movaps 112(%[to]),%%xmm8   \n\t"
    "addq $112,%[to]            \n\t"
    "movaps  16(%[to]),%%xmm9   \n\t"
    "movaps  32(%[to]),%%xmm10  \n\t"
    "movaps  48(%[to]),%%xmm11  \n\t"
    "movaps  64(%[to]),%%xmm12  \n\t"
    "movaps  80(%[to]),%%xmm13  \n\t"
    "movaps  96(%[to]),%%xmm14  \n\t"
    "movaps 112(%[to]),%%xmm15  \n\t"
#else
/* System V ABI: rbp, rbx, and r12-r15 are non-volatile */
    "movq %%rbp, 8(%[from])     \n\t" /* backup non-volatile registers */
    "movq %%rbx,16(%[from])     \n\t"
    "movq %%r12,24(%[from])     \n\t"
    "movq %%r13,32(%[from])     \n\t"
    "movq %%r14,40(%[from])     \n\t"
    "movq %%r15,48(%[from])     \n\t"

    "movq  8(%[to]),%%rbp       \n\t" /* restore non-volatile registers */
    "movq 16(%[to]),%%rbx       \n\t"
    "movq 24(%[to]),%%r12       \n\t"
    "movq 32(%[to]),%%r13       \n\t"
    "movq 40(%[to]),%%r14       \n\t"
    "movq 48(%[to]),%%r15       \n\t"
#endif
    "jmp *%%rax                 \n\t" /* jump to "to" thread */
    : /* no outputs */
    : [to] "r" (to), [from] "r" (from)
    : "rax"
  );
}

#ifdef __cplusplus
}
#endif
