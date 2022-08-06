extern void decl(void);

#if defined(RV32_ONLY)

#   if defined(XLEN) && XLEN != 32
#       error "RV32_ONLY doesn't make sense if XLEN != 32"
#   endif

#else

#   if defined(XLEN) && XLEN != 64
#       error "RV64 doesn't make sense if XLEN != 64"
#   endif

#endif
