/*
  libco.x86_gcc (2015-11-20)
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
  assert(0);  /* called only if cothread_t entrypoint returns */
}

cothread_t co_active() {
  if(!co_active_handle) co_active_handle = &co_active_buffer;
  return co_active_handle;
}

cothread_t co_create(unsigned int size, void (*entrypoint)(void)) {
  cothread_t handle;

  if(!co_active_handle) co_active_handle = &co_active_buffer;
  size += 256;  /* allocate additional space for storage */
  size &= ~15;  /* align stack to 16-byte boundary */

  if(handle = (cothread_t)malloc(size)) {
    long *p = (long*)((char*)handle + size);  /* seek to top of stack */
    *--p = (long)crash;                       /* crash if entrypoint returns */
    *--p = (long)entrypoint;                  /* start of function */
    *(long*)handle = (long)p;                 /* stack pointer */
  }

  return handle;
}

void co_delete(cothread_t handle) {
  free(handle);
}

#if defined(__APPLE__) || defined(_WIN32)
  #define SYM(x) "_" #x
#else
  #define SYM(x) #x
#endif

__asm__(
  ".globl " SYM(co_switch) "          \n\t"
  SYM(co_switch) ":                   \n\t"
  "movl 4(%esp),%ecx                  \n\t" /* argument on stack */
  "movl " SYM(co_active_handle) ",%edx\n\t" /* from */
  "movl %ecx," SYM(co_active_handle) "\n\t" /* to */

  "movl %esp,(%edx)  \n\t" /* save old stack pointer */
  "movl (%ecx),%esp  \n\t" /* load new stack pointer */
  "popl %eax         \n\t" /* pop return address off stack */

  "movl %ebp, 4(%edx)\n\t" /* backup non-volatile registers */
  "movl %esi, 8(%edx)\n\t"
  "movl %edi,12(%edx)\n\t"
  "movl %ebx,16(%edx)\n\t"

  "movl  4(%ecx),%ebp\n\t" /* restore non-volatile registers */
  "movl  8(%ecx),%esi\n\t"
  "movl 12(%ecx),%edi\n\t"
  "movl 16(%ecx),%ebx\n\t"

  "jmp *%eax         \n\t" /* jump to "to" thread */
);

#undef SYM

#ifdef __cplusplus
}
#endif
