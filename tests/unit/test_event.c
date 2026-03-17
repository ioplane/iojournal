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

static bool ij_pointer_within_copy_arena(const ij_event_copy_t *event, const void *ptr)
{
    const char *begin = event->text_arena;
    const char *end = event->text_arena + event->text_arena_size;
    const char *value = ptr;

    return value >= begin && value < end;
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
    TEST_ASSERT_TRUE(ij_pointer_within_copy_arena(&copied_event,
                                                  copied_event.attributes[0].value.as.string.data));

    ij_event_copy_dispose(&copied_event);
}

void test_event_copy_rejects_duplicate_attribute_keys(void)
{
    ij_attr_t attributes[] = {
        {.key = "request_id", .value = ij_test_string_value("req-1")},
        {.key = "request_id", .value = ij_test_string_value("req-2")},
    };
    ij_event_t event = ij_test_minimal_event(attributes, 2U);
    ij_event_copy_t copied_event = {0};

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_INVALID_ARGUMENT,
                          ij_event_copy_from_input(&copied_event, &event,
                                                   IJ_REDACTION_MODE_ENABLED));
    TEST_ASSERT_NULL(copied_event.text_arena);
    TEST_ASSERT_NULL(copied_event.attributes);
}

void test_event_copy_rejects_invalid_utf8_message(void)
{
    char invalid_message[] = {'o', 'k', ' ', (char)0xff, '\0'};
    ij_event_t event = ij_test_minimal_event(NULL, 0U);
    ij_event_copy_t copied_event = {0};

    event.message = invalid_message;

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_INVALID_ARGUMENT,
                          ij_event_copy_from_input(&copied_event, &event,
                                                   IJ_REDACTION_MODE_ENABLED));
    TEST_ASSERT_NULL(copied_event.text_arena);
    TEST_ASSERT_NULL(copied_event.attributes);
}

void test_event_copy_rejects_total_text_over_limit_without_attributes(void)
{
    char huge_message[IJ_MESSAGE_MAX_LEN + 1U];
    char huge_source_file[1200];
    char huge_source_function[1200];
    size_t total_text_size;
    ij_event_t event = ij_test_minimal_event(NULL, 0U);
    ij_event_copy_t copied_event = {0};

    memset(huge_message, 'x', sizeof(huge_message) - 1U);
    huge_message[sizeof(huge_message) - 1U] = '\0';
    memset(huge_source_file, 'f', sizeof(huge_source_file) - 1U);
    huge_source_file[sizeof(huge_source_file) - 1U] = '\0';
    memset(huge_source_function, 'g', sizeof(huge_source_function) - 1U);
    huge_source_function[sizeof(huge_source_function) - 1U] = '\0';
    event.message = huge_message;
    event.source_file = huge_source_file;
    event.source_function = huge_source_function;

    total_text_size = strlen(event.event_name) + strlen(event.message) + strlen(event.logger) +
                      strlen(event.source_file) + strlen(event.source_function);

    TEST_ASSERT_GREATER_THAN_UINT32((uint32_t)IJ_EVENT_TEXT_MAX_LEN, (uint32_t)total_text_size);
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_INVALID_ARGUMENT,
                          ij_event_copy_from_input(&copied_event, &event,
                                                   IJ_REDACTION_MODE_ENABLED));
    TEST_ASSERT_NULL(copied_event.text_arena);
    TEST_ASSERT_NULL(copied_event.attributes);
}

void test_event_copy_packs_text_fields_into_one_arena(void)
{
    ij_attr_t attributes[] = {
        {.key = "auth.token", .value = ij_test_string_value("x")},
        {.key = "service.name", .value = ij_test_string_value("iojournal")},
    };
    ij_event_t event = ij_test_minimal_event(attributes, 2U);
    ij_event_copy_t copied_event = {0};

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_event_copy_from_input(&copied_event, &event,
                                                                 IJ_REDACTION_MODE_ENABLED));
    TEST_ASSERT_NOT_NULL(copied_event.text_arena);
    TEST_ASSERT_GREATER_THAN_UINT32(0U, (uint32_t)copied_event.text_arena_size);
    TEST_ASSERT_TRUE(ij_pointer_within_copy_arena(&copied_event, copied_event.event_name.data));
    TEST_ASSERT_TRUE(ij_pointer_within_copy_arena(&copied_event, copied_event.message.data));
    TEST_ASSERT_TRUE(ij_pointer_within_copy_arena(&copied_event, copied_event.logger_name.data));
    TEST_ASSERT_TRUE(ij_pointer_within_copy_arena(&copied_event, copied_event.source_file.data));
    TEST_ASSERT_TRUE(
        ij_pointer_within_copy_arena(&copied_event, copied_event.source_function.data));
    TEST_ASSERT_TRUE(
        ij_pointer_within_copy_arena(&copied_event, copied_event.attributes[0].key.data));
    TEST_ASSERT_TRUE(ij_pointer_within_copy_arena(&copied_event,
                                                  copied_event.attributes[0].value.as.string.data));
    TEST_ASSERT_TRUE(
        ij_pointer_within_copy_arena(&copied_event, copied_event.attributes[1].key.data));
    TEST_ASSERT_TRUE(ij_pointer_within_copy_arena(&copied_event,
                                                  copied_event.attributes[1].value.as.string.data));

    ij_event_copy_dispose(&copied_event);
}

void test_event_copy_dispose_clears_packed_storage(void)
{
    ij_attr_t attributes[] = {
        {.key = "auth.token", .value = ij_test_string_value("x")},
    };
    ij_event_t event = ij_test_minimal_event(attributes, 1U);
    ij_event_copy_t copied_event = {0};

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_event_copy_from_input(&copied_event, &event,
                                                                 IJ_REDACTION_MODE_ENABLED));

    ij_event_copy_dispose(&copied_event);

    TEST_ASSERT_NULL(copied_event.text_arena);
    TEST_ASSERT_EQUAL_size_t(0U, copied_event.text_arena_size);
    TEST_ASSERT_NULL(copied_event.attributes);
    TEST_ASSERT_EQUAL_size_t(0U, copied_event.attribute_count);
}

void test_event_copy_tracks_json_escape_flags_for_ascii_and_special_fields(void)
{
    ij_attr_t attributes[] = {
        {.key = "plain_key", .value = ij_test_string_value("plain-value")},
        {.key = "quoted\"key", .value = ij_test_string_value("line\nbreak")},
    };
    ij_event_t event = ij_test_minimal_event(attributes, 2U);
    ij_event_copy_t copied_event = {0};

    event.message = "plain-message";

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_event_copy_from_input(&copied_event, &event,
                                                                 IJ_REDACTION_MODE_DISABLED));
    TEST_ASSERT_FALSE(copied_event.event_name.needs_json_escape);
    TEST_ASSERT_FALSE(copied_event.message.needs_json_escape);
    TEST_ASSERT_FALSE(copied_event.attributes[0].key.needs_json_escape);
    TEST_ASSERT_FALSE(copied_event.attributes[0].value.as.string.needs_json_escape);
    TEST_ASSERT_TRUE(copied_event.attributes[1].key.needs_json_escape);
    TEST_ASSERT_TRUE(copied_event.attributes[1].value.as.string.needs_json_escape);

    ij_event_copy_dispose(&copied_event);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_validate_event_accepts_minimal_event);
    RUN_TEST(test_validate_event_rejects_duplicate_attribute_keys);
    RUN_TEST(test_event_copy_redacts_sensitive_attribute_values);
    RUN_TEST(test_event_copy_rejects_duplicate_attribute_keys);
    RUN_TEST(test_event_copy_rejects_invalid_utf8_message);
    RUN_TEST(test_event_copy_rejects_total_text_over_limit_without_attributes);
    RUN_TEST(test_event_copy_packs_text_fields_into_one_arena);
    RUN_TEST(test_event_copy_dispose_clears_packed_storage);
    RUN_TEST(test_event_copy_tracks_json_escape_flags_for_ascii_and_special_fields);
    return UNITY_END();
}
