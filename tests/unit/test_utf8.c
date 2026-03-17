#include "ij_internal.h"

#include <unity/unity.h>

void setUp(void)
{
}

void tearDown(void)
{
}

void test_utf8_valid_ascii(void)
{
    TEST_ASSERT_TRUE(ij_utf8_scalar_validate("Hello, world!", 13));
}

void test_utf8_valid_two_byte(void)
{
    TEST_ASSERT_TRUE(ij_utf8_scalar_validate("\xc3\xa9", 2));
}

void test_utf8_valid_three_byte(void)
{
    TEST_ASSERT_TRUE(ij_utf8_scalar_validate("\xe6\x97\xa5", 3));
}

void test_utf8_valid_four_byte(void)
{
    TEST_ASSERT_TRUE(ij_utf8_scalar_validate("\xf0\x9d\x84\x9e", 4));
}

void test_utf8_valid_mixed(void)
{
    TEST_ASSERT_TRUE(ij_utf8_scalar_validate("Hello \xc3\xa9 \xe6\x97\xa5 \xf0\x9d\x84\x9e", 18));
}

void test_utf8_invalid_bare_continuation(void)
{
    TEST_ASSERT_FALSE(ij_utf8_scalar_validate("\x80", 1));
}

void test_utf8_invalid_0xff(void)
{
    TEST_ASSERT_FALSE(ij_utf8_scalar_validate("\xff", 1));
}

void test_utf8_invalid_overlong_two_byte(void)
{
    TEST_ASSERT_FALSE(ij_utf8_scalar_validate("\xc0\x80", 2));
}

void test_utf8_invalid_surrogate_half(void)
{
    TEST_ASSERT_FALSE(ij_utf8_scalar_validate("\xed\xa0\x80", 3));
}

void test_utf8_invalid_above_max_codepoint(void)
{
    TEST_ASSERT_FALSE(ij_utf8_scalar_validate("\xf4\x90\x80\x80", 4));
}

void test_utf8_truncated_two_byte_at_end(void)
{
    TEST_ASSERT_FALSE(ij_utf8_scalar_validate("\xc3", 1));
}

void test_utf8_truncated_three_byte_at_end(void)
{
    TEST_ASSERT_FALSE(ij_utf8_scalar_validate("\xe6\x97", 2));
}

void test_utf8_truncated_four_byte_at_end(void)
{
    TEST_ASSERT_FALSE(ij_utf8_scalar_validate("\xf0\x9d\x84", 3));
}

void test_utf8_empty_string(void)
{
    TEST_ASSERT_TRUE(ij_utf8_scalar_validate("", 0));
}

void test_utf8_dispatch_matches_scalar(void)
{
    const char *data = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
    size_t len = 64;
    TEST_ASSERT_TRUE(ij_utf8_scalar_validate(data, len));
    TEST_ASSERT_TRUE(ij_utf8_is_valid_dispatch(data, len));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_utf8_valid_ascii);
    RUN_TEST(test_utf8_valid_two_byte);
    RUN_TEST(test_utf8_valid_three_byte);
    RUN_TEST(test_utf8_valid_four_byte);
    RUN_TEST(test_utf8_valid_mixed);
    RUN_TEST(test_utf8_invalid_bare_continuation);
    RUN_TEST(test_utf8_invalid_0xff);
    RUN_TEST(test_utf8_invalid_overlong_two_byte);
    RUN_TEST(test_utf8_invalid_surrogate_half);
    RUN_TEST(test_utf8_invalid_above_max_codepoint);
    RUN_TEST(test_utf8_truncated_two_byte_at_end);
    RUN_TEST(test_utf8_truncated_three_byte_at_end);
    RUN_TEST(test_utf8_truncated_four_byte_at_end);
    RUN_TEST(test_utf8_empty_string);
    RUN_TEST(test_utf8_dispatch_matches_scalar);
    return UNITY_END();
}
