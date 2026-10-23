#if !defined(__XTENSA_WINDOWED_ABI__) || defined(__XTENSA_CALL0_ABI__)
#error incorrect S3 windowed ABI in ClangImporter
#endif
#if !defined(__XTENSA_EL__) || __SIZEOF_POINTER__ != 4
#error incorrect S3 data model in ClangImporter
#endif
#ifndef EXPECT_S32C1I
#define EXPECT_S32C1I 1
#endif
#if __XCHAL_HAVE_S32C1I != EXPECT_S32C1I
#error incorrect s32c1i setting in ClangImporter
#endif
_Static_assert(_Alignof(double) == 8, "64-bit double alignment");

