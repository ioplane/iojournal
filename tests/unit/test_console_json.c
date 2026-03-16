#include "ij_internal.h"

#include <unity/unity.h>

#include <string.h>

static ij_event_t ij_test_console_event(void)
{
    static const ij_attr_t attributes[] = {
        {.key = "request_id",
         .value =
             {
                 .kind = IJ_ATTR_VALUE_STRING,
                 .as.string =
                     {
                         .data = "req-42",
                         .len = 6U,
                     },
             }},
        {.key = "auth.token",
         .value =
             {
                 .kind = IJ_ATTR_VALUE_STRING,
                 .as.string =
                     {
                         .data = "secret-token",
                         .len = 12U,
                     },
             }},
    };
    ij_event_t event = {
        .timestamp = {.unix_seconds = 0, .nanoseconds = 123000000U},
        .level = IJ_LEVEL_INFO,
        .event_name = "startup.complete",
        .message = "hello\n\"world\"",
        .logger = "iojournal.test",
        .attributes = attributes,
        .attribute_count = 2U,
        .trace_id = NULL,
        .span_id = NULL,
        .trace_flags = 0U,
        .has_trace_context = false,
        .source_file = NULL,
        .source_line = 0U,
        .source_function = NULL,
    };

    return event;
}

static ij_event_t ij_test_numeric_console_event(void)
{
    static const ij_attr_t attributes[] = {
        {.key = "signed_value",
         .value =
             {
                 .kind = IJ_ATTR_VALUE_SIGNED,
                 .as.signed_value = -42,
             }},
        {.key = "unsigned_value",
         .value =
             {
                 .kind = IJ_ATTR_VALUE_UNSIGNED,
                 .as.unsigned_value = 42U,
             }},
        {.key = "bool_value",
         .value =
             {
                 .kind = IJ_ATTR_VALUE_BOOL,
                 .as.bool_value = true,
             }},
        {.key = "double_value",
         .value =
             {
                 .kind = IJ_ATTR_VALUE_DOUBLE,
                 .as.double_value = 3.5,
             }},
    };
    ij_event_t event = {
        .timestamp = {.unix_seconds = 0, .nanoseconds = 123000000U},
        .level = IJ_LEVEL_INFO,
        .event_name = "numeric.emit",
        .message = "numbers",
        .logger = "iojournal.test",
        .attributes = attributes,
        .attribute_count = 4U,
        .trace_id = NULL,
        .span_id = NULL,
        .trace_flags = 0U,
        .has_trace_context = false,
        .source_file = NULL,
        .source_line = 0U,
        .source_function = NULL,
    };

    return event;
}

static ij_event_t ij_test_warn_console_event(void)
{
    ij_event_t event = {
        .timestamp = {.unix_seconds = 0, .nanoseconds = 123000000U},
        .level = IJ_LEVEL_WARN,
        .event_name = "warn.emit",
        .message = "warn-path",
        .logger = "iojournal.test",
        .attributes = NULL,
        .attribute_count = 0U,
        .trace_id = NULL,
        .span_id = NULL,
        .trace_flags = 0U,
        .has_trace_context = false,
        .source_file = NULL,
        .source_line = 0U,
        .source_function = NULL,
    };

    return event;
}

static ij_event_t ij_test_special_key_console_event(void)
{
    static const ij_attr_t attributes[] = {
        {.key = "ctx.\"quoted\"\nkey",
         .value =
             {
                 .kind = IJ_ATTR_VALUE_STRING,
                 .as.string =
                     {
                         .data = "value",
                         .len = 5U,
                     },
             }},
    };
    ij_event_t event = {
        .timestamp = {.unix_seconds = 0, .nanoseconds = 123000000U},
        .level = IJ_LEVEL_INFO,
        .event_name = "special.key",
        .message = "plain",
        .logger = "iojournal.test",
        .attributes = attributes,
        .attribute_count = 1U,
        .trace_id = NULL,
        .span_id = NULL,
        .trace_flags = 0U,
        .has_trace_context = false,
        .source_file = NULL,
        .source_line = 0U,
        .source_function = NULL,
    };

    return event;
}

void setUp(void)
{
}

void tearDown(void)
{
}

void test_timestamp_format_uses_canonical_rfc3339_utc_millis(void)
{
    char timestamp[32];

    TEST_ASSERT_EQUAL_INT(
        IJ_STATUS_OK,
        ij_format_timestamp_rfc3339((ij_timestamp_t){.unix_seconds = 0, .nanoseconds = 123000000U},
                                    timestamp, sizeof(timestamp)));
    TEST_ASSERT_EQUAL_STRING("1970-01-01T00:00:00.123Z", timestamp);
}

void test_json_console_encoder_uses_stable_field_order_and_redaction(void)
{
    ij_event_t event = ij_test_console_event();
    ij_event_copy_t copied_event = {0};
    char buffer[1024];
    size_t output_size = 0U;

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_event_copy_from_input(&copied_event, &event,
                                                                 IJ_REDACTION_MODE_ENABLED));
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_json_console_encode(&copied_event, buffer,
                                                               sizeof(buffer), &output_size));
    TEST_ASSERT_EQUAL_STRING(
        "{\"timestamp\":\"1970-01-01T00:00:00.123Z\","
        "\"level\":\"info\","
        "\"event_name\":\"startup.complete\","
        "\"message\":\"hello\\n\\\"world\\\"\","
        "\"logger\":\"iojournal.test\","
        "\"attributes\":{\"request_id\":\"req-42\",\"auth.token\":\"[REDACTED]\"}}",
        buffer);
    TEST_ASSERT_EQUAL_size_t(strlen(buffer), output_size);

    ij_event_copy_dispose(&copied_event);
}

void test_ndjson_encoder_appends_single_newline_and_exact_length(void)
{
    ij_event_t event = ij_test_console_event();
    ij_event_copy_t copied_event = {0};
    char buffer[1024];
    size_t output_size = 0U;

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_event_copy_from_input(&copied_event, &event,
                                                                 IJ_REDACTION_MODE_ENABLED));
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK,
                          ij_ndjson_encode(&copied_event, buffer, sizeof(buffer), &output_size));
    TEST_ASSERT_EQUAL_STRING(
        "{\"timestamp\":\"1970-01-01T00:00:00.123Z\","
        "\"level\":\"info\","
        "\"event_name\":\"startup.complete\","
        "\"message\":\"hello\\n\\\"world\\\"\","
        "\"logger\":\"iojournal.test\","
        "\"attributes\":{\"request_id\":\"req-42\",\"auth.token\":\"[REDACTED]\"}}\n",
        buffer);
    TEST_ASSERT_EQUAL_size_t(strlen(buffer), output_size);

    ij_event_copy_dispose(&copied_event);
}

void test_json_console_encoder_preserves_scalar_attribute_text(void)
{
    ij_event_t event = ij_test_numeric_console_event();
    ij_event_copy_t copied_event = {0};
    char buffer[1024];
    size_t output_size = 0U;

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_event_copy_from_input(&copied_event, &event,
                                                                 IJ_REDACTION_MODE_ENABLED));
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_json_console_encode(&copied_event, buffer,
                                                               sizeof(buffer), &output_size));
    TEST_ASSERT_EQUAL_STRING("{\"timestamp\":\"1970-01-01T00:00:00.123Z\","
                             "\"level\":\"info\","
                             "\"event_name\":\"numeric.emit\","
                             "\"message\":\"numbers\","
                             "\"logger\":\"iojournal.test\","
                             "\"attributes\":{\"signed_value\":-42,\"unsigned_value\":42,"
                             "\"bool_value\":true,\"double_value\":3.5}}",
                             buffer);
    TEST_ASSERT_EQUAL_size_t(strlen(buffer), output_size);

    ij_event_copy_dispose(&copied_event);
}

void test_json_console_encoder_preserves_warn_level_literal(void)
{
    ij_event_t event = ij_test_warn_console_event();
    ij_event_copy_t copied_event = {0};
    char buffer[512];
    size_t output_size = 0U;

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_event_copy_from_input(&copied_event, &event,
                                                                 IJ_REDACTION_MODE_ENABLED));
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_json_console_encode(&copied_event, buffer,
                                                               sizeof(buffer), &output_size));
    TEST_ASSERT_EQUAL_STRING("{\"timestamp\":\"1970-01-01T00:00:00.123Z\","
                             "\"level\":\"warn\","
                             "\"event_name\":\"warn.emit\","
                             "\"message\":\"warn-path\","
                             "\"logger\":\"iojournal.test\"}",
                             buffer);
    TEST_ASSERT_EQUAL_size_t(strlen(buffer), output_size);

    ij_event_copy_dispose(&copied_event);
}

void test_json_console_encoder_preserves_escaped_attribute_keys(void)
{
    ij_event_t event = ij_test_special_key_console_event();
    ij_event_copy_t copied_event = {0};
    char buffer[1024];
    size_t output_size = 0U;

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_event_copy_from_input(&copied_event, &event,
                                                                 IJ_REDACTION_MODE_ENABLED));
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_json_console_encode(&copied_event, buffer,
                                                               sizeof(buffer), &output_size));
    TEST_ASSERT_EQUAL_STRING("{\"timestamp\":\"1970-01-01T00:00:00.123Z\","
                             "\"level\":\"info\","
                             "\"event_name\":\"special.key\","
                             "\"message\":\"plain\","
                             "\"logger\":\"iojournal.test\","
                             "\"attributes\":{\"ctx.\\\"quoted\\\"\\nkey\":\"value\"}}",
                             buffer);
    TEST_ASSERT_EQUAL_size_t(strlen(buffer), output_size);

    ij_event_copy_dispose(&copied_event);
}

void test_json_console_encode_fails_with_tiny_buffer(void)
{
    ij_event_t event = ij_test_console_event();
    ij_event_copy_t copied_event = {0};
    char buffer[10];
    size_t output_size = 0U;

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_event_copy_from_input(&copied_event, &event,
                                                                 IJ_REDACTION_MODE_ENABLED));
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_ENCODE_ERROR,
                          ij_json_console_encode(&copied_event, buffer, 10, &output_size));

    ij_event_copy_dispose(&copied_event);
}

void test_json_console_encode_fails_with_zero_buffer(void)
{
    ij_event_t event = ij_test_console_event();
    ij_event_copy_t copied_event = {0};
    size_t output_size = 0U;

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_event_copy_from_input(&copied_event, &event,
                                                                 IJ_REDACTION_MODE_ENABLED));
    ij_status_t status = ij_json_console_encode(&copied_event, NULL, 0, &output_size);
    TEST_ASSERT_TRUE(status == IJ_STATUS_ENCODE_ERROR || status == IJ_STATUS_INVALID_ARGUMENT);

    ij_event_copy_dispose(&copied_event);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_timestamp_format_uses_canonical_rfc3339_utc_millis);
    RUN_TEST(test_json_console_encoder_uses_stable_field_order_and_redaction);
    RUN_TEST(test_ndjson_encoder_appends_single_newline_and_exact_length);
    RUN_TEST(test_json_console_encoder_preserves_scalar_attribute_text);
    RUN_TEST(test_json_console_encoder_preserves_warn_level_literal);
    RUN_TEST(test_json_console_encoder_preserves_escaped_attribute_keys);
    RUN_TEST(test_json_console_encode_fails_with_tiny_buffer);
    RUN_TEST(test_json_console_encode_fails_with_zero_buffer);
    return UNITY_END();
}
