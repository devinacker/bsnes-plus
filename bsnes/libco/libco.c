/*
  libco
  auto-selection module
  license: public domain
*/

#if defined(__GNUC__) && defined(__i386__)
  #if defined(LIBCO_NO_INLINE_ASM)
    #include "x86.c"
  #else
    #include "x86_gcc.c"
  #endif
#elif defined(__GNUC__) && defined(__amd64__)
  #if defined(LIBCO_NO_INLINE_ASM)
    #include "amd64.c"
  #else
    #include "amd64_gcc.c"
  #endif
#elif defined(__GNUC__) && defined(_ARCH_PPC)
  #include "ppc.c"
#elif defined(__GNUC__)
  #include "sjlj.c"
#elif defined(_MSC_VER) && defined(_M_IX86)
  #include "x86.c"
#elif defined(_MSC_VER) && defined(_M_AMD64)
  #include "amd64.c"
#elif defined(_MSC_VER)
  #include "fiber.c"
#else
  #error "libco: unsupported processor, compiler or operating system"
#endif
