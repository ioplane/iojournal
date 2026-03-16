#include "ij_internal.h"

#include <unity/unity.h>

#include <stdio.h>
#include <string.h>

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

void test_redaction_key_matching_accepts_all_builtin_exact_keys(void)
{
    static const char *keys[] = {
        "password",   "passwd",        "passphrase",    "secret",      "client_secret",
        "token",      "access_token",  "refresh_token", "id_token",    "api_key",
        "apikey",     "authorization", "session_id",    "session",     "cookie",
        "set_cookie", "private_key",   "secret_key",    "signing_key",
    };

    for (size_t i = 0U; i < (sizeof(keys) / sizeof(keys[0])); ++i) {
        TEST_ASSERT_TRUE(ij_key_should_redact(keys[i]));
    }
}

void test_redaction_key_matching_accepts_dotted_suffix_variants(void)
{
    static const char *keys[] = {
        "password",   "passwd",        "passphrase",    "secret",      "client_secret",
        "token",      "access_token",  "refresh_token", "id_token",    "api_key",
        "apikey",     "authorization", "session_id",    "session",     "cookie",
        "set_cookie", "private_key",   "secret_key",    "signing_key",
    };
    char dotted_key[64];

    for (size_t i = 0U; i < (sizeof(keys) / sizeof(keys[0])); ++i) {
        int written = snprintf(dotted_key, sizeof(dotted_key), "ctx.%s", keys[i]);

        TEST_ASSERT_TRUE(written > 0);
        TEST_ASSERT_TRUE((size_t)written < sizeof(dotted_key));
        TEST_ASSERT_TRUE(ij_key_should_redact(dotted_key));
    }
}

void test_redaction_key_matching_rejects_known_non_matches(void)
{
    TEST_ASSERT_FALSE(ij_key_should_redact("service.name"));
    TEST_ASSERT_FALSE(ij_key_should_redact("trace_id"));
    TEST_ASSERT_FALSE(ij_key_should_redact("user_id"));
    TEST_ASSERT_FALSE(ij_key_should_redact("public_key_id"));
}

void test_redaction_length_aware_entrypoints_preserve_semantics(void)
{
    ij_owned_attr_value_t value = {
        .kind = IJ_ATTR_VALUE_STRING,
        .as.string =
            {
                .data = "top-secret",
                .len = 10U,
            },
    };
    char buffer[32] = "top-secret";

    value.as.string.data = buffer;
    TEST_ASSERT_TRUE(ij_key_should_redact_n("authorization", 13U));
    TEST_ASSERT_TRUE(ij_key_should_redact_n("ctx.client_secret", strlen("ctx.client_secret")));
    TEST_ASSERT_FALSE(ij_key_should_redact_n("service.name", strlen("service.name")));

    ij_redact_owned_attr_value_n("authorization", 13U, &value, sizeof(buffer));
    TEST_ASSERT_EQUAL_STRING(IJ_REDACTED_LITERAL, value.as.string.data);
    TEST_ASSERT_EQUAL_size_t(sizeof(IJ_REDACTED_LITERAL) - 1U, value.as.string.len);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_level_filter_accepts_events_at_or_above_minimum);
    RUN_TEST(test_level_filter_rejects_events_below_minimum);
    RUN_TEST(test_redaction_key_matching_is_case_insensitive);
    RUN_TEST(test_redaction_key_matching_accepts_all_builtin_exact_keys);
    RUN_TEST(test_redaction_key_matching_accepts_dotted_suffix_variants);
    RUN_TEST(test_redaction_key_matching_rejects_known_non_matches);
    RUN_TEST(test_redaction_length_aware_entrypoints_preserve_semantics);
    return UNITY_END();
}
