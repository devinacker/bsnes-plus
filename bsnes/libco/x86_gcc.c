/*
  libco.x86_gcc (2015-04-20)
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

static thread_local long co_active_buffer[64];
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
  size += 256; /* allocate additional space for storage */
  size &= ~15; /* align stack to 16-byte boundary */

  if(handle = (cothread_t)malloc(size)) {
    long *p = (long*)((char*)handle + size); /* seek to top of stack */
    *--p = (long)crash;                      /* crash if entrypoint returns */
    *--p = (long)entrypoint;                 /* start of function */
    *(long*)handle = (long)p;                /* stack pointer */
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
    "movl %%esp,(%[from])       \n\t" /* save old stack pointer */
    "movl (%[to]),%%esp         \n\t" /* load new stack pointer */

    "movl %%ebp, 4(%[from])     \n\t" /* backup non-volatile registers */
    "movl %%esi, 8(%[from])     \n\t"
    "movl %%edi,12(%[from])     \n\t"
    "movl %%ebx,16(%[from])     \n\t"

    "movl  4(%[to]),%%ebp       \n\t" /* restore non-volatile registers */
    "movl  8(%[to]),%%esi       \n\t"
    "movl 12(%[to]),%%edi       \n\t"
    "movl 16(%[to]),%%ebx       \n\t"

    : /* no outputs */
    : [to] "r" (to), [from] "r" (from)
  );
  /* returns to "to" thread */
}

#ifdef __cplusplus
}
#endif
