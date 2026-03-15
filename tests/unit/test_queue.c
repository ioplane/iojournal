#include "ij_internal.h"

#include <unity/unity.h>

static ij_event_t ij_test_queue_event(const char *event_name, const char *message)
{
    static const ij_attr_t attributes[] = {
        {.key = "request_id",
         .value =
             {
                 .kind = IJ_ATTR_VALUE_STRING,
                 .as.string =
                     {
                         .data = "req-1",
                         .len = 5U,
                     },
             }},
    };
    ij_event_t event = {
        .timestamp = {.unix_seconds = 0, .nanoseconds = 123000000U},
        .level = IJ_LEVEL_INFO,
        .event_name = event_name,
        .message = message,
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

void test_ring_enqueue_then_dequeue_preserves_event_order(void)
{
    ij_ring_t ring = {0};
    ij_event_copy_t first = {0};
    ij_event_copy_t second = {0};
    ij_event_copy_t dequeued = {0};
    ij_event_t first_event = ij_test_queue_event("first.event", "first");
    ij_event_t second_event = ij_test_queue_event("second.event", "second");

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_ring_init(&ring, IJ_QUEUE_CAPACITY_MIN));
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_event_copy_from_input(&first, &first_event,
                                                                 IJ_REDACTION_MODE_ENABLED));
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_event_copy_from_input(&second, &second_event,
                                                                 IJ_REDACTION_MODE_ENABLED));
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_ring_enqueue(&ring, &first));
    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_ring_enqueue(&ring, &second));

    TEST_ASSERT_TRUE(ij_ring_try_dequeue(&ring, &dequeued));
    TEST_ASSERT_EQUAL_STRING("first.event", dequeued.event_name.data);
    ij_event_copy_dispose(&dequeued);

    TEST_ASSERT_TRUE(ij_ring_try_dequeue(&ring, &dequeued));
    TEST_ASSERT_EQUAL_STRING("second.event", dequeued.event_name.data);
    ij_event_copy_dispose(&dequeued);

    ij_ring_destroy(&ring);
}

void test_ring_reports_queue_full_when_capacity_is_exhausted(void)
{
    ij_ring_t ring = {0};
    ij_event_t event = ij_test_queue_event("pressure.event", "full");

    TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_ring_init(&ring, IJ_QUEUE_CAPACITY_MIN));

    for (size_t i = 0; i < IJ_QUEUE_CAPACITY_MIN; ++i) {
        ij_event_copy_t copied_event = {0};

        TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_event_copy_from_input(&copied_event, &event,
                                                                     IJ_REDACTION_MODE_ENABLED));
        TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_ring_enqueue(&ring, &copied_event));
    }

    {
        ij_event_copy_t overflow_event = {0};

        TEST_ASSERT_EQUAL_INT(IJ_STATUS_OK, ij_event_copy_from_input(&overflow_event, &event,
                                                                     IJ_REDACTION_MODE_ENABLED));
        TEST_ASSERT_EQUAL_INT(IJ_STATUS_QUEUE_FULL, ij_ring_enqueue(&ring, &overflow_event));
        ij_event_copy_dispose(&overflow_event);
    }

    ij_ring_destroy(&ring);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_ring_enqueue_then_dequeue_preserves_event_order);
    RUN_TEST(test_ring_reports_queue_full_when_capacity_is_exhausted);
    return UNITY_END();
}
