#include "ij_internal.h"

#include <unity/unity.h>

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
    TEST_ASSERT_TRUE(output_size > 0U);

    ij_event_copy_dispose(&copied_event);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_timestamp_format_uses_canonical_rfc3339_utc_millis);
    RUN_TEST(test_json_console_encoder_uses_stable_field_order_and_redaction);
    return UNITY_END();
}
