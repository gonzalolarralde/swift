#if !defined(__XTENSA_WINDOWED_ABI__) || defined(__XTENSA_CALL0_ABI__)
#error incorrect chip windowed ABI in ClangImporter
#endif
#if !defined(__XTENSA_EL__) || __SIZEOF_POINTER__ != 4
#error incorrect chip data model in ClangImporter
#endif
#if __XCHAL_HAVE_FP != EXPECT_FP
#error incorrect chip FP feature in ClangImporter
#endif
#if __XCHAL_HAVE_S32C1I != EXPECT_CAS
#error incorrect chip CAS feature in ClangImporter
#endif
_Static_assert(_Alignof(double) == 8, "64-bit double alignment");
