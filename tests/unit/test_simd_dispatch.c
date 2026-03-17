#include "ij_internal.h"

#include <unity/unity.h>

void setUp(void)
{
}

void tearDown(void)
{
}

void test_simd_dispatch_reports_valid_kinds_for_hotspot_targets(void)
{
    ij_simd_kind_t json_kind = ij_simd_json_escape_kind();
    ij_simd_kind_t redact_kind = ij_simd_redact_match_kind();
    ij_simd_kind_t utf8_kind = ij_simd_utf8_kind();

    TEST_ASSERT_TRUE(json_kind == IJ_SIMD_KIND_SCALAR || json_kind == IJ_SIMD_KIND_X86_AVX2 ||
                     json_kind == IJ_SIMD_KIND_ARM_NEON);
    TEST_ASSERT_TRUE(redact_kind == IJ_SIMD_KIND_SCALAR || redact_kind == IJ_SIMD_KIND_X86_AVX2 ||
                     redact_kind == IJ_SIMD_KIND_ARM_NEON);
    TEST_ASSERT_TRUE(utf8_kind == IJ_SIMD_KIND_SCALAR || utf8_kind == IJ_SIMD_KIND_X86_AVX2 ||
                     utf8_kind == IJ_SIMD_KIND_ARM_NEON);
}

void test_simd_dispatch_support_flags_match_selected_kinds(void)
{
    if (ij_simd_json_escape_kind() == IJ_SIMD_KIND_X86_AVX2 ||
        ij_simd_redact_match_kind() == IJ_SIMD_KIND_X86_AVX2 ||
        ij_simd_utf8_kind() == IJ_SIMD_KIND_X86_AVX2) {
        TEST_ASSERT_TRUE(ij_simd_runtime_has_x86_avx2());
    }

#if defined(__aarch64__)
    if (ij_simd_json_escape_kind() == IJ_SIMD_KIND_ARM_NEON ||
        ij_simd_redact_match_kind() == IJ_SIMD_KIND_ARM_NEON ||
        ij_simd_utf8_kind() == IJ_SIMD_KIND_ARM_NEON) {
        TEST_ASSERT_TRUE(ij_simd_runtime_has_arm_neon());
    }
#endif
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_simd_dispatch_reports_valid_kinds_for_hotspot_targets);
    RUN_TEST(test_simd_dispatch_support_flags_match_selected_kinds);
    return UNITY_END();
}
