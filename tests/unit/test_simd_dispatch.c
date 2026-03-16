#include "ij_internal.h"

#include <string.h>
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

void test_json_escape_scalar_matches_dispatch_for_safe_ascii(void)
{
    unsigned char buf[64];
    memset(buf, 'a', sizeof(buf));

    size_t scalar_result = ij_json_escape_scalar_find_first_special(buf, sizeof(buf));
    size_t dispatch_result = ij_json_escape_find_first_special(buf, sizeof(buf));

    TEST_ASSERT_EQUAL_size_t(scalar_result, dispatch_result);
    TEST_ASSERT_EQUAL_size_t(sizeof(buf), scalar_result);
}

void test_json_escape_scalar_matches_dispatch_for_special_at_boundaries(void)
{
    static const size_t positions[] = {0, 31, 32, 63};

    for (size_t i = 0; i < sizeof(positions) / sizeof(positions[0]); i++) {
        unsigned char buf[64];
        memset(buf, 'a', sizeof(buf));
        buf[positions[i]] = '"';

        size_t scalar_result = ij_json_escape_scalar_find_first_special(buf, sizeof(buf));
        size_t dispatch_result = ij_json_escape_find_first_special(buf, sizeof(buf));

        TEST_ASSERT_EQUAL_size_t(scalar_result, dispatch_result);
        TEST_ASSERT_EQUAL_size_t(positions[i], scalar_result);
    }
}

void test_json_escape_scalar_matches_dispatch_for_control_chars(void)
{
    unsigned char buf[64];
    memset(buf, 'a', sizeof(buf));
    buf[16] = '\n';
    buf[48] = '\t';

    size_t scalar_result = ij_json_escape_scalar_find_first_special(buf, sizeof(buf));
    size_t dispatch_result = ij_json_escape_find_first_special(buf, sizeof(buf));

    TEST_ASSERT_EQUAL_size_t(scalar_result, dispatch_result);
    TEST_ASSERT_EQUAL_size_t(16, scalar_result);
}

void test_utf8_scalar_matches_dispatch_for_valid_long_string(void)
{
    char buf[64];
    memset(buf, 'A', sizeof(buf));

    bool scalar_result = ij_utf8_scalar_validate(buf, sizeof(buf));
    bool dispatch_result = ij_utf8_is_valid_dispatch(buf, sizeof(buf));

    TEST_ASSERT_TRUE(scalar_result);
    TEST_ASSERT_TRUE(dispatch_result);
}

void test_utf8_scalar_matches_dispatch_for_invalid_at_position_33(void)
{
    char buf[64];
    memset(buf, 'A', sizeof(buf));
    buf[33] = (char)0xff;

    bool scalar_result = ij_utf8_scalar_validate(buf, sizeof(buf));
    bool dispatch_result = ij_utf8_is_valid_dispatch(buf, sizeof(buf));

    TEST_ASSERT_FALSE(scalar_result);
    TEST_ASSERT_FALSE(dispatch_result);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_simd_dispatch_reports_valid_kinds_for_hotspot_targets);
    RUN_TEST(test_simd_dispatch_support_flags_match_selected_kinds);
    RUN_TEST(test_json_escape_scalar_matches_dispatch_for_safe_ascii);
    RUN_TEST(test_json_escape_scalar_matches_dispatch_for_special_at_boundaries);
    RUN_TEST(test_json_escape_scalar_matches_dispatch_for_control_chars);
    RUN_TEST(test_utf8_scalar_matches_dispatch_for_valid_long_string);
    RUN_TEST(test_utf8_scalar_matches_dispatch_for_invalid_at_position_33);
    return UNITY_END();
}
