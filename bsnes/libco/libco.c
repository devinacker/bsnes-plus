/*
  libco
  license: public domain
*/

#if defined(__clang__) || defined(__GNUC__)
  #if defined(__i386__)
    #if defined(LIBCO_NO_INLINE_ASM)
      #include "x86.c"
    #else
      #include "x86_gcc.c"
    #endif
  #elif defined(__amd64__)
    #if defined(LIBCO_NO_INLINE_ASM)
      #include "amd64.c"
    #else
      #include "amd64_gcc.c"
    #endif
  #elif defined(__arm__)
    #include "arm.c"
  #elif defined(_ARCH_PPC)
    #include "ppc.c"
  #elif defined(_WIN32)
    #include "fiber.c"
  #else
    #include "sjlj.c"
  #endif
#elif defined(_MSC_VER)
  #if defined(_M_IX86)
    #include "x86.c"
  #elif defined(_M_AMD64)
    #include "amd64.c"
  #else
    #include "fiber.c"
  #endif
#else
  #error "libco: unsupported processor, compiler or operating system"
#endif
