/*
  libco.amd64_sysv (2015-04-20)
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
  assert(0); /* called only if cothread_t entrypoint returns */
}

cothread_t co_active() {
  if(!co_active_handle) co_active_handle = &co_active_buffer;
  return co_active_handle;
}

cothread_t co_create(unsigned int size, void (*entrypoint)(void)) {
  cothread_t handle;

  if(!co_active_handle) co_active_handle = &co_active_buffer;
  size += 512; /* allocate additional space for storage */
  size &= ~15; /* align stack to 16-byte boundary */

  if(handle = (cothread_t)malloc(size)) {
    long long *p = (long long*)((char*)handle + size); /* seek to top of stack */
    *--p = (long long)crash;                           /* crash if entrypoint returns */
    *--p = (long long)entrypoint;                      /* start of function */
    *(long long*)handle = (long long)p;                /* stack pointer */
  }

  return handle;
}

void co_delete(cothread_t handle) {
  free(handle);
}

void co_switch(cothread_t to) {
  register cothread_t from = co_active_handle;
  co_active_handle = to;

  __asm__ __volatile__(
    "movq %%rsp,(%[from])       \n\t" /* save old stack pointer */
    "movq (%[to]),%%rsp         \n\t" /* load new stack pointer */

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

    : /* no outputs */
    : [to] "r" (to), [from] "r" (from)
  );
  /* returns to "to" thread */
}

#ifdef __cplusplus
}
#endif
