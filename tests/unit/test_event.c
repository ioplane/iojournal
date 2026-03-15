#include "ij_internal.h"

#include <unity/unity.h>

#include <string.h>

static ij_event_t ij_test_minimal_event(const ij_attr_t *attributes, size_t attribute_count)
{
    ij_event_t event = {
        .timestamp = {.unix_seconds = 0, .nanoseconds = 123000000U},
        .level = IJ_LEVEL_INFO,
        .event_name = "startup.complete",
        .message = "service started",
        .logger = "iojournal.test",
        .attributes = attributes,
        .attribute_count = attribute_count,
        .trace_id = NULL,
        .span_id = NULL,
        .trace_flags = 0U,
        .has_trace_context = false,
        .source_file = "tests/unit/test_event.c",
        .source_line = 42U,
        .source_function = "ij_test_minimal_event",
    };

    return event;
}

static ij_attr_value_t ij_test_string_value(const char *value)
{
    ij_attr_value_t attr_value = {
        .kind = IJ_ATTR_VALUE_STRING,
        .as.string =
            {
                .data = value,
                .len = strlen(value),
            },
    };

    return attr_value;
}

void setUp(void)
{
}

void tearDown(void)
{
}

void test_validate_event_accepts_minimal_event(void)
{
    ij_event_t event = ij_test_minimal_event(NULL, 0U);

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_validate_event(&event));
}

void test_validate_event_rejects_duplicate_attribute_keys(void)
{
    ij_attr_t attributes[] = {
        {.key = "request_id", .value = ij_test_string_value("req-1")},
        {.key = "request_id", .value = ij_test_string_value("req-2")},
    };
    ij_event_t event = ij_test_minimal_event(attributes, 2U);

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_INVALID_ARGUMENT, ij_validate_event(&event));
}

void test_event_copy_redacts_sensitive_attribute_values(void)
{
    ij_attr_t attributes[] = {
        {.key = "auth.token", .value = ij_test_string_value("secret-token")},
        {.key = "service.name", .value = ij_test_string_value("iojournal")},
    };
    ij_event_t event = ij_test_minimal_event(attributes, 2U);
    ij_event_copy_t copied_event = {0};

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_event_copy_from_input(&copied_event, &event,
                                                                 IJ_REDACTION_MODE_ENABLED));
    TEST_ASSERT_EQUAL_size_t(2U, copied_event.attribute_count);
    TEST_ASSERT_EQUAL_STRING("[REDACTED]", copied_event.attributes[0].value.as.string.data);
    TEST_ASSERT_EQUAL_STRING("iojournal", copied_event.attributes[1].value.as.string.data);

    ij_event_copy_dispose(&copied_event);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_validate_event_accepts_minimal_event);
    RUN_TEST(test_validate_event_rejects_duplicate_attribute_keys);
    RUN_TEST(test_event_copy_redacts_sensitive_attribute_values);
    return UNITY_END();
}
