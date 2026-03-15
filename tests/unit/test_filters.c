#include "ij_internal.h"

#include <unity/unity.h>

void setUp(void)
{
}

void tearDown(void)
{
}

void test_level_filter_accepts_events_at_or_above_minimum(void)
{
    TEST_ASSERT_TRUE(ij_level_is_enabled(IJ_LEVEL_INFO, IJ_LEVEL_INFO));
    TEST_ASSERT_TRUE(ij_level_is_enabled(IJ_LEVEL_INFO, IJ_LEVEL_ERROR));
}

void test_level_filter_rejects_events_below_minimum(void)
{
    TEST_ASSERT_FALSE(ij_level_is_enabled(IJ_LEVEL_WARN, IJ_LEVEL_INFO));
    TEST_ASSERT_FALSE(ij_level_is_enabled(IJ_LEVEL_ERROR, IJ_LEVEL_DEBUG));
}

void test_redaction_key_matching_is_case_insensitive(void)
{
    TEST_ASSERT_TRUE(ij_key_should_redact("Authorization"));
    TEST_ASSERT_TRUE(ij_key_should_redact("auth.token"));
    TEST_ASSERT_TRUE(ij_key_should_redact("client_secret"));
    TEST_ASSERT_FALSE(ij_key_should_redact("service.name"));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_level_filter_accepts_events_at_or_above_minimum);
    RUN_TEST(test_level_filter_rejects_events_below_minimum);
    RUN_TEST(test_redaction_key_matching_is_case_insensitive);
    return UNITY_END();
}
