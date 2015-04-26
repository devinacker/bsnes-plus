/*
  libco.x86_gcc (2015-04-22)
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

/*
  The compiler *must not* create a stack frame for this function!
  Unfortunately, GCC does not support __attribute__((naked)) on x86,
  so we must do the best we can by forcing omit-frame-pointer and
  explicitly specifying a volatile register for the local variable
  (some GCCs have brain damage and may put a local variable in ebp
  even when volatile registers are available)
*/
#ifdef __clang__
  #define NAKED __attribute__((naked))
#else
  #define NAKED __attribute__((optimize("omit-frame-pointer")))
#endif
void NAKED co_switch(cothread_t to) {
  register cothread_t from __asm__("edx") = co_active_handle;
  co_active_handle = to;

  __asm__ __volatile__(
    "movl %%esp,(%[from])       \n\t" /* save old stack pointer */
    "movl (%[to]),%%esp         \n\t" /* load new stack pointer */
    "popl %%eax                 \n\t" /* pop return address off stack */

    "movl %%ebp, 4(%[from])     \n\t" /* backup non-volatile registers */
    "movl %%esi, 8(%[from])     \n\t"
    "movl %%edi,12(%[from])     \n\t"
    "movl %%ebx,16(%[from])     \n\t"

    "movl  4(%[to]),%%ebp       \n\t" /* restore non-volatile registers */
    "movl  8(%[to]),%%esi       \n\t"
    "movl 12(%[to]),%%edi       \n\t"
    "movl 16(%[to]),%%ebx       \n\t"

    "jmp *%%eax                 \n\t" /* jump to "to" thread */
    : /* no outputs */
    : [to] "r" (to), [from] "r" (from)
    : "eax"
  );
}

#ifdef __cplusplus
}
#endif
